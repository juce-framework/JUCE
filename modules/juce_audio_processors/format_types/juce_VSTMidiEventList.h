/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifdef __aeffect__ // NB: this must come first, *before* the header-guard.

#ifndef JUCE_VSTMIDIEVENTLIST_H_INCLUDED
#define JUCE_VSTMIDIEVENTLIST_H_INCLUDED

//==============================================================================
/** Holds a set of VSTMidiEvent objects and makes it easy to add
    events to the list.

    This is used by both the VST hosting code and the plugin wrapper.
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

        VstMidiEvent* const e = (VstMidiEvent*) (events->events [numEventsUsed]);
        events->numEvents = ++numEventsUsed;

        if (numBytes <= 4)
        {
            if (e->type == kVstSysExType)
            {
                delete[] (((VstMidiSysexEvent*) e)->sysexDump);
                e->type = kVstMidiType;
                e->byteSize = sizeof (VstMidiEvent);
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
            VstMidiSysexEvent* const se = (VstMidiSysexEvent*) e;

            if (se->type == kVstSysExType)
                delete[] se->sysexDump;

            se->sysexDump = new char [numBytes];
            memcpy (se->sysexDump, midiData, (size_t) numBytes);

            se->type = kVstSysExType;
            se->byteSize = sizeof (VstMidiSysexEvent);
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
    static void addEventsToMidiBuffer (const VstEvents* events, MidiBuffer& dest)
    {
        for (int i = 0; i < events->numEvents; ++i)
        {
            const VstEvent* const e = events->events[i];

            if (e != nullptr)
            {
                if (e->type == kVstMidiType)
                {
                    dest.addEvent ((const juce::uint8*) ((const VstMidiEvent*) e)->midiData,
                                   4, e->deltaFrames);
                }
                else if (e->type == kVstSysExType)
                {
                    dest.addEvent ((const juce::uint8*) ((const VstMidiSysexEvent*) e)->sysexDump,
                                   (int) ((const VstMidiSysexEvent*) e)->dumpBytes,
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

            const size_t size = 20 + sizeof (VstEvent*) * (size_t) numEventsNeeded;

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
    HeapBlock<VstEvents> events;

private:
    int numEventsUsed, numEventsAllocated;

    static VstEvent* allocateVSTEvent()
    {
        VstEvent* const e = (VstEvent*) std::calloc (1, sizeof (VstMidiEvent) > sizeof (VstMidiSysexEvent) ? sizeof (VstMidiEvent)
                                                                                                           : sizeof (VstMidiSysexEvent));
        e->type = kVstMidiType;
        e->byteSize = sizeof (VstMidiEvent);
        return e;
    }

    static void freeVSTEvent (VstEvent* e)
    {
        if (e->type == kVstSysExType)
            delete[] (((VstMidiSysexEvent*) e)->sysexDump);

        std::free (e);
    }
};

#endif   // JUCE_VSTMIDIEVENTLIST_H_INCLUDED
#endif   // JUCE_VSTMIDIEVENTLIST_H_INCLUDED
