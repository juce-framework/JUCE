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

#ifndef JUCE_MIDIOUTPUT_H_INCLUDED
#define JUCE_MIDIOUTPUT_H_INCLUDED


//==============================================================================
/**
    Controls a physical MIDI output device.

    To create one of these, use the static getDevices() method to get a list of the
    available output devices, then use the openDevice() method to try to open one.

    @see MidiInput
*/
class JUCE_API  MidiOutput  : private Thread
{
public:
    //==============================================================================
    /** Returns a list of the available midi output devices.

        You can open one of the devices by passing its index into the
        openDevice() method.

        @see getDefaultDeviceIndex, openDevice
    */
    static StringArray getDevices();

    /** Returns the index of the default midi output device to use.

        This refers to the index in the list returned by getDevices().
    */
    static int getDefaultDeviceIndex();

    /** Tries to open one of the midi output devices.

        This will return a MidiOutput object if it manages to open it. You can then
        send messages to this device, and delete it when no longer needed.

        If the device can't be opened, this will return a null pointer.

        @param deviceIndex  the index of a device from the list returned by getDevices()
        @see getDevices
    */
    static MidiOutput* openDevice (int deviceIndex);


   #if JUCE_LINUX || JUCE_MAC || JUCE_IOS || DOXYGEN
    /** This will try to create a new midi output device (Not available on Windows).

        This will attempt to create a new midi output device that other apps can connect
        to and use as their midi input.

        Returns nullptr if a device can't be created.

        @param deviceName   the name to use for the new device
    */
    static MidiOutput* createNewDevice (const String& deviceName);
   #endif

    //==============================================================================
    /** Destructor. */
    ~MidiOutput();

    /** Returns the name of this device. */
    const String& getName() const noexcept                      { return name; }

    /** Sends out a MIDI message immediately. */
    void sendMessageNow (const MidiMessage& message);

    /** Sends out a sequence of MIDI messages immediately. */
    void sendBlockOfMessagesNow (const MidiBuffer& buffer);

    //==============================================================================
    /** This lets you supply a block of messages that will be sent out at some point
        in the future.

        The MidiOutput class has an internal thread that can send out timestamped
        messages - this appends a set of messages to its internal buffer, ready for
        sending.

        This will only work if you've already started the thread with startBackgroundThread().

        A time is specified, at which the block of messages should be sent. This time uses
        the same time base as Time::getMillisecondCounter(), and must be in the future.

        The samplesPerSecondForBuffer parameter indicates the number of samples per second
        used by the MidiBuffer. Each event in a MidiBuffer has a sample position, and the
        samplesPerSecondForBuffer value is needed to convert this sample position to a
        real time.
    */
    void sendBlockOfMessages (const MidiBuffer& buffer,
                              double millisecondCounterToStartAt,
                              double samplesPerSecondForBuffer);

    /** Gets rid of any midi messages that had been added by sendBlockOfMessages(). */
    void clearAllPendingMessages();

    /** Starts up a background thread so that the device can send blocks of data.
        Call this to get the device ready, before using sendBlockOfMessages().
    */
    void startBackgroundThread();

    /** Stops the background thread, and clears any pending midi events.
        @see startBackgroundThread
    */
    void stopBackgroundThread();


private:
    //==============================================================================
    void* internal;
    CriticalSection lock;
    struct PendingMessage;
    PendingMessage* firstMessage;
    String name;

    MidiOutput(const String& midiName); // These objects are created with the openDevice() method.
    void run() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiOutput)
};


#endif   // JUCE_MIDIOUTPUT_H_INCLUDED
