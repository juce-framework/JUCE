/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Opens a Bluetooth MIDI pairing dialogue that allows the user to view and
    connect to Bluetooth MIDI devices that are currently found nearby.

    The dialogue will ignore non-MIDI Bluetooth devices.

    Only after a Bluetooth MIDI device has been paired will its MIDI ports
    be available through JUCE's MidiInput and MidiOutput classes.

    This dialogue is currently only available on macOS targeting versions 10.11+,
    iOS and Android. When targeting older versions of macOS you should instead
    pair Bluetooth MIDI devices using the "Audio MIDI Setup" app (located in
    /Applications/Utilities). On Windows, you should use the system settings. On
    Linux, Bluetooth MIDI devices are currently not supported.

    @tags{Audio}
*/
class JUCE_API BluetoothMidiDevicePairingDialogue
{
public:

    /** Opens the Bluetooth MIDI pairing dialogue, if it is available.

        @param  exitCallback A callback which will be called when the modal
                bluetooth dialog is closed.
        @param  btWindowBounds The bounds of the bluetooth window that will
                be opened. The dialog itself is opened by the OS so cannot
                be customised by JUCE.
        @return true if the dialogue was opened, false on error.

        @see ModalComponentManager::Callback
    */
    static bool open (ModalComponentManager::Callback* exitCallback = nullptr,
                      Rectangle<int>* btWindowBounds = nullptr);

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

} // namespace juce
