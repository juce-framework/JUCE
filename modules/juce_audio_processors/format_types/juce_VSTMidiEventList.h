/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

// NB: this must come first, *before* the header-guard.
#ifdef JUCE_VSTINTERFACE_H_INCLUDED

namespace juce
{

//==============================================================================
/** Holds a set of VSTMidiEvent objects and makes it easy to add
    events to the list.

    This is used by both the VST hosting code and the plugin wrapper.

    @tags{Audio}
*/
class VSTMidiEventList
{
public:
    //==============================================================================
    VSTMidiEventList()
        : numEventsUsed (0), numEventsAllocated (0)
    {
    }

    ~VSTMidiEventList()
    {
        freeEvents();
    }

    //==============================================================================
    void clear()
    {
        numEventsUsed = 0;

        if (events != nullptr)
            events->numEvents = 0;
    }

    void addEvent (const void* const midiData, const int numBytes, const int frameOffset)
    {
        ensureSize (numEventsUsed + 1);

        Vst2::VstMidiEvent* const e = (Vst2::VstMidiEvent*) (events->events [numEventsUsed]);
        events->numEvents = ++numEventsUsed;

        if (numBytes <= 4)
        {
            if (e->type == Vst2::kVstSysExType)
            {
                delete[] (((Vst2::VstMidiSysexEvent*) e)->sysexDump);
                e->type = Vst2::kVstMidiType;
                e->byteSize = sizeof (Vst2::VstMidiEvent);
                e->noteLength = 0;
                e->noteOffset = 0;
                e->detune = 0;
                e->noteOffVelocity = 0;
            }

            e->deltaFrames = frameOffset;
            memcpy (e->midiData, midiData, (size_t) numBytes);
        }
        else
        {
            Vst2::VstMidiSysexEvent* const se = (Vst2::VstMidiSysexEvent*) e;

            if (se->type == Vst2::kVstSysExType)
                delete[] se->sysexDump;

            se->sysexDump = new char [(size_t) numBytes];
            memcpy (se->sysexDump, midiData, (size_t) numBytes);

            se->type = Vst2::kVstSysExType;
            se->byteSize = sizeof (Vst2::VstMidiSysexEvent);
            se->deltaFrames = frameOffset;
            se->flags = 0;
            se->dumpBytes = numBytes;
            se->resvd1 = 0;
            se->resvd2 = 0;
        }
    }

    //==============================================================================
    // Handy method to pull the events out of an event buffer supplied by the host
    // or plugin.
    static void addEventsToMidiBuffer (const Vst2::VstEvents* events, MidiBuffer& dest)
    {
        for (int i = 0; i < events->numEvents; ++i)
        {
            const Vst2::VstEvent* const e = events->events[i];

            if (e != nullptr)
            {
                if (e->type == Vst2::kVstMidiType)
                {
                    dest.addEvent ((const juce::uint8*) ((const Vst2::VstMidiEvent*) e)->midiData,
                                   4, e->deltaFrames);
                }
                else if (e->type == Vst2::kVstSysExType)
                {
                    dest.addEvent ((const juce::uint8*) ((const Vst2::VstMidiSysexEvent*) e)->sysexDump,
                                   (int) ((const Vst2::VstMidiSysexEvent*) e)->dumpBytes,
                                   e->deltaFrames);
                }
            }
        }
    }

    //==============================================================================
    void ensureSize (int numEventsNeeded)
    {
        if (numEventsNeeded > numEventsAllocated)
        {
            numEventsNeeded = (numEventsNeeded + 32) & ~31;

            const size_t size = 20 + sizeof (Vst2::VstEvent*) * (size_t) numEventsNeeded;

            if (events == nullptr)
                events.calloc (size, 1);
            else
                events.realloc (size, 1);

            for (int i = numEventsAllocated; i < numEventsNeeded; ++i)
                events->events[i] = allocateVSTEvent();

            numEventsAllocated = numEventsNeeded;
        }
    }

    void freeEvents()
    {
        if (events != nullptr)
        {
            for (int i = numEventsAllocated; --i >= 0;)
                freeVSTEvent (events->events[i]);

            events.free();
            numEventsUsed = 0;
            numEventsAllocated = 0;
        }
    }

    //==============================================================================
    HeapBlock<Vst2::VstEvents> events;

private:
    int numEventsUsed, numEventsAllocated;

    static Vst2::VstEvent* allocateVSTEvent()
    {
        auto e = (Vst2::VstEvent*) std::calloc (1, sizeof (Vst2::VstMidiEvent) > sizeof (Vst2::VstMidiSysexEvent) ? sizeof (Vst2::VstMidiEvent)
                                                                                            : sizeof (Vst2::VstMidiSysexEvent));
        e->type = Vst2::kVstMidiType;
        e->byteSize = sizeof (Vst2::VstMidiEvent);
        return e;
    }

    static void freeVSTEvent (Vst2::VstEvent* e)
    {
        if (e->type == Vst2::kVstSysExType)
            delete[] (((Vst2::VstMidiSysexEvent*) e)->sysexDump);

        std::free (e);
    }
};

} // namespace juce

#endif // JUCE_VSTINTERFACE_H_INCLUDED
