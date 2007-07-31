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

#ifndef __JUCE_MIDIOUTPUT_JUCEHEADER__
#define __JUCE_MIDIOUTPUT_JUCEHEADER__

#include "../../../juce_core/text/juce_StringArray.h"
#include "../../../juce_core/threads/juce_Thread.h"
#include "../midi/juce_MidiMessage.h"
#include "../midi/juce_MidiBuffer.h"


//==============================================================================
/**
    Represents a midi output device.

    To create one of these, use the static getDevices() method to find out what
    outputs are available, then use the openDevice() method to try to open one.

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
    static const StringArray getDevices();

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


#if JUCE_LINUX || DOXYGEN
    /** LINUX ONLY - This will try to create a new midi output device.

        This will attempt to create a new midi output device that other apps can connect
        to and use as their midi input.

        Returns 0 if a device can't be created.

        @param deviceName   the name to use for the new device
    */
    static MidiOutput* createNewDevice (const String& deviceName);
#endif

    //==============================================================================
    /** Destructor. */
    ~MidiOutput();

    /** Makes this device output a midi message.

        @see MidiMessage
    */
    void sendMessageNow (const MidiMessage& message);

    /** Sends a midi reset to the device. */
    void reset();


    //==============================================================================
    /** Returns the current volume setting for this device.  */
    bool getVolume (float& leftVol,
                    float& rightVol);

    /** Changes the overall volume for this device.  */
    void setVolume (float leftVol,
                    float rightVol);

    //==============================================================================
    /** This lets you supply a block of messages that will be sent out at some point
        in the future.

        The MidiOutput class has an internal thread that can send out timestamped
        messages - this appends a set of messages to its internal buffer, ready for
        sending.

        This will only work if you've already started the thread with startBackgroundThread().

        A time is supplied, at which the block of messages should be sent. This time uses
        the same time base as Time::getMillisecondCounter(), and must be in the future.

        The samplesPerSecondForBuffer parameter indicates the number of samples per second
        used by the MidiBuffer. Each event in a MidiBuffer has a sample position, and the
        samplesPerSecondForBuffer value is needed to convert this sample position to a
        real time.
    */
    void sendBlockOfMessages (const MidiBuffer& buffer,
                              const double millisecondCounterToStartAt,
                              double samplesPerSecondForBuffer) throw();

    /** Gets rid of any midi messages that had been added by sendBlockOfMessages().
    */
    void clearAllPendingMessages() throw();

    /** Starts up a background thread so that the device can send blocks of data.

        Call this to get the device ready, before using sendBlockOfMessages().
    */
    void startBackgroundThread() throw();

    /** Stops the background thread, and clears any pending midi events.

        @see startBackgroundThread
    */
    void stopBackgroundThread() throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    void* internal;

    struct PendingMessage
    {
        PendingMessage (const uint8* const data, const int len, const double sampleNumber) throw();

        MidiMessage message;
        PendingMessage* next;

        juce_UseDebuggingNewOperator
    };

    CriticalSection lock;
    PendingMessage* firstMessage;

    MidiOutput() throw();
    MidiOutput (const MidiOutput&);

    void run();
};


#endif   // __JUCE_MIDIOUTPUT_JUCEHEADER__
