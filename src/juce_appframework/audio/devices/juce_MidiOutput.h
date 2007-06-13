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
#include "../midi/juce_MidiMessage.h"


//==============================================================================
/**
    Represents a midi output device.

    To create one of these, use the static getDevices() method to find out what
    outputs are available, then use the openDevice() method to try to open one.

    @see MidiInput
*/
class JUCE_API  MidiOutput
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
    juce_UseDebuggingNewOperator

private:
    void* internal;

    MidiOutput();
    MidiOutput (const MidiOutput&);
};


#endif   // __JUCE_MIDIOUTPUT_JUCEHEADER__
