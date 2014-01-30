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

#ifndef JUCE_VST3COMMON_H_INCLUDED
#define JUCE_VST3COMMON_H_INCLUDED

//==============================================================================
#define JUCE_DECLARE_VST3_COM_REF_METHODS \
    Steinberg::uint32 JUCE_CALLTYPE addRef() override   { return (Steinberg::uint32) ++refCount; } \
    Steinberg::uint32 JUCE_CALLTYPE release() override  { const int r = --refCount; if (r == 0) delete this; return (Steinberg::uint32) r; }

#define JUCE_DECLARE_VST3_COM_QUERY_METHODS \
    Steinberg::tresult PLUGIN_API JUCE_CALLTYPE queryInterface (const Steinberg::TUID, void** obj) override \
    { \
        jassertfalse; \
        *obj = nullptr; \
        return Steinberg::kNotImplemented; \
    }

static bool doUIDsMatch (const Steinberg::TUID a, const Steinberg::TUID b) noexcept
{
    return std::memcmp (a, b, sizeof (Steinberg::TUID)) == 0;
}

#define TEST_FOR_AND_RETURN_IF_VALID(ClassType) \
    if (doUIDsMatch (iid, ClassType::iid)) \
    { \
        addRef(); \
        *obj = dynamic_cast<ClassType*> (this); \
        return Steinberg::kResultOk; \
    }

//==============================================================================
static juce::String toString (const Steinberg::char8* string) noexcept      { return juce::String (string); }
static juce::String toString (const Steinberg::char16* string) noexcept     { return juce::String (juce::CharPointer_UTF16 ((juce::CharPointer_UTF16::CharType*) string)); }

// NB: The casts are handled by a Steinberg::UString operator
static juce::String toString (const Steinberg::UString128& string) noexcept { return toString (static_cast<const Steinberg::char16*> (string)); }
static juce::String toString (const Steinberg::UString256& string) noexcept { return toString (static_cast<const Steinberg::char16*> (string)); }

static void toString (Steinberg::Vst::String128 result, const juce::String& source)
{
    Steinberg::UString (result, 128).fromAscii (source.toUTF8());
}

static Steinberg::Vst::TChar* toString (const juce::String& source) noexcept
{
    return reinterpret_cast<Steinberg::Vst::TChar*> (source.toUTF16().getAddress());
}

//==============================================================================
/** The equivalent numChannels and speaker arrangements should always
    match between this function and fillWithCorrespondingSpeakerArrangements().

    There can only be 1 arrangement per channel count. (i.e.: 4 channels == k31Cine OR k40Cine)

    @see fillWithCorrespondingSpeakerArrangements
*/
static Steinberg::Vst::SpeakerArrangement getArrangementForNumChannels (int numChannels) noexcept
{
    using namespace Steinberg::Vst::SpeakerArr;

    if (numChannels >= 24)  return k222;
    if (numChannels >= 14)  return k131;
    if (numChannels >= 13)  return k130;
    if (numChannels >= 12)  return k111;
    if (numChannels >= 11)  return k101;
    if (numChannels >= 10)  return k91;
    if (numChannels >= 9)   return k90;
    if (numChannels >= 8)   return k71CineFullFront;
    if (numChannels >= 7)   return k61Cine;
    if (numChannels >= 6)   return k51;
    if (numChannels >= 5)   return k50;
    if (numChannels >= 4)   return k31Cine;
    if (numChannels >= 3)   return k30Cine;
    if (numChannels >= 2)   return kStereo;
    if (numChannels >= 1)   return kMono;

    return kEmpty;
}

/** The equivalent numChannels and speaker arrangements should always
    match between this function and getArrangementForNumChannels().

    There can only be 1 arrangement per channel count. (i.e.: 4 channels == k31Cine OR k40Cine)

    @see getArrangementForNumChannels
*/
static void fillWithCorrespondingSpeakerArrangements (Array<Steinberg::Vst::SpeakerArrangement>& destination,
                                                      int numChannels)
{
    using namespace Steinberg::Vst::SpeakerArr;

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
    if (numChannels >= 24)  destination.add (k222);
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

    bool loadFrom (Steinberg::FUnknown* o)
    {
        *this = nullptr;
        return o != nullptr && o->queryInterface (ObjectType::iid, (void**) &source) == Steinberg::kResultOk;
    }

    bool loadFrom (Steinberg::IPluginFactory* factory, const Steinberg::TUID& uuid)
    {
        jassert (factory != nullptr);
        *this = nullptr;
        return factory->createInstance (uuid, ObjectType::iid, (void**) &source) == Steinberg::kResultOk;
    }

private:
    ObjectType* source;
};

//==============================================================================
class MidiEventList  : public Steinberg::Vst::IEventList
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
    Steinberg::tresult PLUGIN_API getEvent (Steinberg::int32 index, Steinberg::Vst::Event& e) override
    {
        if (isPositiveAndBelow ((int) index, events.size()))
        {
            e = events.getReference ((int) index);
            return Steinberg::kResultTrue;
        }

        return Steinberg::kResultFalse;
    }

    Steinberg::tresult PLUGIN_API addEvent (Steinberg::Vst::Event& e) override
    {
        events.add (e);
        return Steinberg::kResultTrue;
    }

    //==============================================================================
    static void toMidiBuffer (MidiBuffer& result, Steinberg::Vst::IEventList& eventList)
    {
        for (Steinberg::int32 i = 0; i < eventList.getEventCount(); ++i)
        {
            Steinberg::Vst::Event e;

            if (eventList.getEvent (i, e) == Steinberg::kResultOk)
            {
                switch (e.type)
                {
                    case Steinberg::Vst::Event::kNoteOnEvent:
                        result.addEvent (MidiMessage::noteOn (createSafeChannel (e.noteOn.channel),
                                                              createSafeNote (e.noteOn.pitch),
                                                              (Steinberg::uint8) denormaliseToMidiValue (e.noteOn.velocity)),
                                         e.sampleOffset);
                        break;

                    case Steinberg::Vst::Event::kNoteOffEvent:
                        result.addEvent (MidiMessage::noteOff (createSafeChannel (e.noteOff.channel),
                                                               createSafeNote (e.noteOff.pitch),
                                                               (Steinberg::uint8) denormaliseToMidiValue (e.noteOff.velocity)),
                                         e.sampleOffset);
                        break;

                    case Steinberg::Vst::Event::kPolyPressureEvent:
                        result.addEvent (MidiMessage::aftertouchChange (createSafeChannel (e.polyPressure.channel),
                                                                        createSafeNote (e.polyPressure.pitch),
                                                                        denormaliseToMidiValue (e.polyPressure.pressure)),
                                         e.sampleOffset);
                        break;

                    case Steinberg::Vst::Event::kDataEvent:
                        result.addEvent (MidiMessage::createSysExMessage (e.data.bytes, e.data.size),
                                         e.sampleOffset);
                        break;

                    default:
                        break;
                }
            }
        }
    }

    static void toEventList (Steinberg::Vst::IEventList& result, MidiBuffer& midiBuffer)
    {
        MidiBuffer::Iterator iterator (midiBuffer);
        MidiMessage msg;
        int midiEventPosition = 0;

        enum { maxNumEvents = 2048 }; // Steinberg's Host Checker states that no more than 2048 events are allowed at once
        int numEvents = 0;

        while (iterator.getNextEvent (msg, midiEventPosition))
        {
            if (++numEvents > maxNumEvents)
                break;

            Steinberg::Vst::Event e = { 0 };

            if (msg.isNoteOn())
            {
                e.type              = Steinberg::Vst::Event::kNoteOnEvent;
                e.noteOn.channel    = createSafeChannel (msg.getChannel());
                e.noteOn.pitch      = createSafeNote (msg.getNoteNumber());
                e.noteOn.velocity   = normaliseMidiValue (msg.getVelocity());
                e.noteOn.length     = 0;
                e.noteOn.tuning     = 0.0f;
                e.noteOn.noteId     = -1;
            }
            else if (msg.isNoteOff())
            {
                e.type              = Steinberg::Vst::Event::kNoteOffEvent;
                e.noteOff.channel   = createSafeChannel (msg.getChannel());
                e.noteOff.pitch     = createSafeNote (msg.getNoteNumber());
                e.noteOff.velocity  = normaliseMidiValue (msg.getVelocity());
                e.noteOff.tuning    = 0.0f;
                e.noteOff.noteId    = -1;
            }
            else if (msg.isSysEx())
            {
                e.type          = Steinberg::Vst::Event::kDataEvent;
                e.data.bytes    = msg.getSysExData();
                e.data.size     = msg.getSysExDataSize();
                e.data.type     = Steinberg::Vst::DataEvent::kMidiSysEx;
            }
            else if (msg.isAftertouch())
            {
                e.type                   = Steinberg::Vst::Event::kPolyPressureEvent;
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
    Array<Steinberg::Vst::Event, CriticalSection> events;
    Atomic<int> refCount;

    static Steinberg::int16 createSafeChannel (int channel) noexcept  { return (Steinberg::int16) jlimit (0, 15, channel - 1); }
    static int createSafeChannel (Steinberg::int16 channel) noexcept  { return (int) jlimit (1, 16, channel + 1); }

    static Steinberg::int16 createSafeNote (int note) noexcept        { return (Steinberg::int16) jlimit (0, 127, note); }
    static int createSafeNote (Steinberg::int16 note) noexcept        { return jlimit (0, 127, (int) note); }

    static float normaliseMidiValue (int value) noexcept              { return jlimit (0.0f, 1.0f, (float) value / 127.0f); }
    static int denormaliseToMidiValue (float value) noexcept          { return roundToInt (jlimit (0.0f, 127.0f, value * 127.0f)); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEventList)
};

//==============================================================================
namespace VST3BufferExchange
{
    typedef Array<float*> Bus;
    typedef Array<Bus> BusMap;

    /** Assigns a series of AudioSampleBuffer's channels to an AudioBusBuffers'

        @warning For speed, does not check the channel count and offsets
                 according to the AudioSampleBuffer
    */
    void associateBufferTo (Steinberg::Vst::AudioBusBuffers& vstBuffers,
                            Bus& bus,
                            const AudioSampleBuffer& buffer,
                            int numChannels, int channelStartOffset,
                            int sampleOffset = 0) noexcept
    {
        const int channelEnd = numChannels + channelStartOffset;
        jassert (channelEnd >= 0 && channelEnd <= buffer.getNumChannels());

        bus.clearQuick();

        for (int i = channelStartOffset; i < channelEnd; ++i)
            bus.add (buffer.getSampleData (i, sampleOffset));

        vstBuffers.channelBuffers32 = bus.getRawDataPointer();
        vstBuffers.numChannels      = numChannels;
        vstBuffers.silenceFlags     = 0;
    }

    static void mapBufferToBusses (Array<Steinberg::Vst::AudioBusBuffers>& result,
                                   Steinberg::Vst::IAudioProcessor& processor,
                                   BusMap& busMapToUse,
                                   bool isInput, int numBusses,
                                   AudioSampleBuffer& source)
    {
        int channelIndexOffset = 0;

        for (int i = 0; i < numBusses; ++i)
        {
            Steinberg::Vst::SpeakerArrangement arrangement = 0;
            processor.getBusArrangement (isInput ? Steinberg::Vst::kInput : Steinberg::Vst::kOutput,
                                         (Steinberg::int32) i, arrangement);

            const int numChansForBus = BigInteger ((juce::int64) arrangement).countNumberOfSetBits();

            if (i >= result.size())
                result.add (Steinberg::Vst::AudioBusBuffers());

            if (i >= busMapToUse.size())
                busMapToUse.add (Bus());

            if (numChansForBus > 0)
            {
                associateBufferTo (result.getReference (i),
                                   busMapToUse.getReference (i),
                                   source, numChansForBus, channelIndexOffset);
            }

            channelIndexOffset += numChansForBus;
        }
    }
}

#endif //JUCE_VST3COMMON_H_INCLUDED
