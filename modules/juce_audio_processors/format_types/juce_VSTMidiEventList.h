/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
    // "events" is expected to be a const- or non-const-ref to Vst2::VstEvents.
    template <typename Events>
    static auto& getEvent (Events& events, int index)
    {
        using EventType = decltype (&*events.events);

        // We static cast rather than using a direct array index here to circumvent
        // UB sanitizer's bounds-checks. The original struct is supposed to contain
        // a variable-length array, but the declaration uses a size of "2" for this
        // member.
        return static_cast<EventType> (events.events)[index];
    }

    Vst2::VstEvent* const& getEvent (int index) const { return getEvent (*events, index); }
    Vst2::VstEvent*      & getEvent (int index)       { return getEvent (*events, index); }

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

    void addEvent (const void* const midiData, int numBytes, int frameOffset)
    {
        ensureSize (numEventsUsed + 1);

        void* const ptr = getEvent (numEventsUsed);
        events->numEvents = ++numEventsUsed;

        if (numBytes <= 4)
        {
            auto* const e = static_cast<Vst2::VstMidiEvent*> (ptr);

            if (e->type == Vst2::kVstSysExType)
            {
                delete[] reinterpret_cast<Vst2::VstMidiSysexEvent*> (e)->sysexDump;
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
            auto* const se = static_cast<Vst2::VstMidiSysexEvent*> (ptr);

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
            const auto* const e = getEvent (*events, i);

            if (e != nullptr)
            {
                const void* const ptr = e;

                if (e->type == Vst2::kVstMidiType)
                {
                    dest.addEvent ((const juce::uint8*) static_cast<const Vst2::VstMidiEvent*> (ptr)->midiData,
                                   4, e->deltaFrames);
                }
                else if (e->type == Vst2::kVstSysExType)
                {
                    const auto* se = static_cast<const Vst2::VstMidiSysexEvent*> (ptr);
                    dest.addEvent ((const juce::uint8*) se->sysexDump,
                                   (int) se->dumpBytes,
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

            const size_t size = 20 + (size_t) numEventsNeeded * sizeof (Vst2::VstEvent*);

            if (events == nullptr)
                events.calloc (size, 1);
            else
                events.realloc (size, 1);

            for (int i = numEventsAllocated; i < numEventsNeeded; ++i)
                getEvent (i) = allocateVSTEvent();

            numEventsAllocated = numEventsNeeded;
        }
    }

    void freeEvents()
    {
        if (events != nullptr)
        {
            for (int i = numEventsAllocated; --i >= 0;)
                freeVSTEvent (getEvent (i));

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
        constexpr auto size = jmax (sizeof (Vst2::VstMidiEvent), sizeof (Vst2::VstMidiSysexEvent));

        if (auto* e = static_cast<Vst2::VstEvent*> (std::calloc (1, size)))
        {
            e->type = Vst2::kVstMidiType;
            e->byteSize = sizeof (Vst2::VstMidiEvent);
            return e;
        }

        return nullptr;
    }

    static void freeVSTEvent (Vst2::VstEvent* e)
    {
        if (e->type == Vst2::kVstSysExType)
        {
            delete[] (reinterpret_cast<Vst2::VstMidiSysexEvent*> (e)->sysexDump);
        }

        std::free (e);
    }
};

} // namespace juce

#endif // JUCE_VSTINTERFACE_H_INCLUDED
