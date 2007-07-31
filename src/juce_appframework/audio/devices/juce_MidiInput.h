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

#ifndef __JUCE_MIDIINPUT_JUCEHEADER__
#define __JUCE_MIDIINPUT_JUCEHEADER__

#include "../midi/juce_MidiMessage.h"
#include "../../../juce_core/text/juce_StringArray.h"
class MidiInput;


//==============================================================================
/**
    Receives midi messages from a midi input device.

    This class is overridden to handle incoming midi messages. See the MidiInput
    class for more details.

    @see MidiInput
*/
class JUCE_API  MidiInputCallback
{
public:
    /** Destructor. */
    virtual ~MidiInputCallback()  {}


    /** Receives an incoming message.

        @param source   the MidiInput object that generated the message
        @param message  the incoming message. The message's timestamp is set to a value
                        equivalent to (Time::getMillisecondCounter() / 1000.0) to specify the
                        time when the message arrived.
    */
    virtual void handleIncomingMidiMessage (MidiInput* source,
                                            const MidiMessage& message) = 0;

    /** Notification sent each time a packet of a multi-packet sysex message arrives.

        If a long sysex message is broken up into multiple packets, this callback is made
        for each packet that arrives until the message is finished, at which point
        the normal handleIncomingMidiMessage() callback will be made with the entire
        message.

        The message passed in will contain the start of a sysex, but won't be finished
        with the terminating 0x7f byte.
    */
    virtual void handlePartialSysexMessage (MidiInput* source,
                                            const uint8* messageData,
                                            const int numBytesSoFar,
                                            const double timestamp)
    {
        // (this bit is just to avoid compiler warnings about unused variables)
        (void) source; (void) messageData; (void) numBytesSoFar; (void) timestamp;
    }
};

//==============================================================================
/**
    Represents a midi input device.

    To create one of these, use the static getDevices() method to find out what inputs are
    available, and then use the openDevice() method to try to open one.

    @see MidiOutput
*/
class JUCE_API  MidiInput
{
public:
    //==============================================================================
    /** Returns a list of the available midi input devices.

        You can open one of the devices by passing its index into the
        openDevice() method.

        @see getDefaultDeviceIndex, openDevice
    */
    static const StringArray getDevices();

    /** Returns the index of the default midi input device to use.

        This refers to the index in the list returned by getDevices().
    */
    static int getDefaultDeviceIndex();

    /** Tries to open one of the midi input devices.

        This will return a MidiInput object if it manages to open it. You can then
        call start() and stop() on this device, and delete it when no longer needed.

        If the device can't be opened, this will return a null pointer.

        @param deviceIndex  the index of a device from the list returned by getDevices()
        @param callback     the object that will receive the midi messages from this device.

        @see MidiInputCallback, getDevices
    */
    static MidiInput* openDevice (int deviceIndex,
                                  MidiInputCallback* callback);

#if JUCE_LINUX || DOXYGEN
    /** LINUX ONLY - This will try to create a new midi input device.

        This will attempt to create a new midi input device with the specified name,
        for other apps to connect to.

        Returns 0 if a device can't be created.

        @param deviceName   the name to use for the new device
        @param callback     the object that will receive the midi messages from this device.
    */
    static MidiInput* createNewDevice (const String& deviceName,
                                       MidiInputCallback* callback);
#endif

    //==============================================================================
    /** Destructor. */
    ~MidiInput();

    /** Returns the name of this device.
    */
    const String getName() const throw()                    { return name; }

    /** Allows you to set a custom name for the device, in case you don't like the name
        it was given when created.
    */
    void setName (const String& newName) throw()            { name = newName; }

    //==============================================================================
    /** Starts the device running.

        After calling this, the device will start sending midi messages to the
        MidiInputCallback object that was specified when the openDevice() method
        was called.

        @see stop
    */
    void start();

    /** Stops the device running.

        @see start
    */
    void stop();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String name;
    void* internal;

    MidiInput (const String& name);
    MidiInput (const MidiInput&);
};


#endif   // __JUCE_MIDIINPUT_JUCEHEADER__
