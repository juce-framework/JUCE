/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__
#define __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__

#include "juce_MidiInput.h"


//==============================================================================
/**
    Collects incoming realtime MIDI messages and turns them into blocks suitable for
    processing by a block-based audio callback.

    The class can also be used as either a MidiKeyboardStateListener or a MidiInputCallback
    so it can easily use a midi input or keyboard component as its source.

    @see MidiMessage, MidiInput
*/
class JUCE_API  MidiMessageCollector    : public MidiKeyboardStateListener,
                                          public MidiInputCallback
{
public:
    //==============================================================================
    /** Creates a MidiMessageCollector. */
    MidiMessageCollector();

    /** Destructor. */
    ~MidiMessageCollector();

    //==============================================================================
    /** Clears any messages from the queue.

        You need to call this method before starting to use the collector, so that
        it knows the correct sample rate to use.
    */
    void reset (double sampleRate);

    /** Takes an incoming real-time message and adds it to the queue.

        The message's timestamp is taken, and it will be ready for retrieval as part
        of the block returned by the next call to removeNextBlockOfMessages().

        This method is fully thread-safe when overlapping calls are made with
        removeNextBlockOfMessages().
    */
    void addMessageToQueue (const MidiMessage& message);

    /** Removes all the pending messages from the queue as a buffer.

        This will also correct the messages' timestamps to make sure they're in
        the range 0 to numSamples - 1.

        This call should be made regularly by something like an audio processing
        callback, because the time that it happens is used in calculating the
        midi event positions.

        This method is fully thread-safe when overlapping calls are made with
        addMessageToQueue().
    */
    void removeNextBlockOfMessages (MidiBuffer& destBuffer, int numSamples);


    //==============================================================================
    /** @internal */
    void handleNoteOn (MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity);
    /** @internal */
    void handleNoteOff (MidiKeyboardState* source, int midiChannel, int midiNoteNumber);
    /** @internal */
    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);

private:
    //==============================================================================
    double lastCallbackTime;
    CriticalSection midiCallbackLock;
    MidiBuffer incomingMessages;
    double sampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMessageCollector)
};


#endif   // __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__
