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
class ManagedAudioBufferList final : public AudioBufferList
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
    static Ref create (std::size_t numBuffers)
    {
        static_assert (alignof (ManagedAudioBufferList) <= alignof (std::max_align_t));

        if (std::unique_ptr<std::byte[]> storage { new std::byte[storageSizeForNumBuffers (numBuffers)] })
            return Ref { new (storage.release()) ManagedAudioBufferList (numBuffers) };

        return nullptr;
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
    static_assert (std::is_standard_layout_v<T>);
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
class CoreAudioInternal final : private Timer,
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
        const auto tempBufSize = (size_t) bufferSize + 4;

        auto streams = getStreams();
        const auto total = std::accumulate (streams.begin(), streams.end(), (size_t) 0,
                                            [] (auto n, const auto& s) { return n + (s != nullptr ? s->channels : 0); });
        audioBuffer.clear();
        audioBuffer.resize (total * tempBufSize);
        audioBufferLengthInSamples = (size_t) bufferSize;

        size_t channels = 0;
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

        auto newInput  = rawToUniquePtr (inStream  != nullptr ? new Stream (true,  *this, activeIns)  : nullptr);
        auto newOutput = rawToUniquePtr (outStream != nullptr ? new Stream (false, *this, activeOuts) : nullptr);

        auto newBitDepth = jmax (getBitDepth (newInput), getBitDepth (newOutput));

       #if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE
        audioWorkgroup = [this]() -> AudioWorkgroup
        {
            AudioObjectPropertyAddress pa;
            pa.mSelector = kAudioDevicePropertyIOThreadOSWorkgroup;
            pa.mScope    = kAudioObjectPropertyScopeWildcard;
            pa.mElement  = juceAudioObjectPropertyElementMain;

            if (auto* workgroup = audioObjectGetProperty<os_workgroup_t> (deviceID, pa).value_or (nullptr))
            {
                ScopeGuard scope { [&] { os_release (workgroup); } };
                return makeRealAudioWorkgroup (workgroup);
            }

            return {};
        }();
       #endif

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
        callbacksAllowed = false;
        const ScopeGuard scope { [&] { callbacksAllowed = true; } };

        stopTimer();

        stop (false);

        if (! setNominalSampleRate (newSampleRate))
        {
            updateDetailsFromDevice (ins, outs);
            return "Couldn't change sample rate";
        }

        if (! audioObjectSetProperty (deviceID, { kAudioDevicePropertyBufferFrameSize,
                                                  kAudioObjectPropertyScopeGlobal,
                                                  juceAudioObjectPropertyElementMain },
                                      static_cast<UInt32> (bufferSizeSamples), err2log()))
        {
            updateDetailsFromDevice (ins, outs);
            return "Couldn't change buffer size";
        }

        // Annoyingly, after changing the rate and buffer size, some devices fail to
        // correctly report their new settings until some random time in the future, so
        // after calling updateDetailsFromDevice, we need to manually bodge these values
        // to make sure we're using the correct numbers..
        updateDetailsFromDevice (ins, outs);
        sampleRate = newSampleRate;
        bufferSize = bufferSizeSamples;

        if (sampleRates.size() == 0)
            return "Device has no available sample-rates";

        if (bufferSizes.size() == 0)
            return "Device has no available buffer-sizes";

        return {};
    }

    bool start (AudioIODeviceCallback* callbackToNotify)
    {
        const ScopedLock sl (callbackLock);

        if (callback == nullptr && callbackToNotify != nullptr)
        {
            callback = callbackToNotify;
            callback->audioDeviceAboutToStart (&owner);
        }

        for (auto* stream : getStreams())
            if (stream != nullptr)
                stream->previousSampleTime = invalidSampleTime;

        owner.hadDiscontinuity = false;

        if (scopedProcID.get() == nullptr && deviceID != 0)
        {
            scopedProcID = [&self = *this,
                            &lock = callbackLock,
                            nextProcID = ScopedAudioDeviceIOProcID { *this, deviceID, audioIOProc },
                            dID = deviceID]() mutable -> ScopedAudioDeviceIOProcID
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

                    if (self.OK (AudioDeviceStart (dID, procID)))
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

    void audioCallback (const AudioTimeStamp* inputTimestamp,
                        const AudioTimeStamp* outputTimestamp,
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

        const auto actualBufferSizeSamples = std::invoke ([&]
        {
            size_t result = 0;

            for (auto [streamPtr, data] : { std::tuple (&inStream,  static_cast<const AudioBufferList*> (inInputData)),
                                            std::tuple (&outStream, static_cast<const AudioBufferList*> (outOutputData)) })
            {
                auto& stream = *streamPtr;
                const auto numChannels = (int) getChannels (stream);

                for (auto i = 0; i < numChannels; ++i)
                {
                    const auto info = stream->channelInfo.getReference (i);
                    const auto stride = (size_t) info.dataStrideSamples;

                    if (stride == 0)
                        continue;

                    const auto bufSizeSamples = data->mBuffers[info.streamNum].mDataByteSize / (sizeof (float) * stride);

                    // Not all stream buffer sizes are equal!
                    jassert (result == 0 || result == bufSizeSamples);

                    result = bufSizeSamples;
                }
            }

            return result;
        });

        if (callback != nullptr)
        {
            for (auto* stream : getStreams())
            {
                if (stream == nullptr)
                    continue;

                const auto timeStamp = stream == inStream.get() ? inputTimestamp : outputTimestamp;
                owner.hadDiscontinuity |= stream->checkTimestampsForDiscontinuity (timeStamp);
            }

            for (size_t offset = 0; offset < actualBufferSizeSamples;)
            {
                const auto numSamplesInChunk = jmin (actualBufferSizeSamples - offset, audioBufferLengthInSamples);
                processBufferChunk (offset, numSamplesInChunk, inputTimestamp, outputTimestamp, inInputData, outOutputData);
                offset += numSamplesInChunk;
            }
        }
        else
        {
            for (UInt32 i = 0; i < outOutputData->mNumberBuffers; ++i)
                zeromem (outOutputData->mBuffers[i].mData,
                         outOutputData->mBuffers[i].mDataByteSize);
        }

        for (auto* stream : getStreams())
            if (stream != nullptr)
                stream->previousSampleTime += static_cast<Float64> (actualBufferSizeSamples);
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
        Stream (bool isInput, CoreAudioInternal& parent, const BigInteger& activeRequested)
            : input (isInput),
              latency (getLatencyFromDevice (isInput, parent)),
              bitDepth (getBitDepthFromDevice (isInput, parent)),
              chanNames (getChannelNames (isInput, parent)),
              activeChans ([&activeRequested, clearFrom = chanNames.size()]
                           {
                               auto result = activeRequested;
                               result.setRange (clearFrom, result.getHighestBit() + 1 - clearFrom, false);
                               return result;
                           }()),
              channelInfo (getChannelInfos (isInput, parent, activeChans))
        {}

        size_t allocateTempBuffers (size_t tempBufSize, size_t channelCount, Span<float> buffer)
        {
            tempBuffers.clear();
            tempBuffers.resize (channels + 2);

            for (size_t i = 0; i < channels; ++i)
            {
                const auto offset = channelCount++ * tempBufSize;
                jassert (offset + tempBufSize <= buffer.size());
                tempBuffers[i] = buffer.data() + offset;
            }

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

        bool checkTimestampsForDiscontinuity (const AudioTimeStamp* timestamp) noexcept
        {
            if (channels > 0)
            {
                jassert (timestamp == nullptr || (((timestamp->mFlags & kAudioTimeStampSampleTimeValid) != 0)
                                               && ((timestamp->mFlags & kAudioTimeStampHostTimeValid)   != 0)));

                if (exactlyEqual (previousSampleTime, invalidSampleTime))
                    previousSampleTime = timestamp != nullptr ? timestamp->mSampleTime : 0.0;

                if (timestamp != nullptr && std::fabs (previousSampleTime - timestamp->mSampleTime) >= 1.0)
                {
                    previousSampleTime = timestamp->mSampleTime;
                    return true;
                }
            }

            return false;
        }

        //==============================================================================
        const bool input;
        const int latency;
        const int bitDepth;
        const StringArray chanNames;
        const BigInteger activeChans;
        const Array<CallbackDetailsForChannel> channelInfo;
        const size_t channels = (size_t) channelInfo.size();
        Float64 previousSampleTime;

        std::vector<float*> tempBuffers;

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
    static size_t       getChannels         (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::channels); }
    static int          getNumChannelNames  (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::chanNames).size(); }
    static String       getChannelNames     (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::chanNames).joinIntoString (" "); }
    static BigInteger   getActiveChannels   (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, &Stream::activeChans); }
    static float**      getTempBuffers      (const std::unique_ptr<Stream>& ptr) { return getWithDefault (ptr, [] (auto& s) { return s.tempBuffers.data(); }); }

    //==============================================================================
    static constexpr Float64 invalidSampleTime = std::numeric_limits<Float64>::max();

    CoreAudioIODevice& owner;
    int bitDepth = 32;
    std::atomic<int> xruns = 0;
    Array<double> sampleRates;
    Array<int> bufferSizes;
    AudioDeviceID deviceID;
    std::unique_ptr<Stream> inStream, outStream;

    AudioWorkgroup audioWorkgroup;

private:
    template <typename Iterator>
    struct StrideIterator
    {
        StrideIterator (Iterator iteratorIn, ptrdiff_t strideIn)
            : iterator (std::move (iteratorIn)), stride (strideIn) {}

        StrideIterator& operator++()
        {
            iterator += stride;
            return *this;
        }

        StrideIterator operator++ (int)
        {
            auto copy = *this;
            operator++();
            return copy;
        }

        // decltype (auto) here because the return types may be references
        decltype (auto) operator* () const { return *iterator; }

        bool operator== (const StrideIterator& other) const { return iterator == other.iterator; }
        bool operator!= (const StrideIterator& other) const { return iterator != other.iterator; }

        StrideIterator& operator+= (ptrdiff_t x)
        {
            iterator += stride * x;
            return *this;
        }

        StrideIterator operator+ (ptrdiff_t x) const
        {
            return StrideIterator { *this } += x;
        }

        Iterator iterator;
        ptrdiff_t stride;
    };

    void processBufferChunk (size_t sampleOffset,
                             size_t numSamplesInChunk,
                             const AudioTimeStamp* inputTimestamp,
                             const AudioTimeStamp* outputTimestamp,
                             const AudioBufferList* inInputData,
                             AudioBufferList* outOutputData)
    {
        // precondition
        jassert (callback != nullptr);

        const auto numInputChans  = (int) getChannels (inStream);
        const auto numOutputChans = (int) getChannels (outStream);

        // copy from input buffer to temporary buffer
        for (auto index = 0; index < numInputChans; ++index)
        {
            const auto info = inStream->channelInfo.getReference (index);
            const auto src = StrideIterator { ((const float*) inInputData->mBuffers[info.streamNum].mData) + info.dataOffsetSamples,
                                              info.dataStrideSamples }
                           + (ptrdiff_t) sampleOffset;

            if (src.stride == 0) // if this is zero, info is invalid
                continue;

            std::copy (src, src + (ptrdiff_t) numSamplesInChunk, inStream->tempBuffers[(size_t) index]);
        }

        // only pass a timestamp for the first chunk of each buffer
        const auto* timeStamp = std::invoke ([&]() -> const AudioTimeStamp*
        {
            if (sampleOffset != 0)
                return nullptr;

            return numOutputChans > 0 ? outputTimestamp : inputTimestamp;
        });

        const auto nanos = timeStamp != nullptr ? timeConversions.hostTimeToNanos (timeStamp->mHostTime) : 0;
        const AudioIODeviceCallbackContext context
        {
            timeStamp != nullptr ? &nanos : nullptr,
        };

        callback->audioDeviceIOCallbackWithContext (getTempBuffers (inStream),
                                                    numInputChans,
                                                    getTempBuffers (outStream),
                                                    numOutputChans,
                                                    (int) numSamplesInChunk,
                                                    context);

        // copy from temporary buffer to output buffer
        for (auto index = 0; index < numOutputChans; ++index)
        {
            const auto info = outStream->channelInfo.getReference (index);
            const auto dest = StrideIterator { ((float*) outOutputData->mBuffers[info.streamNum].mData) + info.dataOffsetSamples,
                                               info.dataStrideSamples }
                            + (ptrdiff_t) sampleOffset;

            if (dest.stride == 0) // if this is zero, info is invalid
                continue;

            const auto* src = outStream->tempBuffers[(size_t) index];
            std::copy (src, src + numSamplesInChunk, dest);
        }
    }

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
    std::vector<float> audioBuffer;
    size_t audioBufferLengthInSamples = 0;
    Atomic<int> callbacksAllowed { 1 };

    //==============================================================================
    void timerCallback() override
    {
        JUCE_COREAUDIOLOG ("Device changed");

        stopTimer();
        auto oldSampleRate = sampleRate;
        auto oldBufferSize = bufferSize;

        if (! updateDetailsFromDevice())
            owner.stopWithPendingCallback();
        else if (oldBufferSize != bufferSize || ! approximatelyEqual (oldSampleRate, sampleRate))
            owner.restart();
    }

    void handleAsyncUpdate() override
    {
        if (owner.deviceType != nullptr)
            owner.deviceType->audioDeviceListChanged();
    }

    static OSStatus audioIOProc (AudioDeviceID /*inDevice*/,
                                 [[maybe_unused]] const AudioTimeStamp* inNow,
                                 const AudioBufferList* inInputData,
                                 const AudioTimeStamp* inInputTime,
                                 AudioBufferList* outOutputData,
                                 const AudioTimeStamp* inOutputTime,
                                 void* device)
    {
        static_cast<CoreAudioInternal*> (device)->audioCallback (inInputTime, inOutputTime, inInputData, outOutputData);
        return noErr;
    }

    static OSStatus deviceListenerProc (AudioDeviceID /*inDevice*/,
                                        UInt32 numAddresses,
                                        const AudioObjectPropertyAddress* pa,
                                        void* inClientData)
    {
        auto& intern = *static_cast<CoreAudioInternal*> (inClientData);

        const auto xruns = std::count_if (pa, pa + numAddresses, [] (const AudioObjectPropertyAddress& x)
        {
            return x.mSelector == kAudioDeviceProcessorOverload;
        });

        intern.xruns += (int) xruns;

        const auto detailsChanged = std::any_of (pa, pa + numAddresses, [] (const AudioObjectPropertyAddress& x)
        {
            constexpr UInt32 selectors[]
            {
                kAudioDevicePropertyBufferSize,
                kAudioDevicePropertyBufferFrameSize,
                kAudioDevicePropertyNominalSampleRate,
                kAudioDevicePropertyStreamFormat,
                kAudioDevicePropertyDeviceIsAlive,
                kAudioStreamPropertyPhysicalFormat,
            };

            return std::find (std::begin (selectors), std::end (selectors), x.mSelector) != std::end (selectors);
        });

        const auto requestedRestart = std::any_of (pa, pa + numAddresses, [] (const AudioObjectPropertyAddress& x)
        {
            constexpr UInt32 selectors[]
            {
                kAudioDevicePropertyDeviceHasChanged,
                kAudioObjectPropertyOwnedObjects,
            };

            return std::find (std::begin (selectors), std::end (selectors), x.mSelector) != std::end (selectors);
        });

        if (detailsChanged)
            intern.deviceDetailsChanged();

        if (requestedRestart)
            intern.deviceRequestedRestart();

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
class CoreAudioIODevice final : public AudioIODevice,
                                private Timer
{
public:
    CoreAudioIODevice (CoreAudioIODeviceType* dt,
                       const String& deviceName,
                       AudioDeviceID inputDeviceId,
                       AudioDeviceID outputDeviceId)
        : AudioIODevice (deviceName, "CoreAudio"),
          deviceType (dt)
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

    int getIndexOfDevice (bool asInput) const           { return deviceType->getDeviceNames (asInput).indexOf (getName()); }

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
        const ScopedLock sl (startStopLock);

        if (internal->start (callback))
            pendingCallback = nullptr;
    }

    void stop() override
    {
        stopAndGetLastCallback();

        const ScopedLock sl (startStopLock);
        pendingCallback = nullptr;
    }

    void stopWithPendingCallback()
    {
        const ScopedLock sl (startStopLock);

        if (pendingCallback == nullptr)
            pendingCallback = stopAndGetLastCallback();
    }

    AudioWorkgroup getWorkgroup() const override
    {
        return internal->audioWorkgroup;
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

        stopWithPendingCallback();
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

    WeakReference<CoreAudioIODeviceType> deviceType;
    bool hadDiscontinuity;

private:
    std::unique_ptr<CoreAudioInternal> internal;
    bool isOpen_ = false;
    String lastError;
    //  When non-null, this indicates that the device has been stopped with the intent to restart
    //  using the same callback. That is, this should only be non-null when the device is stopped.
    AudioIODeviceCallback* pendingCallback = nullptr;
    AsyncRestarter* restarter = nullptr;
    BigInteger inputChannelsRequested, outputChannelsRequested;
    CriticalSection startStopLock;

    AudioIODeviceCallback* stopAndGetLastCallback() const
    {
        auto* lastCallback = internal->stop (true);

        if (lastCallback != nullptr)
            lastCallback->audioDeviceStopped();

        return lastCallback;
    }

    void timerCallback() override
    {
        stopTimer();

        stopWithPendingCallback();

        internal->updateDetailsFromDevice();

        open (inputChannelsRequested,
              outputChannelsRequested,
              getCurrentSampleRate(),
              getCurrentBufferSizeSamples());

        const ScopedLock sl { startStopLock };
        start (pendingCallback);
    }

    static OSStatus hardwareListenerProc (AudioDeviceID /*inDevice*/,
                                          UInt32 numAddresses,
                                          const AudioObjectPropertyAddress* pa,
                                          void* inClientData)
    {
        const auto detailsChanged = std::any_of (pa, pa + numAddresses, [] (const AudioObjectPropertyAddress& x)
        {
            return x.mSelector == kAudioHardwarePropertyDevices;
        });

        if (detailsChanged)
            static_cast<CoreAudioInternal*> (inClientData)->deviceDetailsChanged();

        return noErr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioIODevice)
};


//==============================================================================
class AudioIODeviceCombiner final : public AudioIODevice,
                                    private AsyncRestarter,
                                    private Timer
{
public:
    AudioIODeviceCombiner (const String& deviceName, CoreAudioIODeviceType* deviceType,
                           std::unique_ptr<CoreAudioIODevice>&& inputDevice,
                           std::unique_ptr<CoreAudioIODevice>&& outputDevice)
        : AudioIODevice (deviceName, "CoreAudio"),
          owner (deviceType),
          currentSampleRate (inputDevice->getCurrentSampleRate()),
          currentBufferSize (inputDevice->getCurrentBufferSizeSamples()),
          inputWrapper  (*this, std::move (inputDevice),  true),
          outputWrapper (*this, std::move (outputDevice), false)
    {
        if (getAvailableSampleRates().isEmpty())
            lastError = TRANS ("The input and output devices don't share a common sample rate!");
    }

    ~AudioIODeviceCombiner() override
    {
        close();
    }

    auto getDeviceWrappers()       { return std::array<      DeviceWrapper*, 2> { { &inputWrapper, &outputWrapper } }; }
    auto getDeviceWrappers() const { return std::array<const DeviceWrapper*, 2> { { &inputWrapper, &outputWrapper } }; }

    int getIndexOfDevice (bool asInput) const
    {
        return asInput ? inputWrapper.getIndexOfDevice (true)
                       : outputWrapper.getIndexOfDevice (false);
    }

    StringArray getOutputChannelNames() override        { return outputWrapper.getChannelNames(); }
    StringArray getInputChannelNames()  override        { return inputWrapper .getChannelNames(); }
    BigInteger getActiveOutputChannels() const override { return outputWrapper.getActiveChannels(); }
    BigInteger getActiveInputChannels() const override  { return inputWrapper .getActiveChannels(); }

    Array<double> getAvailableSampleRates() override
    {
        auto commonRates = inputWrapper.getAvailableSampleRates();
        commonRates.removeValuesNotIn (outputWrapper.getAvailableSampleRates());

        return commonRates;
    }

    Array<int> getAvailableBufferSizes() override
    {
        auto commonSizes = inputWrapper.getAvailableBufferSizes();
        commonSizes.removeValuesNotIn (outputWrapper.getAvailableBufferSizes());

        return commonSizes;
    }

    bool isOpen() override                          { return active; }
    bool isPlaying() override                       { return callback != nullptr; }
    double getCurrentSampleRate() override          { return currentSampleRate; }
    int getCurrentBufferSizeSamples() override      { return currentBufferSize; }

    int getCurrentBitDepth() override
    {
        return jmin (32, inputWrapper.getCurrentBitDepth(), outputWrapper.getCurrentBitDepth());
    }

    int getDefaultBufferSize() override
    {
        return jmax (0, inputWrapper.getDefaultBufferSize(), outputWrapper.getDefaultBufferSize());
    }

    AudioWorkgroup getWorkgroup() const override
    {
        return inputWrapper.getWorkgroup();
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
        targetLatency = bufferSize;

        for (auto& d : getDeviceWrappers())
        {
            auto err = d->open (  d->isInput() ? inputChannels  : BigInteger(),
                                ! d->isInput() ? outputChannels : BigInteger(),
                                sampleRate, bufferSize);

            if (err.isNotEmpty())
            {
                close();
                lastError = err;
                return err;
            }

            targetLatency += d->getLatencyInSamples();
        }

        const auto numOuts = outputWrapper.getChannelNames().size();

        fifo.setSize (numOuts, targetLatency + (bufferSize * 2));
        scratchBuffer.setSize (numOuts, bufferSize);

        return {};
    }

    void close() override
    {
        stop();
        fifo.clear();
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

            if (! approximatelyEqual (deviceSampleRate, sampleRateRequested))
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

        open (inputChannelsRequested, outputChannelsRequested, newSampleRate, newBufferSize);

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
        return targetLatency - getInputLatencyInSamples();
    }

    int getInputLatencyInSamples() override
    {
        return inputWrapper.getLatencyInSamples();
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
            fifo.clear();
            reset();

            {
                ScopedErrorForwarder forwarder (*this, newCallback);

                for (auto& d : getDeviceWrappers())
                    d->start (d);

                if (! forwarder.encounteredError() && newCallback != nullptr)
                    newCallback->audioDeviceAboutToStart (this);
                else if (lastError.isEmpty())
                    lastError = TRANS ("Failed to initialise all requested devices.");
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

    int getXRunCount() const noexcept override
    {
        return xruns.load();
    }

private:
    static constexpr auto invalidSampleTime = std::numeric_limits<std::uint64_t>::max();

    WeakReference<CoreAudioIODeviceType> owner;
    CriticalSection callbackLock;
    AudioIODeviceCallback* callback = nullptr;
    AudioIODeviceCallback* previousCallback = nullptr;
    double currentSampleRate = 0;
    int currentBufferSize = 0;
    bool active = false;
    String lastError;
    AudioSampleBuffer fifo, scratchBuffer;
    CriticalSection closeLock;
    int targetLatency = 0;
    std::atomic<int> xruns { -1 };
    std::atomic<uint64_t> lastValidReadPosition { invalidSampleTime };

    BigInteger inputChannelsRequested, outputChannelsRequested;
    double sampleRateRequested = 44100;
    int bufferSizeRequested = 512;

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
            d->stop();

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
        xruns.store (0);
        fifo.clear();
        scratchBuffer.clear();

        for (auto& d : getDeviceWrappers())
            d->reset();
    }

    // AbstractFifo cannot be used here for two reasons:
    // 1) We use absolute timestamps as the fifo's read/write positions. This not only makes the code
    //    more readable (especially when checking for underruns/overflows) but also simplifies the
    //    initial setup when actual latency is not known yet until both callbacks have fired.
    // 2) AbstractFifo doesn't have the necessary mechanics to recover from underrun/overflow conditions
    //    in a lock-free and data-race free way. It's great if you don't care (i.e. overwrite and/or
    //    read stale data) or can abort the operation entirely, but this is not the case here. We
    //    need bespoke underrun/overflow handling here which fits this use-case.
    template <typename Callback>
    void accessFifo (const uint64_t startPos, const int numChannels, const int numItems, Callback&& operateOnRange)
    {
        const auto fifoSize = fifo.getNumSamples();
        auto fifoPos = static_cast<int> (startPos % static_cast<std::uint64_t> (fifoSize));

        for (int pos = 0; pos < numItems;)
        {
            const auto max = std::min (numItems - pos, fifoSize - fifoPos);

            struct Args { int fifoPos, inputPos, nItems, channel; };

            for (auto ch = 0; ch < numChannels; ++ch)
                operateOnRange (Args { fifoPos, pos, max, ch });

            fifoPos = (fifoPos + max) % fifoSize;
            pos += max;
        }
    }

    void inputAudioCallback (const float* const* channels, int numChannels, int n, const AudioIODeviceCallbackContext& context) noexcept
    {
        auto& writePos = inputWrapper.sampleTime;

        {
            ScopedLock lock (callbackLock);

            if (callback != nullptr)
            {
                const auto numActiveOutputChannels = outputWrapper.getActiveChannels().countNumberOfSetBits();
                jassert (numActiveOutputChannels <= scratchBuffer.getNumChannels());

                callback->audioDeviceIOCallbackWithContext (channels,
                                                            numChannels,
                                                            scratchBuffer.getArrayOfWritePointers(),
                                                            numActiveOutputChannels,
                                                            n,
                                                            context);
            }
            else
            {
                scratchBuffer.clear();
            }
        }

        auto currentWritePos = writePos.load();
        const auto nextWritePos = currentWritePos + static_cast<std::uint64_t> (n);

        writePos.compare_exchange_strong (currentWritePos, nextWritePos);

        if (currentWritePos == invalidSampleTime)
            return;

        const auto readPos = outputWrapper.sampleTime.load();

        // check for fifo overflow
        if (readPos != invalidSampleTime)
        {
            // write will overlap previous read
            if (readPos > currentWritePos || (currentWritePos + static_cast<std::uint64_t> (n) - readPos) > static_cast<std::uint64_t> (fifo.getNumSamples()))
            {
                xrun();
                return;
            }
        }

        accessFifo (currentWritePos, scratchBuffer.getNumChannels(), n, [&] (const auto& args)
        {
            FloatVectorOperations::copy (fifo.getWritePointer (args.channel, args.fifoPos),
                                         scratchBuffer.getReadPointer (args.channel, args.inputPos),
                                         args.nItems);
        });

        {
            auto invalid = invalidSampleTime;
            lastValidReadPosition.compare_exchange_strong (invalid, nextWritePos);
        }
    }

    void outputAudioCallback (float* const* channels, int numChannels, int n) noexcept
    {
        auto& readPos = outputWrapper.sampleTime;
        auto currentReadPos = readPos.load();

        if (currentReadPos == invalidSampleTime)
            return;

        const auto writePos = inputWrapper.sampleTime.load();

        // check for fifo underrun
        if (writePos != invalidSampleTime)
        {
            if ((currentReadPos + static_cast<std::uint64_t> (n)) > writePos)
            {
                xrun();
                return;
            }
        }

        // If there was an xrun, we want to output zeros until we're sure that there's some valid
        // input for us to read.
        const auto longN = static_cast<uint64_t> (n);
        const auto nextReadPos = currentReadPos + longN;
        const auto validReadPos = lastValidReadPosition.load();
        const auto sanitisedValidReadPos = validReadPos != invalidSampleTime ? validReadPos : nextReadPos;
        const auto numZerosToWrite = sanitisedValidReadPos <= currentReadPos
                                   ? 0
                                   : jmin (longN, sanitisedValidReadPos - currentReadPos);

        for (auto i = 0; i < numChannels; ++i)
            std::fill (channels[i], channels[i] + numZerosToWrite, 0.0f);

        accessFifo (currentReadPos + numZerosToWrite, numChannels, static_cast<int> (longN - numZerosToWrite), [&] (const auto& args)
        {
            FloatVectorOperations::copy (channels[args.channel] + args.inputPos + numZerosToWrite,
                                         fifo.getReadPointer (args.channel, args.fifoPos),
                                         args.nItems);
        });

        // use compare exchange here as we need to avoid the case
        // where we overwrite readPos being equal to invalidSampleTime
        readPos.compare_exchange_strong (currentReadPos, nextReadPos);
    }

    void xrun() noexcept
    {
        for (auto& d : getDeviceWrappers())
            d->sampleTime.store (invalidSampleTime);

        ++xruns;
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
            if (! approximatelyEqual (d->getCurrentSampleRate(), currentSampleRate))
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
    struct DeviceWrapper final : public AudioIODeviceCallback
    {
        DeviceWrapper (AudioIODeviceCombiner& cd, std::unique_ptr<CoreAudioIODevice> d, bool shouldBeInput)
            : owner (cd),
              device (std::move (d)),
              input (shouldBeInput)
        {
            device->setAsyncRestarter (&owner);
        }

        ~DeviceWrapper() override
        {
            device->close();
        }

        void reset()
        {
            sampleTime.store (invalidSampleTime);
        }

        void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                               int numInputChannels,
                                               float* const* outputChannelData,
                                               int numOutputChannels,
                                               int numSamples,
                                               const AudioIODeviceCallbackContext& context) override
        {
            if (std::exchange (device->hadDiscontinuity, false))
                owner.xrun();

            updateSampleTimeFromContext (context);

            if (input)
                owner.inputAudioCallback (inputChannelData, numInputChannels, numSamples, context);
            else
                owner.outputAudioCallback (outputChannelData, numOutputChannels, numSamples);
        }

        void audioDeviceAboutToStart (AudioIODevice* d)        override { owner.handleAudioDeviceAboutToStart (d); }
        void audioDeviceStopped()                              override { owner.handleAudioDeviceStopped(); }
        void audioDeviceError (const String& errorMessage)     override { owner.handleAudioDeviceError (errorMessage); }

        bool setCurrentSampleRate (double newSampleRate)                { return device->setCurrentSampleRate (newSampleRate); }
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
        void start (AudioIODeviceCallback* callbackToNotify)      const { return device->start (callbackToNotify); }
        void stop()                                               const { return device->stop(); }
        void close()                                              const { return device->close(); }
        AudioWorkgroup getWorkgroup()                             const { return device->getWorkgroup(); }

        String open (const BigInteger& inputChannels, const BigInteger& outputChannels, double sampleRate, int bufferSizeSamples) const
        {
            return device->open (inputChannels, outputChannels, sampleRate, bufferSizeSamples);
        }

        std::uint64_t nsToSampleTime (std::uint64_t ns) const noexcept
        {
            return static_cast<std::uint64_t> (std::round (static_cast<double> (ns) * device->getCurrentSampleRate() * 1e-9));
        }

        void updateSampleTimeFromContext (const AudioIODeviceCallbackContext& context) noexcept
        {
            auto callbackSampleTime = context.hostTimeNs != nullptr ? nsToSampleTime (*context.hostTimeNs) : 0;

            if (input)
                callbackSampleTime += static_cast<std::uint64_t> (owner.targetLatency);

            auto copy = invalidSampleTime;

            if (sampleTime.compare_exchange_strong (copy, callbackSampleTime) && (! input))
                owner.lastValidReadPosition = invalidSampleTime;
        }

        bool isInput() const { return input; }

        std::atomic<std::uint64_t> sampleTime { invalidSampleTime };

    private:

        //==============================================================================
        AudioIODeviceCombiner& owner;
        std::unique_ptr<CoreAudioIODevice> device;
        const bool input;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviceWrapper)
    };

    /* If the current AudioIODeviceCombiner::callback is nullptr, it sets itself as the callback
       and forwards error related callbacks to the provided callback
    */
    class ScopedErrorForwarder final : public AudioIODeviceCallback
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

            // The audio device may stop because it's about to be restarted with new settings.
            // Stopping the device doesn't necessarily count as an error.
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
class CoreAudioIODeviceType final : public AudioIODeviceType,
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

        for (const auto audioDevice : audioDevices)
        {
            if (const auto optionalName = audioObjectGetProperty<CFStringRef> (audioDevice, { kAudioDevicePropertyDeviceNameCFString,
                                                                                              kAudioObjectPropertyScopeWildcard,
                                                                                              juceAudioObjectPropertyElementMain }))
            {
                if (const CFUniquePtr<CFStringRef> name { *optionalName })
                {
                    const auto nameString = String::fromCFString (name.get());

                    if (const auto numIns  = getNumChannels (audioDevice, true); numIns > 0)
                    {
                        inputDeviceNames.add (nameString);
                        inputIds.add (audioDevice);
                    }

                    if (const auto numOuts = getNumChannels (audioDevice, false); numOuts > 0)
                    {
                        outputDeviceNames.add (nameString);
                        outputIds.add (audioDevice);
                    }
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
            return d->getIndexOfDevice (asInput);

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
            return std::make_unique<CoreAudioIODevice> (this, combinedName, inputDeviceID, outputDeviceID).release();

        auto in = inputDeviceID != 0 ? std::make_unique<CoreAudioIODevice> (this, inputDeviceName, inputDeviceID, 0)
                                     : nullptr;

        auto out = outputDeviceID != 0 ? std::make_unique<CoreAudioIODevice> (this, outputDeviceName, 0, outputDeviceID)
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
