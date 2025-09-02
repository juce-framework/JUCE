/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getAndroidBluetoothManager, "getAndroidBluetoothManager", "(Landroid/content/Context;)Lcom/rmsl/juce/JuceMidiSupport$BluetoothMidiManager;")

DECLARE_JNI_CLASS (AndroidJuceMidiSupport, "com/rmsl/juce/JuceMidiSupport")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (getMidiBluetoothAddresses, "getMidiBluetoothAddresses", "()[Ljava/lang/String;") \
 METHOD (pairBluetoothMidiDevice, "pairBluetoothMidiDevice", "(Ljava/lang/String;)Z") \
 METHOD (unpairBluetoothMidiDevice, "unpairBluetoothMidiDevice", "(Ljava/lang/String;)V") \
 METHOD (getHumanReadableStringForBluetoothAddress, "getHumanReadableStringForBluetoothAddress", "(Ljava/lang/String;)Ljava/lang/String;") \
 METHOD (getBluetoothDeviceStatus, "getBluetoothDeviceStatus", "(Ljava/lang/String;)I") \
 METHOD (startStopScan, "startStopScan", "(Z)V")

DECLARE_JNI_CLASS (AndroidBluetoothManager, "com/rmsl/juce/JuceMidiSupport$BluetoothMidiManager")
#undef JNI_CLASS_MEMBERS

//==============================================================================
struct AndroidBluetoothMidiInterface
{
    static void startStopScan (bool startScanning)
    {
        JNIEnv* env = getEnv();
        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidJuceMidiSupport, AndroidJuceMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() != nullptr)
            env->CallVoidMethod (btManager.get(), AndroidBluetoothManager.startStopScan, (jboolean) (startScanning ? 1 : 0));
    }

    static StringArray getBluetoothMidiDevicesNearby()
    {
        StringArray retval;

        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidJuceMidiSupport, AndroidJuceMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        // if this is null then bluetooth is not enabled
        if (btManager.get() == nullptr)
            return {};

        jobjectArray jDevices = (jobjectArray) env->CallObjectMethod (btManager.get(),
                                                                      AndroidBluetoothManager.getMidiBluetoothAddresses);
        LocalRef<jobjectArray> devices (jDevices);

        const int count = env->GetArrayLength (devices.get());

        for (int i = 0; i < count; ++i)
        {
            LocalRef<jstring> string ((jstring)  env->GetObjectArrayElement (devices.get(), i));
            retval.add (juceString (string));
        }

        return retval;
    }

    //==============================================================================
    static bool pairBluetoothMidiDevice (const String& bluetoothAddress)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidJuceMidiSupport, AndroidJuceMidiSupport.getAndroidBluetoothManager, getAppContext().get()));
        if (btManager.get() == nullptr)
            return false;

        jboolean result = env->CallBooleanMethod (btManager.get(), AndroidBluetoothManager.pairBluetoothMidiDevice,
                                                  javaString (bluetoothAddress).get());

        return result;
    }

    static void unpairBluetoothMidiDevice (const String& bluetoothAddress)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidJuceMidiSupport, AndroidJuceMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() != nullptr)
            env->CallVoidMethod (btManager.get(), AndroidBluetoothManager.unpairBluetoothMidiDevice,
                                 javaString (bluetoothAddress).get());
    }

    //==============================================================================
    static String getHumanReadableStringForBluetoothAddress (const String& address)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidJuceMidiSupport, AndroidJuceMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() == nullptr)
            return address;

        LocalRef<jstring> string ((jstring) env->CallObjectMethod (btManager.get(),
                                                                   AndroidBluetoothManager.getHumanReadableStringForBluetoothAddress,
                                                                   javaString (address).get()));


        if (string.get() == nullptr)
            return address;

        return juceString (string);
    }

    //==============================================================================
    enum PairStatus
    {
        unpaired = 0,
        paired = 1,
        pairing = 2
    };

    static PairStatus isBluetoothDevicePaired (const String& address)
    {
        JNIEnv* env = getEnv();

        LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidJuceMidiSupport, AndroidJuceMidiSupport.getAndroidBluetoothManager, getAppContext().get()));

        if (btManager.get() == nullptr)
            return unpaired;

        return static_cast<PairStatus> (env->CallIntMethod (btManager.get(), AndroidBluetoothManager.getBluetoothDeviceStatus,
                                                            javaString (address).get()));
    }
};

//==============================================================================
struct AndroidBluetoothMidiDevice
{
    enum ConnectionStatus
    {
        offline,
        connected,
        disconnected,
        connecting,
        disconnecting
    };

    AndroidBluetoothMidiDevice (String deviceName, String address, ConnectionStatus status)
        : name (deviceName), bluetoothAddress (address), connectionStatus (status)
    {
        // can't create a device without a valid name and bluetooth address!
        jassert (! name.isEmpty());
        jassert (! bluetoothAddress.isEmpty());
    }

    bool operator== (const AndroidBluetoothMidiDevice& other) const noexcept
    {
        return bluetoothAddress == other.bluetoothAddress;
    }

    bool operator!= (const AndroidBluetoothMidiDevice& other) const noexcept
    {
        return ! operator== (other);
    }

    const String name, bluetoothAddress;
    ConnectionStatus connectionStatus;
};

//==============================================================================
class AndroidBluetoothMidiDevicesListBox final : public ListBox,
                                                 private ListBoxModel,
                                                 private Timer
{
public:
    //==============================================================================
    AndroidBluetoothMidiDevicesListBox()
        : timerPeriodInMs (1000)
    {
        setRowHeight (40);
        setModel (this);
        setOutlineThickness (1);
        startTimer (timerPeriodInMs);
    }

    void pairDeviceThreadFinished() // callback from PairDeviceThread
    {
        updateDeviceList();
        startTimer (timerPeriodInMs);
    }

private:
    //==============================================================================
    typedef AndroidBluetoothMidiDevice::ConnectionStatus DeviceStatus;

    int getNumRows() override
    {
        return devices.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g,
                           int width, int height, bool) override
    {
        if (isPositiveAndBelow (rowNumber, devices.size()))
        {
            const AndroidBluetoothMidiDevice& device = devices.getReference (rowNumber);
            const String statusString (getDeviceStatusString (device.connectionStatus));

            g.fillAll (Colours::white);

            const float xmargin = 3.0f;
            const float ymargin = 3.0f;
            const float fontHeight = 0.4f * (float) height;
            const float deviceNameWidth = 0.6f * (float) width;

            g.setFont (fontHeight);

            g.setColour (getDeviceNameFontColour (device.connectionStatus));
            g.drawText (device.name,
                        Rectangle<float> (xmargin, ymargin, deviceNameWidth - (2.0f * xmargin), (float) height - (2.0f * ymargin)),
                        Justification::topLeft, true);

            g.setColour (getDeviceStatusFontColour (device.connectionStatus));
            g.drawText (statusString,
                        Rectangle<float> (deviceNameWidth + xmargin, ymargin,
                                          (float) width - deviceNameWidth - (2.0f * xmargin), (float) height - (2.0f * ymargin)),
                        Justification::topRight, true);

            g.setColour (Colours::grey);
            g.drawHorizontalLine (height - 1, xmargin, (float) width);
        }
    }

    //==============================================================================
    static Colour getDeviceNameFontColour (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline)
            return Colours::grey;

        return Colours::black;
    }

    static Colour getDeviceStatusFontColour (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline
            || deviceStatus == AndroidBluetoothMidiDevice::connecting
            || deviceStatus == AndroidBluetoothMidiDevice::disconnecting)
            return Colours::grey;

        if (deviceStatus == AndroidBluetoothMidiDevice::connected)
            return Colours::green;

        return Colours::black;
    }

    static String getDeviceStatusString (DeviceStatus deviceStatus) noexcept
    {
        if (deviceStatus == AndroidBluetoothMidiDevice::offline)        return "Offline";
        if (deviceStatus == AndroidBluetoothMidiDevice::connected)      return "Connected";
        if (deviceStatus == AndroidBluetoothMidiDevice::disconnected)   return "Not connected";
        if (deviceStatus == AndroidBluetoothMidiDevice::connecting)     return "Connecting...";
        if (deviceStatus == AndroidBluetoothMidiDevice::disconnecting)  return "Disconnecting...";

        // unknown device state!
        jassertfalse;
        return "Status unknown";
    }

    //==============================================================================
    void listBoxItemClicked (int row, const MouseEvent&) override
    {
        const AndroidBluetoothMidiDevice& device = devices.getReference (row);

        if (device.connectionStatus == AndroidBluetoothMidiDevice::disconnected)
            disconnectedDeviceClicked (row);

        else if (device.connectionStatus == AndroidBluetoothMidiDevice::connected)
            connectedDeviceClicked (row);
    }

    void timerCallback() override
    {
        updateDeviceList();
    }

    //==============================================================================
    struct PairDeviceThread final : public Thread,
                                    private AsyncUpdater
    {
        PairDeviceThread (const String& bluetoothAddressOfDeviceToPair,
                          AndroidBluetoothMidiDevicesListBox& ownerListBox)
            : Thread (SystemStats::getJUCEVersion() + ": Bluetooth MIDI Device Pairing Thread"),
              bluetoothAddress (bluetoothAddressOfDeviceToPair),
              owner (&ownerListBox)
        {
            startThread();
        }

        void run() override
        {
            AndroidBluetoothMidiInterface::pairBluetoothMidiDevice (bluetoothAddress);
            triggerAsyncUpdate();
        }

        void handleAsyncUpdate() override
        {
            if (owner != nullptr)
                owner->pairDeviceThreadFinished();

            delete this;
        }

    private:
        String bluetoothAddress;
        Component::SafePointer<AndroidBluetoothMidiDevicesListBox> owner;
    };

    //==============================================================================
    void disconnectedDeviceClicked (int row)
    {
        stopTimer();

        AndroidBluetoothMidiDevice& device = devices.getReference (row);
        device.connectionStatus = AndroidBluetoothMidiDevice::connecting;
        updateContent();
        repaint();

        new PairDeviceThread (device.bluetoothAddress, *this);
    }

    void connectedDeviceClicked (int row)
    {
        AndroidBluetoothMidiDevice& device = devices.getReference (row);
        device.connectionStatus = AndroidBluetoothMidiDevice::disconnecting;
        updateContent();
        repaint();
        AndroidBluetoothMidiInterface::unpairBluetoothMidiDevice (device.bluetoothAddress);
    }

    //==============================================================================
    void updateDeviceList()
    {
        StringArray bluetoothAddresses = AndroidBluetoothMidiInterface::getBluetoothMidiDevicesNearby();

        Array<AndroidBluetoothMidiDevice> newDevices;

        for (String* address = bluetoothAddresses.begin();
             address != bluetoothAddresses.end(); ++address)
        {
            String name = AndroidBluetoothMidiInterface::getHumanReadableStringForBluetoothAddress (*address);

            DeviceStatus status;
            switch (AndroidBluetoothMidiInterface::isBluetoothDevicePaired (*address))
            {
                case AndroidBluetoothMidiInterface::pairing:
                    status = AndroidBluetoothMidiDevice::connecting;
                    break;
                case AndroidBluetoothMidiInterface::paired:
                    status = AndroidBluetoothMidiDevice::connected;
                    break;
                case AndroidBluetoothMidiInterface::unpaired:
                default:
                    status = AndroidBluetoothMidiDevice::disconnected;
            }

            newDevices.add (AndroidBluetoothMidiDevice (name, *address, status));
        }

        devices.swapWith (newDevices);
        updateContent();
        repaint();
    }

    Array<AndroidBluetoothMidiDevice> devices;
    const int timerPeriodInMs;
};

//==============================================================================
class BluetoothMidiSelectorOverlay final : public Component
{
public:
    BluetoothMidiSelectorOverlay (ModalComponentManager::Callback* exitCallbackToUse,
                                  const Rectangle<int>& boundsToUse)
        : bounds (boundsToUse)
    {
        std::unique_ptr<ModalComponentManager::Callback> exitCallback (exitCallbackToUse);

        AndroidBluetoothMidiInterface::startStopScan (true);

        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (ComponentPeer::windowHasDropShadow);

        if (bounds.isEmpty())
            setBounds (0, 0, getParentWidth(), getParentHeight());
        else
            setBounds (bounds);

        toFront (true);
        setOpaque (! bounds.isEmpty());

        addAndMakeVisible (bluetoothDevicesList);
        enterModalState (true, exitCallback.release(), true);
    }

    ~BluetoothMidiSelectorOverlay() override
    {
        AndroidBluetoothMidiInterface::startStopScan (false);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (bounds.isEmpty() ? Colours::black.withAlpha (0.6f) : Colours::black);

        g.setColour (Colour (0xffdfdfdf));
        Rectangle<int> overlayBounds = getOverlayBounds();
        g.fillRect (overlayBounds);

        g.setColour (Colours::black);
        g.setFont (16);
        g.drawText ("Bluetooth MIDI Devices",
                    overlayBounds.removeFromTop (20).reduced (3, 3),
                    Justification::topLeft, true);

        overlayBounds.removeFromTop (2);

        g.setFont (12);
        g.drawText ("tap to connect/disconnect",
                    overlayBounds.removeFromTop (18).reduced (3, 3),
                    Justification::topLeft, true);
    }

    void inputAttemptWhenModal() override           { exitModalState (0); }
    void mouseDrag (const MouseEvent&) override     {}
    void mouseDown (const MouseEvent&) override     { exitModalState (0); }
    void resized() override                         { update(); }
    void parentSizeChanged() override               { update(); }

private:
    Rectangle<int> bounds;

    void update()
    {
        if (bounds.isEmpty())
            setBounds (0, 0, getParentWidth(), getParentHeight());
        else
            setBounds (bounds);

        bluetoothDevicesList.setBounds (getOverlayBounds().withTrimmedTop (40));
    }

    Rectangle<int> getOverlayBounds() const noexcept
    {
        if (bounds.isEmpty())
        {
            const int pw = getParentWidth();
            const int ph = getParentHeight();

            return Rectangle<int> (pw, ph).withSizeKeepingCentre (jmin (400, pw - 14),
                                                                  jmin (300, ph - 40));
        }

        return bounds.withZeroOrigin();
    }

    AndroidBluetoothMidiDevicesListBox bluetoothDevicesList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BluetoothMidiSelectorOverlay)
};

//==============================================================================
bool BluetoothMidiDevicePairingDialogue::open (ModalComponentManager::Callback* exitCallbackPtr,
                                               Rectangle<int>* btBounds)
{
    std::unique_ptr<ModalComponentManager::Callback> exitCallback (exitCallbackPtr);

    auto boundsToUse = (btBounds != nullptr ? *btBounds : Rectangle<int> {});

    if (! RuntimePermissions::isGranted (RuntimePermissions::bluetoothMidi))
    {
        // If you hit this assert, you probably forgot to get RuntimePermissions::bluetoothMidi.
        // This is not going to work, boo! The pairing dialogue won't be able to scan for or
        // find any devices, it will just display an empty list, so don't bother opening it.
        jassertfalse;
        return false;
    }

    new BluetoothMidiSelectorOverlay (exitCallback.release(), boundsToUse);
    return true;
}

bool BluetoothMidiDevicePairingDialogue::isAvailable()
{
    auto* env = getEnv();

    LocalRef<jobject> btManager (env->CallStaticObjectMethod (AndroidJuceMidiSupport, AndroidJuceMidiSupport.getAndroidBluetoothManager, getAppContext().get()));
    return btManager != nullptr;
}

} // namespace juce
