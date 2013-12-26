/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#if JUCE_PLUGINHOST_VST3

} // namespace juce

// Wow, those VST guys really don't worry too much about compiler warnings.
#if JUCE_MSVC
 #pragma warning (disable: 4505)
 #pragma warning (push, 0)
 #pragma warning (disable: 4702)
#elif JUCE_CLANG
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wreorder"
 #pragma clang diagnostic ignored "-Wunsequenced"
 #pragma clang diagnostic ignored "-Wint-to-pointer-cast"
 #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Woverloaded-virtual"
 #pragma clang diagnostic ignored "-Wshadow"
#endif

/*  These files come with the Steinberg VST3 SDK - to get them, you'll need to
    visit the Steinberg website and agree to whatever is currently required to
    get them.

    Then, you'll need to make sure your include path contains your "VST SDK3"
    directory (or whatever you've named it on your machine). The introjucer has
    a special box for setting this path.
*/
#include <base/source/baseiids.cpp>
#include <base/source/fatomic.cpp>
#include <base/source/fbuffer.cpp>
#include <base/source/fdebug.cpp>
#include <base/source/fobject.cpp>
#include <base/source/frect.cpp>
#include <base/source/fstreamer.cpp>
#include <base/source/fstring.cpp>
#include <base/source/fthread.cpp>
#include <base/source/updatehandler.cpp>
#include <pluginterfaces/base/conststringtable.cpp>
#include <pluginterfaces/base/funknown.cpp>
#include <pluginterfaces/base/ustring.cpp>
#include <public.sdk/source/main/pluginfactoryvst3.cpp>
#include <public.sdk/source/common/memorystream.cpp>
#include <public.sdk/source/common/pluginview.cpp>
#include <public.sdk/source/vst/vstbus.cpp>
#include <public.sdk/source/vst/vstinitiids.cpp>
#include <public.sdk/source/vst/vstcomponent.cpp>
#include <public.sdk/source/vst/vstcomponentbase.cpp>
#include <public.sdk/source/vst/vstparameters.cpp>
#include <public.sdk/source/vst/vstsinglecomponenteffect.cpp>
#include <public.sdk/source/vst/hosting/hostclasses.cpp>

#if JUCE_MSVC
 #pragma warning (pop)
#elif JUCE_CLANG
 #pragma clang diagnostic pop
#endif

#undef ASSERT
#undef WARNING
#undef PRINTSYSERROR
#undef DEBUGSTR
#undef DBPRT0
#undef DBPRT1
#undef DBPRT2
#undef DBPRT3
#undef DBPRT4
#undef DBPRT5
#undef calloc
#undef free
#undef malloc
#undef realloc
#undef NEW
#undef NEWVEC
#undef VERIFY
#undef VERIFY_IS
#undef VERIFY_NOT
#undef META_CREATE_FUNC
#undef CLASS_CREATE_FUNC
#undef SINGLE_CREATE_FUNC
#undef _META_CLASS
#undef _META_CLASS_IFACE
#undef _META_CLASS_SINGLE
#undef META_CLASS
#undef META_CLASS_IFACE
#undef META_CLASS_SINGLE
#undef OBJ_METHODS
#undef SINGLETON
#undef QUERY_INTERFACE

namespace juce
{

using namespace Steinberg;

//==============================================================================
struct VST3Classes
{

#ifndef JUCE_VST3_DEBUGGING
 #define JUCE_VST3_DEBUGGING 0
#endif

#if JUCE_VST3_DEBUGGING
 #define VST3_DBG(a) Logger::writeToLog (a);
#else
 #define VST3_DBG(a)
#endif

#if JUCE_DEBUG
static int warnOnFailure (int result)
{
    const char* message = "Unknown result!";

    switch (result)
    {
        case kResultOk:         return result;
        case kNotImplemented:   message = "kNotImplemented";  break;
        case kNoInterface:      message = "kNoInterface";     break;
        case kResultFalse:      message = "kResultFalse";     break;
        case kInvalidArgument:  message = "kInvalidArgument"; break;
        case kInternalError:    message = "kInternalError";   break;
        case kNotInitialized:   message = "kNotInitialized";  break;
        case kOutOfMemory:      message = "kOutOfMemory";     break;
        default:                break;
    }

    DBG (message);
    return result;
}
#else
 #define warnOnFailure(x) x
#endif

//==============================================================================
template <class ObjectType>
class ComSmartPtr
{
public:
    ComSmartPtr() noexcept : source (nullptr) {}
    ComSmartPtr (ObjectType* object) noexcept  : source (object)              { if (source != nullptr) source->addRef(); }
    ComSmartPtr (const ComSmartPtr& other) noexcept : source (other.source)   { if (source != nullptr) source->addRef(); }
    ~ComSmartPtr()                                                            { if (source != nullptr) source->release(); }

    operator ObjectType*() const noexcept    { return source; }
    ObjectType* get() const noexcept         { return source; }
    ObjectType& operator*() const noexcept   { return *source; }
    ObjectType* operator->() const noexcept  { return source; }

    ComSmartPtr& operator= (const ComSmartPtr& other)       { return operator= (other.source); }

    ComSmartPtr& operator= (ObjectType* const newObjectToTakePossessionOf)
    {
        ComSmartPtr p (newObjectToTakePossessionOf);
        std::swap (p.source, source);
        return *this;
    }

    bool operator== (ObjectType* const other) noexcept { return source == other; }
    bool operator!= (ObjectType* const other) noexcept { return source != other; }

    bool loadFrom (FUnknown* o)
    {
        *this = nullptr;
        return o != nullptr && o->queryInterface (ObjectType::iid, (void**) &source) == kResultOk;
    }

    bool loadFrom (IPluginFactory* factory, const TUID& uuid)
    {
        jassert (factory != nullptr);
        *this = nullptr;
        return factory->createInstance (uuid, ObjectType::iid, (void**) &source) == kResultOk;
    }

private:
    ObjectType* source;
};

//==============================================================================
#define JUCE_DECLARE_VST3_COM_REF_METHODS \
    Steinberg::uint32 JUCE_CALLTYPE addRef()   { return (Steinberg::uint32) ++refCount; } \
    Steinberg::uint32 JUCE_CALLTYPE release()  { const int r = --refCount; if (r == 0) delete this; return (Steinberg::uint32) r; }

#define JUCE_DECLARE_VST3_COM_QUERY_METHODS \
    tresult PLUGIN_API JUCE_CALLTYPE queryInterface (const TUID, void** obj) \
    {\
        jassertfalse; \
        *obj = nullptr; \
        return kNotImplemented; \
    }

//==============================================================================
static String toString (const char8*  string) noexcept   { return String (string); }
static String toString (const char16* string) noexcept   { return String (CharPointer_UTF16 ((CharPointer_UTF16::CharType*) string)); }

// NB: The casts are handled by a Steinberg::UString operator
static String toString (const UString128& string) noexcept   { return toString (static_cast<const char16*> (string)); }
static String toString (const UString256& string) noexcept   { return toString (static_cast<const char16*> (string)); }

//==============================================================================
static int getHashForTUID (const TUID& tuid) noexcept
{
    int value = 0;

    for (int i = 0; i < numElementsInArray (tuid); ++i)
        value = (value * 31) + tuid[i];

    return value;
}

template<typename ObjectType>
static void fillDescriptionWith (PluginDescription& description, ObjectType& object)
{
    description.version  = toString (object.version).trim();
    description.category = toString (object.subCategories).trim();

    if (description.manufacturerName.isEmpty())
        description.manufacturerName = toString (object.vendor).trim();
}

static void createPluginDescription (PluginDescription& description,
                                     const File& pluginFile, const String& company, const String& name,
                                     const PClassInfo& info, PClassInfo2* info2, PClassInfoW* infoW,
                                     int numInputs, int numOutputs)
{
    description.fileOrIdentifier    = pluginFile.getFullPathName();
    description.lastFileModTime     = pluginFile.getLastModificationTime();
    description.manufacturerName    = company;
    description.name                = name;
    description.pluginFormatName    = "VST3";
    description.numInputChannels    = numInputs;
    description.numOutputChannels   = numOutputs;
    description.uid                 = getHashForTUID (info.cid);

    if (infoW != nullptr)      fillDescriptionWith (description, *infoW);
    else if (info2 != nullptr) fillDescriptionWith (description, *info2);

    if (description.category.isEmpty())
        description.category = toString (info.category).trim();

    description.isInstrument = description.category.containsIgnoreCase ("Instrument"); // This seems to be the only way to find that out! ARGH!
}

static int getNumSingleDirectionBussesFor (Vst::IComponent* component,
                                           bool checkInputs,
                                           bool checkAudioChannels)
{
    jassert (component != nullptr);

    return (int) component->getBusCount (checkAudioChannels ? Vst::kAudio : Vst::kEvent,
                                         checkInputs ? Vst::kInput : Vst::kOutput);
}

/** Gives the total number of channels for a particular type of bus direction and media type */
static int getNumSingleDirectionChannelsFor (Vst::IComponent* component,
                                             bool checkInputs,
                                             bool checkAudioChannels)
{
    jassert (component != nullptr);

    const Vst::BusDirections direction  = checkInputs ? Vst::kInput : Vst::kOutput;
    const Vst::MediaTypes mediaType     = checkAudioChannels ? Vst::kAudio : Vst::kEvent;
    const Steinberg::int32 numBuses     = component->getBusCount (mediaType, direction);

    int numChannels = 0;

    for (Steinberg::int32 i = numBuses; --i >= 0;)
    {
        Vst::BusInfo busInfo;
        warnOnFailure (component->getBusInfo (mediaType, direction, i, busInfo));
        numChannels += (int) busInfo.channelCount;
    }

    return numChannels;
}

static void activateAllBussesOfType (Vst::IComponent* component,
                                     bool activateInputs,
                                     bool activateAudioChannels)
{
    jassert (component != nullptr);

    const Vst::BusDirections direction  = activateInputs ? Vst::kInput : Vst::kOutput;
    const Vst::MediaTypes mediaType     = activateAudioChannels ? Vst::kAudio : Vst::kEvent;
    const Steinberg::int32 numBuses     = component->getBusCount (mediaType, direction);

    for (Steinberg::int32 i = numBuses; --i >= 0;)
        warnOnFailure (component->activateBus (mediaType, direction, i, true));
}

//==============================================================================
/** Assigns a complete AudioSampleBuffer's channels to an AudioBusBuffers' */
static void associateWholeBufferTo (Vst::AudioBusBuffers& vstBuffers, const AudioSampleBuffer& buffer) noexcept
{
    vstBuffers.channelBuffers32 = buffer.getArrayOfChannels();
    vstBuffers.numChannels      = buffer.getNumChannels();
    vstBuffers.silenceFlags     = 0;
}

//==============================================================================
static void toProcessContext (Vst::ProcessContext& context, AudioPlayHead* playHead, double sampleRate)
{
    jassert (sampleRate > 0.0); //Must always be valid, as stated by the VST3 SDK

    using namespace Vst;

    zerostruct (context);
    context.sampleRate = sampleRate;

    if (playHead != nullptr)
    {
        AudioPlayHead::CurrentPositionInfo position;
        playHead->getCurrentPosition (position);

        context.projectTimeSamples  = position.timeInSamples; //Must always be valid, as stated by the VST3 SDK
        context.projectTimeMusic    = position.timeInSeconds; //Does not always need to be valid...
        context.tempo               = position.bpm;
        context.timeSigNumerator    = position.timeSigNumerator;
        context.timeSigDenominator  = position.timeSigDenominator;
        context.barPositionMusic    = position.ppqPositionOfLastBarStart;
        context.cycleStartMusic     = position.ppqLoopStart;
        context.cycleEndMusic       = position.ppqLoopEnd;

        switch (position.frameRate)
        {
            case AudioPlayHead::fps24: context.frameRate.framesPerSecond = 24; break;
            case AudioPlayHead::fps25: context.frameRate.framesPerSecond = 25; break;
            case AudioPlayHead::fps30: context.frameRate.framesPerSecond = 30; break;

            case AudioPlayHead::fps2997:
            case AudioPlayHead::fps2997drop:
            case AudioPlayHead::fps30drop:
            {
                context.frameRate.framesPerSecond = 30;
                context.frameRate.flags = FrameRate::kDropRate;

                if (position.frameRate == AudioPlayHead::fps2997drop)
                    context.frameRate.flags |= FrameRate::kPullDownRate;
            }
            break;

            default:    jassertfalse; break; // New frame rate?
        }

        if (position.isPlaying)     context.state |= ProcessContext::kPlaying;
        if (position.isRecording)   context.state |= ProcessContext::kRecording;
        if (position.isLooping)     context.state |= ProcessContext::kCycleActive;
    }
    else
    {
        context.tempo                       = 120.0;
        context.frameRate.framesPerSecond   = 30;
        context.timeSigNumerator            = 4;
        context.timeSigDenominator          = 4;
    }

    if (context.projectTimeMusic >= 0.0)        context.state |= ProcessContext::kProjectTimeMusicValid;
    if (context.barPositionMusic >= 0.0)        context.state |= ProcessContext::kBarPositionValid;
    if (context.tempo > 0.0)                    context.state |= ProcessContext::kTempoValid;
    if (context.frameRate.framesPerSecond > 0)  context.state |= ProcessContext::kSmpteValid;

    if (context.cycleStartMusic >= 0.0
         && context.cycleEndMusic > 0.0
         && context.cycleEndMusic > context.cycleStartMusic)
    {
        context.state |= ProcessContext::kCycleValid;
    }

    if (context.timeSigNumerator > 0 && context.timeSigDenominator > 0)
        context.state |= ProcessContext::kTimeSigValid;
}

//==============================================================================
/** Get a list of speaker arrangements as per their speaker names

    (e.g.: 2 regular channels, aliased as 'kStringStereoS', is "L R")
*/
static StringArray getSpeakerArrangements()
{
    using namespace Vst::SpeakerArr;

    const Vst::CString arrangements[] =
    {
        kStringMonoS,       kStringStereoS,         kStringStereoRS,    kStringStereoCS,
        kStringStereoSS,    kStringStereoCLfeS,     kString30CineS,     kString30MusicS,
        kString31CineS,     kString31MusicS,        kString40CineS,     kString40MusicS,
        kString41CineS,     kString41MusicS,        kString50S,         kString51S,
        kString60CineS,     kString60MusicS,        kString61CineS,     kString61MusicS,
        kString70CineS,     kString70MusicS,        kString71CineS,     kString71MusicS,
        kString80CineS,     kString80MusicS,        kString81CineS,     kString81MusicS,
        kString80CubeS,             kStringBFormat1stOrderS,    kString71CineTopCenterS,
        kString71CineCenterHighS,   kString71CineFrontHighS,    kString71CineSideHighS,
        kString71CineFullRearS,     kString90S,                 kString91S,
        kString100S,        kString101S,            kString110S,        kString111S,
        kString130S,        kString131S,            kString102S,        kString122S,
        nullptr
    };

    return StringArray (arrangements);
}

/** Get a list of speaker arrangements as per their named configurations

    (e.g.: 2 regular channels, aliased as 'kStringStereoS', is "L R")
*/
static StringArray getNamedSpeakerArrangements()
{
    using namespace Vst::SpeakerArr;

    const Vst::CString arrangements[] =
    {
        kStringEmpty,       kStringMono,            kStringStereo,              kStringStereoR,
        kStringStereoC,     kStringStereoSide,      kStringStereoCLfe,          kString30Cine,
        kString30Music,     kString31Cine,          kString31Music,             kString40Cine,
        kString40Music,     kString41Cine,          kString41Music,             kString50,
        kString51,          kString60Cine,          kString60Music,             kString61Cine,
        kString61Music,     kString70Cine,          kString70Music,             kString71Cine,
        kString71Music,     kString71CineTopCenter, kString71CineCenterHigh,
        kString71CineFrontHigh,         kString71CineSideHigh,          kString71CineFullRear,
        kString80Cine,      kString80Music,         kString80Cube,              kString81Cine,
        kString81Music,     kString102,             kString122,                 kString90,
        kString91,          kString100,             kString101,                 kString110,
        kString111,         kString130,             kString131,
        nullptr
    };

    return StringArray (arrangements);
}

static Vst::SpeakerArrangement getSpeakerArrangementFrom (const String& string)
{
    return Vst::SpeakerArr::getSpeakerArrangementFromString (string.toUTF8());
}

/**
    @note There can only be 1 arrangement per channel count. (i.e.: 4 channels == k31Cine OR k40Cine)
*/
static void fillWithCorrespondingSpeakerArrangements (Array<Vst::SpeakerArrangement>& destination,
                                                      int numChannels)
{
    using namespace Vst::SpeakerArr;

    destination.clearQuick();

    if (numChannels <= 0)
    {
        destination.add (kEmpty);
        return;
    }

    /*
        The order of the arrangement checks must be descending, since most plugins test for
        the first arrangement to match their number of specified channels.
    */
    if (numChannels >= 14)  destination.add (k131);
    if (numChannels >= 13)  destination.add (k130);
    if (numChannels >= 12)  destination.add (k111);
    if (numChannels >= 11)  destination.add (k101);
    if (numChannels >= 10)  destination.add (k91);
    if (numChannels >= 9)   destination.add (k90);
    if (numChannels >= 8)   destination.add (k71CineFullFront);
    if (numChannels >= 7)   destination.add (k61Cine);
    if (numChannels >= 6)   destination.add (k51);
    if (numChannels >= 5)   destination.add (k50);
    if (numChannels >= 4)   destination.add (k31Cine);
    if (numChannels >= 3)   destination.add (k30Cine);
    if (numChannels >= 2)   destination.add (kStereo);
    if (numChannels >= 1)   destination.add (kMono);
}

//==============================================================================
static StringArray getPluginEffectCategories()
{
    using namespace Vst::PlugType;

    const Vst::CString categories[] =
    {
        kFxAnalyzer,            kFxDelay,       kFxDistortion,      kFxDynamics,
        kFxEQ,                  kFxFilter,      kFx,                kFxInstrument,
        kFxInstrumentExternal,  kFxSpatial,     kFxGenerator,       kFxMastering,
        kFxModulation,          kFxPitchShift,  kFxRestoration,     kFxReverb,
        kFxSurround,            kFxTools,       kSpatial,           kSpatialFx,
        nullptr
    };

    return StringArray (categories);
}

static StringArray getPluginInstrumentCategories()
{
    using namespace Vst::PlugType;

    const Vst::CString categories[] =
    {
        kInstrumentSynthSampler,    kInstrumentDrum,
        kInstrumentSampler,         kInstrumentSynth,
        kInstrumentExternal,        kFxInstrument,
        kFxInstrumentExternal,      kFxSpatial,
        kFxGenerator,
        nullptr
    };

    return StringArray (categories);
}

//==============================================================================
class MidiEventList  : public Vst::IEventList
{
public:
    MidiEventList() {}
    virtual ~MidiEventList() {}

    JUCE_DECLARE_VST3_COM_REF_METHODS
    JUCE_DECLARE_VST3_COM_QUERY_METHODS

    //==============================================================================
    void clear()
    {
        events.clearQuick();
    }

    Steinberg::int32 PLUGIN_API getEventCount() override
    {
        return (Steinberg::int32) events.size();
    }

    // NB: This has to cope with out-of-range indexes from some plugins.
    tresult PLUGIN_API getEvent (Steinberg::int32 index, Vst::Event& e) override
    {
        if (isPositiveAndBelow ((int) index, events.size()))
        {
            e = events.getReference ((int) index);
            return kResultTrue;
        }

        return kResultFalse;
    }

    tresult PLUGIN_API addEvent (Vst::Event& e) override
    {
        events.add (e);
        return kResultTrue;
    }

    //==============================================================================
    static void toMidiBuffer (MidiBuffer& result, Vst::IEventList& eventList)
    {
        using namespace Vst;

        for (Steinberg::int32 i = 0; i < eventList.getEventCount(); ++i)
        {
            Event e;

            if (eventList.getEvent (i, e) == kResultOk)
            {
                switch (e.type)
                {
                    case Event::kNoteOnEvent:
                        result.addEvent (MidiMessage::noteOn (createSafeChannel (e.noteOn.channel),
                                                              createSafeNote (e.noteOn.pitch),
                                                              (uint8) denormaliseToMidiValue (e.noteOn.velocity)),
                                         e.sampleOffset);
                        break;

                    case Event::kNoteOffEvent:
                        result.addEvent (MidiMessage::noteOff (createSafeChannel (e.noteOff.channel),
                                                               createSafeNote (e.noteOff.pitch),
                                                               (uint8) denormaliseToMidiValue (e.noteOff.velocity)),
                                         e.sampleOffset);
                        break;

                    case Event::kPolyPressureEvent:
                        result.addEvent (MidiMessage::aftertouchChange (createSafeChannel (e.polyPressure.channel),
                                                                        createSafeNote (e.polyPressure.pitch),
                                                                        denormaliseToMidiValue (e.polyPressure.pressure)),
                                         e.sampleOffset);
                        break;

                    case Event::kDataEvent:
                        result.addEvent (MidiMessage::createSysExMessage (e.data.bytes, e.data.size),
                                         e.sampleOffset);
                        break;

                    default:
                        break;
                }
            }
        }
    }

    static void toEventList (Vst::IEventList& result, MidiBuffer& midiBuffer)
    {
        using namespace Vst;

        MidiBuffer::Iterator iterator (midiBuffer);
        MidiMessage msg;
        int midiEventPosition = 0;

        enum { maxNumEvents = 2048 }; // Steinberg's Host Checker states no more than 2048 events are allowed at once
        int numEvents = 0;

        while (iterator.getNextEvent (msg, midiEventPosition))
        {
            if (++numEvents > maxNumEvents)
                break;

            Event e = { 0 };

            if (msg.isNoteOn())
            {
                e.type              = Event::kNoteOnEvent;
                e.noteOn.channel    = createSafeChannel (msg.getChannel());
                e.noteOn.pitch      = createSafeNote (msg.getNoteNumber());
                e.noteOn.velocity   = normaliseMidiValue (msg.getVelocity());
                e.noteOn.length     = 0;
                e.noteOn.tuning     = 0.0f;
                e.noteOn.noteId     = -1;
            }
            else if (msg.isNoteOff())
            {
                e.type              = Event::kNoteOffEvent;
                e.noteOff.channel   = createSafeChannel (msg.getChannel());
                e.noteOff.pitch     = createSafeNote (msg.getNoteNumber());
                e.noteOff.velocity  = normaliseMidiValue (msg.getVelocity());
                e.noteOff.tuning    = 0.0f;
                e.noteOff.noteId    = -1;
            }
            else if (msg.isSysEx())
            {
                e.type          = Event::kDataEvent;
                e.data.bytes    = msg.getSysExData();
                e.data.size     = msg.getSysExDataSize();
                e.data.type     = DataEvent::kMidiSysEx;
            }
            else if (msg.isAftertouch())
            {
                e.type                   = Event::kPolyPressureEvent;
                e.polyPressure.channel   = createSafeChannel (msg.getChannel());
                e.polyPressure.pitch     = createSafeNote (msg.getNoteNumber());
                e.polyPressure.pressure  = normaliseMidiValue (msg.getAfterTouchValue());
            }
            else
            {
                continue;
            }

            e.busIndex = 0;
            e.sampleOffset = midiEventPosition;

            result.addEvent (e);
        }
    }

private:
    Array<Vst::Event, CriticalSection> events;
    Atomic<int32> refCount;

    static Steinberg::int16 createSafeChannel (int channel) noexcept  { return (Steinberg::int16) jlimit (0, 15, channel - 1); }
    static int createSafeChannel (Steinberg::int16 channel) noexcept  { return (int) jlimit (1, 16, channel + 1); }

    static Steinberg::int16 createSafeNote (int note) noexcept        { return (Steinberg::int16) jlimit (0, 127, note); }
    static int createSafeNote (Steinberg::int16 note) noexcept        { return jlimit (0, 127, (int) note); }

    static float normaliseMidiValue (int value) noexcept              { return jlimit (0.0f, 1.0f, (float) value / 127.0f); }
    static int denormaliseToMidiValue (float value) noexcept          { return roundToInt (jlimit (0.0f, 127.0f, value * 127.0f)); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEventList)
};

//==============================================================================
class VST3PluginInstance;

class VST3HostContext  : public Vst::IComponentHandler,  // From VST V3.0.0
                         public Vst::IComponentHandler2, // From VST V3.1.0 (a very well named class, of course!)
                         public Vst::IComponentHandler3, // From VST V3.5.0 (also very well named!)
                         public Vst::IContextMenuTarget,
                         public Vst::IHostApplication,
                         public Vst::IParamValueQueue,
                         public Vst::IUnitHandler
{
public:
    VST3HostContext (VST3PluginInstance* pluginInstance)  : owner (pluginInstance)
    {
        appName = File::getSpecialLocation (File::currentApplicationFile).getFileNameWithoutExtension();
        attributeList = new AttributeList (this);
    }

    virtual ~VST3HostContext() {}

    JUCE_DECLARE_VST3_COM_REF_METHODS

    FUnknown* getFUnknown()     { return static_cast<Vst::IComponentHandler*> (this); }

    //==============================================================================
    tresult PLUGIN_API beginEdit (Vst::ParamID) override
    {
        // XXX todo..
        return kResultFalse;
    }

    tresult PLUGIN_API performEdit (Vst::ParamID id, Vst::ParamValue valueNormalized) override
    {
        if (owner != nullptr)
            return owner->editController->setParamNormalized (id, valueNormalized);

        return kResultFalse;
    }

    tresult PLUGIN_API endEdit (Vst::ParamID) override
    {
        // XXX todo..
        return kResultFalse;
    }

    tresult PLUGIN_API restartComponent (Steinberg::int32) override
    {
        if (owner != nullptr)
        {
            owner->reset();
            owner->updateHostDisplay();
            return kResultTrue;
        }

        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API setDirty (TBool) override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API requestOpenEditor (FIDString name) override
    {
        (void) name;
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API startGroupEdit() override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API finishGroupEdit() override
    {
        jassertfalse;
        return kResultFalse;
    }

    Vst::IContextMenu* PLUGIN_API createContextMenu (IPlugView*, const Vst::ParamID*) override
    {
        jassertfalse;
        return nullptr;
    }

    tresult PLUGIN_API executeMenuItem (Steinberg::int32) override
    {
        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API getName (Vst::String128 name) override
    {
        Steinberg::String str (appName.toUTF8());
        str.copyTo (name, 0, 127);
        return kResultOk;
    }

    tresult PLUGIN_API createInstance (TUID cid, TUID iid, void** obj) override
    {
        *obj = nullptr;

        if (! doIdsMatch (cid, iid))
        {
            jassertfalse;
            return kInvalidArgument;
        }

        if (doIdsMatch (cid, Vst::IMessage::iid) && doIdsMatch (iid, Vst::IMessage::iid))
        {
            ComSmartPtr<Message> m (new Message (*this, attributeList));
            messageQueue.add (m);
            m->addRef();
            *obj = m;
            return kResultOk;
        }
        else if (doIdsMatch (cid, Vst::IAttributeList::iid) && doIdsMatch (iid, Vst::IAttributeList::iid))
        {
            ComSmartPtr<AttributeList> l (new AttributeList (this));
            l->addRef();
            *obj = l;
            return kResultOk;
        }

        jassertfalse;
        return kNotImplemented;
    }

    //==============================================================================
    Vst::ParamID PLUGIN_API getParameterId() override
    {
        jassertfalse;
        return 0;
    }

    Steinberg::int32 PLUGIN_API getPointCount() override
    {
        jassertfalse;
        return 0;
    }

    tresult PLUGIN_API getPoint (Steinberg::int32, Steinberg::int32&, Vst::ParamValue&) override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API addPoint (Steinberg::int32, Vst::ParamValue, Steinberg::int32&) override
    {
        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API notifyUnitSelection (Vst::UnitID) override
    {
        jassertfalse;
        return kResultFalse;
    }

    tresult PLUGIN_API notifyProgramListChange (Vst::ProgramListID, Steinberg::int32) override
    {
        jassertfalse;
        return kResultFalse;
    }

    //==============================================================================
    tresult PLUGIN_API queryInterface (const TUID iid, void** obj) override
    {
        if (doIdsMatch (iid, Vst::IAttributeList::iid))
        {
            *obj = attributeList.get();
            return kResultOk;
        }

        #define TEST_FOR_AND_RETURN_IF_VALID(ClassType) \
            if (doIdsMatch (iid, ClassType::iid)) \
            { \
                addRef(); \
                *obj = dynamic_cast<ClassType*> (this); \
                return kResultOk; \
            }

        TEST_FOR_AND_RETURN_IF_VALID (Vst::IComponentHandler)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IComponentHandler2)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IComponentHandler3)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IContextMenuTarget)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IHostApplication)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IParamValueQueue)
        TEST_FOR_AND_RETURN_IF_VALID (Vst::IUnitHandler)
        #undef TEST_FOR_AND_RETURN_IF_VALID

        *obj = nullptr;
        return kNotImplemented;
    }

private:
    //==============================================================================
    Atomic<int32> refCount;
    String appName;
    VST3PluginInstance* owner;

    //==============================================================================
    static bool doIdsMatch (const TUID a, const TUID b) noexcept
    {
        return std::memcmp (a, b, sizeof (TUID)) == 0;
    }

    //==============================================================================
    class Message  : public Vst::IMessage
    {
    public:
        Message (VST3HostContext& o, Vst::IAttributeList* list)
           : owner (o), attributeList (list)
        {
        }

        Message (VST3HostContext& o, Vst::IAttributeList* list, FIDString id)
           : owner (o), attributeList (list), messageId (toString (id))
        {
        }

        Message (VST3HostContext& o, Vst::IAttributeList* list, FIDString id, const var& v)
           : value (v), owner (o), attributeList (list), messageId (toString (id))
        {
        }

        virtual ~Message() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        FIDString PLUGIN_API getMessageID()              { return messageId.toRawUTF8(); }
        void PLUGIN_API setMessageID (FIDString id)      { messageId = toString (id); }
        Vst::IAttributeList* PLUGIN_API getAttributes()  { return attributeList; }

        var value;

    private:
        VST3HostContext& owner;
        ComSmartPtr<Vst::IAttributeList> attributeList;
        String messageId;
        Atomic<int32> refCount;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Message)
    };

    Array<ComSmartPtr<Message>, CriticalSection> messageQueue;

    //==============================================================================
    class AttributeList  : public Vst::IAttributeList
    {
    public:
        AttributeList (VST3HostContext* o)  : owner (o) {}
        virtual ~AttributeList() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        //==============================================================================
        tresult PLUGIN_API setInt (AttrID id, Steinberg::int64 value) override
        {
            jassert (id != nullptr);

            if (! setValueForId (id, value))
                owner->messageQueue.add (ComSmartPtr<Message> (new Message (*owner, this, id, value)));

            return kResultTrue;
        }

        tresult PLUGIN_API setFloat (AttrID id, double value) override
        {
            jassert (id != nullptr);

            if (! setValueForId (id, value))
                owner->messageQueue.add (ComSmartPtr<Message> (new Message (*owner, this, id, value)));

            return kResultTrue;
        }

        tresult PLUGIN_API setString (AttrID id, const Vst::TChar* string) override
        {
            jassert (id != nullptr);
            jassert (string != nullptr);

            const String text (toString (string));

            if (! setValueForId (id, text))
                owner->messageQueue.add (ComSmartPtr<Message> (new Message (*owner, this, id, text)));

            return kResultTrue;
        }

        tresult PLUGIN_API setBinary (AttrID id, const void* data, Steinberg::uint32 size) override
        {
            jassert (id != nullptr);
            jassert (data != nullptr && size > 0);

            MemoryBlock block (data, (size_t) size);

            if (! setValueForId (id, block))
                owner->messageQueue.add (ComSmartPtr<Message> (new Message (*owner, this, id, block)));

            return kResultTrue;
        }

        //==============================================================================
        tresult PLUGIN_API getInt (AttrID id, Steinberg::int64& result) override
        {
            jassert (id != nullptr);

            if (fetchValueForId (id, result))
                return kResultTrue;

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API getFloat (AttrID id, double& result) override
        {
            jassert (id != nullptr);

            if (fetchValueForId (id, result))
                return kResultTrue;

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API getString (AttrID id, Vst::TChar* result, Steinberg::uint32 length) override
        {
            jassert (id != nullptr);

            String stringToFetch;
            if (fetchValueForId (id, stringToFetch))
            {
                Steinberg::String str (stringToFetch.toRawUTF8());
                str.copyTo (result, 0, (Steinberg::int32) jmin (length, (Steinberg::uint32) std::numeric_limits<Steinberg::int32>::max()));

                return kResultTrue;
            }

            jassertfalse;
            return kResultFalse;
        }

        tresult PLUGIN_API getBinary (AttrID id, const void*& data, Steinberg::uint32& size) override
        {
            jassert (id != nullptr);

            for (int i = owner->messageQueue.size(); --i >= 0;)
            {
                Message* const message = owner->messageQueue.getReference (i);

                if (std::strcmp (message->getMessageID(), id) == 0)
                {
                    if (MemoryBlock* binaryData = message->value.getBinaryData())
                    {
                        data = binaryData->getData();
                        size = (Steinberg::uint32) binaryData->getSize();
                        return kResultTrue;
                    }
                }
            }

            return kResultFalse;
        }

    private:
        VST3HostContext* owner;
        Atomic<int32> refCount;

        //==============================================================================
        template<typename Type>
        bool setValueForId (AttrID id, const Type& value)
        {
            jassert (id != nullptr);

            for (int i = owner->messageQueue.size(); --i >= 0;)
            {
                VST3HostContext::Message* const message = owner->messageQueue.getReference (i);

                if (std::strcmp (message->getMessageID(), id) == 0)
                {
                    message->value = value;
                    return true;
                }
            }

            return false; // No message found with that Id
        }

        template<typename Type>
        bool fetchValueForId (AttrID id, Type& value)
        {
            jassert (id != nullptr);

            for (int i = owner->messageQueue.size(); --i >= 0;)
            {
                VST3HostContext::Message* const message = owner->messageQueue.getReference (i);

                if (std::strcmp (message->getMessageID(), id) == 0)
                {
                    value = message->value;
                    return true;
                }
            }

            return false;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttributeList)
    };

    ComSmartPtr<AttributeList> attributeList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3HostContext)
};

//==============================================================================
class DescriptionFactory
{
public:
    DescriptionFactory (VST3HostContext* host, IPluginFactory* pluginFactory)
        : vst3HostContext (host), factory (pluginFactory)
    {
        jassert (pluginFactory != nullptr);
    }

    virtual ~DescriptionFactory() {}

    Result findDescriptionsAndPerform (const File& file)
    {
        StringArray foundNames;
        PFactoryInfo factoryInfo;
        factory->getFactoryInfo (&factoryInfo);
        const String companyName (toString (factoryInfo.vendor).trim());

        Result result (Result::ok());

        const Steinberg::int32 numClasses = factory->countClasses();
        for (Steinberg::int32 i = 0; i < numClasses; ++i)
        {
            PClassInfo info;
            factory->getClassInfo (i, &info);

            if (std::strcmp (info.category, kVstAudioEffectClass) != 0)
                continue;

            const String name (toString (info.name).trim());

            if (foundNames.contains (name, true))
                continue;

            ScopedPointer<PClassInfo2> info2;
            ScopedPointer<PClassInfoW> infoW;

            {
                ComSmartPtr<IPluginFactory2> pf2;
                ComSmartPtr<IPluginFactory3> pf3;

                if (pf2.loadFrom (factory))
                {
                    info2 = new PClassInfo2();
                    pf2->getClassInfo2 (i, info2);
                }

                if (pf3.loadFrom (factory))
                {
                    infoW = new PClassInfoW();
                    pf3->getClassInfoUnicode (i, infoW);
                }
            }

            foundNames.add (name);

            int numInputs = 0, numOutputs = 0;

            {
                ComSmartPtr<Vst::IComponent> component;

                if (component.loadFrom (factory, info.cid))
                {
                    if (component->initialize (vst3HostContext->getFUnknown()) == kResultOk)
                    {
                        numInputs  = getNumSingleDirectionChannelsFor (component, true, true);
                        numOutputs = getNumSingleDirectionChannelsFor (component, false, true);

                        component->terminate();
                    }
                    else
                    {
                        jassertfalse;
                    }
                }
            }

            PluginDescription desc;

            createPluginDescription (desc, file, companyName, name,
                                     info, info2, infoW, numInputs, numOutputs);

            result = performOnDescription (desc);

            if (result.failed())
                break;
        }

        return result;
    }

protected:
    virtual Result performOnDescription (PluginDescription& description) = 0;

private:
    ComSmartPtr<VST3HostContext> vst3HostContext;
    ComSmartPtr<IPluginFactory> factory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DescriptionFactory)
};

struct MatchingDescriptionFinder : public DescriptionFactory
{
    MatchingDescriptionFinder (VST3HostContext* host, IPluginFactory* pluginFactory, const PluginDescription& desc)
       : DescriptionFactory (host, pluginFactory), description (desc)
    {
    }

    static const char* getSuccessString() noexcept { return "Found Description"; }

    Result performOnDescription (PluginDescription& desc)
    {
        if (description.isDuplicateOf (desc))
            return Result::fail (getSuccessString());

        return Result::ok();
    }

private:
    const PluginDescription& description;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatchingDescriptionFinder)
};

struct DescriptionLister : public DescriptionFactory
{
    DescriptionLister (VST3HostContext* host, IPluginFactory* pluginFactory)
        : DescriptionFactory (host, pluginFactory)
    {
    }

    Result performOnDescription (PluginDescription& desc)
    {
        list.add (new PluginDescription (desc));
        return Result::ok();
    }

    OwnedArray<PluginDescription> list;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DescriptionLister)
};

//==============================================================================
struct DLLHandle
{
    DLLHandle (const String& modulePath)
    {
        if (modulePath.trim().isNotEmpty())
            open (modulePath);
    }

    ~DLLHandle()
    {
        typedef bool (PLUGIN_API *ExitModuleFn) ();

       #if JUCE_WINDOWS
        if (ExitModuleFn exitFn = (ExitModuleFn) getFunction ("ExitDll"))
            exitFn();

        library.close();

       #else
        if (bundleRef != nullptr)
        {
            if (ExitModuleFn exitFn = (ExitModuleFn) getFunction ("bundleExit"))
                exitFn();

            CFRelease (bundleRef);
            bundleRef = nullptr;
        }
       #endif
    }

    void open (const PluginDescription& description)
    {
       #if JUCE_WINDOWS
        jassert (description.fileOrIdentifier.isNotEmpty());
        jassert (File (description.fileOrIdentifier).existsAsFile());
        library.open (description.fileOrIdentifier);
       #else
        open (description.fileOrIdentifier);
       #endif
    }

    IPluginFactory* JUCE_CALLTYPE getPluginFactory()
    {
        if (GetFactoryProc proc = (GetFactoryProc) getFunction ("GetPluginFactory"))
            return proc();

        return nullptr;
    }

    void* getFunction (const char* functionName)
    {
       #if JUCE_WINDOWS
        return library.getFunction (functionName);
       #else
        if (bundleRef == nullptr)
            return nullptr;

        CFStringRef name = String (functionName).toCFString();
        void* fn = CFBundleGetFunctionPointerForName (bundleRef, name);
        CFRelease (name);
        return fn;
       #endif
    }

private:
   #if JUCE_WINDOWS
    DynamicLibrary library;

    bool open (const String& filePath)
    {
        if (library.open (filePath))
        {
            typedef bool (PLUGIN_API *InitModuleProc) ();
            if (InitModuleProc proc = (InitModuleProc) getFunction ("InitDll"))
            {
                if (proc())
                    return true;
            }
            else
            {
                return true;
            }

            library.close();
        }

        return false;
    }

   #else
    CFBundleRef bundleRef;

    bool open (const String& filePath)
    {
        const File file (filePath);
        const char* const utf8 = file.getFullPathName().toRawUTF8();

        if (CFURLRef url = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*) utf8, (CFIndex) std::strlen (utf8), file.isDirectory()))
        {
            bundleRef = CFBundleCreate (kCFAllocatorDefault, url);
            CFRelease (url);

            if (bundleRef != 0)
            {
                CFErrorRef error = 0;

                if (CFBundleLoadExecutableAndReturnError (bundleRef, &error))
                {
                    typedef bool (*BundleEntryProc)(CFBundleRef);

                    if (BundleEntryProc proc = (BundleEntryProc) getFunction ("bundleEntry"))
                    {
                        if (proc (bundleRef))
                            return true;
                    }
                    else
                    {
                        return true;
                    }
                }

                if (error != 0)
                {
                    if (CFStringRef failureMessage = CFErrorCopyFailureReason (error))
                    {
                        DBG (String::fromCFString (failureMessage));
                        CFRelease (failureMessage);
                    }

                    CFRelease (error);
                }

                CFRelease (bundleRef);
                bundleRef = 0;
            }
        }

        return false;
    }
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DLLHandle)
};

//==============================================================================
class VST3ModuleHandle  : public ReferenceCountedObject
{
public:
    explicit VST3ModuleHandle (const File& pluginFile)  : file (pluginFile)
    {
        getActiveModules().add (this);
    }

    ~VST3ModuleHandle()
    {
        getActiveModules().removeFirstMatchingValue (this);
    }

    /**
        Since there is no apparent indication if a VST3 plugin is a shell or not,
        we're stuck iterating through a VST3's factory, creating a description
        for every housed plugin.
    */
    static bool getAllDescriptionsForFile (OwnedArray<PluginDescription>& results,
                                           const String& fileOrIdentifier)
    {
        DLLHandle tempModule (fileOrIdentifier);

        ComSmartPtr<IPluginFactory> pluginFactory (tempModule.getPluginFactory());

        if (pluginFactory != nullptr)
        {
            ComSmartPtr<VST3HostContext> host (new VST3HostContext (nullptr));
            DescriptionLister lister (host, pluginFactory);
            const Result result (lister.findDescriptionsAndPerform (File (fileOrIdentifier)));

            results.addCopiesOf (lister.list);

            return result.wasOk();
        }

        return false;
    }

    //==============================================================================
    typedef ReferenceCountedObjectPtr<VST3ModuleHandle> Ptr;

    static VST3ModuleHandle::Ptr findOrCreateModule (const File& file, const PluginDescription& description)
    {
        Array<VST3ModuleHandle*>& activeModules = getActiveModules();

        for (int i = activeModules.size(); --i >= 0;)
        {
            VST3ModuleHandle* const module = activeModules.getUnchecked (i);

            // VST3s are basically shells, you must therefore check their name along with their file:
            if (module->file == file && module->name == description.name)
                return module;
        }

        VST3ModuleHandle::Ptr m (new VST3ModuleHandle (file));

        if (! m->open (file, description))
            m = nullptr;

        return m;
    }

    //==============================================================================
    IPluginFactory* getPluginFactory()      { return dllHandle->getPluginFactory(); }

    File file;
    String name;

private:
    ScopedPointer<DLLHandle> dllHandle;

    //==============================================================================
    static Array<VST3ModuleHandle*>& getActiveModules()
    {
        static Array<VST3ModuleHandle*> activeModules;
        return activeModules;
    }

    //==============================================================================
    bool open (const File& f, const PluginDescription& description)
    {
        dllHandle = new DLLHandle (f.getFullPathName());

        ComSmartPtr<IPluginFactory> pluginFactory (dllHandle->getPluginFactory());

        ComSmartPtr<VST3HostContext> host (new VST3HostContext (nullptr));
        MatchingDescriptionFinder finder (host, pluginFactory, description);

        const Result result (finder.findDescriptionsAndPerform (f));

        if (result.getErrorMessage() == MatchingDescriptionFinder::getSuccessString())
        {
            name = description.name;
            return true;
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3ModuleHandle)
};

//==============================================================================
class VST3PluginWindow : public AudioProcessorEditor,
                         public ComponentMovementWatcher,
                         public IPlugFrame
{
public:
    VST3PluginWindow (AudioProcessor* owner, IPlugView* pluginView)
      : AudioProcessorEditor (owner),
        ComponentMovementWatcher (this),
        view (pluginView),
        pluginHandle (nullptr),
        recursiveResize (false)
    {
        setSize (10, 10);
        setOpaque (true);
        setVisible (true);

        ViewRect rect;
        warnOnFailure (view->getSize (&rect));
        resizeWithRect (*this, rect);

        view->setFrame (this); // Done after to avoid recursive calls from plugins...
    }

    ~VST3PluginWindow()
    {
        view->removed();
        getAudioProcessor()->editorBeingDeleted (this);

       #if JUCE_MAC
        dummyComponent.setView (nullptr);
        [pluginHandle release];
       #endif
    }

    JUCE_DECLARE_VST3_COM_REF_METHODS
    JUCE_DECLARE_VST3_COM_QUERY_METHODS

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        view->onWheel (wheel.deltaY);
    }

    void focusGained (FocusChangeType) override { view->onFocus (true); }
    void focusLost (FocusChangeType) override   { view->onFocus (false); }

    /** It seems that most, if not all, plugins do their own keyboard hooks,
        but IPlugView does have a set of keyboard related methods...
    */
    bool keyStateChanged (bool /*isKeyDown*/) override { return true; }
    bool keyPressed (const KeyPress& /*key*/) override { return true; }

    //==============================================================================
    void componentMovedOrResized (bool, bool wasResized) override
    {
        if (recursiveResize)
            return;

        Component* const topComp = getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
           #if JUCE_WINDOWS
            const Point<int> pos (topComp->getLocalPoint (this, Point<int>()));
           #endif

            recursiveResize = true;

            ViewRect rect;

            if (wasResized && view->canResize() == kResultTrue)
            {
                rect.right  = (Steinberg::int32) getWidth();
                rect.bottom = (Steinberg::int32) getHeight();
                view->onSize (&rect);
            }
            else
            {
                view->getSize (&rect);
            }

           #if JUCE_WINDOWS
            SetWindowPos (pluginHandle, 0,
                          pos.x, pos.y, rect.getWidth(), rect.getHeight(),
                          isVisible() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
           #elif JUCE_MAC
            dummyComponent.setBounds (0, 0, (int) rect.getWidth(), (int) rect.getHeight());
           #endif

            Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate(); // Some plugins don't update their cursor correctly when mousing out the window

            recursiveResize = false;
        }
    }

    void componentPeerChanged() override { }

    void componentVisibilityChanged() override
    {
        attachPluginWindow();
        componentMovedOrResized (true, true);
    }

    tresult PLUGIN_API resizeView (IPlugView* incomingView, ViewRect* newSize) override
    {
        if (incomingView != nullptr
             && newSize != nullptr
             && incomingView == view)
        {
            resizeWithRect (dummyComponent, *newSize);
            setSize (dummyComponent.getWidth(), dummyComponent.getHeight());
            return kResultTrue;
        }

        jassertfalse;
        return kInvalidArgument;
    }

private:
    //==============================================================================
    Atomic<int> refCount;
    ComSmartPtr<IPlugView> view;

   #if JUCE_WINDOWS
    class ChildComponent  : public Component
    {
    public:
        ChildComponent() {}
        void paint (Graphics& g) override  { g.fillAll (Colours::cornflowerblue); }

        using Component::createNewPeer;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildComponent)
    };

    ChildComponent dummyComponent;
    ScopedPointer<ComponentPeer> peer;
    typedef HWND HandleFormat;
   #elif JUCE_MAC
    NSViewComponent dummyComponent;
    typedef NSView* HandleFormat;
   #else
    Component dummyComponent;
    typedef void* HandleFormat;
   #endif

    HandleFormat pluginHandle; //< Don't delete this
    bool recursiveResize;

    //==============================================================================
    static void resizeWithRect (Component& comp, const ViewRect& rect)
    {
        comp.setBounds (rect.left, rect.top,
                        jmax (10, std::abs ((int) rect.getWidth())),
                        jmax (10, std::abs ((int) rect.getHeight())));
    }

    void attachPluginWindow()
    {
        if (pluginHandle == nullptr)
        {
           #if JUCE_WINDOWS
            if (Component* topComp = getTopLevelComponent())
                peer = dummyComponent.createNewPeer (0, topComp->getWindowHandle());
            else
                peer = nullptr;

            if (peer != nullptr)
                pluginHandle = (HandleFormat) peer->getNativeHandle();
           #elif JUCE_MAC
            dummyComponent.setBounds (getBounds().withZeroOrigin());
            addAndMakeVisible (dummyComponent);
            pluginHandle = [[NSView alloc] init];
            dummyComponent.setView (pluginHandle);
           #endif

            if (pluginHandle != nullptr)
                view->attached (pluginHandle,
                               #if JUCE_WINDOWS
                                kPlatformTypeHWND);
                               #else
                                kPlatformTypeNSView);
                               #endif
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginWindow)
};

//==============================================================================
class VST3PluginInstance : public AudioPluginInstance
{
public:
    VST3PluginInstance (const VST3ModuleHandle::Ptr& handle)
      : module (handle),
        numInputAudioBusses (0),
        numOutputAudioBusses (0),
        resultBuffer (1, 1),
        inputParameterChanges (new ParameterChangeList()),
        outputParameterChanges (new ParameterChangeList()),
        midiInputs (new MidiEventList()),
        midiOutputs (new MidiEventList()),
        isComponentInitialised (false),
        isControllerInitialised (false)
    {
        host = new VST3HostContext (this);
    }

    ~VST3PluginInstance()
    {
        jassert (getActiveEditor() == nullptr); // You must delete any editors before deleting the plugin instance!

        releaseResources();

        if (editControllerConnection != nullptr && componentConnection != nullptr)
        {
            editControllerConnection->disconnect (componentConnection);
            componentConnection->disconnect (editControllerConnection);
        }

        editController->setComponentHandler (nullptr);

        if (isControllerInitialised)    editController->terminate();
        if (isComponentInitialised)     component->terminate();

        editController = nullptr;
        component = nullptr;
    }

    bool initialise()
    {
       #if JUCE_WINDOWS
        // On Windows it's highly advisable to create your plugins using the message thread,
        // because many plugins need a chance to create HWNDs that will get their messages
        // delivered by the main message thread, and that's not possible from a background thread.
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
       #endif

        ComSmartPtr<IPluginFactory> factory (module->getPluginFactory());

        PFactoryInfo factoryInfo;
        factory->getFactoryInfo (&factoryInfo);
        company = toString (factoryInfo.vendor).trim();

        if (! fetchComponentAndController (factory, factory->countClasses()))
            return false;

        if (warnOnFailure (editController->initialize (host->getFUnknown())) != kResultTrue)
            return false;

        isControllerInitialised = true;
        editController->setComponentHandler (host);
        grabInformationObjects();
        synchroniseStates();
        interconnectComponentAndController();
        setupIO();
        return true;
    }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& description) const override
    {
        jassert (module != nullptr);

        createPluginDescription (description, module->file,
                                 company, module->name,
                                 *info, info2, infoW,
                                 getNumInputChannels(),
                                 getNumOutputChannels());
    }

    void* getPlatformSpecificData() override   { return component; }
    void refreshParameterList() override {}

    //==============================================================================
    const String getName() const override
    {
        return module != nullptr ? module->name : String::empty;
    }

    typedef Array<Array<float*> > BusMap;

    /** Assigns a series of AudioSampleBuffer's channels to an AudioBusBuffers'

        @warning For speed, does not check the channel count and offsets
                 according to the AudioSampleBuffer
    */
    void associateBufferTo (Vst::AudioBusBuffers& vstBuffers,
                            BusMap& busMap,
                            const AudioSampleBuffer& buffer,
                            int numChannels, int channelStartOffset,
                            int sampleOffset = 0) noexcept
    {
        const int channelEnd = numChannels + channelStartOffset;
        jassert (channelEnd >= 0 && channelEnd <= buffer.getNumChannels());

        busMap.add (Array<float*>());
        Array<float*>& chans = busMap.getReference (busMap.size() - 1);

        for (int i = channelStartOffset; i < channelEnd; ++i)
            chans.add (buffer.getSampleData (i, sampleOffset));

        vstBuffers.channelBuffers32 = chans.getRawDataPointer();
        vstBuffers.numChannels      = numChannels;
        vstBuffers.silenceFlags     = 0;
    }

    void mapAudioSampleBufferToBusses (Array<Vst::AudioBusBuffers>& result,
                                       AudioSampleBuffer& source,
                                       int numBusses, bool isInput)
    {
        result.clearQuick();

        BusMap& busMapToUse = isInput ? inputBusMap : outputBusMap;
        busMapToUse.clearQuick();

        int channelIndexOffset = 0;

        for (int i = 0; i < numBusses; ++i)
        {
            Vst::SpeakerArrangement arrangement = 0;
            processor->getBusArrangement (isInput ? Vst::kInput : Vst::kOutput,
                                          (Steinberg::int32) i, arrangement);

            const int numChansForBus = BigInteger ((int64) arrangement).countNumberOfSetBits();

            result.add (Vst::AudioBusBuffers());

            associateBufferTo (result.getReference (i), busMapToUse, source,
                               BigInteger ((int64) arrangement).countNumberOfSetBits(),
                               channelIndexOffset);

            channelIndexOffset += numChansForBus;
        }
    }

    void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock) override
    {
        using namespace Vst;

        const int numInputs = getNumInputChannels();
        const int numOutputs = getNumOutputChannels();

        // Needed for having the same sample rate in processBlock(); some plugins need this!
        setPlayConfigDetails (numInputs, numOutputs, sampleRate, estimatedSamplesPerBlock);

        ProcessSetup setup;
        setup.symbolicSampleSize    = kSample32;
        setup.maxSamplesPerBlock    = estimatedSamplesPerBlock;
        setup.sampleRate            = sampleRate;
        setup.processMode           = isNonRealtime() ? kOffline : kRealtime;

        resultBuffer.setSize (numOutputs, estimatedSamplesPerBlock, false, true, true);

        warnOnFailure (processor->setupProcessing (setup));

        if (! isComponentInitialised)
            isComponentInitialised = component->initialize (host->getFUnknown()) == kResultTrue;

        editController->setComponentHandler (host);

        warnOnFailure (component->setActive (true));
        warnOnFailure (processor->setProcessing (true));

        Array<SpeakerArrangement> inArrangements, outArrangements;

        fillWithCorrespondingSpeakerArrangements (inArrangements, numInputs);
        fillWithCorrespondingSpeakerArrangements (outArrangements, numOutputs);

        warnOnFailure (processor->setBusArrangements (inArrangements.getRawDataPointer(), numInputAudioBusses,
                                                      outArrangements.getRawDataPointer(), numOutputAudioBusses));
    }

    void releaseResources() override
    {
        JUCE_TRY
        {
            resultBuffer.setSize (1, 1, false, true, true);

            if (processor != nullptr)
                processor->setProcessing (false);

            if (component != nullptr)
                component->setActive (false);
        }
        JUCE_CATCH_ALL_ASSERT
    }

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override
    {
        using namespace Vst;

        if (processor != nullptr
             && processor->canProcessSampleSize (kSample32) == kResultTrue)
        {
            const int numSamples = buffer.getNumSamples();

            ProcessData data;
            data.processMode            = isNonRealtime() ? kOffline : kRealtime;
            data.symbolicSampleSize     = kSample32;
            data.numInputs              = numInputAudioBusses;
            data.numOutputs             = numOutputAudioBusses;
            data.inputParameterChanges  = inputParameterChanges;
            data.outputParameterChanges = outputParameterChanges;
            data.numSamples             = (Steinberg::int32) numSamples;

            updateTimingInformation (data, getSampleRate());

            for (int i = getNumInputChannels(); i < buffer.getNumChannels(); ++i)
                buffer.clear (i, 0, numSamples);

            associateTo (data, buffer);
            associateTo (data, midiMessages);

            processor->process (data);

            MidiEventList::toMidiBuffer (midiMessages, *midiOutputs);
        }
    }

    //==============================================================================
    String getChannelName (int channelIndex, bool forInput, bool forAudioChannel) const
    {
        const int numBusses = getNumSingleDirectionBussesFor (component, forInput, forAudioChannel);
        int numCountedChannels = 0;

        for (int i = 0; i < numBusses; ++i)
        {
            Vst::BusInfo busInfo (getBusInfo (forInput, forAudioChannel, i));

            numCountedChannels += busInfo.channelCount;

            if (channelIndex < numCountedChannels)
                return toString (busInfo.name);
        }

        return String::empty;
    }

    const String getInputChannelName  (int channelIndex) const override   { return getChannelName (channelIndex, true, true); }
    const String getOutputChannelName (int channelIndex) const override   { return getChannelName (channelIndex, false, true); }

    bool isInputChannelStereoPair (int channelIndex) const override
    {
        if (channelIndex < 0 || channelIndex >= getNumInputChannels())
            return false;

        return getBusInfo (true, true).channelCount == 2;
    }

    bool isOutputChannelStereoPair (int channelIndex) const override
    {
        if (channelIndex < 0 || channelIndex >= getNumOutputChannels())
            return false;

        return getBusInfo (false, true).channelCount == 2;
    }

    bool acceptsMidi() const override    { return getBusInfo (true,  false).channelCount > 0; }
    bool producesMidi() const override   { return getBusInfo (false, false).channelCount > 0; }

    //==============================================================================
    bool silenceInProducesSilenceOut() const override
    {
        return processor == nullptr;
    }

    /** May return a negative value as a means of informing us that the plugin has "infinite tail," or 0 for "no tail." */
    double getTailLengthSeconds() const override
    {
        if (processor != nullptr)
            return (double) jmin ((int) jmax ((Steinberg::uint32) 0, processor->getTailSamples()), 0x7fffffff)
                   * getSampleRate();

        return 0.0;
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override
    {
        if (view == nullptr)
            view = tryCreatingView();

        if (view != nullptr)
            return new VST3PluginWindow (this, view);

        return nullptr;
    }

    bool hasEditor() const override
    {
        if (view == nullptr)
            view = tryCreatingView();

        return view != nullptr;
    }

    //==============================================================================
    int getNumParameters() override
    {
        if (editController != nullptr)
            return (int) editController->getParameterCount();

        return 0;
    }

    const String getParameterName (int parameterIndex) override
    {
        return toString (getParameterInfoForIndex (parameterIndex).title);
    }

    float getParameter (int parameterIndex) override
    {
        if (editController != nullptr)
        {
            const uint32 id = getParameterInfoForIndex (parameterIndex).id;
            return (float) editController->getParamNormalized (id);
        }

        return 0.0f;
    }

    const String getParameterText (int parameterIndex) override
    {
        if (editController != nullptr)
        {
            const uint32 id = getParameterInfoForIndex (parameterIndex).id;

            Vst::String128 result;
            warnOnFailure (editController->getParamStringByValue (id, editController->getParamNormalized (id), result));

            return toString (result);
        }

        return String::empty;
    }

    void setParameter (int parameterIndex, float newValue) override
    {
        if (editController != nullptr)
        {
            const uint32 id = getParameterInfoForIndex (parameterIndex).id;
            editController->setParamNormalized (id, (double) newValue);
        }
    }

    //==============================================================================
    int getNumPrograms() override                        { return getProgramListInfo (0).programCount; }
    int getCurrentProgram() override                     { return 0; }
    void setCurrentProgram (int) override                {}
    void changeProgramName (int, const String&) override {}

    const String getProgramName (int index) override
    {
        Vst::String128 result;
        unitInfo->getProgramName (getProgramListInfo (0).id, index, result);
        return toString (result);
    }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        XmlElement state ("VST3PluginState");

        appendStateFrom (state, component, "IComponent");
        appendStateFrom (state, editController, "IEditController");

        AudioProcessor::copyXmlToBinary (state, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        ScopedPointer<XmlElement> head (AudioProcessor::getXmlFromBinary (data, sizeInBytes));

        if (head != nullptr)
        {
            ScopedPointer<Steinberg::MemoryStream> s (createMemoryStreamForState (*head, "IComponent"));

            if (s != nullptr && component != nullptr)
                component->setState (s);

            if (editController != nullptr)
            {
                if (s != nullptr)
                    editController->setComponentState (s);

                s = createMemoryStreamForState (*head, "IEditController");

                if (s != nullptr)
                    editController->setState (s);
            }
        }
    }

    /** @note Not applicable to VST3 */
    void getCurrentProgramStateInformation (MemoryBlock& destData) override
    {
        destData.setSize (0, true);
    }

    /** @note Not applicable to VST3 */
    void setCurrentProgramStateInformation (const void* data, int sizeInBytes) override
    {
        (void) data;
        (void) sizeInBytes;
    }

    ComSmartPtr<Vst::IEditController> editController;

private:
    //==============================================================================
    VST3ModuleHandle::Ptr module;
    ComSmartPtr<VST3HostContext> host;

    // Information objects:
    String company;
    ScopedPointer<PClassInfo> info;
    ScopedPointer<PClassInfo2> info2;
    ScopedPointer<PClassInfoW> infoW;

    // Rudimentary interfaces:
    ComSmartPtr<Vst::IAudioProcessor> processor;
    ComSmartPtr<Vst::IComponent> component;
    ComSmartPtr<Vst::IComponentHandler> componentHandler;
    ComSmartPtr<Vst::IComponentHandler2> componentHandler2;
    ComSmartPtr<Vst::IEditController2> editController2;
    ComSmartPtr<Vst::IUnitInfo> unitInfo;
    ComSmartPtr<Vst::IProgramListData> programListData;
    ComSmartPtr<Vst::IUnitData> unitData;
    ComSmartPtr<Vst::IConnectionPoint> componentConnection;
    ComSmartPtr<Vst::IConnectionPoint> editControllerConnection;

    mutable ComSmartPtr<IPlugView> view;

    /** The number of IO busses MUST match that of the plugin,
        even if there aren't enough channels to process,
        as very poorly specified by the Steinberg SDK
    */
    int numInputAudioBusses, numOutputAudioBusses;
    AudioSampleBuffer resultBuffer;
    BusMap inputBusMap, outputBusMap;
    Array<Vst::AudioBusBuffers> inputBusses, outputBusses;

    //==============================================================================
    template <typename Type>
    static void appendStateFrom (XmlElement& head, ComSmartPtr<Type>& object, const String& identifier)
    {
        if (object != nullptr)
        {
            Steinberg::MemoryStream stream;

            if (object->getState (&stream) == kResultTrue)
            {
                MemoryBlock info (stream.getData(), (std::size_t) stream.getSize());
                head.createNewChildElement (identifier)->addTextElement (info.toBase64Encoding());
            }
        }
    }

    static Steinberg::MemoryStream* createMemoryStreamForState (XmlElement& head, StringRef identifier)
    {
        Steinberg::MemoryStream* stream = nullptr;

        if (XmlElement* const state = head.getChildByName (identifier))
        {
            MemoryBlock mem;

            if (mem.fromBase64Encoding (state->getAllSubText()))
                stream = new Steinberg::MemoryStream (mem.getData(), (TSize) mem.getSize());
        }

        return stream;
    }

    //==============================================================================
    class ParameterChangeList  : public Vst::IParameterChanges
    {
    public:
        ParameterChangeList() {}
        virtual ~ParameterChangeList() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        Steinberg::int32 PLUGIN_API getParameterCount()   { return 0; }

        Vst::IParamValueQueue* PLUGIN_API getParameterData (Steinberg::int32)
        {
            return nullptr;
        }

        Vst::IParamValueQueue* PLUGIN_API addParameterData (const Vst::ParamID&, Steinberg::int32& index)
        {
            index = 0;
            return nullptr;
        }

    private:
        Atomic<int32> refCount;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterChangeList)
    };

    ComSmartPtr<ParameterChangeList> inputParameterChanges, outputParameterChanges;
    ComSmartPtr<MidiEventList> midiInputs, midiOutputs;
    Vst::ProcessContext timingInfo; //< Only use this in processBlock()!
    bool isComponentInitialised, isControllerInitialised;

    //==============================================================================
    bool fetchComponentAndController (IPluginFactory* factory, const Steinberg::int32 numClasses)
    {
        jassert (numClasses >= 0); // The plugin must provide at least an IComponent and IEditController!

        for (Steinberg::int32 j = 0; j < numClasses; ++j)
        {
            info = new PClassInfo();
            factory->getClassInfo (j, info);

            if (std::strcmp (info->category, kVstAudioEffectClass) != 0)
                continue;

            const String name (toString (info->name).trim());

            if (module->name != name)
                continue;

            {
                ComSmartPtr<IPluginFactory2> pf2;
                ComSmartPtr<IPluginFactory3> pf3;

                if (pf2.loadFrom (factory))
                {
                    info2 = new PClassInfo2();
                    pf2->getClassInfo2 (j, info2);
                }
                else
                {
                    info2 = nullptr;
                }

                if (pf3.loadFrom (factory))
                {
                    pf3->setHostContext (host->getFUnknown());
                    infoW = new PClassInfoW();
                    pf3->getClassInfoUnicode (j, infoW);
                }
                else
                {
                    infoW = nullptr;
                }
            }

            bool failed = true;

            if (component.loadFrom (factory, info->cid) && component != nullptr)
            {
                warnOnFailure (component->setIoMode (isNonRealtime() ? Vst::kOffline : Vst::kRealtime));

                if (warnOnFailure (component->initialize (host->getFUnknown())) != kResultOk)
                    return false;

                isComponentInitialised = true;

                // Get the IEditController:
                TUID controllerCID = { 0 };

                if (component->getControllerClassId (controllerCID) == kResultTrue && FUID (controllerCID).isValid())
                    editController.loadFrom (factory, controllerCID);

                if (editController == nullptr)
                {
                    // Try finding the IEditController the long way around:
                    for (Steinberg::int32 i = 0; i < numClasses; ++i)
                    {
                        PClassInfo classInfo;
                        factory->getClassInfo (i, &classInfo);

                        if (std::strcmp (classInfo.category, kVstComponentControllerClass) == 0)
                            editController.loadFrom (factory, classInfo.cid);
                    }
                }

                if (editController == nullptr)
                    editController.loadFrom (component);

                failed = editController == nullptr;
            }

            if (failed)
            {
                jassertfalse; // The plugin won't function without a valid IComponent and IEditController implementation!

                if (component != nullptr)
                {
                    component->terminate();
                    component = nullptr;
                }

                if (editController != nullptr)
                {
                    editController->terminate();
                    editController = nullptr;
                }

                break;
            }

            return true;
        }

        return false;
    }

    /** Some plugins need to be "connected" to intercommunicate between their implemented classes */
    void interconnectComponentAndController()
    {
        componentConnection.loadFrom (component);
        editControllerConnection.loadFrom (editController);

        if (componentConnection != nullptr && editControllerConnection != nullptr)
        {
            warnOnFailure (editControllerConnection->connect (componentConnection));
            warnOnFailure (componentConnection->connect (editControllerConnection));
        }
    }

    void synchroniseStates()
    {
        Steinberg::MemoryStream stream;

        if (component->getState (&stream) == kResultTrue)
            if (stream.seek (0, Steinberg::IBStream::kIBSeekSet, nullptr) == kResultTrue)
                warnOnFailure (editController->setComponentState (&stream));
    }

    void grabInformationObjects()
    {
        processor.loadFrom (component);
        unitInfo.loadFrom (component);
        programListData.loadFrom (component);
        unitData.loadFrom (component);
        editController2.loadFrom (component);
        componentHandler.loadFrom (component);
        componentHandler2.loadFrom (component);

        if (processor == nullptr)           processor.loadFrom (editController);
        if (unitInfo == nullptr)            unitInfo.loadFrom (editController);
        if (programListData == nullptr)     programListData.loadFrom (editController);
        if (unitData == nullptr)            unitData.loadFrom (editController);
        if (editController2 == nullptr)     editController2.loadFrom (editController);
        if (componentHandler == nullptr)    componentHandler.loadFrom (editController);
        if (componentHandler2 == nullptr)   componentHandler2.loadFrom (editController);
    }

    void setupIO()
    {
        activateAllBussesOfType (component, true, true);    // Activate audio inputs
        activateAllBussesOfType (component, false, true);   // Activate audio outputs
        activateAllBussesOfType (component, true, false);   // Activate MIDI inputs
        activateAllBussesOfType (component, false, false);  // Activate MIDI outputs

        Vst::ProcessSetup setup;
        setup.symbolicSampleSize   = Vst::kSample32;
        setup.maxSamplesPerBlock   = 1024;
        setup.sampleRate           = 44100.0;
        setup.processMode          = Vst::kRealtime;

        warnOnFailure (processor->setupProcessing (setup));

        numInputAudioBusses = getNumSingleDirectionBussesFor (component, true, true);
        numOutputAudioBusses = getNumSingleDirectionBussesFor (component, false, true);

        setPlayConfigDetails (getNumSingleDirectionChannelsFor (component, true, true),
                              getNumSingleDirectionChannelsFor (component, false, true),
                              setup.sampleRate, (int) setup.maxSamplesPerBlock);
    }

    //==============================================================================
    Vst::BusInfo getBusInfo (bool forInput, bool forAudio, int index = 0) const
    {
        Vst::BusInfo busInfo;

        component->getBusInfo (forAudio ? Vst::kAudio : Vst::kEvent,
                               forInput ? Vst::kInput : Vst::kOutput,
                               (Steinberg::int32) index, busInfo);

        return busInfo;
    }

    //==============================================================================
    ComSmartPtr<IPlugView> tryCreatingView() const
    {
        ComSmartPtr<IPlugView> v (editController->createView (Vst::ViewType::kEditor));

        if (v == nullptr)
            v = editController->createView (nullptr);

        if (v == nullptr)
            v.loadFrom (editController);

        return v;
    }

    //==============================================================================
    void associateTo (Vst::ProcessData& destination, AudioSampleBuffer& buffer)
    {
        resultBuffer.clear();

        mapAudioSampleBufferToBusses (inputBusses, buffer, numInputAudioBusses, true);
        mapAudioSampleBufferToBusses (outputBusses, resultBuffer, numOutputAudioBusses, false);

        destination.inputs  = inputBusses.getRawDataPointer();
        destination.outputs = outputBusses.getRawDataPointer();
    }

    void associateTo (Vst::ProcessData& destination, MidiBuffer& midiBuffer)
    {
        midiInputs->clear();
        midiOutputs->clear();

        MidiEventList::toEventList (*midiInputs, midiBuffer);

        destination.inputEvents = midiInputs;
        destination.outputEvents = midiOutputs;
    }

    void updateTimingInformation (Vst::ProcessData& destination, double processSampleRate)
    {
        toProcessContext (timingInfo, getPlayHead(), processSampleRate);
        destination.processContext = &timingInfo;
    }

    Vst::ParameterInfo getParameterInfoForIndex (int index) const
    {
        Vst::ParameterInfo paramInfo = { 0 };

        if (processor != nullptr)
            editController->getParameterInfo (index, paramInfo);

        return paramInfo;
    }

    Vst::ProgramListInfo getProgramListInfo (int index) const
    {
        Vst::ProgramListInfo paramInfo = { 0 };

        if (unitInfo != nullptr)
            unitInfo->getProgramListInfo (index, paramInfo);

        return paramInfo;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginInstance)
};

};

//==============================================================================
VST3PluginFormat::VST3PluginFormat() {}
VST3PluginFormat::~VST3PluginFormat() {}

void VST3PluginFormat::findAllTypesForFile (OwnedArray<PluginDescription>& results, const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    VST3Classes::VST3ModuleHandle::getAllDescriptionsForFile (results, fileOrIdentifier);
}

AudioPluginInstance* VST3PluginFormat::createInstanceFromDescription (const PluginDescription& description, double, int)
{
    ScopedPointer<VST3Classes::VST3PluginInstance> result;

    if (fileMightContainThisPluginType (description.fileOrIdentifier))
    {
        File file (description.fileOrIdentifier);

        const File previousWorkingDirectory (File::getCurrentWorkingDirectory());
        file.getParentDirectory().setAsCurrentWorkingDirectory();

        if (const VST3Classes::VST3ModuleHandle::Ptr module = VST3Classes::VST3ModuleHandle::findOrCreateModule (file, description))
        {
            result = new VST3Classes::VST3PluginInstance (module);

            if (! result->initialise())
                result = nullptr;
        }

        previousWorkingDirectory.setAsCurrentWorkingDirectory();
    }

    return result.release();
}

bool VST3PluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    const File f (fileOrIdentifier);

    return f.hasFileExtension (".vst3")
          #if JUCE_MAC
           && f.exists();
          #else
           && f.existsAsFile();
          #endif
}

String VST3PluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier; //Impossible to tell because every VST3 is a type of shell...
}

bool VST3PluginFormat::pluginNeedsRescanning (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).getLastModificationTime() != description.lastFileModTime;
}

bool VST3PluginFormat::doesPluginStillExist (const PluginDescription& description)
{
    return File (description.fileOrIdentifier).exists();
}

StringArray VST3PluginFormat::searchPathsForPlugins (const FileSearchPath& directoriesToSearch, const bool recursive)
{
    StringArray results;

    for (int i = 0; i < directoriesToSearch.getNumPaths(); ++i)
        recursiveFileSearch (results, directoriesToSearch[i], recursive);

    return results;
}

void VST3PluginFormat::recursiveFileSearch (StringArray& results, const File& directory, const bool recursive)
{
    DirectoryIterator iter (directory, false, "*", File::findFilesAndDirectories);

    while (iter.next())
    {
        const File f (iter.getFile());
        bool isPlugin = false;

        if (fileMightContainThisPluginType (f.getFullPathName()))
        {
            isPlugin = true;
            results.add (f.getFullPathName());
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (results, f, true);
    }
}

FileSearchPath VST3PluginFormat::getDefaultLocationsToSearch()
{
   #if JUCE_WINDOWS
    const String programFiles (File::getSpecialLocation (File::globalApplicationsDirectory).getFullPathName());
    return FileSearchPath (programFiles + "\\Common Files\\VST3");
   #elif JUCE_MAC
    return FileSearchPath ("~/Library/Audio/Plug-Ins/VST3;/Library/Audio/Plug-Ins/VST3");
   #else
    return FileSearchPath();
   #endif
}

#endif //JUCE_PLUGINHOST_VST3
