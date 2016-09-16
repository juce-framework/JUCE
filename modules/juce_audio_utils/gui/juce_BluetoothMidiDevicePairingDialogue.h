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

#ifndef JUCE_BLUETOOTHMIDIDEVICPAIRINGCOMPONENT_H_INCLUDED
#define JUCE_BLUETOOTHMIDIDEVICPAIRINGCOMPONENT_H_INCLUDED

//==============================================================================
/**
    Opens a Bluetooth MIDI pairing dialogue that allows the user to view and
    connect to Bluetooth MIDI devices that are currently found nearby.

    The dialogue will ignore non-MIDI Bluetooth devices.

    Only after a Bluetooth MIDI device has been paired will its MIDI ports
    be available through JUCE's MidiInput and MidiOutput classes.

    This dialogue is currently only available on iOS and Android. On OSX,
    you should instead pair Bluetooth MIDI devices using the "Audio MIDI Setup"
    app (located in /Applications/Utilities). On Windows, you should use
    the system settings. On Linux, Bluetooth MIDI devices are currently not
    supported.
*/

class JUCE_API BluetoothMidiDevicePairingDialogue
{
public:

    /** Opens the Bluetooth MIDI pairing dialogue, if it is available.

        @return true if the dialogue was opened, false on error.
    */
    static bool open();

    /** Checks if a Bluetooth MIDI pairing dialogue is available on this
        platform.

        On iOS, this will be true for iOS versions 8.0 and higher.

        On Android, this will be true only for Android SDK versions 23 and
        higher, and additionally only if the device itself supports MIDI
        over Bluetooth.

        On desktop platforms, this will typically be false as the bluetooth
        pairing is not done inside the app but by other means.

        @return true if the Bluetooth MIDI pairing dialogue is available,
                false otherwise.
    */
    static bool isAvailable();
};


#endif   // JUCE_BLUETOOTHMIDIDEVICPAIRINGCOMPONENT_H_INCLUDED
