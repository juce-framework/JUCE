/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_MIDIMESSAGECOLLECTOR_H_INCLUDED
#define JUCE_MIDIMESSAGECOLLECTOR_H_INCLUDED


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

        Precondition: numSamples must be greater than 0.
    */
    void removeNextBlockOfMessages (MidiBuffer& destBuffer, int numSamples);


    //==============================================================================
    /** @internal */
    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    /** @internal */
    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    /** @internal */
    void handleIncomingMidiMessage (MidiInput*, const MidiMessage&) override;

private:
    //==============================================================================
    double lastCallbackTime;
    CriticalSection midiCallbackLock;
    MidiBuffer incomingMessages;
    double sampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMessageCollector)
};


#endif   // JUCE_MIDIMESSAGECOLLECTOR_H_INCLUDED
