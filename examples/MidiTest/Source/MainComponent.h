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

#ifndef JUCE_MIDITEST_MAINCOMPONENT_H
#define JUCE_MIDITEST_MAINCOMPONENT_H

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
                              private ButtonListener
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

#endif   // JUCE_MIDITEST_MAINCOMPONENT_H
