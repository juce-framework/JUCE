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

#pragma once

#include "JuceHeader.h"

//==============================================================================

class MidiDeviceListBox;
struct MidiDeviceListEntry;

//==============================================================================
class MainContentComponent  : public Component,
                              private Timer,
                              private MidiKeyboardStateListener,
                              private MidiInputCallback,
                              private MessageListener,
                              private Button::Listener
{
public:
    //==============================================================================
    MainContentComponent ();
    ~MainContentComponent();

    //==============================================================================
    void timerCallback () override;
    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleMessage (const Message& msg) override;

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;

    void openDevice (bool isInput, int index);
    void closeDevice (bool isInput, int index);

    int getNumMidiInputs() const noexcept;
    int getNumMidiOutputs() const noexcept;

    ReferenceCountedObjectPtr<MidiDeviceListEntry> getMidiDevice (int index, bool isInputDevice) const noexcept;
private:
    //==============================================================================
    void handleIncomingMidiMessage (MidiInput *source, const MidiMessage &message) override;
    void sendToOutputs(const MidiMessage& msg);

    //==============================================================================
    bool hasDeviceListChanged (const StringArray& deviceNames, bool isInputDevice);
    ReferenceCountedObjectPtr<MidiDeviceListEntry> findDeviceWithName (const String& name, bool isInputDevice) const;
    void closeUnpluggedDevices (StringArray& currentlyPluggedInDevices, bool isInputDevice);
    void updateDeviceList (bool isInputDeviceList);

    //==============================================================================
    void addLabelAndSetStyle (Label& label);

    //==============================================================================
    Label midiInputLabel;
    Label midiOutputLabel;
    Label incomingMidiLabel;
    Label outgoingMidiLabel;
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent midiKeyboard;
    TextEditor midiMonitor;
    TextButton pairButton;

    ScopedPointer<MidiDeviceListBox> midiInputSelector;
    ScopedPointer<MidiDeviceListBox> midiOutputSelector;

    ReferenceCountedArray<MidiDeviceListEntry> midiInputs;
    ReferenceCountedArray<MidiDeviceListEntry> midiOutputs;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
