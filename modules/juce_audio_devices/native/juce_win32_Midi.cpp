/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct MidiServiceType
{
    struct InputWrapper
    {
        virtual ~InputWrapper() {}

        virtual String getDeviceName() = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
    };

    struct OutputWrapper
    {
        virtual ~OutputWrapper() {}

        virtual String getDeviceName() = 0;
        virtual void sendMessageNow (const MidiMessage&) = 0;
    };

    MidiServiceType() {}
    virtual ~MidiServiceType() {}

    virtual StringArray getDevices (bool) = 0;
    virtual int getDefaultDeviceIndex (bool) = 0;

    virtual InputWrapper* createInputWrapper (MidiInput&, int, MidiInputCallback&) = 0;
    virtual OutputWrapper* createOutputWrapper (int) = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiServiceType)
};

//==============================================================================
struct Win32MidiService  : public MidiServiceType,
                           private Timer
{
    Win32MidiService() {}

    StringArray getDevices (bool isInput) override
    {
        return isInput ? Win32InputWrapper::getDevices()
                       : Win32OutputWrapper::getDevices();
    }

    int getDefaultDeviceIndex (bool isInput) override
    {
        return isInput ? Win32InputWrapper::getDefaultDeviceIndex()
                       : Win32OutputWrapper::getDefaultDeviceIndex();
    }

    InputWrapper* createInputWrapper (MidiInput& input, int index, MidiInputCallback& callback) override
    {
        return new Win32InputWrapper (*this, input, index, callback);
    }

    OutputWrapper* createOutputWrapper (int index) override
    {
        return new Win32OutputWrapper (*this, index);
    }

private:
    struct Win32InputWrapper;

    //==============================================================================
    struct MidiInCollector  : public ReferenceCountedObject
    {
        MidiInCollector (Win32MidiService& s, const String& name)  : deviceName (name), midiService (s) {}

        ~MidiInCollector()
        {
            stop();

            if (deviceHandle != 0)
            {
                for (int count = 5; --count >= 0;)
                {
                    if (midiInClose (deviceHandle) == MMSYSERR_NOERROR)
                        break;

                    Sleep (20);
                }
            }
        }

        using Ptr = ReferenceCountedObjectPtr<MidiInCollector>;

        void addClient (Win32InputWrapper* c)
        {
            const ScopedLock sl (clientLock);
            jassert (! clients.contains (c));
            clients.add (c);
        }

        void removeClient (Win32InputWrapper* c)
        {
            const ScopedLock sl (clientLock);
            clients.removeFirstMatchingValue (c);
            startOrStop();
            midiService.asyncCheckForUnusedCollectors();
        }

        void handleMessage (const uint8* bytes, uint32 timeStamp)
        {
            if (bytes[0] >= 0x80 && isStarted.load())
            {
                {
                    auto len = MidiMessage::getMessageLengthFromFirstByte (bytes[0]);
                    auto time = convertTimeStamp (timeStamp);
                    const ScopedLock sl (clientLock);

                    for (auto* c : clients)
                        c->pushMidiData (bytes, len, time);
                }

                writeFinishedBlocks();
            }
        }

        void handleSysEx (MIDIHDR* hdr, uint32 timeStamp)
        {
            if (isStarted.load() && hdr->dwBytesRecorded > 0)
            {
                {
                    auto time = convertTimeStamp (timeStamp);
                    const ScopedLock sl (clientLock);

                    for (auto* c : clients)
                        c->pushMidiData (hdr->lpData, (int) hdr->dwBytesRecorded, time);
                }

                writeFinishedBlocks();
            }
        }

        void startOrStop()
        {
            const ScopedLock sl (clientLock);

            if (countRunningClients() == 0)
                stop();
            else
                start();
        }

        void start()
        {
            if (deviceHandle != 0 && ! isStarted.load())
            {
                activeMidiCollectors.addIfNotAlreadyThere (this);

                for (int i = 0; i < (int) numHeaders; ++i)
                {
                    headers[i].prepare (deviceHandle);
                    headers[i].write (deviceHandle);
                }

                startTime = Time::getMillisecondCounterHiRes();
                auto res = midiInStart (deviceHandle);

                if (res == MMSYSERR_NOERROR)
                    isStarted = true;
                else
                    unprepareAllHeaders();
            }
        }

        void stop()
        {
            if (isStarted.load())
            {
                isStarted = false;
                midiInReset (deviceHandle);
                midiInStop (deviceHandle);
                activeMidiCollectors.removeFirstMatchingValue (this);
                unprepareAllHeaders();
            }
        }

        static void CALLBACK midiInCallback (HMIDIIN, UINT uMsg, DWORD_PTR dwInstance,
                                             DWORD_PTR midiMessage, DWORD_PTR timeStamp)
        {
            auto* collector = reinterpret_cast<MidiInCollector*> (dwInstance);

            // This is primarily a check for the collector being a dangling
            // pointer, as the callback can sometimes be delayed
            if (activeMidiCollectors.contains (collector))
            {
                if (uMsg == MIM_DATA)
                    collector->handleMessage ((const uint8*) &midiMessage, (uint32) timeStamp);
                else if (uMsg == MIM_LONGDATA)
                    collector->handleSysEx ((MIDIHDR*) midiMessage, (uint32) timeStamp);
            }
        }

        String deviceName;
        HMIDIIN deviceHandle = 0;

    private:
        Win32MidiService& midiService;
        CriticalSection clientLock;
        Array<Win32InputWrapper*> clients;
        std::atomic<bool> isStarted { false };
        double startTime = 0;

        // This static array is used to prevent occasional callbacks to objects that are
        // in the process of being deleted
        static Array<MidiInCollector*, CriticalSection> activeMidiCollectors;

        int countRunningClients() const
        {
            int num = 0;

            for (auto* c : clients)
                if (c->started)
                    ++num;

            return num;
        }

        struct MidiHeader
        {
            MidiHeader() {}

            void prepare (HMIDIIN device)
            {
                zerostruct (hdr);
                hdr.lpData = data;
                hdr.dwBufferLength = (DWORD) numElementsInArray (data);

                midiInPrepareHeader (device, &hdr, sizeof (hdr));
            }

            void unprepare (HMIDIIN device)
            {
                if ((hdr.dwFlags & WHDR_DONE) != 0)
                {
                    int c = 10;
                    while (--c >= 0 && midiInUnprepareHeader (device, &hdr, sizeof (hdr)) == MIDIERR_STILLPLAYING)
                        Thread::sleep (20);

                    jassert (c >= 0);
                }
            }

            void write (HMIDIIN device)
            {
                hdr.dwBytesRecorded = 0;
                midiInAddBuffer (device, &hdr, sizeof (hdr));
            }

            void writeIfFinished (HMIDIIN device)
            {
                if ((hdr.dwFlags & WHDR_DONE) != 0)
                    write (device);
            }

            MIDIHDR hdr;
            char data[256];

            JUCE_DECLARE_NON_COPYABLE (MidiHeader)
        };

        enum { numHeaders = 32 };
        MidiHeader headers[numHeaders];

        void writeFinishedBlocks()
        {
            for (int i = 0; i < (int) numHeaders; ++i)
                headers[i].writeIfFinished (deviceHandle);
        }

        void unprepareAllHeaders()
        {
            for (int i = 0; i < (int) numHeaders; ++i)
                headers[i].unprepare (deviceHandle);
        }

        double convertTimeStamp (uint32 timeStamp)
        {
            auto t = startTime + timeStamp;
            auto now = Time::getMillisecondCounterHiRes();

            if (t > now)
            {
                if (t > now + 2.0)
                    startTime -= 1.0;

                t = now;
            }

            return t * 0.001;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInCollector)
    };

    //==============================================================================
    struct Win32InputWrapper  : public InputWrapper
    {
        Win32InputWrapper (Win32MidiService& parentService,
                           MidiInput& midiInput, int index, MidiInputCallback& c)
            : input (midiInput), callback (c)
        {
            collector = getOrCreateCollector (parentService, index);
            collector->addClient (this);
        }

        ~Win32InputWrapper()
        {
            collector->removeClient (this);
        }

        static MidiInCollector::Ptr getOrCreateCollector (Win32MidiService& parentService, int index)
        {
            auto names = getDevices();
            UINT deviceID = MIDI_MAPPER;
            String deviceName;

            if (isPositiveAndBelow (index, names.size()))
            {
                deviceName = names[index];
                deviceID = index;
            }

            const ScopedLock sl (parentService.activeCollectorLock);

            for (auto& c : parentService.activeCollectors)
                if (c->deviceName == deviceName)
                    return c;

            MidiInCollector::Ptr c (new MidiInCollector (parentService, deviceName));

            HMIDIIN h;
            auto err = midiInOpen (&h, deviceID,
                                   (DWORD_PTR) &MidiInCollector::midiInCallback,
                                   (DWORD_PTR) (MidiInCollector*) c.get(),
                                   CALLBACK_FUNCTION);

            if (err != MMSYSERR_NOERROR)
                throw std::runtime_error ("Failed to create Windows input device wrapper");

            c->deviceHandle = h;
            parentService.activeCollectors.add (c);
            return c;
        }

        static StringArray getDevices()
        {
            StringArray s;
            auto num = midiInGetNumDevs();

            for (UINT i = 0; i < num; ++i)
            {
                MIDIINCAPS mc = { 0 };

                if (midiInGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
                    s.add (String (mc.szPname, (size_t) numElementsInArray (mc.szPname)));
            }

            s.appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));
            return s;
        }

        static int getDefaultDeviceIndex()  { return 0; }

        void start() override   { started = true;  concatenator.reset(); collector->startOrStop(); }
        void stop() override    { started = false; collector->startOrStop(); concatenator.reset(); }

        String getDeviceName() override     { return collector->deviceName; }

        void pushMidiData (const void* inputData, int numBytes, double time)
        {
            concatenator.pushMidiData (inputData, numBytes, time, &input, callback);
        }

        MidiInput& input;
        MidiInputCallback& callback;
        MidiDataConcatenator concatenator { 4096 };
        MidiInCollector::Ptr collector;
        bool started = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32InputWrapper)
    };

    //==============================================================================
    struct MidiOutHandle    : public ReferenceCountedObject
    {
        using Ptr = ReferenceCountedObjectPtr<MidiOutHandle>;

        MidiOutHandle (Win32MidiService& parent, const String& name, HMIDIOUT h)
            : owner (parent), deviceName (name), handle (h)
        {
            owner.activeOutputHandles.add (this);
        }

        ~MidiOutHandle()
        {
            if (handle != nullptr)
                midiOutClose (handle);

            owner.activeOutputHandles.removeFirstMatchingValue (this);
        }

        Win32MidiService& owner;
        String deviceName;
        HMIDIOUT handle;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiOutHandle)
    };

    //==============================================================================
    struct Win32OutputWrapper  : public OutputWrapper
    {
        Win32OutputWrapper (Win32MidiService& p, int index) : parent (p)
        {
            auto names = getDevices();
            UINT deviceID = MIDI_MAPPER;

            if (isPositiveAndBelow (index, names.size()))
            {
                deviceName = names[index];
                deviceID = index;
            }

            if (deviceID == MIDI_MAPPER)
            {
                // use the microsoft sw synth as a default - best not to allow deviceID
                // to be MIDI_MAPPER, or else device sharing breaks
                for (int i = 0; i < names.size(); ++i)
                    if (names[i].containsIgnoreCase ("microsoft"))
                        deviceID = (UINT) i;
            }

            for (int i = parent.activeOutputHandles.size(); --i >= 0;)
            {
                auto* activeHandle = parent.activeOutputHandles.getUnchecked (i);

                if (activeHandle->deviceName == deviceName)
                {
                    han = activeHandle;
                    return;
                }
            }

            for (int i = 4; --i >= 0;)
            {
                HMIDIOUT h = 0;
                auto res = midiOutOpen (&h, deviceID, 0, 0, CALLBACK_NULL);

                if (res == MMSYSERR_NOERROR)
                {
                    han = new MidiOutHandle (parent, deviceName, h);
                    return;
                }

                if (res == MMSYSERR_ALLOCATED)
                    Sleep (100);
                else
                    break;
            }

            throw std::runtime_error ("Failed to create Windows output device wrapper");
        }

        void sendMessageNow (const MidiMessage& message) override
        {
            if (message.getRawDataSize() > 3 || message.isSysEx())
            {
                MIDIHDR h = { 0 };

                h.lpData = (char*) message.getRawData();
                h.dwBytesRecorded = h.dwBufferLength  = (DWORD) message.getRawDataSize();

                if (midiOutPrepareHeader (han->handle, &h, sizeof (MIDIHDR)) == MMSYSERR_NOERROR)
                {
                    auto res = midiOutLongMsg (han->handle, &h, sizeof (MIDIHDR));

                    if (res == MMSYSERR_NOERROR)
                    {
                        while ((h.dwFlags & MHDR_DONE) == 0)
                            Sleep (1);

                        int count = 500; // 1 sec timeout

                        while (--count >= 0)
                        {
                            res = midiOutUnprepareHeader (han->handle, &h, sizeof (MIDIHDR));

                            if (res == MIDIERR_STILLPLAYING)
                                Sleep (2);
                            else
                                break;
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < 50; ++i)
                {
                    if (midiOutShortMsg (han->handle, *(unsigned int*) message.getRawData()) != MIDIERR_NOTREADY)
                        break;

                    Sleep (1);
                }
            }
        }

        static Array<MIDIOUTCAPS> getDeviceCaps()
        {
            Array<MIDIOUTCAPS> devices;
            auto num = midiOutGetNumDevs();

            for (UINT i = 0; i < num; ++i)
            {
                MIDIOUTCAPS mc = { 0 };

                if (midiOutGetDevCaps (i, &mc, sizeof (mc)) == MMSYSERR_NOERROR)
                    devices.add (mc);
            }

            return devices;
        }

        static StringArray getDevices()
        {
            StringArray s;

            for (auto& mc : getDeviceCaps())
                s.add (String (mc.szPname, (size_t) numElementsInArray (mc.szPname)));

            s.appendNumbersToDuplicates (false, false, CharPointer_UTF8 ("-"), CharPointer_UTF8 (""));
            return s;
        }

        static int getDefaultDeviceIndex()
        {
            int n = 0;

            for (auto& mc : getDeviceCaps())
            {
                if ((mc.wTechnology & MOD_MAPPER) != 0)
                    return n;

                ++n;
            }

            return 0;
        }

        String getDeviceName() override    { return deviceName; }

        Win32MidiService& parent;
        String deviceName;
        MidiOutHandle::Ptr han;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32OutputWrapper)
    };

    //==============================================================================
    void asyncCheckForUnusedCollectors()
    {
        startTimer (10);
    }

    void timerCallback() override
    {
        stopTimer();

        const ScopedLock sl (activeCollectorLock);

        for (int i = activeCollectors.size(); --i >= 0;)
            if (activeCollectors.getObjectPointer(i)->getReferenceCount() == 1)
                activeCollectors.remove (i);
    }

    CriticalSection activeCollectorLock;
    ReferenceCountedArray<MidiInCollector> activeCollectors;
    Array<MidiOutHandle*> activeOutputHandles;
};

Array<Win32MidiService::MidiInCollector*, CriticalSection> Win32MidiService::MidiInCollector::activeMidiCollectors;

//==============================================================================
//==============================================================================
#if JUCE_USE_WINRT_MIDI
using namespace Microsoft::WRL;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Devices::Midi;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

//==============================================================================
class WinRTMidiService  : public MidiServiceType
{
public:
    //==============================================================================
    WinRTMidiService()
    {
        if (! WinRTWrapper::getInstance()->isInitialised())
            throw std::runtime_error ("Failed to initialise the WinRT wrapper");

        midiInFactory = WinRTWrapper::getInstance()->getWRLFactory<IMidiInPortStatics> (&RuntimeClass_Windows_Devices_Midi_MidiInPort[0]);

        if (midiInFactory == nullptr)
            throw std::runtime_error ("Failed to create midi in factory");

        midiOutFactory = WinRTWrapper::getInstance()->getWRLFactory<IMidiOutPortStatics> (&RuntimeClass_Windows_Devices_Midi_MidiOutPort[0]);

        if (midiOutFactory == nullptr)
            throw std::runtime_error ("Failed to create midi out factory");

        inputDeviceWatcher.reset (new MidiIODeviceWatcher<IMidiInPortStatics> (midiInFactory));

        if (! inputDeviceWatcher->start())
            throw std::runtime_error ("Failed to start midi input device watcher");

        outputDeviceWatcher.reset (new MidiIODeviceWatcher<IMidiOutPortStatics> (midiOutFactory));

        if (! outputDeviceWatcher->start())
            throw std::runtime_error ("Failed to start midi output device watcher");
    }

    ~WinRTMidiService() {}

    StringArray getDevices (bool isInput) override
    {
        return isInput ? inputDeviceWatcher ->getDevices()
                       : outputDeviceWatcher->getDevices();
    }

    int getDefaultDeviceIndex (bool isInput) override
    {
        return isInput ? inputDeviceWatcher ->getDefaultDeviceIndex()
                       : outputDeviceWatcher->getDefaultDeviceIndex();
    }

    InputWrapper* createInputWrapper (MidiInput& input, int index, MidiInputCallback& callback) override
    {
        return new WinRTInputWrapper (*this, input, index, callback);
    }

    OutputWrapper* createOutputWrapper (int index) override
    {
        return new WinRTOutputWrapper (*this, index);
    }

    template <typename COMFactoryType>
    struct MidiIODeviceWatcher
    {
        struct DeviceInfo
        {
            String name, id;
            bool isDefault = false;
        };

        MidiIODeviceWatcher (WinRTWrapper::ComPtr<COMFactoryType>& comFactory)  : factory (comFactory)
        {
        }

        ~MidiIODeviceWatcher()
        {
            stop();
        }

        bool start()
        {
            HSTRING deviceSelector;
            auto hr = factory->GetDeviceSelector (&deviceSelector);

            if (FAILED (hr))
                return false;

            auto deviceInformationFactory = WinRTWrapper::getInstance()->getWRLFactory<IDeviceInformationStatics> (&RuntimeClass_Windows_Devices_Enumeration_DeviceInformation[0]);

            if (deviceInformationFactory == nullptr)
                return false;

            hr = deviceInformationFactory->CreateWatcherAqsFilter (deviceSelector, watcher.resetAndGetPointerAddress());

            if (FAILED (hr))
                return false;

            struct DeviceEnumerationThread  : public Thread
            {
                DeviceEnumerationThread (String threadName, MidiIODeviceWatcher<COMFactoryType>& p)
                    : Thread (threadName), parent (p)
                {}

                void run() override
                {
                    auto parentPtr = &parent;

                    parent.watcher->add_Added (
                        Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformation*>> (
                            [parentPtr](IDeviceWatcher*, IDeviceInformation* info) { return parentPtr->addDevice (info); }
                        ).Get(),
                        &parent.deviceAddedToken);

                    parent.watcher->add_Removed (
                        Callback<ITypedEventHandler<DeviceWatcher*, DeviceInformationUpdate*>> (
                            [parentPtr](IDeviceWatcher*, IDeviceInformationUpdate* info) { return parentPtr->removeDevice (info); }
                        ).Get(),
                        &parent.deviceRemovedToken);

                    EventRegistrationToken deviceEnumerationCompletedToken { 0 };
                    parent.watcher->add_EnumerationCompleted (
                        Callback<ITypedEventHandler<DeviceWatcher*, IInspectable*>> (
                            [this](IDeviceWatcher*, IInspectable*) { enumerationCompleted.signal(); return S_OK; }
                        ).Get(),
                        &deviceEnumerationCompletedToken);

                    parent.watcher->Start();
                    enumerationCompleted.wait();

                    if (deviceEnumerationCompletedToken.value != 0)
                        parent.watcher->remove_EnumerationCompleted (deviceEnumerationCompletedToken);
                }

                MidiIODeviceWatcher<COMFactoryType>& parent;
                WaitableEvent enumerationCompleted;
            };

            DeviceEnumerationThread enumerationThread ("WinRT Device Enumeration Thread", *this);
            enumerationThread.startThread();
            enumerationThread.waitForThreadToExit (4000);

            return true;
        }

        bool stop()
        {
            if (watcher == nullptr)
                return true;

            if (deviceAddedToken.value != 0)
            {
                auto hr = watcher->remove_Added (deviceAddedToken);

                if (FAILED (hr))
                    return false;

                deviceAddedToken.value = 0;
            }

            if (deviceRemovedToken.value != 0)
            {
                auto hr = watcher->remove_Removed (deviceRemovedToken);

                if (FAILED (hr))
                    return false;

                deviceRemovedToken.value = 0;
            }

            auto hr = watcher->Stop();

            if (FAILED (hr))
                return false;

            watcher = nullptr;
            return true;
        }

        HRESULT addDevice (IDeviceInformation* addedDeviceInfo)
        {
            boolean isEnabled;
            auto hr = addedDeviceInfo->get_IsEnabled (&isEnabled);

            if (FAILED (hr))
                return S_OK;

            if (! isEnabled)
                return S_OK;

            const ScopedLock lock (deviceChanges);

            DeviceInfo info;

            HSTRING name;
            hr = addedDeviceInfo->get_Name (&name);

            if (FAILED (hr))
                return S_OK;

            info.name = WinRTWrapper::getInstance()->hStringToString (name);

            HSTRING id;
            hr = addedDeviceInfo->get_Id (&id);

            if (FAILED (hr))
                return S_OK;

            info.id = WinRTWrapper::getInstance()->hStringToString (id);

            boolean isDefault;
            hr = addedDeviceInfo->get_IsDefault (&isDefault);

            if (FAILED (hr))
                return S_OK;

            info.isDefault = isDefault != 0;
            connectedDevices.add (info);
            return S_OK;
        }

        HRESULT removeDevice (IDeviceInformationUpdate* removedDeviceInfo)
        {
            const ScopedLock lock (deviceChanges);

            HSTRING removedDeviceIdHstr;
            removedDeviceInfo->get_Id (&removedDeviceIdHstr);
            auto removedDeviceId = WinRTWrapper::getInstance()->hStringToString (removedDeviceIdHstr);

            for (int i = 0; i < connectedDevices.size(); ++i)
            {
                if (connectedDevices[i].id == removedDeviceId)
                {
                    connectedDevices.remove (i);
                    break;
                }
            }

            return S_OK;
        }

        StringArray getDevices()
        {
            {
                const ScopedLock lock (deviceChanges);
                lastQueriedConnectedDevices = connectedDevices;
            }

            StringArray result;

            for (auto info : lastQueriedConnectedDevices.get())
                result.add (info.name);

            return result;
        }

        int getDefaultDeviceIndex()
        {
            auto& lastDevices = lastQueriedConnectedDevices.get();

            for (int i = 0; i < lastDevices.size(); ++i)
                if (lastDevices[i].isDefault)
                    return i;

            return 0;
        }

        String getDeviceNameFromIndex (int index)
        {
            if (isPositiveAndBelow (index, lastQueriedConnectedDevices.get().size()))
                return lastQueriedConnectedDevices.get()[index].name;

            return {};
        }

        String getDeviceID (const String& name)
        {
            const ScopedLock lock (deviceChanges);

            for (auto info : connectedDevices)
                if (info.name == name)
                    return info.id;

            return {};
        }

        WinRTWrapper::ComPtr<COMFactoryType>& factory;

        EventRegistrationToken deviceAddedToken   { 0 },
                               deviceRemovedToken { 0 };

        WinRTWrapper::ComPtr<IDeviceWatcher> watcher;

        Array<DeviceInfo> connectedDevices;
        CriticalSection deviceChanges;
        ThreadLocalValue<Array<DeviceInfo>> lastQueriedConnectedDevices;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiIODeviceWatcher);
    };

    //==============================================================================
    template <typename COMFactoryType, typename COMInterfaceType, typename COMType>
    struct OpenMidiPortThread  : public Thread
    {
        OpenMidiPortThread (String threadName, String midiDeviceID,
                            WinRTWrapper::ComPtr<COMFactoryType>& comFactory,
                            WinRTWrapper::ComPtr<COMInterfaceType>& comPort)
            : Thread (threadName),
              deviceID (midiDeviceID),
              factory (comFactory),
              port (comPort)
        {
        }

        ~OpenMidiPortThread()
        {
            stopThread (2000);
        }

        void run() override
        {
            WinRTWrapper::ScopedHString hDeviceId (deviceID);
            WinRTWrapper::ComPtr<IAsyncOperation<COMType*>> asyncOp;
            auto hr = factory->FromIdAsync (hDeviceId.get(), asyncOp.resetAndGetPointerAddress());

            if (FAILED (hr))
                return;

            hr = asyncOp->put_Completed (Callback<IAsyncOperationCompletedHandler<COMType*>> (
                [this] (IAsyncOperation<COMType*>* asyncOpPtr, AsyncStatus)
                {
                    if (asyncOpPtr == nullptr)
                        return E_ABORT;

                    auto hr = asyncOpPtr->GetResults (port.resetAndGetPointerAddress());

                    if (FAILED (hr))
                        return hr;

                    portOpened.signal();
                    return S_OK;
                }
            ).Get());

            // When using Bluetooth the asynchronous port opening operation will occasionally
            // hang, so we use a timeout. We will be able to remove this when Microsoft
            // improves the Bluetooth MIDI stack.
            portOpened.wait (2000);
        }

        const String deviceID;
        WinRTWrapper::ComPtr<COMFactoryType>& factory;
        WinRTWrapper::ComPtr<COMInterfaceType>& port;
        WaitableEvent portOpened { true };
    };

    //==============================================================================
    struct WinRTInputWrapper  : public InputWrapper
    {
        WinRTInputWrapper (WinRTMidiService& service, MidiInput& input, int index, MidiInputCallback& cb)
            : inputDevice (input),
              callback (cb)
        {
            const ScopedLock lock (service.inputDeviceWatcher->deviceChanges);

            deviceName = service.inputDeviceWatcher->getDeviceNameFromIndex (index);

            if (deviceName.isEmpty())
                throw std::runtime_error ("Invalid device index");

            auto deviceID = service.inputDeviceWatcher->getDeviceID (deviceName);

            if (deviceID.isEmpty())
                throw std::runtime_error ("Device unavailable");

            OpenMidiPortThread<IMidiInPortStatics, IMidiInPort, MidiInPort> portThread ("Open WinRT MIDI input port",
                                                                                        deviceID,
                                                                                        service.midiInFactory,
                                                                                        midiInPort);
            portThread.startThread();
            portThread.waitForThreadToExit (-1);

            if (midiInPort == nullptr)
                throw std::runtime_error ("Timed out waiting for midi input port creation");

            startTime = Time::getMillisecondCounterHiRes();

            auto hr = midiInPort->add_MessageReceived (
                Callback<ITypedEventHandler<MidiInPort*, MidiMessageReceivedEventArgs*>> (
                    [this] (IMidiInPort*, IMidiMessageReceivedEventArgs* args) { return midiInMessageReceived (args); }
                ).Get(),
                &midiInMessageToken);

            if (FAILED (hr))
                throw std::runtime_error ("Failed to set midi input callback");
        }

        ~WinRTInputWrapper()
        {
            if (midiInMessageToken.value != 0)
                midiInPort->remove_MessageReceived (midiInMessageToken);

            midiInPort = nullptr;
        }

        void start() override
        {
            if (! isStarted)
            {
                concatenator.reset();
                isStarted = true;
            }
        }

        void stop() override
        {
            if (isStarted)
            {
                isStarted = false;
                concatenator.reset();
            }
        }

        String getDeviceName() override         { return deviceName; }

        HRESULT midiInMessageReceived (IMidiMessageReceivedEventArgs* args)
        {
            if (! isStarted)
                return S_OK;

            WinRTWrapper::ComPtr<IMidiMessage> message;
            auto hr = args->get_Message (message.resetAndGetPointerAddress());

            if (FAILED (hr))
                return hr;

            WinRTWrapper::ComPtr<IBuffer> buffer;
            hr = message->get_RawData (buffer.resetAndGetPointerAddress());

            if (FAILED (hr))
                return hr;

            WinRTWrapper::ComPtr<Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
            hr = buffer->QueryInterface (bufferByteAccess.resetAndGetPointerAddress());

            if (FAILED (hr))
                return hr;

            uint8_t* bufferData = nullptr;
            hr = bufferByteAccess->Buffer (&bufferData);

            if (FAILED (hr))
                return hr;

            uint32_t numBytes = 0;
            hr = buffer->get_Length (&numBytes);

            if (FAILED (hr))
                return hr;

            ABI::Windows::Foundation::TimeSpan timespan;
            hr = message->get_Timestamp (&timespan);

            if (FAILED (hr))
                return hr;

            concatenator.pushMidiData (bufferData, numBytes,
                                       convertTimeStamp (timespan.Duration),
                                       &inputDevice, callback);
            return S_OK;
        }

        double convertTimeStamp (int64 timestamp)
        {
            auto millisecondsSinceStart = static_cast<double> (timestamp) / 10000.0;
            auto t = startTime + millisecondsSinceStart;
            auto now = Time::getMillisecondCounterHiRes();

            if (t > now)
            {
                if (t > now + 2.0)
                    startTime -= 1.0;

                t = now;
            }

            return t * 0.001;
        }

        MidiInput& inputDevice;
        MidiInputCallback& callback;
        String deviceName;
        MidiDataConcatenator concatenator { 4096 };
        WinRTWrapper::ComPtr<IMidiInPort> midiInPort;
        EventRegistrationToken midiInMessageToken { 0 };

        double startTime = 0;
        bool isStarted = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTInputWrapper);
    };

    //==============================================================================
    struct WinRTOutputWrapper  : public OutputWrapper
    {
        WinRTOutputWrapper (WinRTMidiService& service, int index)
        {
            const ScopedLock lock (service.outputDeviceWatcher->deviceChanges);

            deviceName = service.outputDeviceWatcher->getDeviceNameFromIndex (index);

            if (deviceName.isEmpty())
                throw std::runtime_error ("Invalid device index");

            auto deviceID = service.outputDeviceWatcher->getDeviceID (deviceName);

            if (deviceID.isEmpty())
                throw std::runtime_error ("Device unavailable");

            OpenMidiPortThread<IMidiOutPortStatics, IMidiOutPort, IMidiOutPort> portThread ("Open WinRT MIDI output port",
                                                                                            deviceID,
                                                                                            service.midiOutFactory,
                                                                                            midiOutPort);
            portThread.startThread();
            portThread.waitForThreadToExit (-1);

            if (midiOutPort == nullptr)
                throw std::runtime_error ("Timed out waiting for midi output port creation");

            auto bufferFactory = WinRTWrapper::getInstance()->getWRLFactory<IBufferFactory> (&RuntimeClass_Windows_Storage_Streams_Buffer[0]);

            if (bufferFactory == nullptr)
                throw std::runtime_error ("Failed to create output buffer factory");

            auto hr = bufferFactory->Create (static_cast<UINT32> (65536), buffer.resetAndGetPointerAddress());

            if (FAILED (hr))
                throw std::runtime_error ("Failed to create output buffer");

            hr = buffer->QueryInterface (bufferByteAccess.resetAndGetPointerAddress());

            if (FAILED (hr))
                throw std::runtime_error ("Failed to get buffer byte access");

            hr = bufferByteAccess->Buffer (&bufferData);

            if (FAILED (hr))
                throw std::runtime_error ("Failed to get buffer data pointer");
        }

        ~WinRTOutputWrapper() {}

        void sendMessageNow (const MidiMessage& message) override
        {
            auto numBytes = message.getRawDataSize();
            auto hr = buffer->put_Length (numBytes);

            if (FAILED (hr))
                jassertfalse;

            memcpy_s (bufferData, numBytes, message.getRawData(), numBytes);
            midiOutPort->SendBuffer (buffer);
        }

        String getDeviceName() override    { return deviceName; }

        String deviceName;
        WinRTWrapper::ComPtr<IMidiOutPort> midiOutPort;
        WinRTWrapper::ComPtr<IBuffer> buffer;
        WinRTWrapper::ComPtr<Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
        uint8_t* bufferData = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTOutputWrapper);
    };

    WinRTWrapper::ComPtr<IMidiInPortStatics>  midiInFactory;
    WinRTWrapper::ComPtr<IMidiOutPortStatics> midiOutFactory;

    std::unique_ptr<MidiIODeviceWatcher<IMidiInPortStatics>>  inputDeviceWatcher;
    std::unique_ptr<MidiIODeviceWatcher<IMidiOutPortStatics>> outputDeviceWatcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTMidiService)
};

#endif   // JUCE_USE_WINRT_MIDI

//==============================================================================
struct MidiService :  public DeletedAtShutdown
{
    MidiService()
    {
       #if JUCE_USE_WINRT_MIDI
        try
        {
            internal.reset (new WinRTMidiService());
            return;
        }
        catch (std::runtime_error&) {}
       #endif

        internal.reset (new Win32MidiService());
    }

    ~MidiService()
    {
        clearSingletonInstance();
    }

    static MidiServiceType& getService()
    {
        jassert (getInstance()->internal != nullptr);
        return *getInstance()->internal.get();
    }

    JUCE_DECLARE_SINGLETON (MidiService, false)

private:
    std::unique_ptr<MidiServiceType> internal;
};

JUCE_IMPLEMENT_SINGLETON (MidiService)

//==============================================================================
StringArray MidiInput::getDevices()
{
    return MidiService::getService().getDevices (true);
}

int MidiInput::getDefaultDeviceIndex()
{
    return MidiService::getService().getDefaultDeviceIndex (true);
}

MidiInput::MidiInput (const String& deviceName)  : name (deviceName)
{
}

MidiInput* MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    if (callback == nullptr)
        return nullptr;

    std::unique_ptr<MidiInput> in (new MidiInput (String()));
    std::unique_ptr<MidiServiceType::InputWrapper> wrapper;

    try
    {
        wrapper.reset (MidiService::getService().createInputWrapper (*in, index, *callback));
    }
    catch (std::runtime_error&)
    {
        return nullptr;
    }

    in->setName (wrapper->getDeviceName());
    in->internal = wrapper.release();
    return in.release();
}

MidiInput::~MidiInput()
{
    delete static_cast<MidiServiceType::InputWrapper*> (internal);
}

void MidiInput::start()   { static_cast<MidiServiceType::InputWrapper*> (internal)->start(); }
void MidiInput::stop()    { static_cast<MidiServiceType::InputWrapper*> (internal)->stop(); }

//==============================================================================
StringArray MidiOutput::getDevices()
{
    return MidiService::getService().getDevices (false);
}

int MidiOutput::getDefaultDeviceIndex()
{
    return MidiService::getService().getDefaultDeviceIndex (false);
}

MidiOutput* MidiOutput::openDevice (int index)
{
    std::unique_ptr<MidiServiceType::OutputWrapper> wrapper;

    try
    {
        wrapper.reset (MidiService::getService().createOutputWrapper (index));
    }
    catch (std::runtime_error&)
    {
        return nullptr;
    }

    std::unique_ptr<MidiOutput> out (new MidiOutput (wrapper->getDeviceName()));
    out->internal = wrapper.release();
    return out.release();
}

MidiOutput::~MidiOutput()
{
    stopBackgroundThread();
    delete static_cast<MidiServiceType::OutputWrapper*> (internal);
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    static_cast<MidiServiceType::OutputWrapper*> (internal)->sendMessageNow (message);
}

} // namespace juce
