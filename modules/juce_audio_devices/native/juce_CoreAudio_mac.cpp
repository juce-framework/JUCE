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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

//==============================================================================
class ScopedCFDictionary;

class ScopedCFArray
{
public:
    ScopedCFArray() = default;

    void appendDictionary (const ScopedCFDictionary& dictionary);
    CFArrayRef get() const { return array.get(); }

private:
    CFUniquePtr<CFMutableArrayRef> array { CFArrayCreateMutable (nullptr, 0,
                                                                 &kCFTypeArrayCallBacks) };
};

class ScopedCFDictionary
{
public:
    void setString (const String& key, const String& value)
    {
        const CFUniquePtr<CFStringRef> cfValue (value.toCFString());
        setRawValue (key, cfValue.get());
    }

    void setInt (const String& key, UInt32 value)
    {
        const CFUniquePtr<CFNumberRef> cfValue (CFNumberCreate (nullptr, kCFNumberIntType, &value));
        setRawValue (key, cfValue.get());
    }

    void setArray (const String& key, const ScopedCFArray& array)
    {
        setRawValue (key, array.get());
    }

    CFDictionaryRef get() const { return dict.get(); }

private:
    void setRawValue (const String& key, const void* value)
    {
        const CFUniquePtr<CFStringRef> cfKey (key.toCFString());
        CFDictionarySetValue (dict.get(), cfKey.get(), value);
    }

    CFUniquePtr<CFMutableDictionaryRef> dict { CFDictionaryCreateMutable (nullptr, 0,
                                                                          &kCFTypeDictionaryKeyCallBacks,
                                                                          &kCFTypeDictionaryValueCallBacks) };
};

void ScopedCFArray::appendDictionary (const ScopedCFDictionary& dictionary)
{
    CFArrayAppendValue (array.get(), dictionary.get());
}

//==============================================================================
#ifndef JUCE_COREAUDIO_LOGGING_ENABLED
 #define JUCE_COREAUDIO_LOGGING_ENABLED 0
#endif

#if JUCE_COREAUDIO_LOGGING_ENABLED
 template <typename... Args>
 void logCoreAudioMessage (Args&&... args)
 {
     String message;
     (message += ... += String { std::forward<Args> (args) });

     Logger::writeToLog ("CoreAudio: " + message);
 }

 #define JUCE_COREAUDIO_LOG(...) logCoreAudioMessage (__VA_ARGS__)
#else
 #define JUCE_COREAUDIO_LOG(...)
#endif

template <typename IntType>
static String getFourCharStringOrHex (IntType value)
{
    const std::array<char, 5> chars
    {
        static_cast<char> ((value >> 24) & 0xff),
        static_cast<char> ((value >> 16) & 0xff),
        static_cast<char> ((value >> 8)  & 0xff),
        static_cast<char> (value & 0xff),
        '\0'
    };

    if (CharPointer_ASCII::isValidString (chars.data(), 4))
        return String { chars.data() }.quoted ('\'');

    return "0x" + String::toHexString (value);
}

struct InertCallback
{
    template <typename... Ts>
    void operator() (Ts&&...) {}
};

template <typename OnError = InertCallback>
static bool checkStatus (OSStatus status, OnError&& onError = {})
{
    if (status == noErr)
        return true;

    if (JUCE_COREAUDIO_LOGGING_ENABLED || ! std::is_same_v<OnError, InertCallback>)
    {
        String message { "error: " + getFourCharStringOrHex (status)};

        JUCE_COREAUDIO_LOG (message);

        NullCheckedInvocation::invoke (onError, message);
    }

    return false;
}

class OSStatusHandler
{
public:
    OSStatusHandler() = default;
    OSStatusHandler (const OSStatusHandler&) = default;
    OSStatusHandler (OSStatusHandler&&) = default;
    virtual ~OSStatusHandler() = default;

    OSStatusHandler& operator= (const OSStatusHandler&) = default;
    OSStatusHandler& operator= (OSStatusHandler&&) = default;

protected:
    bool checkStatus (OSStatus status) const
    {
        return juce::checkStatus (status, [&] (auto error) { onError (error); });
    }

    virtual void onError (const String&) const {}
};

template <typename Fn>
static bool tryMultiple (Fn predicate, int maxNumTries)
{
    if (predicate())
        return true;

    for (auto i = 0; i < (maxNumTries - 1); ++i)
    {
        JUCE_COREAUDIO_LOG ("Failed, trying again...");
        Thread::yield();

        if (predicate())
            return true;
    }

    return false;
}

//==============================================================================
enum class PlaybackDirection : size_t
{
    input,
    output
};

constexpr inline std::array<PlaybackDirection, 2> getAllPlaybackDirections()
{
    return { PlaybackDirection::input, PlaybackDirection::output };
}

constexpr PlaybackDirection opposite (PlaybackDirection direction)
{
    constexpr PlaybackDirection arr[] { PlaybackDirection::output, PlaybackDirection::input };
    return arr[toUnderlyingType (direction)];
}

//==============================================================================
using PropertySelector = AudioObjectPropertySelector;
using PropertyElement = AudioObjectPropertyElement;

enum class PropertyScope : AudioObjectPropertyScope
{
    global = kAudioObjectPropertyScopeGlobal,
    input = kAudioObjectPropertyScopeInput,
    output = kAudioObjectPropertyScopeOutput,
    wildcard = kAudioObjectPropertyScopeWildcard
};

class PropertyAddress
{
public:
    static constexpr auto propertyElementMain =
       #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (12, 0)
        kAudioObjectPropertyElementMain;
       #else
        kAudioObjectPropertyElementMaster;
       #endif

    PropertyAddress (PropertySelector selector)
        : PropertyAddress (selector, PropertyScope::global)
    {}

    PropertyAddress (PropertySelector selector, PropertyScope scope)
        : PropertyAddress (selector, scope, propertyElementMain)
    {}

    PropertyAddress (PropertySelector selector, PropertyElement element)
        : PropertyAddress (selector, PropertyScope::global, element)
    {}

    PropertyAddress (PropertySelector selector, PropertyScope scope, PropertyElement element)
        : address ({ selector, toUnderlyingType (scope), element })
    {}

    PropertyAddress (PropertySelector selector, PlaybackDirection direction)
        : PropertyAddress (selector, toScope (direction))
    {}

    PropertyAddress (PropertySelector selector, PlaybackDirection direction, PropertyElement element)
        : PropertyAddress (selector, toScope (direction), element)
    {}

    const AudioObjectPropertyAddress* get() const { return &address; }

    String toString() const
    {
        return "Selector: " + getFourCharStringOrHex (address.mSelector)
             + ", Scope: " + getFourCharStringOrHex (address.mScope)
             + ", Element: " + getFourCharStringOrHex (address.mElement);
    }

private:
    static constexpr PropertyScope toScope (PlaybackDirection direction)
    {
        constexpr PropertyScope arr[] { PropertyScope::input, PropertyScope::output };
        return arr[toUnderlyingType (direction)];
    }

    AudioObjectPropertyAddress address;
};

//==============================================================================
template <typename CFType>
class CFProperty
{
public:
    CFProperty() = default;

    CFProperty (const CFProperty& other)
        : value (other.value)
    {
        if (value != nullptr)
            CFRetain (value);
    }

    CFProperty& operator= (const CFProperty& other)
    {
        CFProperty { other }.swap (*this);
        return *this;
    }

    CFProperty (CFProperty&& other) noexcept
        : value (std::exchange (other.value, {}))
    {
    }

    CFProperty& operator= (CFProperty&& other) noexcept
    {
        CFProperty { std::move (other) }.swap (*this);
        return *this;
    }

    ~CFProperty()
    {
        if (value != nullptr)
            CFRelease (value);
    }

    CFType get() const { return value; }

    explicit operator CFType() const { return value; }

private:
    void swap (CFProperty& other) noexcept
    {
        std::swap (other.value, value);
    }

    CFType value{};
};

using CFStringProperty = CFProperty<CFStringRef>;
using CFDictionaryProperty = CFProperty<CFDictionaryRef>;

//==============================================================================
template <typename T>
class TypedMemoryBlock
{
public:
    TypedMemoryBlock() = default;
    explicit TypedMemoryBlock (MemoryBlock dataIn) : data (std::move (dataIn)) {}
    bool isEmpty() const { return data.isEmpty(); }
    const T* get() const { return reinterpret_cast<const T*> (data.getData()); }
    operator const T*() const { return get(); }

private:
    MemoryBlock data;
};

//==============================================================================
class PropertyListener
{
public:
    using Callback = std::function<void()>;

    PropertyListener (AudioObjectID objectIdIn,
                      PropertySelector selector,
                      PropertyScope scope,
                      Callback callbackIn)
        : objectId (objectIdIn),
          address (selector, scope, kAudioObjectPropertyElementWildcard),
          callback (std::move (callbackIn))
    {
        if (objectId == kAudioObjectUnknown)
            return;

        AudioObjectAddPropertyListener (objectId, address.get(), &listenerCallback, this);
    }

    ~PropertyListener()
    {
        if (objectId == kAudioObjectUnknown)
            return;

        AudioObjectRemovePropertyListener (objectId, address.get(), &listenerCallback, this);
    }

private:
    static OSStatus listenerCallback (AudioObjectID,
                                      UInt32 numberOfAddresses,
                                      const AudioObjectPropertyAddress*,
                                      void* inClientData)
    {
        for (UInt32 i = 0; i < numberOfAddresses; ++i)
        {
            auto* listener = static_cast<PropertyListener*> (inClientData);
            NullCheckedInvocation::invoke (listener->callback);
        }

        return noErr;
    }

    AudioObjectID objectId{};
    PropertyAddress address;
    Callback callback{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyListener)
    JUCE_DECLARE_NON_MOVEABLE (PropertyListener)
};

template <typename T>
String propertyValueToString (const T& value)
{
    return String { value };
}

template<>
String propertyValueToString (const CFStringProperty& str)
{
    return String::fromCFString (str.get());
}

template<>
String propertyValueToString (const CFDictionaryRef& dict)
{
    return String::fromCFString (makeCFUniquePtr (CFCopyDescription (dict)).get());
}

template<>
String propertyValueToString (const CFDictionaryProperty& dict)
{
    return dict.get() != nullptr ? propertyValueToString (dict.get()) : String{};
}

template<>
String propertyValueToString (const os_workgroup_t&)
{
    return "Workgroup";
}

template<>
String propertyValueToString (const AudioValueRange& range)
{
    return String { "AudioValueRange: " } + String { range.mMinimum } + " -> " + String { range.mMaximum } ;
}

template<>
String propertyValueToString (const std::byte& byte)
{
    return String { "0x" } + String::toHexString (&byte, 1);
}

template<>
String propertyValueToString (const AudioStreamBasicDescription& stream)
{
    return String {   "  AudioStreamBasicDescription: " }
         + String { "\n    mSampleRate: " }         + String { stream.mSampleRate }
         + String { "\n    mFormatID: " }           + getFourCharStringOrHex (stream.mFormatID)
         + String { "\n    mFormatFlags: " }        + BigInteger { stream.mFormatFlags }.toString (2)
         + String { "\n    mBytesPerPacket: " }     + String { stream.mBytesPerPacket }
         + String { "\n    mFramesPerPacket: " }    + String { stream.mFramesPerPacket }
         + String { "\n    mBytesPerFrame: " }      + String { stream.mBytesPerFrame }
         + String { "\n    mChannelsPerFrame: " }   + String { stream.mChannelsPerFrame }
         + String { "\n    mBitsPerChannel: " }     + String { stream.mBitsPerChannel };
}


template<>
String propertyValueToString (const AudioBufferList& bufferList)
{
    auto string = String {   "  AudioBufferList: " }
                + String { "\n    mNumberBuffers: " } + String { bufferList.mNumberBuffers };

    for (UInt32 i = 0; i < bufferList.mNumberBuffers; ++i)
    {
        const auto& buffer = bufferList.mBuffers[i];

        string += String { "\n    AudioBuffer " } + String { i }
                + String { "\n      mNumberChannels: " } + String { buffer.mNumberChannels }
                + String { "\n      mDataByteSize: " }   + String { buffer.mDataByteSize };
    }

    return string;
}

//==============================================================================
class AudioObject
{
public:
    explicit AudioObject (AudioObjectID objectIdIn)
        : objectId (objectIdIn)
    {
        jassert (isValid());
        JUCE_COREAUDIO_LOG ("Instantiated AudioObject: ", objectId);

       #if JUCE_COREAUDIO_LOGGING_ENABLED
        AudioObjectShow (objectId);
       #endif
    }

    AudioObject() = default;
    AudioObject (const AudioObject&) = default;
    AudioObject (AudioObject&&) = default;
    ~AudioObject() = default;

    AudioObject& operator= (const AudioObject& other) = default;
    AudioObject& operator= (AudioObject&& other) = default;

    AudioObjectID getId() const { return objectId; }

    bool isValid() const { return objectId != kAudioObjectUnknown; }

    AudioClassID getClass() const
    {
        return getPropertyOrDefault<AudioClassID> (kAudioObjectPropertyClass);
    }

    template <typename PropertyType>
    std::optional<PropertyType> getProperty (PropertyAddress address) const
    {
        return getProperty<PropertyType> (address, DataType{});
    }

    template <typename PropertyType>
    PropertyType getPropertyOrDefault (PropertyAddress address) const
    {
        return getProperty<PropertyType> (address, DataType{}).value_or (PropertyType{});
    }

    template <typename PropertyType, typename ArgType>
    PropertyType getPropertyOrDefault (PropertyAddress address, const ArgType& arg) const
    {
        return getProperty<PropertyType> (address, DataType { arg }).value_or (PropertyType{});
    }

    template <typename PropertyType>
    std::vector<PropertyType> getPropertyArray (PropertyAddress address) const
    {
        return getPropertyArray<PropertyType> (address, DataType{});
    }

    template <typename PropertyType>
    TypedMemoryBlock<PropertyType> getVariableSizeProperty (PropertyAddress address) const
    {
        return getVariableSizeProperty<PropertyType> (address, DataType{});
    }

    template <typename PropertyType>
    bool setProperty (PropertyAddress address, const PropertyType& newValue)
    {
        JUCE_COREAUDIO_LOG ("Setting property...");
        JUCE_COREAUDIO_LOG ("  Object:   ", getId());
        JUCE_COREAUDIO_LOG ("  Selector: ", getFourCharStringOrHex (address.get()->mSelector));
        JUCE_COREAUDIO_LOG ("  Scope:    ", getFourCharStringOrHex (address.get()->mScope));
        JUCE_COREAUDIO_LOG ("  Element:  ", getFourCharStringOrHex (address.get()->mElement));

        if (! isPropertySettable (address))
        {
            JUCE_COREAUDIO_LOG ("  Property not settable!");
            return false;
        }

        JUCE_COREAUDIO_LOG ("  Value: ", propertyValueToString (newValue));

        return setPropertyData (address, DataType { newValue });
    }

    std::shared_ptr<PropertyListener> createPropertyListener (PropertySelector selector, PropertyListener::Callback callback)
    {
        return std::make_shared<PropertyListener> (getId(), selector, PropertyScope::wildcard, std::move (callback));
    }

    bool operator< (const AudioObject& other) const
    {
        return objectId < other.objectId;
    }

    bool operator== (const AudioObject& other) const
    {
        return objectId == other.objectId;
    }

    bool operator!= (const AudioObject& other) const
    {
        return ! (*this == other);
    }

private:
    class DataType
    {
    public:
        template <typename T>
        explicit DataType (T& arg)
            : data (&arg),
              size ((UInt32) sizeof (T))
        {}

        DataType() = default;

        const void* data{};
        UInt32 size{};
    };

    // We deliberately avoid AudioObjectHasProperty, for some properties on an
    // Apogee Symphony I/O Mk2 it can lock up coreaudiod (likely a driver bug).
    // Missing properties are detected instead when the Get/IsProperty* calls
    // below fail through checkStatus.

    size_t getPropertySize (PropertyAddress address, DataType arg) const
    {
        if (! isValid())
            return {};

        UInt32 size{};

        if (checkStatus (AudioObjectGetPropertyDataSize (getId(), address.get(), arg.size, arg.data, &size)))
            return (size_t) size;

        JUCE_COREAUDIO_LOG (" Failed to get property size!");
        return {};
    }

    template <typename PropertyType>
    bool copyPropertyData (PropertyAddress address, Span<PropertyType> property, DataType arg) const
    {
        if (! isValid())
            return {};

        auto size = (UInt32) (property.getSizeInBytes());

        if (size == 0)
        {
            JUCE_COREAUDIO_LOG ("  Property has a size of 0!");
            return {};
        }

        JUCE_COREAUDIO_LOG ("  Property size: ", size, " bytes");

        const auto result = checkStatus (AudioObjectGetPropertyData (getId(),
                                                                     address.get(),
                                                                     arg.size,
                                                                     arg.data,
                                                                     &size,
                                                                     property.data()));

        if (result == false)
        {
            JUCE_COREAUDIO_LOG ("  Failed to get property!");
            return false;
        }

        JUCE_COREAUDIO_LOG ("  ", size, " bytes retrieved");

        if (size == 0)
            return {};

        // AudioObjectGetPropertyData seems to have written more data than was
        // provided! This will likely result in unpredictable behaviour!
        jassert ((size_t) size <= property.getSizeInBytes());

        return result;
    }

    bool isPropertySettable (PropertyAddress address) const
    {
        if (! isValid())
            return {};

        Boolean isSettable = NO;

        if (checkStatus (AudioObjectIsPropertySettable (getId(), address.get(), &isSettable)))
            return isSettable == YES;

        return false;
    }

    bool setPropertyData (PropertyAddress address, DataType property)
    {
        return checkStatus (AudioObjectSetPropertyData (getId(),
                                                        address.get(),
                                                        0,
                                                        nullptr,
                                                        property.size,
                                                        property.data));
    }

    template <typename PropertyType>
    std::vector<PropertyType> getPropertyArray (PropertyAddress address, DataType arg) const
    {
        JUCE_COREAUDIO_LOG ("Getting property array...");
        JUCE_COREAUDIO_LOG ("  Object:   ", getId());
        JUCE_COREAUDIO_LOG ("  Selector: ", getFourCharStringOrHex (address.get()->mSelector));
        JUCE_COREAUDIO_LOG ("  Scope:    ", getFourCharStringOrHex (address.get()->mScope));

        const auto size = getPropertySize (address, arg);

        std::vector<PropertyType> values (size / sizeof (PropertyType));

        JUCE_COREAUDIO_LOG ("  NumElements: ", values.size());

        if (values.empty())
            return {};

        if (! copyPropertyData (address, Span { values }, arg))
            return {};

       #if JUCE_COREAUDIO_LOGGING_ENABLED
        for (const auto& [index, value] : enumerate (values))
            JUCE_COREAUDIO_LOG ("  Value ", index, ": ", propertyValueToString (value));
       #endif

        return values;
    }

    template <typename PropertyType>
    std::optional<PropertyType> getProperty (PropertyAddress address, DataType arg) const
    {
        JUCE_COREAUDIO_LOG ("Getting property...");
        JUCE_COREAUDIO_LOG ("  Object:   ", getId());
        JUCE_COREAUDIO_LOG ("  Selector: ", getFourCharStringOrHex (address.get()->mSelector));
        JUCE_COREAUDIO_LOG ("  Scope:    ", getFourCharStringOrHex (address.get()->mScope));
        JUCE_COREAUDIO_LOG ("  Element:  ", getFourCharStringOrHex (address.get()->mElement));

        const auto size = getPropertySize (address, arg);

        if (size < sizeof (PropertyType))
            return {};

        PropertyType value;

        if (! copyPropertyData (address, Span { &value, 1 }, arg))
            return {};

        JUCE_COREAUDIO_LOG ("  Value: ", propertyValueToString (value));
        return value;
    }

    template <typename PropertyType>
    TypedMemoryBlock<PropertyType> getVariableSizeProperty (PropertyAddress address, DataType arg) const
    {
        JUCE_COREAUDIO_LOG ("Getting property...");
        JUCE_COREAUDIO_LOG ("  Object:   ", getId());
        JUCE_COREAUDIO_LOG ("  Selector: ", getFourCharStringOrHex (address.get()->mSelector));
        JUCE_COREAUDIO_LOG ("  Scope:    ", getFourCharStringOrHex (address.get()->mScope));
        JUCE_COREAUDIO_LOG ("  Element:  ", getFourCharStringOrHex (address.get()->mElement));

        MemoryBlock block (getPropertySize (address, arg));

        if (block.isEmpty())
            return {};

        if (! copyPropertyData (address, Span { (std::byte*) block.getData(), block.getSize() }, arg))
            return {};

        TypedMemoryBlock<PropertyType> value { std::move (block) };

        JUCE_COREAUDIO_LOG ("  Value: ", propertyValueToString (*value.get()));
        return value;
    }

    AudioObjectID objectId { kAudioObjectUnknown };
};

static_assert (sizeof (AudioObject) == sizeof (AudioObjectID));

//==============================================================================
class AudioStream : public AudioObject
{
public:
    using AudioObject::AudioObject;

    int getLatency() const
    {
        return (int) getPropertyOrDefault<UInt32> (kAudioStreamPropertyLatency);
    }

    int getBitDepth() const
    {
        return (int) getPropertyOrDefault<AudioStreamBasicDescription> (kAudioStreamPropertyPhysicalFormat).mBitsPerChannel;
    }
};

static_assert (sizeof (AudioStream) == sizeof (AudioObject));

template<>
String propertyValueToString (const AudioStream& object)
{
    return String { "  AudioStream { " } + String { object.getId() } + "}";
}

//==============================================================================
static int countChannelsInBufferList (const AudioBufferList& buffers)
{
    return std::accumulate (buffers.mBuffers,
                            buffers.mBuffers + buffers.mNumberBuffers,
                            0,
                            [] (int sum, const auto& buffer)
                            {
                                return sum + (int) buffer.mNumberChannels;
                            });
}

//==============================================================================
class AudioDevice : public AudioObject
{
    static constexpr AudioObjectPropertySelector mainVolumeSelector =
       #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (12, 0)
        kAudioHardwareServiceDeviceProperty_VirtualMainVolume;
       #else
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume;
       #endif

public:
    using AudioObject::AudioObject;

    float getMainVolume() const
    {
        return getPropertyOrDefault<Float32> (mainVolumeSelector);
    }

    bool setMainVolume (float newVolume)
    {
        return setProperty (mainVolumeSelector, (Float32) newVolume);
    }

    bool isMuted() const
    {
        if (auto muted = getProperty<UInt32> (kAudioDevicePropertyMute))
            return (bool) *muted;

        return exactlyEqual (getMainVolume(), 0.0f);
    }

    bool setMute (bool shouldMute)
    {
        return setProperty (kAudioDevicePropertyMute, (UInt32) shouldMute)
            || setMainVolume (0.0f);
    }

    std::vector<AudioValueRange> getAvailableSampleRateRanges() const
    {
        return getPropertyArray<AudioValueRange> (kAudioDevicePropertyAvailableNominalSampleRates);
    }

    int getNumChannels (PlaybackDirection direction) const
    {
        const auto buffers = getVariableSizeProperty<AudioBufferList> ({ kAudioDevicePropertyStreamConfiguration, direction });
        return buffers.isEmpty() ? 0 : countChannelsInBufferList (*buffers);
    }

    String getUid() const
    {
        const auto uid = getPropertyOrDefault<CFStringProperty> (kAudioDevicePropertyDeviceUID);
        return String::fromCFString (uid.get());
    }

    int getBufferSize() const
    {
        return (int) getPropertyOrDefault<UInt32> (kAudioDevicePropertyBufferFrameSize);
    }

    AudioValueRange getBufferSizeRange() const
    {
        return getPropertyOrDefault<AudioValueRange> (kAudioDevicePropertyBufferFrameSizeRange);
    }

    double getSampleRate() const
    {
        return getPropertyOrDefault<Float64> (kAudioDevicePropertyNominalSampleRate);
    }

    bool requestSampleRate (double newSampleRate)
    {
        jassert (newSampleRate > 0.0);
        return setProperty (kAudioDevicePropertyNominalSampleRate, (Float64) newSampleRate);
    }

    bool requestBufferSize (int newBufferSize)
    {
        jassert (newBufferSize > 0);
        return setProperty (kAudioDevicePropertyBufferFrameSize, (UInt32) newBufferSize);
    }

    String getName() const
    {
        const auto deviceName = getPropertyOrDefault<CFStringProperty> (kAudioDevicePropertyDeviceNameCFString);
        return String::fromCFString (deviceName.get());
    }

    std::vector<AudioStream> getStreams (PlaybackDirection direction) const
    {
        return getPropertyArray<AudioStream> ({ kAudioDevicePropertyStreams, direction });
    }

    int getStreamLatency (PlaybackDirection direction, int stream = 0) const
    {
        const auto streams = getStreams (direction);
        const auto index = (size_t) jmax (stream, 0);

        if (index >= streams.size())
            return {};

        return streams[index].getLatency();
    }

    int getLatency (PlaybackDirection direction) const
    {
        return (int) getPropertyOrDefault<UInt32> ({ kAudioDevicePropertyLatency, direction });
    }

    int getSafetyOffset (PlaybackDirection direction) const
    {
        return (int) getPropertyOrDefault<UInt32> ({ kAudioDevicePropertySafetyOffset, direction });
    }

    int getBitDepth() const
    {
        for (auto direction : getAllPlaybackDirections())
            for (auto stream : getStreams (direction))
                if (auto bitDepth = stream.getBitDepth(); bitDepth > 0)
                    return bitDepth;

        return 24;
    }

    String getChannelName (PlaybackDirection direction, int index) const
    {
        const auto channelNum = (UInt32) (index + 1);
        const PropertyAddress address { kAudioObjectPropertyElementName, direction, channelNum };
        const auto channelName = getPropertyOrDefault<CFStringProperty> (address);
        return String::fromCFString (channelName.get());
    }

    bool isAlive() const
    {
        return (bool) getPropertyOrDefault<UInt32> (kAudioDevicePropertyDeviceIsAlive);
    }

    bool isAggregateDevice() const
    {
        return getClass() == kAudioAggregateDeviceClassID;
    }

    AudioWorkgroup getAudioWorkgroup() const
    {
       #if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE
        if (auto workgroup = getProperty<os_workgroup_t> (kAudioDevicePropertyIOThreadOSWorkgroup))
        {
            const ScopeGuard scope { [&] { os_release (*workgroup); } };
            return makeRealAudioWorkgroup (*workgroup);
        }
       #endif

        return {};
    }
};

static_assert (sizeof (AudioDevice) == sizeof (AudioObject));

template<>
String propertyValueToString (const AudioDevice& object)
{
    return String { "  AudioDevice { " } + String { object.getId() } + "}";
}

//==============================================================================
class SystemObject : public AudioObject
{
public:
    SystemObject() : AudioObject (kAudioObjectSystemObject) {}

    AudioDevice getDefaultDevice (PlaybackDirection direction) const
    {
        static constexpr PropertySelector selectors[] {
            kAudioHardwarePropertyDefaultInputDevice,
            kAudioHardwarePropertyDefaultOutputDevice
        };

        return getPropertyOrDefault<AudioDevice> (selectors[toUnderlyingType (direction)]);
    }

    std::vector<AudioDevice> getAudioDevices() const
    {
        return getPropertyArray<AudioDevice> (kAudioHardwarePropertyDevices);
    }

    AudioDevice translateUidToDevice (const String& uid)
    {
        const auto uidCfString = makeCFUniquePtr (uid.toCFString());
        const auto selector = kAudioHardwarePropertyTranslateUIDToDevice;
        return getPropertyOrDefault<AudioDevice> (selector, uidCfString.get());
    }
};

//==============================================================================
class AggregateAudioDevice : public AudioDevice
{
public:
    using AudioDevice::AudioDevice;

    explicit AggregateAudioDevice (const ScopedCFDictionary& description)
        : AudioDevice (create (description)),
          created (getId() != kAudioObjectUnknown)
    {
        jassert (created);
    }

    ~AggregateAudioDevice()
    {
        if (! created)
            return;

        if (checkStatus (AudioHardwareDestroyAggregateDevice (getId())))
            JUCE_COREAUDIO_LOG ("Destroyed aggregate device: ", getId());
        else
            JUCE_COREAUDIO_LOG ("Failed to destroy aggregate device: ", getId());
    }

    String getClockingDeviceUid() const
    {
        const auto subDeviceSelector =
           #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (12, 0)
            kAudioAggregateDevicePropertyMainSubDevice;
           #else
            kAudioAggregateDevicePropertyMasterSubDevice;
           #endif

        for (auto selector : { kAudioAggregateDevicePropertyClockDevice, subDeviceSelector })
            if (auto uid = getPropertyOrDefault<CFStringProperty> (selector); CFStringGetLength (uid.get()) > 0)
                return String::fromCFString (uid.get());

        if (auto devices = getSubDevices(); ! devices.empty())
            return devices[0].getUid();

        return {};
    }

    std::vector<AudioDevice> getSubDevices() const
    {
        return getPropertyArray<AudioDevice> (kAudioAggregateDevicePropertyActiveSubDeviceList);
    }

    // A Multi-Output Device is a stacked aggregate device. Its output channels are arranged such
    // that the output streams are all fed the same data.
    bool isStacked() const
    {
        const auto composition = getPropertyOrDefault<CFDictionaryProperty> (kAudioAggregateDevicePropertyComposition);

        if (composition.get() == nullptr)
            return false;

        const auto* value = CFDictionaryGetValue (composition.get(), CFSTR (kAudioAggregateDeviceIsStackedKey));

        if (value == nullptr)
            return false;

        // This is a lightly documented part of the Core Audio API and in practice the returned type
        // can be either a CFBoolean or a CFNumber, depending on how the device was created.
        if (CFGetTypeID (value) == CFBooleanGetTypeID())
            return CFBooleanGetValue (static_cast<CFBooleanRef> (value));

        if (CFGetTypeID (value) == CFNumberGetTypeID())
        {
            int stacked{};
            CFNumberGetValue (static_cast<CFNumberRef> (value), kCFNumberIntType, &stacked);
            return stacked != 0;
        }

        return false;
    }

    bool configure (const ScopedCFDictionary& newComposition)
    {
        return setProperty (kAudioAggregateDevicePropertyComposition, newComposition.get());
    }

private:
    static AudioObjectID create (const ScopedCFDictionary& description)
    {
        AudioObjectID objectId = kAudioObjectUnknown;

        if (juce::checkStatus (AudioHardwareCreateAggregateDevice (description.get(), &objectId)))
            JUCE_COREAUDIO_LOG ("Created aggregate device: ", objectId);
        else
            JUCE_COREAUDIO_LOG ("Failed to create aggregate device!");

        return objectId;
    }

    bool created{};
};

//==============================================================================
#define JUCE_SYSTEMAUDIOVOL_IMPLEMENTED 1

float JUCE_CALLTYPE SystemAudioVolume::getGain()
{
    return SystemObject{}.getDefaultDevice (PlaybackDirection::output).getMainVolume();
}

bool JUCE_CALLTYPE SystemAudioVolume::setGain (float gain)
{
    return SystemObject{}.getDefaultDevice (PlaybackDirection::output).setMainVolume (gain);
}

bool JUCE_CALLTYPE SystemAudioVolume::isMuted()
{
    return SystemObject{}.getDefaultDevice (PlaybackDirection::output).isMuted();
}

bool JUCE_CALLTYPE SystemAudioVolume::setMuted (bool mute)
{
    return SystemObject{}.getDefaultDevice (PlaybackDirection::output).setMute (mute);
}

//==============================================================================
class ChannelMap
{
public:
    void set (int source, int destination)
    {
        jassert (source >= 0);

        if ((size_t) source >= map.size())
            map.resize ((size_t) source + 1);

        map[(size_t) source] = destination;
    }

    std::optional<int> get (int source) const
    {
        jassert (source >= 0);

        if ((size_t) source >= map.size())
            return {};

        return map[(size_t) source];
    }

    void clear() { map.clear(); }
    auto begin() const { return map.begin(); }
    auto end() const { return map.end(); }
    auto size() const { return map.size(); }

private:
    std::vector<std::optional<int>> map;
};

//==============================================================================
template <typename T>
class UniqueArray
{
public:
    void addIfNotAlreadyThere (T device)
    {
        if (setData.insert (device).second)
            arrayData.add (device);
    }

    auto& getArray() const { return arrayData; }

    auto begin() const { return arrayData.begin(); }
    auto end() const { return arrayData.end(); }

private:
    std::set<T> setData;
    Array<T> arrayData;
};

//==============================================================================
class AggregateDeviceDescription
{
public:
    AggregateDeviceDescription (const String& nameIn,
                                const std::array<AudioDevice, 2>& devicesIn)
        : name (nameIn)
    {
        rebuild (devicesIn);
    }

    void rebuild (const std::array<AudioDevice, 2>& ioDevices)
    {
        clockingDevice = getClockingDevice (ioDevices[toUnderlyingType (PlaybackDirection::output)]);

        if (! clockingDevice.isValid())
            clockingDevice = getClockingDevice (ioDevices[toUnderlyingType (PlaybackDirection::input)]);

        jassert (clockingDevice.isValid());

        devices = {};
        channelMap = {};

        addDevice (ioDevices[toUnderlyingType (PlaybackDirection::output)], PlaybackDirection::output);
        addDevice (ioDevices[toUnderlyingType (PlaybackDirection::input)], PlaybackDirection::input);
    }

    AggregateAudioDevice createAggregateAudioDevice() const
    {
        return AggregateAudioDevice { toDictionary() };
    }

    const std::array<ChannelMap, 2>& getChannelMap() const
    {
        return channelMap;
    }

private:
    static std::vector<AudioDevice> getAudioDevices (AudioDevice device)
    {
        if (! device.isValid())
            return {};

        if (! device.isAggregateDevice())
            return { device };

        return AggregateAudioDevice { device.getId() }.getSubDevices();
    }

    int getFirstChannelIndexFor (AudioDevice device, PlaybackDirection direction) const
    {
        auto startIndex = 0;

        for (const auto& d : devices)
        {
            if (d == device)
                break;

            startIndex += d.getNumChannels (direction);
        }

        return startIndex;
    }

    void addStackedOutputDevice (AudioDevice device)
    {
        const auto dir = PlaybackDirection::output;

        for (auto audioDevice : getAudioDevices (device))
        {
            devices.addIfNotAlreadyThere (audioDevice);

            const auto numChannels = std::min (device.getNumChannels (dir),
                                               audioDevice.getNumChannels (dir));

            const auto aggregateChannelIndex = getFirstChannelIndexFor (audioDevice, dir);

            for (auto channel = 0; channel < numChannels; ++channel)
                channelMap[toUnderlyingType (dir)].set (aggregateChannelIndex + channel, channel);
        }
    }

    void addConcatenatedDevice (AudioDevice device, PlaybackDirection direction)
    {
        int deviceChannelIndex = 0;

        for (auto audioDevice : getAudioDevices (device))
        {
            devices.addIfNotAlreadyThere (audioDevice);

            const auto numChannels = audioDevice.getNumChannels (direction);
            const auto aggregateChannelIndex = getFirstChannelIndexFor (audioDevice, direction);

            for (auto channel = 0; channel < numChannels; ++channel)
                channelMap[toUnderlyingType (direction)].set (aggregateChannelIndex + channel, deviceChannelIndex + channel);

            deviceChannelIndex += numChannels;
        }
    }

    void addDevice (AudioDevice device, PlaybackDirection direction)
    {
        if (! device.isValid())
            return;

        const auto stacked = device.isAggregateDevice() && AggregateAudioDevice { device.getId() }.isStacked();

        if (direction == PlaybackDirection::output && stacked)
            addStackedOutputDevice (device);
        else
            addConcatenatedDevice (device, direction);
    }

    ScopedCFDictionary toDictionary() const
    {
        const auto clockingDeviceUid = clockingDevice.getUid();

        ScopedCFArray subDevices;

        for (auto device : devices)
        {
            const auto uid = device.getUid();

            ScopedCFDictionary subDevice;
            subDevice.setString (kAudioSubDeviceUIDKey, uid);
            subDevice.setInt (kAudioSubDeviceDriftCompensationKey, uid != clockingDeviceUid);
            subDevice.setInt (kAudioSubDeviceDriftCompensationQualityKey,
                             #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (14, 0)
                              kAudioAggregateDriftCompensationHighQuality);
                             #else
                              kAudioSubDeviceDriftCompensationHighQuality);
                             #endif


            subDevices.appendDictionary (subDevice);
        }

        constexpr auto subDeviceKey =
           #if defined (kAudioAggregateDeviceMainSubDeviceKey)
            kAudioAggregateDeviceMainSubDeviceKey;
           #else
            kAudioAggregateDeviceMasterSubDeviceKey;
           #endif

        ScopedCFDictionary description;
        description.setString (kAudioAggregateDeviceNameKey, name);
        description.setString (kAudioAggregateDeviceUIDKey, Uuid{}.toString());
        description.setString (kAudioAggregateDeviceClockDeviceKey, clockingDeviceUid);
        description.setString (subDeviceKey, clockingDeviceUid);
        description.setInt (kAudioAggregateDeviceIsPrivateKey, ! JUCE_COREAUDIO_LOGGING_ENABLED);
        description.setArray (kAudioAggregateDeviceSubDeviceListKey, subDevices);

        return description;
    }

    static AudioDevice getClockingDevice (AudioDevice device)
    {
        if (! device.isValid())
            return {};

        if (! device.isAggregateDevice())
            return device;

        const AggregateAudioDevice aggregate { device.getId() };

        if (const auto uid = aggregate.getClockingDeviceUid(); uid.isNotEmpty())
            return SystemObject{}.translateUidToDevice (uid);

        jassertfalse;
        return {};
    }

    String name;
    AudioDevice clockingDevice;
    UniqueArray<AudioDevice> devices;
    std::array<ChannelMap, 2> channelMap;
};

//==============================================================================
class AudioDeviceProcessor : private OSStatusHandler
{
public:
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void audioDeviceProcessorError (const String& message) = 0;
        virtual void audioDeviceProcessorCallback (int numSamples,
                                                   const AudioTimeStamp* inNow,
                                                   const AudioBufferList* inInputData,
                                                   const AudioTimeStamp* inInputTime,
                                                   AudioBufferList* outOutputData,
                                                   const AudioTimeStamp* inOutputTime) = 0;
    };

    AudioDeviceProcessor (AudioDevice deviceIn, Listener* listenerIn)
        : device (deviceIn),
          listener (listenerIn)
    {}

    bool start()
    {
        numOverloads = 0;
        return created
            && device.isAlive()
            && tryMultiple ([&] { return checkStatus (AudioDeviceStart (device.getId(), processorId)); }, 3);
    }

    void stop()
    {
        if (created
            && device.isAlive()
            && tryMultiple ([&] { return checkStatus (AudioDeviceStop (device.getId(), processorId)); }, 3))
        {
            running = false;
        }
    }

    virtual ~AudioDeviceProcessor()
    {
        if (created && device.isAlive())
            checkStatus (AudioDeviceDestroyIOProcID (device.getId(), processorId));
    }

    bool isRunning() const { return running; }

    int getNumOverloads() const { return isRunning() ? numOverloads.load() : -1; }

private:
    virtual void onError (const String& message) const
    {
        listener->audioDeviceProcessorError (message);
    }

    AudioDevice device{};
    Listener* listener{};
    std::atomic<int> numOverloads{};
    std::shared_ptr<PropertyListener> onOverload = device.createPropertyListener (kAudioDeviceProcessorOverload, [&] { ++numOverloads; });
    std::atomic<bool> running{};
    AudioDeviceIOProcID processorId{};
    AudioDeviceIOBlock callback = ^(const AudioTimeStamp* inNow,
                                    const AudioBufferList* inInputData,
                                    const AudioTimeStamp* inInputTime,
                                    AudioBufferList* outOutputData,
                                    const AudioTimeStamp* inOutputTime)
    {
        const auto numSamples = (int) std::invoke ([&]
        {
            const auto& buffer = inInputData->mNumberBuffers > 0 ? inInputData->mBuffers[0]
                                                                 : outOutputData->mBuffers[0];

            return (size_t) buffer.mDataByteSize / (size_t) buffer.mNumberChannels / sizeof (float);
        });

        listener->audioDeviceProcessorCallback (numSamples,
                                                inNow,
                                                inInputData,
                                                inInputTime,
                                                outOutputData,
                                                inOutputTime);
    };

    std::atomic<bool> created = checkStatus (AudioDeviceCreateIOProcIDWithBlock (&processorId, device.getId(), nullptr, callback));
    std::shared_ptr<PropertyListener> onDeviceDied = device.createPropertyListener (kAudioDevicePropertyDeviceIsAlive, [&]
    {
        if (! device.isAlive())
            running = false;
    });
};

//==============================================================================
struct DeviceSetup
{
    double sampleRate{};
    int bufferSize{};
    BigInteger activeChannels[2]{};
};

struct DeviceSetupChangedListener
{
    virtual ~DeviceSetupChangedListener() = default;
    virtual void deviceSetupChanged() = 0;
};

static constexpr const char* ioStrings[] { "Input", "Output" };

//==============================================================================
class PlaybackHandler : private AudioDeviceProcessor::Listener
{
public:
    PlaybackHandler (AudioDevice coreAudioDeviceIn,
                     DeviceSetupChangedListener* listenerIn)
        : coreAudioDevice (coreAudioDeviceIn),
          listener (listenerIn)
    {}

    ~PlaybackHandler() override { stop(); }

    void start (AudioIODevice* juceDevice,
                DeviceSetup newSetup,
                std::array<ChannelMap, 2> newChannelMap,
                AudioIODeviceCallback* newCallback)
    {
        const SpinLock::ScopedLockType lock { mutex };

        if (playing)
            stopInternal();

        {
            const std::scoped_lock errorLock { errorMutex };
            callback = newCallback;
        }

        setup = newSetup;
        activeChannelMap = {};
        detectedSetupChange = false;

        for (const auto direction : getAllPlaybackDirections())
        {
            const auto& activeChans = setup.activeChannels[toUnderlyingType (direction)];

            for (auto [deviceChannel, virtualDeviceChannel] : enumerate (newChannelMap[toUnderlyingType (direction)], int{}))
            {
                if (virtualDeviceChannel.has_value() && activeChans[*virtualDeviceChannel])
                {
                    const auto targetBufferChannel = activeChans.getBitRange (0, *virtualDeviceChannel)
                                                                .countNumberOfSetBits();

                    JUCE_COREAUDIO_LOG ("Mapping ", ioStrings[toUnderlyingType (direction)], " channel ", deviceChannel, " -> ", targetBufferChannel);
                    activeChannelMap[toUnderlyingType (direction)].set (deviceChannel, targetBufferChannel);
                }
            }

            buffers[direction].setSize (activeChans.countNumberOfSetBits(), setup.bufferSize);
        }

        callback->audioDeviceAboutToStart (juceDevice);
        playing = true;

        if (! processor.start())
        {
            callback->audioDeviceError ("Failed to start device");
            stopInternal();
        }
    }

    void stop()
    {
        const SpinLock::ScopedLockType lock { mutex };

        if (playing)
            stopInternal();
    }

    AudioIODeviceCallback* getCallback() const
    {
        const SpinLock::ScopedLockType lock { mutex };
        return callback;
    }

    bool isPlaying() const { return playing; }

    void onError (const String& message) const
    {
        const std::scoped_lock lock { errorMutex };

        if (callback != nullptr)
            callback->audioDeviceError (message);
    }

    int getNumOverloads() const { return processor.getNumOverloads(); }

private:
    void stopInternal()
    {
        processor.stop();
        playing = false;
        callback->audioDeviceStopped();

        const std::scoped_lock lock { errorMutex };
        callback = nullptr;
    }

    void audioDeviceProcessorError (const String& message) final
    {
        onError (message);
    }

    template <typename Fn>
    static void forEachActiveSample (const AudioBufferList* buffers,
                                     const ChannelMap& activeChannelMap,
                                     int numSamples,
                                     Fn sampleCallback)
    {
        if (buffers == nullptr)
            return;

        auto deviceChannel = 0;

        for (UInt32 i = 0; i < buffers->mNumberBuffers; ++i)
        {
            const auto& buffer = buffers->mBuffers[i];
            const auto numChannels = (size_t) buffer.mNumberChannels;
            auto* bufferData = (float*) buffer.mData;

            for (size_t bufferChannel = 0; bufferChannel < numChannels; ++bufferChannel, ++deviceChannel)
            {
                const auto activeChannel = activeChannelMap.get (deviceChannel);

                if (! activeChannel)
                   continue;

                auto* data = bufferData + bufferChannel;

                for (auto index = 0; index < numSamples; ++index, data += numChannels)
                    sampleCallback (*activeChannel, index, *data);
            }
        }
    }

    void audioDeviceProcessorCallback (int numSamples,
                                       const AudioTimeStamp* inNow,
                                       const AudioBufferList* inInputData,
                                       const AudioTimeStamp*,
                                       AudioBufferList* outOutputData,
                                       const AudioTimeStamp*) override
    {
        const SpinLock::ScopedTryLockType lock { mutex };

        if (! lock.isLocked() || detectedSetupChange)
            return;

        using Direction = PlaybackDirection;

        if (numSamples != setup.bufferSize)
        {
            JUCE_COREAUDIO_LOG ("Detected buffer size change from ", setup.bufferSize, " samples -> ", numSamples, " samples");
            listener->deviceSetupChanged();
            detectedSetupChange = true;
            return;
        }

        auto inputSamples = buffers[Direction::input].getArrayOfWritePointers();

        forEachActiveSample (inInputData,
                             activeChannelMap[toUnderlyingType (Direction::input)],
                             numSamples,
                             [&] (auto channel, auto sample, auto value)
        {
            inputSamples[channel][sample] = value;
        });

        const AudioIODeviceCallbackContext context { inNow != nullptr ? &inNow->mHostTime : nullptr };
        callback->audioDeviceIOCallbackWithContext (buffers[Direction::input].getArrayOfReadPointers(),
                                                    buffers[Direction::input].getNumChannels(),
                                                    buffers[Direction::output].getArrayOfWritePointers(),
                                                    buffers[Direction::output].getNumChannels(),
                                                    numSamples,
                                                    context);

        const auto outputSamples = buffers[Direction::output].getArrayOfReadPointers();

        forEachActiveSample (outOutputData,
                             activeChannelMap[toUnderlyingType (Direction::output)],
                             numSamples,
                             [&] (auto channel, auto sample, auto& value)
        {
            value = outputSamples[channel][sample];
        });
    }

    AudioDevice coreAudioDevice;
    DeviceSetup setup;
    std::array<ChannelMap, 2> activeChannelMap;
    DeviceSetupChangedListener* listener;
    AudioIODeviceCallback* callback{};
    AudioDeviceProcessor processor { coreAudioDevice, this };
    std::map<PlaybackDirection, AudioBuffer<float>> buffers;
    std::atomic<bool> playing{};
    std::atomic<bool> detectedSetupChange{};
    mutable SpinLock mutex;
    mutable std::mutex errorMutex;
};

//==============================================================================
class DeviceConfigurator
{
public:
    DeviceConfigurator (AudioIODevice* juceDeviceIn,
                        AudioDevice coreAudioDeviceIn,
                        DeviceSetupChangedListener* listenerIn)
        : juceDevice (juceDeviceIn),
          coreAudioDevice (coreAudioDeviceIn),
          listener (listenerIn)
    {
        currentSetup.sampleRate = coreAudioDevice.getSampleRate();
        currentSetup.bufferSize = coreAudioDevice.getBufferSize();
        currentSetup.activeChannels[toUnderlyingType (PlaybackDirection::input)].setRange (0, juceDevice->getInputChannelNames().size(), true);
        currentSetup.activeChannels[toUnderlyingType (PlaybackDirection::output)].setRange (0, juceDevice->getOutputChannelNames().size(), true);
    }

    enum class Error
    {
        none,
        failedToSetSampleRate,
        failedToSetBufferSize,
    };

    Error configure (const DeviceSetup& setup)
    {
        const std::scoped_lock lock { mutex };

        requestedSetup = setup;
        stage = ConfigurationStage::request;
        return tryNextConfiguration();
    }

    Error reconfigure()
    {
        const std::scoped_lock lock { mutex };
        return tryNextConfiguration();
    }

    DeviceSetup getCurrentSetup() const { return currentSetup; }

private:
    enum class ConfigurationStage
    {
        request,
        retry,
        bufferSearch
    };

    Error tryNextConfiguration()
    {
        switch (stage)
        {
            case ConfigurationStage::request:
            {
                JUCE_COREAUDIO_LOG ("Reconfiguring device to use requested configuration");
                stage = ConfigurationStage::retry;
                return configureDeviceUsingRequestedSetup();
            }

            case ConfigurationStage::retry:
            {
                JUCE_COREAUDIO_LOG ("Reconfiguring device by retrying last requested device configuration");
                stage = ConfigurationStage::bufferSearch;
                return configureDeviceUsingRequestedSetup();
            }

            case ConfigurationStage::bufferSearch:
            {
                if (! bufferSizesToTry.has_value())
                {
                    bufferSizesToTry = juceDevice->getAvailableBufferSizes();
                    bufferSizesToTry->removeFirstMatchingValue (requestedSetup.bufferSize);
                }

                if (bufferSizesToTry->isEmpty())
                {
                    JUCE_COREAUDIO_LOG ("Reconfiguring device to use its current settings");
                    return configureUsingCurrentDeviceSettings();
                }
                else
                {
                    JUCE_COREAUDIO_LOG ("Reconfiguring device to use nearest available buffer size");
                    return configureUsingNearestAvailableBufferSize();
                }
            }
        }

        jassertfalse;
        return {};
    }

    Error configureDeviceUsingRequestedSetup()
    {
        bufferSizesToTry = std::nullopt;

        const auto sampleRate = requestSampleRate (requestedSetup.sampleRate);

        if (! sampleRate.has_value())
            return Error::failedToSetSampleRate;

        const auto bufferSize = requestBufferSize (requestedSetup.bufferSize);

        if (! bufferSize.has_value())
            return Error::failedToSetBufferSize;

        currentSetup.sampleRate = *sampleRate;
        currentSetup.bufferSize = *bufferSize;

        const auto restrictNumBits = [] (const BigInteger& bits, int maxNumBits)
        {
            BigInteger mask;
            mask.setRange (0, maxNumBits, true);
            return bits & mask;
        };

        for (const auto direction : getAllPlaybackDirections())
        {
            const auto directionIndex = toUnderlyingType (direction);
            currentSetup.activeChannels[directionIndex] = restrictNumBits (requestedSetup.activeChannels[directionIndex],
                                                                           getNumChannels (direction));
        }

        return Error::none;
    }

    Error configureUsingNearestAvailableBufferSize()
    {
        jassert (bufferSizesToTry.has_value() && ! bufferSizesToTry->isEmpty());

        const auto nextBufferSize = findNearestValue (Span (*bufferSizesToTry), coreAudioDevice.getBufferSize());
        bufferSizesToTry->removeFirstMatchingValue (nextBufferSize);

        const auto bufferSize = requestBufferSize (nextBufferSize);

        if (! bufferSize.has_value())
            return Error::failedToSetBufferSize;

        currentSetup.bufferSize = *bufferSize;

        return Error::none;
    }

    Error configureUsingCurrentDeviceSettings()
    {
        currentSetup.sampleRate = coreAudioDevice.getSampleRate();
        currentSetup.bufferSize = coreAudioDevice.getBufferSize();
        return Error::none;
    }

    int getNumChannels (PlaybackDirection direction)
    {
        switch (direction)
        {
            case PlaybackDirection::input: return juceDevice->getInputChannelNames().size();
            case PlaybackDirection::output: return juceDevice->getOutputChannelNames().size();
        }

        return {};
    }

    std::optional<double> requestSampleRate (double newSampleRate)
    {
        const auto sampleRates = juceDevice->getAvailableSampleRates();
        const auto targetSampleRate = newSampleRate > 0.0 ? findNearestValue (Span (sampleRates), newSampleRate)
                                                          : coreAudioDevice.getSampleRate();

        const auto tryRequestingSampleRate = [&] (auto dev)
        {
            JUCE_COREAUDIO_LOG ("Requesting sample-rate: ", targetSampleRate);

            if (approximatelyEqual (targetSampleRate, dev.getSampleRate()))
                return true;

            return tryMultiple ([&] { return dev.requestSampleRate (targetSampleRate); }, 3);
        };

        if (! tryRequestingSampleRate (coreAudioDevice))
            return {};

        return targetSampleRate;
    }

    std::optional<int> requestBufferSize (int newBufferSize)
    {
        const auto sizes = juceDevice->getAvailableBufferSizes();
        const auto targetBufferSize = newBufferSize > 0 ? findNearestValue (Span (sizes), newBufferSize)
                                                        : juceDevice->getDefaultBufferSize();

        JUCE_COREAUDIO_LOG ("Requesting buffer-size: ", targetBufferSize);

        if (targetBufferSize == coreAudioDevice.getBufferSize())
            return targetBufferSize;

        if (tryMultiple ([&] { return coreAudioDevice.requestBufferSize (targetBufferSize); }, 3))
            return targetBufferSize;

        return {};
    }

    std::optional<Array<int>> bufferSizesToTry;
    ConfigurationStage stage;
    AudioIODevice* juceDevice;
    AudioDevice coreAudioDevice;
    DeviceSetupChangedListener* listener;
    DeviceSetup requestedSetup;
    DeviceSetup currentSetup;
    std::mutex mutex;
    std::shared_ptr<PropertyListener> onSampleRateChanged = coreAudioDevice.createPropertyListener (kAudioDevicePropertyNominalSampleRate, [&]
    {
        const std::scoped_lock lock { mutex };

        const auto newSampleRate = coreAudioDevice.getSampleRate();

        if (approximatelyEqual (currentSetup.sampleRate, newSampleRate))
            return;

        JUCE_COREAUDIO_LOG ("Detected sample rate change from " + String { currentSetup.sampleRate } + "Hz -> " + String { newSampleRate } + "Hz");
        requestedSetup.sampleRate = newSampleRate;
        stage = ConfigurationStage::request;
        listener->deviceSetupChanged();
    });
};

//==============================================================================
class CoreAudioClasses
{
public:
    class CoreAudioIODeviceType;

private:
    static inline const String aggregateDevicePrefix { "_JucePrivateAggregateDevice_" };

    class CoreAudioIODevice final : public AudioIODevice,
                                    private OSStatusHandler,
                                    private DeviceSetupChangedListener,
                                    private AsyncUpdater
    {
    public:
        struct Listener
        {
            virtual ~Listener() = default;
            virtual void audioDevicePropertyChanged() = 0;
        };

        CoreAudioIODevice (const String& devName,
                           std::array<AudioDevice, 2> ioDevicesIn,
                           Listener* listener)
            : AudioIODevice (devName, "CoreAudio"),
              ioDevices (ioDevicesIn),
              onAnyDevicePropertyChanged (aggregateDevice.createPropertyListener (kAudioDevicePropertyDeviceHasChanged, [&, listener]
              {
                  JUCE_COREAUDIO_LOG ("Device property change detected");
                  listener->audioDevicePropertyChanged();
              }))
        {
            if (getAvailableSampleRates().isEmpty())
                lastError = TRANS ("No matching sample rates");
        }

        AudioDevice getCoreAudioDevice (PlaybackDirection direction) const
        {
            return ioDevices[toUnderlyingType (direction)];
        }

        StringArray getOutputChannelNames() final
        {
            return getChannelNames (PlaybackDirection::output);
        }

        StringArray getInputChannelNames() final
        {
            return getChannelNames (PlaybackDirection::input);
        }

        //==============================================================================
        Array<double> getAvailableSampleRates() final
        {
            const auto getDeviceSampleRates = [] (auto device) -> Array<double>
            {
                UniqueArray<double> sampleRates;

                if (! device.isValid())
                    return sampleRates.getArray();

                for (const auto& range : device.getAvailableSampleRateRanges())
                {
                    if (exactlyEqual (range.mMinimum, range.mMaximum) && range.mMinimum > 0.0)
                    {
                        sampleRates.addIfNotAlreadyThere (range.mMinimum);
                        continue;
                    }

                    for (const auto& rate : SampleRateHelpers::getCommonSampleRates())
                    {
                        if (rate >= range.mMinimum && rate <= range.mMaximum)
                            sampleRates.addIfNotAlreadyThere (rate);
                    }
                }

                return sampleRates.getArray();
            };

            auto inputSampleRates = getDeviceSampleRates (ioDevices[toUnderlyingType (PlaybackDirection::input)]);
            auto outputSampleRates = getDeviceSampleRates (ioDevices[toUnderlyingType (PlaybackDirection::output)]);

            if (outputSampleRates.isEmpty())
                return inputSampleRates;

            if (! inputSampleRates.isEmpty())
                outputSampleRates.removeValuesNotIn (inputSampleRates);

            return outputSampleRates;
        }

        Array<int> getAvailableBufferSizes() final
        {
            const auto range = aggregateDevice.getBufferSizeRange();

            const auto min = roundToInt (range.mMinimum);
            const auto max = roundToInt (range.mMaximum);

            jassert (min > 0);
            jassert (max >= min);

            Array<int> sizes;

            for (auto size = nextPowerOfTwo (min); size <= max; size = nextPowerOfTwo (size + 1))
                sizes.add (size);

            if (sizes.isEmpty())
                sizes.add (min);

            return sizes;
        }

        int getDefaultBufferSize() final
        {
            const auto bufferSizes = getAvailableBufferSizes();
            return jlimit (bufferSizes.getFirst(), bufferSizes.getLast(), 512);
        }

        //==============================================================================
        String open (const BigInteger& inputChannels,
                     const BigInteger& outputChannels,
                     double sampleRate,
                     int bufferSizeSamples) final
        {
            DeviceSetup setup;
            setup.sampleRate = sampleRate;
            setup.bufferSize = bufferSizeSamples;
            setup.activeChannels[toUnderlyingType (PlaybackDirection::input)] = inputChannels;
            setup.activeChannels[toUnderlyingType (PlaybackDirection::output)] = outputChannels;

            const std::scoped_lock lock { mutex };

            JUCE_COREAUDIO_LOG ("Opening device...");

            const ScopeGuard scope { [&] { JUCE_COREAUDIO_LOG (isOpen() ? "Device opened" : ("Failed to open device: " + lastError)); }};

            switch (deviceConfigurator.configure (setup))
            {
                case DeviceConfigurator::Error::failedToSetSampleRate:
                    lastError = TRANS ("Failed to set sample rate");
                    break;

                case DeviceConfigurator::Error::failedToSetBufferSize:
                    lastError = TRANS ("Failed to set buffer size");
                    break;

                case DeviceConfigurator::Error::none:
                    lastError.clear();
                    opened = true;
                    break;
            }

            return lastError;
        }

        void close() final
        {
            const std::scoped_lock lock { mutex };

            JUCE_COREAUDIO_LOG ("Closing device...");

            playbackHandler.stop();
            opened = false;

            JUCE_COREAUDIO_LOG ("Device closed");
        }

        bool isOpen() final
        {
            return opened;
        }

        void start (AudioIODeviceCallback* newCallback) final
        {
            const std::scoped_lock lock { mutex };

            JUCE_COREAUDIO_LOG ("Starting device...");

            const ScopeGuard scope { [&] { JUCE_COREAUDIO_LOG (isPlaying() ? "Device started" : ("Failed to start device: " + lastError)); }};

            if (! isOpen())
            {
                lastError = TRANS ("Device must be opened before trying to start it");
                return;
            }

            if (newCallback == nullptr)
            {
                playbackHandler.stop();
                lastError = TRANS ("AudioIODeviceCallback must not be null");
                return;
            }

            playbackHandler.start (this, deviceConfigurator.getCurrentSetup(), description.getChannelMap(), newCallback);

            if (! playbackHandler.isPlaying())
                lastError = TRANS ("Failed to start device");
        }

        void stop() final
        {
            const std::scoped_lock lock { mutex };

            JUCE_COREAUDIO_LOG ("Stoping device...");

            playbackHandler.stop();

            JUCE_COREAUDIO_LOG ("Device stopped");
        }

        bool isPlaying() final
        {
            return playbackHandler.isPlaying();
        }

        String getLastError() final
        {
            const std::scoped_lock lock { mutex };
            return lastError;
        }

        //==============================================================================
        int getCurrentBufferSizeSamples() final
        {
            return deviceConfigurator.getCurrentSetup().bufferSize;
        }

        double getCurrentSampleRate() final
        {
            return deviceConfigurator.getCurrentSetup().sampleRate;
        }

        int getCurrentBitDepth() final
        {
            return aggregateDevice.getBitDepth();
        }

        BigInteger getActiveChannels (PlaybackDirection direction) const
        {
            return deviceConfigurator.getCurrentSetup().activeChannels[toUnderlyingType (direction)];
        }

        BigInteger getActiveOutputChannels() const final
        {
            return getActiveChannels (PlaybackDirection::output);
        }

        BigInteger getActiveInputChannels() const final
        {
            return getActiveChannels (PlaybackDirection::input);
        }

        int getOutputLatencyInSamples()  final
        {
            if (! ioDevices[toUnderlyingType (PlaybackDirection::output)].isValid())
                return {};

            return aggregateDevice.getLatency (PlaybackDirection::output)
                 + aggregateDevice.getSafetyOffset (PlaybackDirection::output)
                 + aggregateDevice.getBufferSize()
                 + aggregateDevice.getStreamLatency (PlaybackDirection::output);
        }

        int getInputLatencyInSamples()  final
        {
            if (! ioDevices[toUnderlyingType (PlaybackDirection::input)].isValid())
                return {};

            return aggregateDevice.getLatency (PlaybackDirection::input)
                 + aggregateDevice.getSafetyOffset (PlaybackDirection::input)
                 + aggregateDevice.getBufferSize()
                 + aggregateDevice.getStreamLatency (PlaybackDirection::input);
        }

        AudioWorkgroup getWorkgroup() const final
        {
            return aggregateDevice.getAudioWorkgroup();
        }

        //==============================================================================
        int getXRunCount() const noexcept final
        {
            return playbackHandler.getNumOverloads();
        }

    private:
        StringArray getChannelNames (PlaybackDirection direction) const
        {
            const auto device = getCoreAudioDevice (direction);

            if (! device.isValid())
                return {};

            StringArray names;
            const auto numChannels = device.getNumChannels (direction);
            names.ensureStorageAllocated (numChannels);

            struct SubDevice
            {
                String name;
                int remainingChannels{};
            };

            std::vector<SubDevice> subDevices;
            size_t subDeviceIndex{};

            if (device.isAggregateDevice())
            {
                const AggregateAudioDevice aggregate { device.getId() };

                // We can't assign specific device names to the channels with a stacked aggregate device,
                // because each channel will be broadcast to all sub-devices.
                if (! aggregate.isStacked())
                {
                    const auto devs = aggregate.getSubDevices();
                    subDevices.reserve (devs.size());

                    for (const auto& dev : devs)
                        subDevices.push_back ({ dev.getName(), dev.getNumChannels (direction) });
                }
            }

            for (int index = 0; index < numChannels; ++index)
            {
                auto channelName = device.getChannelName (direction, index);

                if (channelName.isEmpty())
                    channelName = ioStrings[toUnderlyingType (direction)] + String (" ") + String (index + 1);

                const auto subDeviceName = std::invoke ([&]() -> String
                {
                    while (subDeviceIndex < subDevices.size() && subDevices[subDeviceIndex].remainingChannels == 0)
                        ++subDeviceIndex;

                    if (subDeviceIndex >= subDevices.size())
                        return {};

                    --subDevices[subDeviceIndex].remainingChannels;
                    return subDevices[subDeviceIndex].name;
                });

                if (subDeviceName.isNotEmpty())
                    channelName += " (" + subDeviceName + ")";

                names.add (channelName);
            }

            return names;
        }

        int getNumChannels (PlaybackDirection direction)
        {
            const auto device = ioDevices[toUnderlyingType (direction)];
            return device.isValid() ? device.getNumChannels (direction) : 0;
        }

        void onError (const String& message) const final
        {
            playbackHandler.onError (message);
        }

        void handleAsyncUpdate() final
        {
            JUCE_COREAUDIO_LOG ("Handling device setup changed...");

            const std::scoped_lock lock { mutex };

            if (! isOpen())
                return;

            const auto wasPlaying = playbackHandler.isPlaying();
            auto* callback = playbackHandler.getCallback();

            if (wasPlaying)
                playbackHandler.stop();

            if (deviceConfigurator.reconfigure() == DeviceConfigurator::Error::none
                && wasPlaying
                && callback != nullptr)
            {
                description.rebuild (ioDevices);
                playbackHandler.start (this, deviceConfigurator.getCurrentSetup(), description.getChannelMap(), callback);
            }
        }

        void deviceSetupChanged() final
        {
            JUCE_COREAUDIO_LOG ("Device setup changed");
            triggerAsyncUpdate();
        }

        //==============================================================================
        std::array<AudioDevice, 2> ioDevices;
        AggregateDeviceDescription description { aggregateDevicePrefix + String ((int) getpid()), ioDevices };
        AggregateAudioDevice aggregateDevice { description.createAggregateAudioDevice() };
        String lastError;
        std::shared_ptr<PropertyListener> onAnyDevicePropertyChanged;
        DeviceConfigurator deviceConfigurator { this, aggregateDevice, this };
        DeviceSetup currentSetup { deviceConfigurator.getCurrentSetup() };
        PlaybackHandler playbackHandler { aggregateDevice, this };
        std::atomic<bool> opened{};
        std::mutex mutex;
    };

public:
    class CoreAudioIODeviceType final : public AudioIODeviceType,
                                        public CoreAudioIODevice::Listener,
                                        private AsyncUpdater
    {
    public:
        CoreAudioIODeviceType() : AudioIODeviceType ("CoreAudio")
        {
            // This removes stale aggregate devices. The private aggregate
            // device stores the pid as part of the name, if the pid does not
            // match any currently running process we can confidently remove it.
            // False positives are not a significant concern here we just want
            // to prevent any significant build up of aggregate devices.

            for (const auto& device : SystemObject{}.getAudioDevices())
            {
                if (! device.isAggregateDevice())
                    continue;

                const auto deviceName = device.getName();

                if (! deviceName.startsWith (aggregateDevicePrefix))
                    continue;

                const auto pid = (pid_t) deviceName.replace (aggregateDevicePrefix, "")
                                                   .getIntValue();

                // Note a signal of 0 ensures no signal is sent to the process
                const auto processExists = pid > 0 && (::kill (pid, 0) == 0 || errno != ESRCH);

                if (! processExists)
                {
                    JUCE_COREAUDIO_LOG ("Destroying stale aggregate device");
                    AudioHardwareDestroyAggregateDevice (device.getId());
                }
            }
        }

        ~CoreAudioIODeviceType() override
        {
            cancelPendingUpdate();
        }

        void scanForDevices() final
        {
            hasScanned = true;

            devices = {};
            deviceNames = {};
            deviceListeners.clear();

            for (auto audioDevice : SystemObject{}.getAudioDevices())
            {
                const auto name = audioDevice.getName();

                if (name.startsWith (aggregateDevicePrefix))
                    continue;

                deviceListeners.push_back (audioDevice.createPropertyListener (kAudioDevicePropertyStreamConfiguration, [&]
                {
                    JUCE_COREAUDIO_LOG ("Device stream configuration changed");
                    needsRescan = true;
                    triggerAsyncUpdate();
                }));

                for (auto direction : getAllPlaybackDirections())
                {
                    if (audioDevice.getNumChannels (direction) == 0)
                        continue;

                    devices[toUnderlyingType (direction)].add (audioDevice);
                    deviceNames[toUnderlyingType (direction)].add (name);
                }
            }

            for (auto direction : getAllPlaybackDirections())
                deviceNames[toUnderlyingType (direction)].appendNumbersToDuplicates (false, true);
        }

        StringArray getDeviceNames (bool wantInputNames) const final
        {
            jassert (hasScanned);
            return wantInputNames ? deviceNames[toUnderlyingType (PlaybackDirection::input)]
                                  : deviceNames[toUnderlyingType (PlaybackDirection::output)];
        }

        int getDefaultDeviceIndex (bool forInput) const final
        {
            const auto direction = forInput ? PlaybackDirection::input
                                            : PlaybackDirection::output;

            const auto defaultDevice = SystemObject{}.getDefaultDevice (direction);
            return jmax (0, getIndexOfDevice (defaultDevice, direction));
        }

        int getIndexOfDevice (AudioIODevice* device, bool asInput) const final
        {
            const auto direction = asInput ? PlaybackDirection::input
                                           : PlaybackDirection::output;

            if (const auto* dev = dynamic_cast<CoreAudioIODevice*> (device))
                return getIndexOfDevice (dev->getCoreAudioDevice (direction), direction);

            return -1;
        }

        bool hasSeparateInputsAndOutputs() const final
        {
            return true;
        }

        AudioIODevice* createDevice (const String& outputDeviceName,
                                     const String& inputDeviceName) override
        {
            jassert (hasScanned); // need to call scanForDevices() before doing this

            const auto inputIndex  = deviceNames[toUnderlyingType (PlaybackDirection::input)].indexOf (inputDeviceName);
            const auto outputIndex = deviceNames[toUnderlyingType (PlaybackDirection::output)].indexOf (outputDeviceName);

            const auto inputDevice  = devices[toUnderlyingType (PlaybackDirection::input)][inputIndex];
            const auto outputDevice = devices[toUnderlyingType (PlaybackDirection::output)][outputIndex];

            if (! inputDevice.isValid() && ! outputDevice.isValid())
                return nullptr;

            const auto deviceName = outputDeviceName.isEmpty() ? inputDeviceName
                                                               : outputDeviceName;

            return std::make_unique<CoreAudioIODevice> (deviceName, std::array { inputDevice, outputDevice }, this)
                .release();
        }

        void audioDevicePropertyChanged() final
        {
            triggerAsyncUpdate();
        }

    private:
        void handleAsyncUpdate() final
        {
            if (needsRescan.exchange (false))
                scanForDevices();

            callDeviceChangeListeners();
        }

        int getIndexOfDevice (AudioDevice deviceToFind, PlaybackDirection direction) const
        {
            jassert (hasScanned);

            for (auto [index, device] : enumerate (devices[toUnderlyingType (direction)], int{}))
                if (device == deviceToFind)
                    return index;

            return -1;
        }

        bool hasScanned = false;
        std::atomic<bool> needsRescan = false;
        std::array<StringArray, 2> deviceNames;
        std::array<Array<AudioDevice>, 2> devices;
        std::vector<std::shared_ptr<PropertyListener>> deviceListeners;
        std::shared_ptr<PropertyListener> onDeviceListChanged = SystemObject{}.createPropertyListener (kAudioHardwarePropertyDevices, [&]
        {
            JUCE_COREAUDIO_LOG ("System device list change detected");
            needsRescan = true;
            triggerAsyncUpdate();
        });
    };
};

#undef JUCE_COREAUDIOLOG

} // namespace juce
