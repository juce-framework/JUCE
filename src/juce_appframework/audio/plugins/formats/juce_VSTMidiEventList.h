/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifdef __aeffect__

#ifndef __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__
#define __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__


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
        : events (0), numEventsUsed (0), numEventsAllocated (0)
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
                juce_free (((VstMidiSysexEvent*) e)->sysexDump);
                e->type = kVstMidiType;
                e->byteSize = sizeof (VstMidiEvent);
                e->noteLength = 0;
                e->noteOffset = 0;
                e->detune = 0;
                e->noteOffVelocity = 0;
            }

            e->deltaFrames = frameOffset;
            memcpy (e->midiData, midiData, numBytes);
        }
        else
        {
            VstMidiSysexEvent* const se = (VstMidiSysexEvent*) e;

            if (se->type == kVstSysExType)
                se->sysexDump = (char*) juce_realloc (se->sysexDump, numBytes);
            else
                se->sysexDump = (char*) juce_malloc (numBytes);

            memcpy (se->sysexDump, midiData, numBytes);

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

            if (e != 0)
            {
                if (e->type == kVstMidiType)
                {
                    dest.addEvent ((const JUCE_NAMESPACE::uint8*) ((const VstMidiEvent*) e)->midiData,
                                   4, e->deltaFrames);
                }
                else if (e->type == kVstSysExType)
                {
                    dest.addEvent ((const JUCE_NAMESPACE::uint8*) ((const VstMidiSysexEvent*) e)->sysexDump,
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

            const int size = 20 + sizeof (VstEvent*) * numEventsNeeded;

            if (events == 0)
                events = (VstEvents*) juce_calloc (size);
            else
                events = (VstEvents*) juce_realloc (events, size);

            for (int i = numEventsAllocated; i < numEventsNeeded; ++i)
            {
                VstMidiEvent* const e = (VstMidiEvent*) juce_calloc (jmax ((int) sizeof (VstMidiEvent),
                                                                           (int) sizeof (VstMidiSysexEvent)));
                e->type = kVstMidiType;
                e->byteSize = sizeof (VstMidiEvent);

                events->events[i] = (VstEvent*) e;
            }

            numEventsAllocated = numEventsNeeded;
        }
    }

    void freeEvents()
    {
        if (events != 0)
        {
            for (int i = numEventsAllocated; --i >= 0;)
            {
                VstMidiEvent* const e = (VstMidiEvent*) (events->events[i]);

                if (e->type == kVstSysExType)
                    juce_free (((VstMidiSysexEvent*) e)->sysexDump);

                juce_free (e);
            }

            juce_free (events);
            events = 0;
            numEventsUsed = 0;
            numEventsAllocated = 0;
        }
    }

    //==============================================================================
    VstEvents* events;

private:
    int numEventsUsed, numEventsAllocated;
};


#endif   // __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__
#endif   // __JUCE_VSTMIDIEVENTLIST_JUCEHEADER__
