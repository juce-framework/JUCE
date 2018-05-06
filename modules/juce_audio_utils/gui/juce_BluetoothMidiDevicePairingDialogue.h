/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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

    This dialogue is currently only available on iOS and Android. On OSX,
    you should instead pair Bluetooth MIDI devices using the "Audio MIDI Setup"
    app (located in /Applications/Utilities). On Windows, you should use
    the system settings. On Linux, Bluetooth MIDI devices are currently not
    supported.

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
