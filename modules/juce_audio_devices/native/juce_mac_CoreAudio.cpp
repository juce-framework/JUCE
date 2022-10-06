/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

#include <juce_audio_basics/native/juce_mac_CoreAudioTimeConversions.h>

namespace juce
{

#if JUCE_COREAUDIO_LOGGING_ENABLED
 #define JUCE_COREAUDIOLOG(a) { String camsg ("CoreAudio: "); camsg << a; Logger::writeToLog (camsg); }
#else
 #define JUCE_COREAUDIOLOG(a)
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnonnull")

constexpr auto juceAudioObjectPropertyElementMain =
       #if defined (MAC_OS_VERSION_12_0)
        kAudioObjectPropertyElementMain;
       #else
        kAudioObjectPropertyElementMaster;
       #endif

//==============================================================================
class ManagedAudioBufferList : public AudioBufferList
{
public:
    struct Deleter
    {
        void operator() (ManagedAudioBufferList* p) const
        {
            if (p != nullptr)
                p->~ManagedAudioBufferList();

            delete[] reinterpret_cast<std::byte*> (p);
        }
    };

    using Ref = std::unique_ptr<ManagedAudioBufferList, Deleter>;

    //==============================================================================
    static auto create (std::size_t numBuffers)
    {
        std::unique_ptr<std::byte[]> storage (new std::byte[storageSizeForNumBuffers (numBuffers)]);
        return Ref { new (storage.release()) ManagedAudioBufferList (numBuffers) };
    }

    //==============================================================================
    static std::size_t storageSizeForNumBuffers (std::size_t numBuffers) noexcept
    {
        return audioBufferListHeaderSize + (numBuffers * sizeof (::AudioBuffer));
    }

    static std::size_t numBuffersForStorageSize (std::size_t bytes) noexcept
    {
        bytes -= audioBufferListHeaderSize;

        // storage size ends between to buffers in AudioBufferList
        jassert ((bytes % sizeof (::AudioBuffer)) == 0);

        return bytes / sizeof (::AudioBuffer);
    }

private:
    // Do not call the base constructor here as this will zero-initialize the first buffer,
    // for which no storage may be available though (when numBuffers == 0).
    explicit ManagedAudioBufferList (std::size_t numBuffers)
    {
        mNumberBuffers = static_cast<UInt32> (numBuffers);
    }

    static constexpr auto audioBufferListHeaderSize = sizeof (AudioBufferList) - sizeof (::AudioBuffer);

    JUCE_DECLARE_NON_COPYABLE (ManagedAudioBufferList)
    JUCE_DECLARE_NON_MOVEABLE (ManagedAudioBufferList)
};

//==============================================================================
struct IgnoreUnused
{
    template <typename... Ts>
    void operator() (Ts&&...) const {}
};

template <typename T>
static auto getDataPtrAndSize (T& t)
{
    static_assert (std::is_pod_v<T>);
    return std::make_tuple (&t, (UInt32) sizeof (T));
}

static auto getDataPtrAndSize (ManagedAudioBufferList::Ref& t)
{
    const auto size = t.get() != nullptr
                    ? ManagedAudioBufferList::storageSizeForNumBuffers (t->mNumberBuffers)
                    : 0;
    return std::make_tuple (t.get(), (UInt32) size);
}

//==============================================================================
[[nodiscard]] static bool audioObjectHasProperty (AudioObjectID objectID, const AudioObjectPropertyAddress address)
{
    return objectID != kAudioObjectUnknown && AudioObjectHasProperty (objectID, &address);
}

template <typename T, typename OnError = IgnoreUnused>
[[nodiscard]] static auto audioObjectGetProperty (AudioObjectID objectID,
                                                  const AudioObjectPropertyAddress address,
                                                  OnError&& onError = {})
{
    using Result = std::conditional_t<std::is_same_v<T, AudioBufferList>, ManagedAudioBufferList::Ref, std::optional<T>>;

    if (! audioObjectHasProperty (objectID, address))
        return Result{};

    auto result = [&]
    {
        if constexpr (std::is_same_v<T, AudioBufferList>)
        {
            UInt32 size{};

            if (auto status = AudioObjectGetPropertyDataSize (objectID, &address, 0, nullptr, &size); status != noErr)
            {
                onError (status);
                return Result{};
            }

            return ManagedAudioBufferList::create (ManagedAudioBufferList::numBuffersForStorageSize (size));
        }
        else
        {
            return T{};
        }
    }();

    auto [ptr, size] = getDataPtrAndSize (result);

    if (size == 0)
        return Result{};

    if (auto status = AudioObjectGetPropertyData (objectID, &address, 0, nullptr, &size, ptr); status != noErr)
    {
        onError (status);
        return Result{};
    }

    return Result { std::move (result) };
}

template <typename T, typename OnError = IgnoreUnused>
static bool audioObjectSetProperty (AudioObjectID objectID,
                                    const AudioObjectPropertyAddress address,
                                    const T value,
                                    OnError&& onError = {})
{
    if (! audioObjectHasProperty (objectID, address))
        return false;

    Boolean isSettable = NO;
    if (auto status = AudioObjectIsPropertySettable (objectID, &address, &isSettable); status != noErr)
    {
        onError (status);
        return false;
    }

    if (! isSettable)
        return false;

    if (auto status = AudioObjectSetPropertyData (objectID, &address, 0, nullptr, static_cast<UInt32> (sizeof (T)), &value); status != noErr)
    {
        onError (status);
        return false;
    }

    return true;
}

template <typename T, typename OnError = IgnoreUnused>
[[nodiscard]] static std::vector<T> audioObjectGetProperties (AudioObjectID objectID,
                                                              const AudioObjectPropertyAddress address,
                                                              OnError&& onError = {})
{
    if (! audioObjectHasProperty (objectID, address))
        return {};

    UInt32 size{};

    if (auto status = AudioObjectGetPropertyDataSize (objectID, &address, 0, nullptr, &size); status != noErr)
    {
        onError (status);
        return {};
    }

    // If this is hit, the number of results is not integral, and the following
    // AudioObjectGetPropertyData will probably write past the end of the result buffer.
    jassert ((size % sizeof (T)) == 0);
    std::vector<T> result (size / sizeof (T));

    if (auto status = AudioObjectGetPropertyData (objectID, &address, 0, nullptr, &size, result.data()); status != noErr)
    {
        onError (status);
        return {};
    }

    return result;
}

//==============================================================================
struct AsyncRestarter
{
    virtual ~AsyncRestarter() = default;
    virtual void restartAsync() = 0;
};

struct SystemVol
{
    explicit SystemVol (AudioObjectPropertySelector selector) noexcept
        : outputDeviceID (audioObjectGetProperty<AudioDeviceID> (kAudioObjectSystemObject, { kAudioHardwarePropertyDefaultOutputDevice,
                                                                                             kAudioObjectPropertyScopeGlobal,
                                                                                             juceAudioObjectPropertyElementMain }).value_or (kAudioObjectUnknown)),
          addr { selector, kAudioDevicePropertyScopeOutput, juceAudioObjectPropertyElementMain }
    {}

    float getGain() const noexcept
    {
        return audioObjectGetProperty<Float32> (outputDeviceID, addr).value_or (0.0f);
    }

    bool setGain (float gain) const noexcept
    {
        return audioObjectSetProperty (outputDeviceID, addr, static_cast<Float32> (gain));
    }

    bool isMuted() const noexcept
    {
        return audioObjectGetProperty<UInt32> (outputDeviceID, addr).value_or (0) != 0;
    }

    bool setMuted (bool mute) const noexcept
    {
        return audioObjectSetProperty (outputDeviceID, addr, static_cast<UInt32> (mute ? 1 : 0));
    }

private:
    AudioDeviceID outputDeviceID;
    AudioObjectPropertyAddress addr;
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

constexpr auto juceAudioHardwareServiceDeviceProperty_VirtualMainVolume =
       #if defined (MAC_OS_VERSION_12_0)
        kAudioHardwareServiceDeviceProperty_VirtualMainVolume;
       #else
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume;
       #endif

#define JUCE_SYSTEMAUDIOVOL_IMPLEMENTED 1
float JUCE_CALLTYPE SystemAudioVolume::getGain()              { return SystemVol (juceAudioHardwareServiceDeviceProperty_VirtualMainVolume).getGain(); }
bool  JUCE_CALLTYPE SystemAudioVolume::setGain (float gain)   { return SystemVol (juceAudioHardwareServiceDeviceProperty_VirtualMainVolume).setGain (gain); }
bool  JUCE_CALLTYPE SystemAudioVolume::isMuted()              { return SystemVol (kAudioDevicePropertyMute).isMuted(); }
bool  JUCE_CALLTYPE SystemAudioVolume::setMuted (bool mute)   { return SystemVol (kAudioDevicePropertyMute).setMuted (mute); }

//==============================================================================
struct CoreAudioClasses
{

class CoreAudioIODeviceType;
class CoreAudioIODevice;

//==============================================================================
class CoreAudioInternal  : private Timer,
                           private AsyncUpdater
{
private:
    // members with deduced return types need to be defined before they
    // are used, so define it here. decltype doesn't help as you can't
    // capture anything in lambdas inside a decltype context.
    auto err2log() const { return [this] (OSStatus err) { OK (err); }; }

public:
    CoreAudioInternal (CoreAudioIODevice& d, AudioDeviceID id, bool hasInput, bool hasOutput)
        : owner (d),
          deviceID (id),
          inStream  (hasInput  ? new Stream (true,  *this, {}) : nullptr),
          outStream (hasOutput ? new Stream (false, *this, {}) : nullptr)
    {
        jassert (deviceID != 0);

        updateDetailsFromDevice();
        JUCE_COREAUDIOLOG ("Creating CoreAudioInternal\n"
                           << (inStream  != nullptr ? ("    inputDeviceId "  + String (deviceID) + "\n") : "")
                           << (outStream != nullptr ? ("    outputDeviceId " + String (deviceID) + "\n") : "")
                           << getDeviceDetails().joinIntoString ("\n    "));

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectAddPropertyListener (deviceID, &pa, deviceListenerProc, this);
    }

    ~CoreAudioInternal() override
    {
        stopTimer();
        cancelPendingUpdate();

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectRemovePropertyListener (deviceID, &pa, deviceListenerProc, this);

        stop (false);
    }

    auto getStreams() const { return std::array<Stream*, 2> { { inStream.get(), outStream.get() } }; }

    void allocateTempBuffers()
    {
        auto tempBufSize = bufferSize + 4;

        auto streams = getStreams();
        const auto total = std::accumulate (streams.begin(), streams.end(), 0,
                                            [] (int n, const auto& s) { return n + (s != nullptr ? s->channels : 0); });
        audioBuffer.calloc (total * tempBufSize);

        auto channels = 0;
        for (auto* stream : streams)
            channels += stream != nullptr ? stream->allocateTempBuffers (tempBufSize, channels, audioBuffer) : 0;
    }

    struct CallbackDetailsForChannel
    {
        int streamNum;
        int dataOffsetSamples;
        int dataStrideSamples;
    };

    Array<double> getSampleRatesFromDevice() const
    {
        Array<double> newSampleRates;

        if (auto ranges = audioObjectGetProperties<AudioValueRange> (deviceID,
                                                                     { kAudioDevicePropertyAvailableNominalSampleRates,
                                                                       kAudioObjectPropertyScopeWildcard,
                                                                       juceAudioObjectPropertyElementMain },
                                                                     err2log()); ! ranges.empty())
        {
            for (const auto rate : SampleRateHelpers::getAllSampleRates())
            {
                for (auto range = ranges.rbegin(); range != ranges.rend(); ++range)
                {
                    if (range->mMinimum - 2 <= rate && rate <= range->mMaximum + 2)
                    {
                        newSampleRates.add (rate);
                        break;
                    }
                }
            }
        }

        if (newSampleRates.isEmpty() && sampleRate > 0)
            newSampleRates.add (sampleRate);

        auto nominalRate = getNominalSampleRate();

        if ((nominalRate > 0) && ! newSampleRates.contains (nominalRate))
            newSampleRates.addUsingDefaultSort (nominalRate);

        return newSampleRates;
    }

    Array<int> getBufferSizesFromDevice() const
    {
        Array<int> newBufferSizes;

        if (auto ranges = audioObjectGetProperties<AudioValueRange> (deviceID, { kAudioDevicePropertyBufferFrameSizeRange,
                                                                                 kAudioObjectPropertyScopeWildcard,
                                                                                 juceAudioObjectPropertyElementMain },
                                                                     err2log()); ! ranges.empty())
        {
            newBufferSizes.add ((int) (ranges[0].mMinimum + 15) & ~15);

            for (int i = 32; i <= 2048; i += 32)
            {
                for (auto range = ranges.rbegin(); range != ranges.rend(); ++range)
                {
                    if (i >= range->mMinimum && i <= range->mMaximum)
                    {
                        newBufferSizes.addIfNotAlreadyThere (i);
                        break;
                    }
                }
            }

            if (bufferSize > 0)
                newBufferSizes.addIfNotAlreadyThere (bufferSize);
        }

        if (newBufferSizes.isEmpty() && bufferSize > 0)
            newBufferSizes.add (bufferSize);

        return newBufferSizes;
    }

    int getFrameSizeFromDevice() const
    {
        return static_cast<int> (audioObjectGetProperty<UInt32> (deviceID, { kAudioDevicePropertyBufferFrameSize,
                                                                             kAudioObjectPropertyScopeWildcard,
                                                                             juceAudioObjectPropertyElementMain }).value_or (0));
    }

    bool isDeviceAlive() const
    {
        return deviceID != 0
                 && audioObjectGetProperty<UInt32> (deviceID, { kAudioDevicePropertyDeviceIsAlive,
                                                                kAudioObjectPropertyScopeWildcard,
                                                                juceAudioObjectPropertyElementMain }, err2log()).value_or (0) != 0;
    }

    bool updateDetailsFromDevice (const BigInteger& activeIns, const BigInteger& activeOuts)
    {
        stopTimer();

        if (! isDeviceAlive())
            return false;

        // this collects all the new details from the device without any locking, then
        // locks + swaps them afterwards.

        auto newSampleRate = getNominalSampleRate();
        auto newBufferSize = getFrameSizeFromDevice();

        auto newBufferSizes = getBufferSizesFromDevice();
        auto newSampleRates = getSampleRatesFromDevice();

        std::unique_ptr<Stream> newInput  (inStream  != nullptr ? new Stream (true,  *this, activeIns)  : nullptr);
        std::unique_ptr<Stream> newOutput (outStream != nullptr ? new Stream (false, *this, activeOuts) : nullptr);

        auto newBitDepth = jmax (getBitDepth (newInput), getBitDepth (newOutput));

        {
            const ScopedLock sl (callbackLock);

            bitDepth = newBitDepth > 0 ? newBitDepth : 32;

            if (newSampleRate > 0)
                sampleRate = newSampleRate;

            bufferSize = newBufferSize;

            sampleRates.swapWith (newSampleRates);
            bufferSizes.swapWith (newBufferSizes);

            std::swap (inStream,  newInput);
            std::swap (outStream, newOutput);

            allocateTempBuffers();
        }

        return true;
    }

    bool updateDetailsFromDevice()
    {
        return updateDetailsFromDevice (getActiveChannels (inStream), getActiveChannels (outStream));
    }

    StringArray getDeviceDetails()
    {
        StringArray result;

        String availableSampleRates ("Available sample rates:");

        for (auto& s : sampleRates)
            availableSampleRates << " " << s;

        result.add (availableSampleRates);
        result.add ("Sample rate: " + String (sampleRate));
        String availableBufferSizes ("Available buffer sizes:");

        for (auto& b : bufferSizes)
            availableBufferSizes << " " << b;

        result.add (availableBufferSizes);
        result.add ("Buffer size: " + String (bufferSize));
        result.add ("Bit depth: " + String (bitDepth));
        result.add ("Input latency: "  + String (getLatency (inStream)));
        result.add ("Output latency: " + String (getLatency (outStream)));
        result.add ("Input channel names: "  + getChannelNames (inStream));
        result.add ("Output channel names: " + getChannelNames (outStream));

        return result;
    }

    static auto getScope (bool input)
    {
        return input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    }

    //==============================================================================
    StringArray getSources (bool input)
    {
        StringArray s;
        auto types = audioObjectGetProperties<OSType> (deviceID, { kAudioDevicePropertyDataSources,
                                                                   kAudioObjectPropertyScopeWildcard,
                                                                   juceAudioObjectPropertyElementMain });

        for (auto type : types)
        {
            AudioValueTranslation avt;
            char buffer[256];

            avt.mInputData = &type;
            avt.mInputDataSize = sizeof (UInt32);
            avt.mOutputData = buffer;
            avt.mOutputDataSize = 256;

            UInt32 transSize = sizeof (avt);

            AudioObjectPropertyAddress pa;
            pa.mSelector = kAudioDevicePropertyDataSourceNameForID;
            pa.mScope = getScope (input);
            pa.mElement = juceAudioObjectPropertyElementMain;

            if (OK (AudioObjectGetPropertyData (deviceID, &pa, 0, nullptr, &transSize, &avt)))
                s.add (buffer);
        }

        return s;
    }

    int getCurrentSourceIndex (bool input) const
    {
        if (deviceID != 0)
        {
            if (auto currentSourceID = audioObjectGetProperty<OSType> (deviceID, { kAudioDevicePropertyDataSource,
                                                                                   getScope (input),
                                                                                   juceAudioObjectPropertyElementMain }, err2log()))
            {
                auto types = audioObjectGetProperties<OSType> (deviceID, { kAudioDevicePropertyDataSources,
                                                                           kAudioObjectPropertyScopeWildcard,
                                                                           juceAudioObjectPropertyElementMain });

                if (auto it = std::find (types.begin(), types.end(), *currentSourceID); it != types.end())
                    return static_cast<int> (std::distance (types.begin(), it));
            }
        }

        return -1;
    }

    void setCurrentSourceIndex (int index, bool input)
    {
        if (deviceID != 0)
        {
            auto types = audioObjectGetProperties<OSType> (deviceID, { kAudioDevicePropertyDataSources,
                                                                       kAudioObjectPropertyScopeWildcard,
                                                                       juceAudioObjectPropertyElementMain });

            if (isPositiveAndBelow (index, static_cast<int> (types.size())))
            {
                audioObjectSetProperty<OSType> (deviceID, { kAudioDevicePropertyDataSource,
                                                            getScope (input),
                                                            juceAudioObjectPropertyElementMain },
                                                types[static_cast<std::size_t> (index)], err2log());
            }
        }
    }

    double getNominalSampleRate() const
    {
        return static_cast<double> (audioObjectGetProperty <Float64> (deviceID, { kAudioDevicePropertyNominalSampleRate,
                                                                                  kAudioObjectPropertyScopeGlobal,
                                                                                  juceAudioObjectPropertyElementMain },
                                                                      err2log()).value_or (0.0));
    }

    bool setNominalSampleRate (double newSampleRate) const
    {
        if (std::abs (getNominalSampleRate() - newSampleRate) < 1.0)
            return true;

        return audioObjectSetProperty (deviceID, { kAudioDevicePropertyNominalSampleRate,
                                                   kAudioObjectPropertyScopeGlobal,
                                                   juceAudioObjectPropertyElementMain },
                                       static_cast<Float64> (newSampleRate), err2log());
    }

    //==============================================================================
    String reopen (const BigInteger& ins, const BigInteger& outs, double newSampleRate, int bufferSizeSamples)
    {
        String error;
        callbacksAllowed = false;
        stopTimer();

        stop (false);

        const auto activeIns  = BigInteger().setRange (0, jmin (ins .getHighestBit() + 1, getNumChannelNames (inStream)),  true);
        const auto activeOuts = BigInteger().setRange (0, jmin (outs.getHighestBit() + 1, getNumChannelNames (outStream)), true);

        if (! setNominalSampleRate (newSampleRate))
        {
            updateDetailsFromDevice (activeIns, activeOuts);
            error = "Couldn't change sample rate";
        }
        else
        {
            if (! audioObjectSetProperty (deviceID, { kAudioDevicePropertyBufferFrameSize,
                                                      kAudioObjectPropertyScopeGlobal,
                                                      juceAudioObjectPropertyElementMain },
                                          static_cast<UInt32> (bufferSizeSamples), err2log()))
            {
                updateDetailsFromDevice (activeIns, activeOuts);
                error = "Couldn't change buffer size";
            }
            else
            {
                // Annoyingly, after changing the rate and buffer size, some devices fail to
                // correctly report their new settings until some random time in the future, so
                // after calling updateDetailsFromDevice, we need to manually bodge these values
                // to make sure we're using the correct numbers..
                updateDetailsFromDevice (activeIns, activeOuts);
                sampleRate = newSampleRate;
                bufferSize = bufferSizeSamples;

                if (sampleRates.size() == 0)
                    error = "Device has no available sample-rates";
                else if (bufferSizes.size() == 0)
                    error = "Device has no available buffer-sizes";
            }
        }

        callbacksAllowed = true;
        return error;
    }

    bool start (AudioIODeviceCallback* callbackToNotify)
    {
        const ScopedLock sl (callbackLock);

        if (callback == nullptr && callbackToNotify != nullptr)
        {
            callback = callbackToNotify;
            callback->audioDeviceAboutToStart (&owner);
        }

        if (scopedProcID.get() == nullptr && deviceID != 0)
        {
            scopedProcID = [&self = *this,
                            &lock = callbackLock,
                            nextProcID = ScopedAudioDeviceIOProcID { *this, deviceID, audioIOProc },
                            deviceID = deviceID]() mutable -> ScopedAudioDeviceIOProcID
            {
                // It *looks* like AudioDeviceStart may start the audio callback running, and then
                // immediately lock an internal mutex.
                // The same mutex is locked before calling the audioIOProc.
                // If we get very unlucky, then we can end up with thread A taking the callbackLock
                // and calling AudioDeviceStart, followed by thread B taking the CoreAudio lock
                // and calling into audioIOProc, which waits on the callbackLock. When thread A
                // continues it attempts to take the CoreAudio lock, and the program deadlocks.

                if (auto* procID = nextProcID.get())
                {
                    const ScopedUnlock su (lock);

                    if (self.OK (AudioDeviceStart (deviceID, procID)))
                        return std::move (nextProcID);
                }

                return {};
            }();
        }

        playing = scopedProcID.get() != nullptr && callback != nullptr;

        return scopedProcID.get() != nullptr;
    }

    AudioIODeviceCallback* stop (bool leaveInterruptRunning)
    {
        const ScopedLock sl (callbackLock);

        auto result = std::exchange (callback, nullptr);

        if (scopedProcID.get() != nullptr && (deviceID != 0) && ! leaveInterruptRunning)
        {
            audioDeviceStopPending = true;

            // wait until AudioDeviceStop() has been called on the IO thread
            for (int i = 40; --i >= 0;)
            {
                if (audioDeviceStopPending == false)
                    break;

                const ScopedUnlock ul (callbackLock);
                Thread::sleep (50);
            }

            scopedProcID = {};
            playing = false;
        }

        return result;
    }

    double getSampleRate() const  { return sampleRate; }
    int getBufferSize() const     { return bufferSize; }

    void audioCallback (const AudioTimeStamp* timeStamp,
                        const AudioBufferList* inInputData,
                        AudioBufferList* outOutputData)
    {
        const ScopedLock sl (callbackLock);

        if (audioDeviceStopPending)
        {
            if (OK (AudioDeviceStop (deviceID, scopedProcID.get())))
                audioDeviceStopPending = false;

            return;
        }

        const auto numInputChans  = getChannels (inStream);
        const auto numOutputChans = getChannels (outStream);

        if (callback != nullptr)
        {
            for (int i = numInputChans; --i >= 0;)
            {
                auto& info = inStream->channelInfo.getReference (i);
                auto dest = inStream->tempBuffers[i];
                auto src = ((const float*) inInputData->mBuffers[info.streamNum].mData) + info.dataOffsetSamples;
                auto stride = info.dataStrideSamples;

                if (stride != 0) // if this is zero, info is invalid
                {
                    for (int j = bufferSize; --j >= 0;)
                    {
                        *dest++ = *src;
                        src += stride;
                    }
                }
            }

            const auto nanos = timeStamp != nullptr ? timeConversions.hostTimeToNanos (timeStamp->mHostTime) : 0;

            callback->audioDeviceIOCallbackWithContext (getTempBuffers (inStream),  numInputChans,
                                                        getTempBuffers (outStream), numOutputChans,
                                                        bufferSize,
                                                        { timeStamp != nullptr ? &nanos : nullptr });

            for (int i = numOutputChans; --i >= 0;)
            {
                auto& info = outStream->channelInfo.getReference (i);
                auto src = outStream->tempBuffers[i];
                auto dest = ((float*) outOutputData->mBuffers[info.streamNum].mData) + info.dataOffsetSamples;
                auto stride = info.dataStrideSamples;

                if (stride != 0) // if this is zero, info is invalid
                {
                    for (int j = bufferSize; --j >= 0;)
                    {
                        *dest = *src++;
                        dest += stride;
                    }
                }
            }
        }
        else
        {
            for (UInt32 i = 0; i < outOutputData->mNumberBuffers; ++i)
                zeromem (outOutputData->mBuffers[i].mData,
                         outOutputData->mBuffers[i].mDataByteSize);
        }
    }

    // called by callbacks (possibly off the main thread)
    void deviceDetailsChanged()
    {
        if (callbacksAllowed.get() == 1)
            startTimer (100);
    }

    // called by callbacks (possibly off the main thread)
    void deviceRequestedRestart()
    {
        owner.restart();
        triggerAsyncUpdate();
    }

    bool isPlaying() const { return playing.load(); }

    //==============================================================================
    struct Stream
    {
        Stream (bool isInput, CoreAudioInternal& parent, const BigInteger& active)
            : input (isInput),
              latency (getLatencyFromDevice (isInput, parent)),
              bitDepth (getBitDepthFromDevice (isInput, parent)),
              activeChans (active),
              chanNames (getChannelNames (isInput, parent)),
              channelInfo (getChannelInfos (isInput, parent, active)),
              channels (static_cast<int> (channelInfo.size()))
        {}

        int allocateTempBuffers (int tempBufSize, int channelCount, HeapBlock<float>& buffer)
        {
            tempBuffers.calloc (channels + 2);

            for (int i = 0; i < channels;  ++i)
                tempBuffers[i] = buffer + channelCount++ * tempBufSize;

            return channels;
        }

        template <typename Visitor>
        static auto visitChannels (bool isInput, CoreAudioInternal& parent, Visitor&& visitor)
        {
            struct Args { int stream, channelIdx, chanNum, streamChannels; };
            using VisitorResultType = typename std::invoke_result_t<Visitor, const Args&>::value_type;
            Array<VisitorResultType> result;
            int chanNum = 0;

            if (auto bufList = audioObjectGetProperty<AudioBufferList> (parent.deviceID, { kAudioDevicePropertyStreamConfiguration,
                                                                                           getScope (isInput),
                                                                                           juceAudioObjectPropertyElementMain }, parent.err2log()))
            {
                const int numStreams = static_cast<int> (bufList->mNumberBuffers);

                for (int i = 0; i < numStreams; ++i)
                {
                    auto& b = bufList->mBuffers[i];

                    for (unsigned int j = 0; j < b.mNumberChannels; ++j)
                    {
                        // Passing an anonymous struct ensures that callback can't confuse the argument order
                        if (auto opt = visitor (Args { i, static_cast<int> (j), chanNum++, static_cast<int> (b.mNumberChannels) }))
                            result.add (std::move (*opt));
                    }
                }
            }

            return result;
        }

        static Array<CallbackDetailsForChannel> getChannelInfos (bool isInput, CoreAudioInternal& parent, const BigInteger& active)
        {
            return visitChannels (isInput, parent,
                                  [&] (const auto& args) -> std::optional<CallbackDetailsForChannel>
                                  {
                                      if (! active[args.chanNum])
                                          return {};

                                      return CallbackDetailsForChannel { args.stream, args.channelIdx, args.streamChannels };
                                  });
        }

        static StringArray getChannelNames (bool isInput, CoreAudioInternal& parent)
        {
            auto names = visitChannels (isInput, parent,
                                        [&] (const auto& args) -> std::optional<String>
                                        {
                                            String name;
                                            const auto element = static_cast<AudioObjectPropertyElement> (args.chanNum + 1);

                                            if (auto nameNSString = audioObjectGetProperty<NSString*> (parent.deviceID, { kAudioObjectPropertyElementName,
                                                                                                                          getScope (isInput),
                                                                                                                          element }).value_or (nullptr))
                                            {
                                                name = nsStringToJuce (nameNSString);
                                                [nameNSString release];
                                            }

                                            if (name.isEmpty())
                                                name << (isInput ? "Input " : "Output ") << (args.chanNum + 1);

                                            return name;
                                        });

            return { names };
        }

        static int getBitDepthFromDevice (bool isInput, CoreAudioInternal& parent)
        {
            return static_cast<int> (audioObjectGetProperty<AudioStreamBasicDescription> (parent.deviceID, { kAudioStreamPropertyPhysicalFormat,
                                                                                                             getScope (isInput),
                                                                                                             juceAudioObjectPropertyElementMain }, parent.err2log())
                                                                                         .value_or (AudioStreamBasicDescription{}).mBitsPerChannel);
        }

        static int getLatencyFromDevice (bool isInput, CoreAudioInternal& parent)
        {
            const auto scope = getScope (isInput);

            const auto deviceLatency  = audioObjectGetProperty<UInt32> (parent.deviceID, { kAudioDevicePropertyLatency,
                                                                                           scope,
                                                                                           juceAudioObjectPropertyElementMain }).value_or (0);

            const auto safetyOffset   = audioObjectGetProperty<UInt32> (parent.deviceID, { kAudioDevicePropertySafetyOffset,
                                                                                           scope,
                                                                                           juceAudioObjectPropertyElementMain }).value_or (0);

            const auto framesInBuffer = audioObjectGetProperty<UInt32> (parent.deviceID, { kAudioDevicePropertyBufferFrameSize,
                                                                                           kAudioObjectPropertyScopeWildcard,
                                                                                           juceAudioObjectPropertyElementMain }).value_or (0);

            UInt32 streamLatency = 0;

            if (auto streams = audioObjectGetProperties<AudioStreamID> (parent.deviceID, { kAudioDevicePropertyStreams,
                                                                                           scope,
                                                                                           juceAudioObjectPropertyElementMain }); ! streams.empty())
                streamLatency = audioObjectGetProperty<UInt32> (streams.front(), { kAudioStreamPropertyLatency,
                                                                                   scope,
                                                                                   juceAudioObjectPropertyElementMain }).value_or (0);

            return static_cast<int> (deviceLatency + safetyOffset + framesInBuffer + streamLatency);
        }

        //==============================================================================
        const bool input;
        const int latency;
        const int bitDepth;
        const BigInteger activeChans;
        const StringArray chanNames;
        const Array<CallbackDetailsForChannel> channelInfo;
        const int channels = 0;

        HeapBlock<float*> tempBuffers;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Stream)
    };

    template <typename Callback>
    static auto getWithDefault (const std::unique_ptr<Stream>& ptr, Callback&& callback)
    {
        return ptr != nullptr ? callback (*ptr) : decltype (callback (*ptr)) {};
    }

    template <typename Value>
    static auto getWithDefault (const std::unique_ptr<Stream>& ptr, Value (Stream::* member))
    {
        return getWithDefault (ptr, [&] (Stream& s) { return s.*member; });
    }

    static int          getLatency          (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::latency); }
    static int          getBitDepth         (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::bitDepth); }
    static int          getChannels         (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::channels); }
    static int          getNumChannelNames  (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::chanNames).size(); }
    static String       getChannelNames     (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::chanNames).joinIntoString (" "); }
    static BigInteger   getActiveChannels   (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::activeChans); }
    static float**      getTempBuffers      (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, [] (auto& s) { return s.tempBuffers.get(); }); }

    //==============================================================================
    static constexpr Float64 invalidSampleTime = std::numeric_limits<Float64>::max();

    CoreAudioIODevice& owner;
    int bitDepth = 32;
    int xruns = 0;
    Array<double> sampleRates;
    Array<int> bufferSizes;
    AudioDeviceID deviceID;
    std::unique_ptr<Stream> inStream, outStream;

private:
    class ScopedAudioDeviceIOProcID
    {
    public:
        ScopedAudioDeviceIOProcID() = default;

        ScopedAudioDeviceIOProcID (CoreAudioInternal& coreAudio, AudioDeviceID d, AudioDeviceIOProc audioIOProc)
            : deviceID (d)
        {
            if (! coreAudio.OK (AudioDeviceCreateIOProcID (deviceID, audioIOProc, &coreAudio, &proc)))
                proc = {};
        }

        ~ScopedAudioDeviceIOProcID() noexcept
        {
            if (proc != AudioDeviceIOProcID{})
                AudioDeviceDestroyIOProcID (deviceID, proc);
        }

        ScopedAudioDeviceIOProcID (ScopedAudioDeviceIOProcID&& other) noexcept
        {
            swap (other);
        }

        ScopedAudioDeviceIOProcID& operator= (ScopedAudioDeviceIOProcID&& other) noexcept
        {
            ScopedAudioDeviceIOProcID { std::move (other) }.swap (*this);
            return *this;
        }

        AudioDeviceIOProcID get() const { return proc; }

    private:
        void swap (ScopedAudioDeviceIOProcID& other) noexcept
        {
            std::swap (other.deviceID, deviceID);
            std::swap (other.proc, proc);
        }

        AudioDeviceID deviceID = {};
        AudioDeviceIOProcID proc = {};
    };

    //==============================================================================
    ScopedAudioDeviceIOProcID scopedProcID;
    CoreAudioTimeConversions timeConversions;
    AudioIODeviceCallback* callback = nullptr;
    CriticalSection callbackLock;
    bool audioDeviceStopPending = false;
    std::atomic<bool> playing { false };
    double sampleRate = 0;
    int bufferSize = 0;
    HeapBlock<float> audioBuffer;
    Atomic<int> callbacksAllowed { 1 };

    //==============================================================================
    void timerCallback() override
    {
        JUCE_COREAUDIOLOG ("Device changed");

        stopTimer();
        auto oldSampleRate = sampleRate;
        auto oldBufferSize = bufferSize;

        if (! updateDetailsFromDevice())
            owner.stopInternal();
        else if ((oldBufferSize != bufferSize || oldSampleRate != sampleRate) && owner.shouldRestartDevice())
            owner.restart();
    }

    void handleAsyncUpdate() override
    {
        if (owner.deviceType != nullptr)
            owner.deviceType->audioDeviceListChanged();
    }

    static OSStatus audioIOProc (AudioDeviceID /*inDevice*/,
                                 const AudioTimeStamp* inNow,
                                 const AudioBufferList* inInputData,
                                 [[maybe_unused]] const AudioTimeStamp* inInputTime,
                                 AudioBufferList* outOutputData,
                                 [[maybe_unused]] const AudioTimeStamp* inOutputTime,
                                 void* device)
    {
        static_cast<CoreAudioInternal*> (device)->audioCallback (inNow, inInputData, outOutputData);
        return noErr;
    }

    static OSStatus deviceListenerProc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/,
                                        const AudioObjectPropertyAddress* pa, void* inClientData)
    {
        auto intern = static_cast<CoreAudioInternal*> (inClientData);

        switch (pa->mSelector)
        {
            case kAudioDeviceProcessorOverload:
                intern->xruns++;
                break;

            case kAudioDevicePropertyBufferSize:
            case kAudioDevicePropertyBufferFrameSize:
            case kAudioDevicePropertyNominalSampleRate:
            case kAudioDevicePropertyStreamFormat:
            case kAudioDevicePropertyDeviceIsAlive:
            case kAudioStreamPropertyPhysicalFormat:
                intern->deviceDetailsChanged();
                break;

            case kAudioDevicePropertyDeviceHasChanged:
            case kAudioObjectPropertyOwnedObjects:
                intern->deviceRequestedRestart();
                break;

            case kAudioDevicePropertyBufferSizeRange:
            case kAudioDevicePropertyVolumeScalar:
            case kAudioDevicePropertyMute:
            case kAudioDevicePropertyPlayThru:
            case kAudioDevicePropertyDataSource:
            case kAudioDevicePropertyDeviceIsRunning:
                break;
        }

        return noErr;
    }

    //==============================================================================
    bool OK (const OSStatus errorCode) const
    {
        if (errorCode == noErr)
            return true;

        const String errorMessage ("CoreAudio error: " + String::toHexString ((int) errorCode));
        JUCE_COREAUDIOLOG (errorMessage);

        if (callback != nullptr)
            callback->audioDeviceError (errorMessage);

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioInternal)
};


//==============================================================================
class CoreAudioIODevice   : public AudioIODevice,
                            private Timer
{
public:
    CoreAudioIODevice (CoreAudioIODeviceType* dt,
                       const String& deviceName,
                       AudioDeviceID inputDeviceId, int inputIndex_,
                       AudioDeviceID outputDeviceId, int outputIndex_)
        : AudioIODevice (deviceName, "CoreAudio"),
          deviceType (dt),
          inputIndex (inputIndex_),
          outputIndex (outputIndex_)
    {
        internal = [this, &inputDeviceId, &outputDeviceId]
        {
            if (outputDeviceId == 0 || outputDeviceId == inputDeviceId)
            {
                jassert (inputDeviceId != 0);
                return std::make_unique<CoreAudioInternal> (*this, inputDeviceId, true, outputDeviceId != 0);
            }

            return std::make_unique<CoreAudioInternal> (*this, outputDeviceId, false, true);
        }();

        jassert (internal != nullptr);

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope    = kAudioObjectPropertyScopeWildcard;
        pa.mElement  = kAudioObjectPropertyElementWildcard;

        AudioObjectAddPropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, internal.get());
    }

    ~CoreAudioIODevice() override
    {
        close();

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioObjectPropertySelectorWildcard;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectRemovePropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, internal.get());
    }

    StringArray getOutputChannelNames() override        { return internal->outStream != nullptr ? internal->outStream->chanNames : StringArray(); }
    StringArray getInputChannelNames() override         { return internal->inStream  != nullptr ? internal->inStream ->chanNames : StringArray(); }

    bool isOpen() override                              { return isOpen_; }

    Array<double> getAvailableSampleRates() override    { return internal->sampleRates; }
    Array<int> getAvailableBufferSizes() override       { return internal->bufferSizes; }

    double getCurrentSampleRate() override              { return internal->getSampleRate(); }
    int getCurrentBitDepth() override                   { return internal->bitDepth; }
    int getCurrentBufferSizeSamples() override          { return internal->getBufferSize(); }
    int getXRunCount() const noexcept override          { return internal->xruns; }

    int getIndexOfDevice (bool asInput) const           { return asInput ? inputIndex : outputIndex; }

    int getDefaultBufferSize() override
    {
        int best = 0;

        for (int i = 0; best < 512 && i < internal->bufferSizes.size(); ++i)
            best = internal->bufferSizes.getUnchecked (i);

        if (best == 0)
            best = 512;

        return best;
    }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sampleRate, int bufferSizeSamples) override
    {
        isOpen_ = true;
        internal->xruns = 0;

        inputChannelsRequested = inputChannels;
        outputChannelsRequested = outputChannels;

        if (bufferSizeSamples <= 0)
            bufferSizeSamples = getDefaultBufferSize();

        if (sampleRate <= 0)
            sampleRate = internal->getNominalSampleRate();

        lastError = internal->reopen (inputChannels, outputChannels, sampleRate, bufferSizeSamples);
        JUCE_COREAUDIOLOG ("Opened: " << getName());

        isOpen_ = lastError.isEmpty();

        return lastError;
    }

    void close() override
    {
        isOpen_ = false;
        internal->stop (false);
    }

    BigInteger getActiveOutputChannels()      const override { return CoreAudioInternal::getActiveChannels (internal->outStream); }
    BigInteger getActiveInputChannels()       const override { return CoreAudioInternal::getActiveChannels (internal->inStream); }
    int getOutputLatencyInSamples()                 override { return CoreAudioInternal::getLatency (internal->outStream); }
    int getInputLatencyInSamples()                  override { return CoreAudioInternal::getLatency (internal->inStream); }

    void start (AudioIODeviceCallback* callback) override
    {
        if (internal->start (callback))
            previousCallback = callback;
    }

    void stop() override
    {
        restartDevice = false;
        stopAndGetLastCallback();
    }

    AudioIODeviceCallback* stopAndGetLastCallback() const
    {
        auto* lastCallback = internal->stop (true);

        if (lastCallback != nullptr)
            lastCallback->audioDeviceStopped();

        return lastCallback;
    }

    AudioIODeviceCallback* stopInternal()
    {
        restartDevice = true;
        return stopAndGetLastCallback();
    }

    bool isPlaying() override
    {
        return internal->isPlaying();
    }

    String getLastError() override
    {
        return lastError;
    }

    void audioDeviceListChanged()
    {
        if (deviceType != nullptr)
            deviceType->audioDeviceListChanged();
    }

    // called by callbacks (possibly off the main thread)
    void restart()
    {
        if (restarter != nullptr)
        {
            restarter->restartAsync();
            return;
        }

        {
            const ScopedLock sl (closeLock);
            previousCallback = stopInternal();
        }

        startTimer (100);
    }

    bool setCurrentSampleRate (double newSampleRate)
    {
        return internal->setNominalSampleRate (newSampleRate);
    }

    void setAsyncRestarter (AsyncRestarter* restarterIn)
    {
        restarter = restarterIn;
    }

    bool shouldRestartDevice() const noexcept    { return restartDevice; }

    WeakReference<CoreAudioIODeviceType> deviceType;
    int inputIndex, outputIndex;

private:
    std::unique_ptr<CoreAudioInternal> internal;
    bool isOpen_ = false, restartDevice = true;
    String lastError;
    AudioIODeviceCallback* previousCallback = nullptr;
    AsyncRestarter* restarter = nullptr;
    BigInteger inputChannelsRequested, outputChannelsRequested;
    CriticalSection closeLock;

    void timerCallback() override
    {
        stopTimer();

        stopInternal();

        internal->updateDetailsFromDevice();

        open (inputChannelsRequested, outputChannelsRequested,
              getCurrentSampleRate(), getCurrentBufferSizeSamples());
        start (previousCallback);
    }

    static OSStatus hardwareListenerProc (AudioDeviceID /*inDevice*/, UInt32 /*inLine*/, const AudioObjectPropertyAddress* pa, void* inClientData)
    {
        switch (pa->mSelector)
        {
            case kAudioHardwarePropertyDevices:
                static_cast<CoreAudioInternal*> (inClientData)->deviceDetailsChanged();
                break;

            case kAudioHardwarePropertyDefaultOutputDevice:
            case kAudioHardwarePropertyDefaultInputDevice:
            case kAudioHardwarePropertyDefaultSystemOutputDevice:
                break;
        }

        return noErr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioIODevice)
};

//==============================================================================
class AudioIODeviceCombiner    : public AudioIODevice,
                                 private AsyncRestarter,
                                 private Thread,
                                 private Timer
{
public:
    AudioIODeviceCombiner (const String& deviceName, CoreAudioIODeviceType* deviceType,
                           std::unique_ptr<CoreAudioIODevice>&& inputDevice,
                           std::unique_ptr<CoreAudioIODevice>&& outputDevice)
        : AudioIODevice (deviceName, "CoreAudio"),
          Thread (deviceName),
          owner (deviceType),
          currentSampleRate (inputDevice->getCurrentSampleRate()),
          currentBufferSize (inputDevice->getCurrentBufferSizeSamples()),
          inputWrapper  (*this, std::move (inputDevice),  true),
          outputWrapper (*this, std::move (outputDevice), false)
    {
        if (getAvailableSampleRates().isEmpty())
            lastError = TRANS("The input and output devices don't share a common sample rate!");
    }

    ~AudioIODeviceCombiner() override
    {
        close();
    }

    auto getDeviceWrappers()       { return std::array<      DeviceWrapper*, 2> { { &inputWrapper, &outputWrapper } }; }
    auto getDeviceWrappers() const { return std::array<const DeviceWrapper*, 2> { { &inputWrapper, &outputWrapper } }; }

    StringArray getOutputChannelNames() override        { return outputWrapper.getChannelNames(); }
    StringArray getInputChannelNames()  override        { return inputWrapper .getChannelNames(); }
    BigInteger getActiveOutputChannels() const override { return outputWrapper.getActiveChannels(); }
    BigInteger getActiveInputChannels() const override  { return inputWrapper .getActiveChannels(); }

    Array<double> getAvailableSampleRates() override
    {
        Array<double> commonRates;
        bool first = true;

        for (auto& d : getDeviceWrappers())
        {
            auto rates = d->getAvailableSampleRates();

            if (first)
            {
                first = false;
                commonRates = rates;
            }
            else
            {
                commonRates.removeValuesNotIn (rates);
            }
        }

        return commonRates;
    }

    Array<int> getAvailableBufferSizes() override
    {
        Array<int> commonSizes;
        bool first = true;

        for (auto& d : getDeviceWrappers())
        {
            auto sizes = d->getAvailableBufferSizes();

            if (first)
            {
                first = false;
                commonSizes = sizes;
            }
            else
            {
                commonSizes.removeValuesNotIn (sizes);
            }
        }

        return commonSizes;
    }

    bool isOpen() override                          { return active; }
    bool isPlaying() override                       { return callback != nullptr; }
    double getCurrentSampleRate() override          { return currentSampleRate; }
    int getCurrentBufferSizeSamples() override      { return currentBufferSize; }

    int getCurrentBitDepth() override
    {
        int depth = 32;

        for (auto& d : getDeviceWrappers())
            depth = jmin (depth, d->getCurrentBitDepth());

        return depth;
    }

    int getDefaultBufferSize() override
    {
        int size = 0;

        for (auto& d : getDeviceWrappers())
            size = jmax (size, d->getDefaultBufferSize());

        return size;
    }

    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sampleRate, int bufferSize) override
    {
        inputChannelsRequested = inputChannels;
        outputChannelsRequested = outputChannels;
        sampleRateRequested = sampleRate;
        bufferSizeRequested = bufferSize;

        close();
        active = true;

        if (bufferSize <= 0)
            bufferSize = getDefaultBufferSize();

        if (sampleRate <= 0)
        {
            auto rates = getAvailableSampleRates();

            for (int i = 0; i < rates.size() && sampleRate < 44100.0; ++i)
                sampleRate = rates.getUnchecked (i);
        }

        currentSampleRate = sampleRate;
        currentBufferSize = bufferSize;

        const int fifoSize = bufferSize * 3 + 1;
        int totalInputChanIndex = 0, totalOutputChanIndex = 0;
        int chanIndex = 0;

        for (auto& d : getDeviceWrappers())
        {
            BigInteger ins (inputChannels >> totalInputChanIndex);
            BigInteger outs (outputChannels >> totalOutputChanIndex);

            int numIns  = d->getInputChannelNames().size();
            int numOuts = d->getOutputChannelNames().size();

            totalInputChanIndex += numIns;
            totalOutputChanIndex += numOuts;

            String err = d->open (ins, outs, sampleRate, bufferSize,
                                  chanIndex, fifoSize);

            if (err.isNotEmpty())
            {
                close();
                lastError = err;
                return err;
            }

            chanIndex += d->numInputChans + d->numOutputChans;
        }

        fifos.setSize (chanIndex, fifoSize);
        fifoReadPointers  = fifos.getArrayOfReadPointers();
        fifoWritePointers = fifos.getArrayOfWritePointers();
        fifos.clear();
        startThread (9);
        threadInitialised.wait();

        return {};
    }

    void close() override
    {
        stop();
        stopThread (10000);
        fifos.clear();
        active = false;

        for (auto& d : getDeviceWrappers())
            d->close();
    }

    void restart (AudioIODeviceCallback* cb)
    {
        const ScopedLock sl (closeLock);

        close();

        auto newSampleRate = sampleRateRequested;
        auto newBufferSize = bufferSizeRequested;

        for (auto& d : getDeviceWrappers())
        {
            auto deviceSampleRate = d->getCurrentSampleRate();

            if (deviceSampleRate != sampleRateRequested)
            {
                if (! getAvailableSampleRates().contains (deviceSampleRate))
                    return;

                for (auto& d2 : getDeviceWrappers())
                    if (&d2 != &d)
                        d2->setCurrentSampleRate (deviceSampleRate);

                newSampleRate = deviceSampleRate;
                break;
            }
        }

        for (auto& d : getDeviceWrappers())
        {
            auto deviceBufferSize = d->getCurrentBufferSizeSamples();

            if (deviceBufferSize != bufferSizeRequested)
            {
                if (! getAvailableBufferSizes().contains (deviceBufferSize))
                    return;

                newBufferSize = deviceBufferSize;
                break;
            }
        }

        open (inputChannelsRequested, outputChannelsRequested,
              newSampleRate, newBufferSize);

        start (cb);
    }

    void restartAsync() override
    {
        {
            const ScopedLock sl (closeLock);

            if (active)
            {
                if (callback != nullptr)
                    previousCallback = callback;

                close();
            }
        }

        startTimer (100);
    }

    int getOutputLatencyInSamples() override
    {
        int lat = 0;

        for (auto& d : getDeviceWrappers())
            lat = jmax (lat, d->device->getOutputLatencyInSamples());

        return lat + currentBufferSize * 2;
    }

    int getInputLatencyInSamples() override
    {
        int lat = 0;

        for (auto& d : getDeviceWrappers())
            lat = jmax (lat, d->device->getInputLatencyInSamples());

        return lat + currentBufferSize * 2;
    }

    void start (AudioIODeviceCallback* newCallback) override
    {
        const auto shouldStart = [&]
        {
            const ScopedLock sl (callbackLock);
            return callback != newCallback;
        }();

        if (shouldStart)
        {
            stop();
            fifos.clear();

            {
                ScopedErrorForwarder forwarder (*this, newCallback);

                for (auto& d : getDeviceWrappers())
                    d->start();

                if (! forwarder.encounteredError() && newCallback != nullptr)
                    newCallback->audioDeviceAboutToStart (this);
                else if (lastError.isEmpty())
                    lastError = TRANS("Failed to initialise all requested devices.");
            }

            const ScopedLock sl (callbackLock);
            previousCallback = callback = newCallback;
        }
    }

    void stop() override    { shutdown ({}); }

    String getLastError() override
    {
        return lastError;
    }

private:
    WeakReference<CoreAudioIODeviceType> owner;
    CriticalSection callbackLock;
    AudioIODeviceCallback* callback = nullptr;
    AudioIODeviceCallback* previousCallback = nullptr;
    double currentSampleRate = 0;
    int currentBufferSize = 0;
    bool active = false;
    String lastError;
    AudioBuffer<float> fifos;
    const float* const* fifoReadPointers = nullptr;
    float* const* fifoWritePointers = nullptr;
    WaitableEvent threadInitialised;
    CriticalSection closeLock;

    BigInteger inputChannelsRequested, outputChannelsRequested;
    double sampleRateRequested = 44100;
    int bufferSizeRequested = 512;

    void run() override
    {
        auto numSamples = currentBufferSize;

        AudioBuffer<float> buffer (fifos.getNumChannels(), numSamples);
        buffer.clear();

        Array<const float*> inputChans;
        Array<float*> outputChans;

        for (auto& d : getDeviceWrappers())
        {
            for (int j = 0; j < d->numInputChans; ++j)   inputChans.add  (buffer.getReadPointer  (d->inputIndex  + j));
            for (int j = 0; j < d->numOutputChans; ++j)  outputChans.add (buffer.getWritePointer (d->outputIndex + j));
        }

        auto numInputChans  = inputChans.size();
        auto numOutputChans = outputChans.size();

        inputChans.add (nullptr);
        outputChans.add (nullptr);

        auto blockSizeMs = jmax (1, (int) (1000 * numSamples / currentSampleRate));

        jassert (numInputChans + numOutputChans == buffer.getNumChannels());

        threadInitialised.signal();

        while (! threadShouldExit())
        {
            readInput (buffer, numSamples, blockSizeMs);

            bool didCallback = true;

            {
                const ScopedLock sl (callbackLock);

                if (callback != nullptr)
                    callback->audioDeviceIOCallbackWithContext ((const float**) inputChans.getRawDataPointer(),
                                                                numInputChans,
                                                                outputChans.getRawDataPointer(),
                                                                numOutputChans,
                                                                numSamples,
                                                                {}); // Can't predict when the next output callback will happen
                else
                    didCallback = false;
            }

            if (didCallback)
            {
                pushOutputData (buffer, numSamples, blockSizeMs);
            }
            else
            {
                for (int i = 0; i < numOutputChans; ++i)
                    FloatVectorOperations::clear (outputChans[i], numSamples);

                reset();
            }
        }
    }

    void timerCallback() override
    {
        stopTimer();

        restart (previousCallback);
    }

    void shutdown (const String& error)
    {
        AudioIODeviceCallback* lastCallback = nullptr;

        {
            const ScopedLock sl (callbackLock);
            std::swap (callback, lastCallback);
        }

        for (auto& d : getDeviceWrappers())
            d->device->stopInternal();

        if (lastCallback != nullptr)
        {
            if (error.isNotEmpty())
                lastCallback->audioDeviceError (error);
            else
                lastCallback->audioDeviceStopped();
        }
    }

    void reset()
    {
        for (auto& d : getDeviceWrappers())
            d->reset();
    }

    void underrun()
    {
    }

    void readInput (AudioBuffer<float>& buffer, const int numSamples, const int blockSizeMs)
    {
        for (auto& d : getDeviceWrappers())
            d->done = (d->numInputChans == 0 || d->isWaitingForInput);

        float totalWaitTimeMs = blockSizeMs * 5.0f;
        constexpr int numReadAttempts = 6;
        auto sumPower2s = [] (int maxPower) { return (1 << (maxPower + 1)) - 1; };
        float waitTime = totalWaitTimeMs / (float) sumPower2s (numReadAttempts - 2);

        for (int numReadAttemptsRemaining = numReadAttempts;;)
        {
            bool anySamplesRemaining = false;

            for (auto& d : getDeviceWrappers())
            {
                if (! d->done)
                {
                    if (d->isInputReady (numSamples))
                    {
                        d->readInput (buffer, numSamples);
                        d->done = true;
                    }
                    else
                    {
                        anySamplesRemaining = true;
                    }
                }
            }

            if (! anySamplesRemaining)
                return;

            if (--numReadAttemptsRemaining == 0)
                break;

            wait (jmax (1, roundToInt (waitTime)));
            waitTime *= 2.0f;
        }

        for (auto& d : getDeviceWrappers())
            if (! d->done)
                for (int i = 0; i < d->numInputChans; ++i)
                    buffer.clear (d->inputIndex + i, 0, numSamples);
    }

    void pushOutputData (AudioBuffer<float>& buffer, const int numSamples, const int blockSizeMs)
    {
        for (auto& d : getDeviceWrappers())
            d->done = (d->numOutputChans == 0);

        for (int tries = 5;;)
        {
            bool anyRemaining = false;

            for (auto& d : getDeviceWrappers())
            {
                if (! d->done)
                {
                    if (d->isOutputReady (numSamples))
                    {
                        d->pushOutputData (buffer, numSamples);
                        d->done = true;
                    }
                    else
                    {
                        anyRemaining = true;
                    }
                }
            }

            if ((! anyRemaining) || --tries == 0)
                return;

            wait (blockSizeMs);
        }
    }

    void handleAudioDeviceAboutToStart (AudioIODevice* device)
    {
        const ScopedLock sl (callbackLock);

        auto newSampleRate = device->getCurrentSampleRate();
        auto commonRates = getAvailableSampleRates();

        if (! commonRates.contains (newSampleRate))
        {
            commonRates.sort();

            if (newSampleRate < commonRates.getFirst() || newSampleRate > commonRates.getLast())
            {
                newSampleRate = jlimit (commonRates.getFirst(), commonRates.getLast(), newSampleRate);
            }
            else
            {
                for (auto it = commonRates.begin(); it < commonRates.end() - 1; ++it)
                {
                    if (it[0] < newSampleRate && it[1] > newSampleRate)
                    {
                        newSampleRate = newSampleRate - it[0] < it[1] - newSampleRate ? it[0] : it[1];
                        break;
                    }
                }
            }
        }

        currentSampleRate = newSampleRate;
        bool anySampleRateChanges = false;

        for (auto& d : getDeviceWrappers())
        {
            if (d->getCurrentSampleRate() != currentSampleRate)
            {
                d->setCurrentSampleRate (currentSampleRate);
                anySampleRateChanges = true;
            }
        }

        if (anySampleRateChanges && owner != nullptr)
            owner->audioDeviceListChanged();

        if (callback != nullptr)
            callback->audioDeviceAboutToStart (device);
    }

    void handleAudioDeviceStopped()                            { shutdown ({}); }
    void handleAudioDeviceError (const String& errorMessage)   { shutdown (errorMessage.isNotEmpty() ? errorMessage : String ("unknown")); }

    //==============================================================================
    struct DeviceWrapper  : public AudioIODeviceCallback
    {
        DeviceWrapper (AudioIODeviceCombiner& cd, std::unique_ptr<CoreAudioIODevice> d, bool shouldBeInput)
            : owner (cd), device (std::move (d)),
              input (shouldBeInput)
        {
            device->setAsyncRestarter (&owner);
        }

        ~DeviceWrapper() override
        {
            close();
        }

        String open (const BigInteger& inputChannels, const BigInteger& outputChannels,
                     double sampleRate, int bufferSize, int channelIndex, int fifoSize)
        {
            inputFifo.setTotalSize (fifoSize);
            outputFifo.setTotalSize (fifoSize);
            inputFifo.reset();
            outputFifo.reset();

            auto err = device->open (input ? inputChannels : BigInteger(),
                                     input ? BigInteger() : outputChannels,
                                     sampleRate, bufferSize);

            numInputChans  = input ? device->getActiveInputChannels().countNumberOfSetBits() : 0;
            numOutputChans = input ? 0 : device->getActiveOutputChannels().countNumberOfSetBits();

            isWaitingForInput = numInputChans > 0;

            inputIndex = channelIndex;
            outputIndex = channelIndex + numInputChans;

            return err;
        }

        void close()
        {
            device->close();
        }

        void start()
        {
            reset();
            device->start (this);
        }

        void reset()
        {
            inputFifo.reset();
            outputFifo.reset();
        }

        StringArray getOutputChannelNames() const  { return input ? StringArray() : device->getOutputChannelNames(); }
        StringArray getInputChannelNames()  const  { return input ? device->getInputChannelNames() : StringArray(); }

        bool isInputReady (int numSamples) const noexcept
        {
            return numInputChans == 0 || inputFifo.getNumReady() >= numSamples;
        }

        void readInput (AudioBuffer<float>& destBuffer, int numSamples)
        {
            if (numInputChans == 0)
                return;

            int start1, size1, start2, size2;
            inputFifo.prepareToRead (numSamples, start1, size1, start2, size2);

            for (int i = 0; i < numInputChans; ++i)
            {
                auto index = inputIndex + i;
                auto dest = destBuffer.getWritePointer (index);
                auto src = owner.fifoReadPointers[index];

                if (size1 > 0)  FloatVectorOperations::copy (dest,         src + start1, size1);
                if (size2 > 0)  FloatVectorOperations::copy (dest + size1, src + start2, size2);
            }

            inputFifo.finishedRead (size1 + size2);
        }

        bool isOutputReady (int numSamples) const noexcept
        {
            return numOutputChans == 0 || outputFifo.getFreeSpace() >= numSamples;
        }

        void pushOutputData (AudioBuffer<float>& srcBuffer, int numSamples)
        {
            if (numOutputChans == 0)
                return;

            int start1, size1, start2, size2;
            outputFifo.prepareToWrite (numSamples, start1, size1, start2, size2);

            for (int i = 0; i < numOutputChans; ++i)
            {
                auto index = outputIndex + i;
                auto dest = owner.fifoWritePointers[index];
                auto src = srcBuffer.getReadPointer (index);

                if (size1 > 0)  FloatVectorOperations::copy (dest + start1, src,         size1);
                if (size2 > 0)  FloatVectorOperations::copy (dest + start2, src + size1, size2);
            }

            outputFifo.finishedWrite (size1 + size2);
        }

        void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                               int numInputChannels,
                                               float* const* outputChannelData,
                                               int numOutputChannels,
                                               int numSamples,
                                               const AudioIODeviceCallbackContext&) override
        {
            if (numInputChannels > 0)
            {
                isWaitingForInput = false;

                int start1, size1, start2, size2;
                inputFifo.prepareToWrite (numSamples, start1, size1, start2, size2);

                if (size1 + size2 < numSamples)
                {
                    inputFifo.reset();
                    inputFifo.prepareToWrite (numSamples, start1, size1, start2, size2);
                }

                for (int i = 0; i < numInputChannels; ++i)
                {
                    auto dest = owner.fifoWritePointers[inputIndex + i];
                    auto src = inputChannelData[i];

                    if (size1 > 0)  FloatVectorOperations::copy (dest + start1, src,         size1);
                    if (size2 > 0)  FloatVectorOperations::copy (dest + start2, src + size1, size2);
                }

                auto totalSize = size1 + size2;
                inputFifo.finishedWrite (totalSize);

                if (numSamples > totalSize)
                {
                    auto samplesRemaining = numSamples - totalSize;

                    for (int i = 0; i < numInputChans; ++i)
                        FloatVectorOperations::clear (owner.fifoWritePointers[inputIndex + i] + totalSize, samplesRemaining);

                    owner.underrun();
                }
            }

            if (numOutputChannels > 0)
            {
                int start1, size1, start2, size2;
                outputFifo.prepareToRead (numSamples, start1, size1, start2, size2);

                if (size1 + size2 < numSamples)
                {
                    Thread::sleep (1);
                    outputFifo.prepareToRead (numSamples, start1, size1, start2, size2);
                }

                for (int i = 0; i < numOutputChannels; ++i)
                {
                    auto dest = outputChannelData[i];
                    auto src = owner.fifoReadPointers[outputIndex + i];

                    if (size1 > 0)  FloatVectorOperations::copy (dest,         src + start1, size1);
                    if (size2 > 0)  FloatVectorOperations::copy (dest + size1, src + start2, size2);
                }

                auto totalSize = size1 + size2;
                outputFifo.finishedRead (totalSize);

                if (numSamples > totalSize)
                {
                    auto samplesRemaining = numSamples - totalSize;

                    for (int i = 0; i < numOutputChannels; ++i)
                        FloatVectorOperations::clear (outputChannelData[i] + totalSize, samplesRemaining);

                    owner.underrun();
                }
            }

            owner.notify();
        }

        double getCurrentSampleRate()                        { return device->getCurrentSampleRate(); }
        bool   setCurrentSampleRate (double newSampleRate)   { return device->setCurrentSampleRate (newSampleRate); }
        int  getCurrentBufferSizeSamples()                   { return device->getCurrentBufferSizeSamples(); }

        void audioDeviceAboutToStart (AudioIODevice* d) override      { owner.handleAudioDeviceAboutToStart (d); }
        void audioDeviceStopped() override                            { owner.handleAudioDeviceStopped(); }
        void audioDeviceError (const String& errorMessage) override   { owner.handleAudioDeviceError (errorMessage); }

        StringArray getChannelNames()                             const { return input ? device->getInputChannelNames()     : device->getOutputChannelNames(); }
        BigInteger getActiveChannels()                            const { return input ? device->getActiveInputChannels()   : device->getActiveOutputChannels(); }
        int getLatencyInSamples()                                 const { return input ? device->getInputLatencyInSamples() : device->getOutputLatencyInSamples(); }
        int getIndexOfDevice (bool asInput)                       const { return device->getIndexOfDevice (asInput); }
        double getCurrentSampleRate()                             const { return device->getCurrentSampleRate(); }
        int getCurrentBufferSizeSamples()                         const { return device->getCurrentBufferSizeSamples(); }
        Array<double> getAvailableSampleRates()                   const { return device->getAvailableSampleRates(); }
        Array<int> getAvailableBufferSizes()                      const { return device->getAvailableBufferSizes(); }
        int getCurrentBitDepth()                                  const { return device->getCurrentBitDepth(); }
        int getDefaultBufferSize()                                const { return device->getDefaultBufferSize(); }

        //==============================================================================
        AudioIODeviceCombiner& owner;
        std::unique_ptr<CoreAudioIODevice> device;
        int inputIndex = 0, numInputChans = 0, outputIndex = 0, numOutputChans = 0;
        bool input;
        std::atomic<bool> isWaitingForInput { false };
        AbstractFifo inputFifo { 32 }, outputFifo { 32 };
        bool done = false;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviceWrapper)
    };

    /* If the current AudioIODeviceCombiner::callback is nullptr, it sets itself as the callback
       and forwards error related callbacks to the provided callback
    */
    class ScopedErrorForwarder  : public AudioIODeviceCallback
    {
    public:
        ScopedErrorForwarder (AudioIODeviceCombiner& ownerIn, AudioIODeviceCallback* cb)
            : owner (ownerIn),
              target (cb)
        {
            const ScopedLock sl (owner.callbackLock);

            if (owner.callback == nullptr)
                owner.callback = this;
        }

        ~ScopedErrorForwarder() override
        {
            const ScopedLock sl (owner.callbackLock);

            if (owner.callback == this)
                owner.callback = nullptr;
        }

        // We only want to be notified about error conditions when the owner's callback is nullptr.
        // This class shouldn't be relied on for forwarding this call.
        void audioDeviceAboutToStart (AudioIODevice*) override {}

        void audioDeviceStopped() override
        {
            if (target != nullptr)
                target->audioDeviceStopped();

            error = true;
        }

        void audioDeviceError (const String& errorMessage) override
        {
            owner.lastError = errorMessage;

            if (target != nullptr)
                target->audioDeviceError (errorMessage);

            error = true;
        }

        bool encounteredError() const { return error; }

    private:
        AudioIODeviceCombiner& owner;
        AudioIODeviceCallback* target;
        bool error = false;
    };

    DeviceWrapper inputWrapper, outputWrapper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioIODeviceCombiner)
};


//==============================================================================
class CoreAudioIODeviceType  : public AudioIODeviceType,
                               private AsyncUpdater
{
public:
    CoreAudioIODeviceType()  : AudioIODeviceType ("CoreAudio")
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectAddPropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, this);
    }

    ~CoreAudioIODeviceType() override
    {
        cancelPendingUpdate();

        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementWildcard;

        AudioObjectRemovePropertyListener (kAudioObjectSystemObject, &pa, hardwareListenerProc, this);
    }

    //==============================================================================
    void scanForDevices() override
    {
        hasScanned = true;

        inputDeviceNames.clear();
        outputDeviceNames.clear();
        inputIds.clear();
        outputIds.clear();

        auto audioDevices = audioObjectGetProperties<AudioDeviceID> (kAudioObjectSystemObject, { kAudioHardwarePropertyDevices,
                                                                                                 kAudioObjectPropertyScopeWildcard,
                                                                                                 juceAudioObjectPropertyElementMain });

        for (auto audioDevice : audioDevices)
        {
            if (auto name = audioObjectGetProperties<char> (audioDevice, { kAudioDevicePropertyDeviceName,
                                                                           kAudioObjectPropertyScopeWildcard,
                                                                           juceAudioObjectPropertyElementMain }); ! name.empty())
            {
                auto nameString = String::fromUTF8 (name.data(), (int) strlen (name.data()));
                auto numIns  = getNumChannels (audioDevice, true);
                auto numOuts = getNumChannels (audioDevice, false);

                if (numIns > 0)
                {
                    inputDeviceNames.add (nameString);
                    inputIds.add (audioDevice);
                }

                if (numOuts > 0)
                {
                    outputDeviceNames.add (nameString);
                    outputIds.add (audioDevice);
                }
            }
        }

        inputDeviceNames.appendNumbersToDuplicates (false, true);
        outputDeviceNames.appendNumbersToDuplicates (false, true);
    }

    StringArray getDeviceNames (bool wantInputNames) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        return wantInputNames ? inputDeviceNames
                              : outputDeviceNames;
    }

    int getDefaultDeviceIndex (bool forInput) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        // if they're asking for any input channels at all, use the default input, so we
        // get the built-in mic rather than the built-in output with no inputs..

        AudioObjectPropertyAddress pa;
        auto selector = forInput ? kAudioHardwarePropertyDefaultInputDevice
                                 : kAudioHardwarePropertyDefaultOutputDevice;
        pa.mScope    = kAudioObjectPropertyScopeWildcard;
        pa.mElement  = juceAudioObjectPropertyElementMain;

        if (auto deviceID = audioObjectGetProperty<AudioDeviceID> (kAudioObjectSystemObject, { selector,
                                                                                               kAudioObjectPropertyScopeWildcard,
                                                                                               juceAudioObjectPropertyElementMain }))
        {
            auto& ids = forInput ? inputIds : outputIds;

            if (auto it = std::find (ids.begin(), ids.end(), deviceID); it != ids.end())
                return static_cast<int> (std::distance (ids.begin(), it));
        }

        return 0;
    }

    int getIndexOfDevice (AudioIODevice* device, bool asInput) const override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        if (auto* d = dynamic_cast<CoreAudioIODevice*> (device))
            return d->getIndexOfDevice (asInput);

        if (auto* d = dynamic_cast<AudioIODeviceCombiner*> (device))
            for (auto* dev : d->getDeviceWrappers())
                if (const auto index = dev->getIndexOfDevice (asInput); index >= 0)
                    return index;

        return -1;
    }

    bool hasSeparateInputsAndOutputs() const override    { return true; }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& inputDeviceName) override
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        auto inputIndex  = inputDeviceNames.indexOf (inputDeviceName);
        auto outputIndex = outputDeviceNames.indexOf (outputDeviceName);

        auto inputDeviceID  = inputIds[inputIndex];
        auto outputDeviceID = outputIds[outputIndex];

        if (inputDeviceID == 0 && outputDeviceID == 0)
            return nullptr;

        auto combinedName = outputDeviceName.isEmpty() ? inputDeviceName
                                                       : outputDeviceName;

        if (inputDeviceID == outputDeviceID)
            return std::make_unique<CoreAudioIODevice> (this, combinedName, inputDeviceID, inputIndex, outputDeviceID, outputIndex).release();

        auto in = inputDeviceID != 0 ? std::make_unique<CoreAudioIODevice> (this, inputDeviceName, inputDeviceID, inputIndex, 0, -1)
                                     : nullptr;

        auto out = outputDeviceID != 0 ? std::make_unique<CoreAudioIODevice> (this, outputDeviceName, 0, -1, outputDeviceID, outputIndex)
                                       : nullptr;

        if (in  == nullptr)  return out.release();
        if (out == nullptr)  return in.release();

        auto combo = std::make_unique<AudioIODeviceCombiner> (combinedName, this, std::move (in), std::move (out));
        return combo.release();
    }

    void audioDeviceListChanged()
    {
        scanForDevices();
        callDeviceChangeListeners();
    }

    //==============================================================================
private:
    StringArray inputDeviceNames, outputDeviceNames;
    Array<AudioDeviceID> inputIds, outputIds;

    bool hasScanned = false;

    void handleAsyncUpdate() override
    {
        audioDeviceListChanged();
    }

    static int getNumChannels (AudioDeviceID deviceID, bool input)
    {
        int total = 0;

        if (auto bufList = audioObjectGetProperty<AudioBufferList> (deviceID, { kAudioDevicePropertyStreamConfiguration,
                                                                                CoreAudioInternal::getScope (input),
                                                                                juceAudioObjectPropertyElementMain }))
        {
            auto numStreams = (int) bufList->mNumberBuffers;

            for (int i = 0; i < numStreams; ++i)
                total += bufList->mBuffers[i].mNumberChannels;
        }

        return total;
    }

    static OSStatus hardwareListenerProc (AudioDeviceID, UInt32, const AudioObjectPropertyAddress*, void* clientData)
    {
        static_cast<CoreAudioIODeviceType*> (clientData)->triggerAsyncUpdate();
        return noErr;
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE (CoreAudioIODeviceType)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioIODeviceType)
};

};

#undef JUCE_COREAUDIOLOG

} // namespace juce
