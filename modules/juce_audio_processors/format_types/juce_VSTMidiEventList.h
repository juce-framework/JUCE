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

// NB: this must come first, *before* the header-guard.
#ifdef JUCE_VSTINTERFACE_H_INCLUDED

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
            events->numberOfEvents = 0;
    }

    void addEvent (const void* const midiData, const int numBytes, const int frameOffset)
    {
        ensureSize (numEventsUsed + 1);

        VstMidiEvent* const e = (VstMidiEvent*) (events->events [numEventsUsed]);
        events->numberOfEvents = ++numEventsUsed;

        if (numBytes <= 4)
        {
            if (e->type == vstSysExEventType)
            {
                delete[] (((VstSysExEvent*) e)->sysExDump);
                e->type = vstMidiEventType;
                e->size = sizeof (VstMidiEvent);
                e->noteSampleLength = 0;
                e->noteSampleOffset = 0;
                e->tuning = 0;
                e->noteVelocityOff = 0;
            }

            e->sampleOffset = frameOffset;
            memcpy (e->midiData, midiData, (size_t) numBytes);
        }
        else
        {
            VstSysExEvent* const se = (VstSysExEvent*) e;

            if (se->type == vstSysExEventType)
                delete[] se->sysExDump;

            se->sysExDump = new char [(size_t) numBytes];
            memcpy (se->sysExDump, midiData, (size_t) numBytes);

            se->type = vstSysExEventType;
            se->size = sizeof (VstSysExEvent);
            se->offsetSamples = frameOffset;
            se->flags = 0;
            se->sysExDumpSize = numBytes;
            se->future1 = 0;
            se->future2 = 0;
        }
    }

    //==============================================================================
    // Handy method to pull the events out of an event buffer supplied by the host
    // or plugin.
    static void addEventsToMidiBuffer (const VstEventBlock* events, MidiBuffer& dest)
    {
        for (int i = 0; i < events->numberOfEvents; ++i)
        {
            const VstEvent* const e = events->events[i];

            if (e != nullptr)
            {
                if (e->type == vstMidiEventType)
                {
                    dest.addEvent ((const juce::uint8*) ((const VstMidiEvent*) e)->midiData,
                                   4, e->sampleOffset);
                }
                else if (e->type == vstSysExEventType)
                {
                    dest.addEvent ((const juce::uint8*) ((const VstSysExEvent*) e)->sysExDump,
                                   (int) ((const VstSysExEvent*) e)->sysExDumpSize,
                                   e->sampleOffset);
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
    HeapBlock<VstEventBlock> events;

private:
    int numEventsUsed, numEventsAllocated;

    static VstEvent* allocateVSTEvent()
    {
        VstEvent* const e = (VstEvent*) std::calloc (1, sizeof (VstMidiEvent) > sizeof (VstSysExEvent) ? sizeof (VstMidiEvent)
                                                                                                           : sizeof (VstSysExEvent));
        e->type = vstMidiEventType;
        e->size = sizeof (VstMidiEvent);
        return e;
    }

    static void freeVSTEvent (VstEvent* e)
    {
        if (e->type == vstSysExEventType)
            delete[] (((VstSysExEvent*) e)->sysExDump);

        std::free (e);
    }
};

#endif   // JUCE_VSTMIDIEVENTLIST_H_INCLUDED
#endif   // JUCE_VSTMIDIEVENTLIST_H_INCLUDED
