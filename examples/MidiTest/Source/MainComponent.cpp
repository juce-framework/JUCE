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

#include "MainComponent.h"

//==============================================================================
struct MidiDeviceListEntry : ReferenceCountedObject
{
    MidiDeviceListEntry (const String& deviceName) : name (deviceName) {}

    String name;
    ScopedPointer<MidiInput> inDevice;
    ScopedPointer<MidiOutput> outDevice;

    typedef ReferenceCountedObjectPtr<MidiDeviceListEntry> Ptr;
};

//==============================================================================
struct MidiCallbackMessage : public Message
{
    MidiCallbackMessage (const MidiMessage& msg) : message (msg) {}
    MidiMessage message;
};

//==============================================================================
class MidiDeviceListBox : public ListBox,
private ListBoxModel
{
public:
    //==============================================================================
    MidiDeviceListBox (const String& name,
                       MainContentComponent& contentComponent,
                       bool isInputDeviceList)
    : ListBox (name, this),
      parent (contentComponent),
      isInput (isInputDeviceList)
    {
        setOutlineThickness (1);
        setMultipleSelectionEnabled (true);
        setClickingTogglesRowSelection (true);
    }

    //==============================================================================
    int getNumRows() override
    {
        return isInput ? parent.getNumMidiInputs()
                       : parent.getNumMidiOutputs();
    }

    //==============================================================================
    void paintListBoxItem (int rowNumber, Graphics &g,
                           int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll (Colour (0xffeeeeee));


        g.setColour (Colours::black);
        g.setFont (height * 0.7f);

        if (isInput)
        {
            if (rowNumber < parent.getNumMidiInputs())
                g.drawText (parent.getMidiDevice (rowNumber, true)->name,
                            5, 0, width, height,
                            Justification::centredLeft, true);
        }
        else
        {
            if (rowNumber < parent.getNumMidiOutputs())
                g.drawText (parent.getMidiDevice (rowNumber, false)->name,
                            5, 0, width, height,
                            Justification::centredLeft, true);
        }
    }

    //==============================================================================
    void selectedRowsChanged (int) override
    {
        SparseSet<int> newSelectedItems = getSelectedRows();
        if (newSelectedItems != lastSelectedItems)
        {
            for (int i = 0; i < lastSelectedItems.size(); ++i)
            {
                if (! newSelectedItems.contains (lastSelectedItems[i]))
                    parent.closeDevice (isInput, lastSelectedItems[i]);
            }

            for (int i = 0; i < newSelectedItems.size(); ++i)
            {
                if (! lastSelectedItems.contains (newSelectedItems[i]))
                    parent.openDevice (isInput, newSelectedItems[i]);
            }

            lastSelectedItems = newSelectedItems;
        }
    }

    //==============================================================================
    void syncSelectedItemsWithDeviceList (const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices)
    {
        SparseSet<int> selectedRows;
        for (int i = 0; i < midiDevices.size(); ++i)
            if (midiDevices[i]->inDevice != nullptr || midiDevices[i]->outDevice != nullptr)
                selectedRows.addRange (Range<int> (i, i+1));

        lastSelectedItems = selectedRows;
        updateContent();
        setSelectedRows (selectedRows, dontSendNotification);
    }

private:
    //==============================================================================
    MainContentComponent& parent;
    bool isInput;
    SparseSet<int> lastSelectedItems;
};

//==============================================================================
MainContentComponent::MainContentComponent ()
    : midiInputLabel ("Midi Input Label", "MIDI Input:"),
      midiOutputLabel ("Midi Output Label", "MIDI Output:"),
      incomingMidiLabel ("Incoming Midi Label", "Received MIDI messages:"),
      outgoingMidiLabel ("Outgoing Midi Label", "Play the keyboard to send MIDI messages..."),
      midiKeyboard (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
      midiMonitor ("MIDI Monitor"),
      pairButton ("MIDI Bluetooth devices..."),
      midiInputSelector (new MidiDeviceListBox ("Midi Input Selector", *this, true)),
      midiOutputSelector (new MidiDeviceListBox ("Midi Input Selector", *this, false))
{
    setSize (732, 520);

    addLabelAndSetStyle (midiInputLabel);
    addLabelAndSetStyle (midiOutputLabel);
    addLabelAndSetStyle (incomingMidiLabel);
    addLabelAndSetStyle (outgoingMidiLabel);

    midiKeyboard.setName ("MIDI Keyboard");
    addAndMakeVisible (midiKeyboard);

    midiMonitor.setMultiLine (true);
    midiMonitor.setReturnKeyStartsNewLine (false);
    midiMonitor.setReadOnly (true);
    midiMonitor.setScrollbarsShown (true);
    midiMonitor.setCaretVisible (false);
    midiMonitor.setPopupMenuEnabled (false);
    midiMonitor.setText (String());
    addAndMakeVisible (midiMonitor);

    if (! BluetoothMidiDevicePairingDialogue::isAvailable())
        pairButton.setEnabled (false);

    addAndMakeVisible (pairButton);
    pairButton.addListener (this);
    keyboardState.addListener (this);

    addAndMakeVisible (midiInputSelector);
    addAndMakeVisible (midiOutputSelector);

    startTimer (500);
}

//==============================================================================
void MainContentComponent::addLabelAndSetStyle (Label& label)
{
    label.setFont (Font (15.00f, Font::plain));
    label.setJustificationType (Justification::centredLeft);
    label.setEditable (false, false, false);
    label.setColour (TextEditor::textColourId, Colours::black);
    label.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (label);
}

//==============================================================================
MainContentComponent::~MainContentComponent()
{
    stopTimer();
    midiInputs.clear();
    midiOutputs.clear();
    keyboardState.removeListener (this);

    midiInputSelector = nullptr;
    midiOutputSelector = nullptr;
    midiOutputSelector = nullptr;
}

//==============================================================================
void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

//==============================================================================
void MainContentComponent::resized()
{
    const int margin = 10;

    midiInputLabel.setBounds (margin, margin,
                              (getWidth() / 2) - (2 * margin), 24);

    midiOutputLabel.setBounds ((getWidth() / 2) + margin, margin,
                               (getWidth() / 2) - (2 * margin), 24);

    midiInputSelector->setBounds (margin, (2 * margin) + 24,
                                  (getWidth() / 2) - (2 * margin),
                                  (getHeight() / 2) - ((4 * margin) + 24 + 24));

    midiOutputSelector->setBounds ((getWidth() / 2) + margin, (2 * margin) + 24,
                                   (getWidth() / 2) - (2 * margin),
                                   (getHeight() / 2) - ((4 * margin) + 24 + 24));

    pairButton.setBounds (margin, (getHeight() / 2) - (margin + 24),
                          getWidth() - (2 * margin), 24);

    outgoingMidiLabel.setBounds (margin, getHeight() / 2, getWidth() - (2*margin), 24);
    midiKeyboard.setBounds (margin, (getHeight() / 2) + (24 + margin), getWidth() - (2*margin), 64);

    incomingMidiLabel.setBounds (margin, (getHeight() / 2) + (24 + (2 * margin) + 64),
                                 getWidth() - (2*margin), 24);

    int y = (getHeight() / 2) + ((2 * 24) + (3 * margin) + 64);
    midiMonitor.setBounds (margin, y,
                           getWidth() - (2*margin), getHeight() - y - margin);
}

//==============================================================================
void MainContentComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &pairButton)
        RuntimePermissions::request (
            RuntimePermissions::bluetoothMidi,
            [] (bool wasGranted) { if (wasGranted) BluetoothMidiDevicePairingDialogue::open(); } );
}

//==============================================================================
bool MainContentComponent::hasDeviceListChanged (const StringArray& deviceNames, bool isInputDevice)
{
    ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                            : midiOutputs;

    if (deviceNames.size() != midiDevices.size())
        return true;

    for (int i = 0; i < deviceNames.size(); ++i)
        if (deviceNames[i] != midiDevices[i]->name)
            return true;

    return false;
}

MidiDeviceListEntry::Ptr MainContentComponent::findDeviceWithName (const String& name, bool isInputDevice) const
{
    const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                                  : midiOutputs;

    for (int i = 0; i < midiDevices.size(); ++i)
        if (midiDevices[i]->name == name)
            return midiDevices[i];

    return nullptr;
}

void MainContentComponent::closeUnpluggedDevices (StringArray& currentlyPluggedInDevices, bool isInputDevice)
{
    ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                            : midiOutputs;

    for (int i = midiDevices.size(); --i >= 0;)
    {
        MidiDeviceListEntry& d = *midiDevices[i];

        if (! currentlyPluggedInDevices.contains (d.name))
        {
            if (isInputDevice ? d.inDevice != nullptr
                              : d.outDevice != nullptr)
                closeDevice (isInputDevice, i);

            midiDevices.remove (i);
        }
    }
}

void MainContentComponent::updateDeviceList (bool isInputDeviceList)
{
    StringArray newDeviceNames = isInputDeviceList ? MidiInput::getDevices()
                                                   : MidiOutput::getDevices();

    if (hasDeviceListChanged (newDeviceNames, isInputDeviceList))
    {

        ReferenceCountedArray<MidiDeviceListEntry>& midiDevices
            = isInputDeviceList ? midiInputs : midiOutputs;

        closeUnpluggedDevices (newDeviceNames, isInputDeviceList);

        ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;

        // add all currently plugged-in devices to the device list
        for (int i = 0; i < newDeviceNames.size(); ++i)
        {
            MidiDeviceListEntry::Ptr entry = findDeviceWithName (newDeviceNames[i], isInputDeviceList);

            if (entry == nullptr)
                entry = new MidiDeviceListEntry (newDeviceNames[i]);

            newDeviceList.add (entry);
        }

        // actually update the device list
        midiDevices = newDeviceList;

        // update the selection status of the combo-box
        if (MidiDeviceListBox* midiSelector = isInputDeviceList ? midiInputSelector : midiOutputSelector)
            midiSelector->syncSelectedItemsWithDeviceList (midiDevices);
    }
}

//==============================================================================
void MainContentComponent::timerCallback ()
{
    updateDeviceList (true);
    updateDeviceList (false);
}

//==============================================================================
void MainContentComponent::handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    MidiMessage m (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
    sendToOutputs (m);
}

//==============================================================================
void MainContentComponent::handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    MidiMessage m (MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
    sendToOutputs (m);
}

//==============================================================================
void MainContentComponent::sendToOutputs(const MidiMessage& msg)
{
    for (int i = 0; i < midiOutputs.size(); ++i)
        if (midiOutputs[i]->outDevice != nullptr)
            midiOutputs[i]->outDevice->sendMessageNow (msg);
}

//==============================================================================
void MainContentComponent::handleIncomingMidiMessage (MidiInput* /*source*/, const MidiMessage &message)
{
    // This is called on the MIDI thread

    if (message.isNoteOnOrOff())
        postMessage (new MidiCallbackMessage (message));
}

//==============================================================================
void MainContentComponent::handleMessage (const Message& msg)
{
    // This is called on the message loop

    const MidiMessage& mm = dynamic_cast<const MidiCallbackMessage&> (msg).message;
    String midiString;
    midiString << (mm.isNoteOn() ? String ("Note on: ") : String ("Note off: "));
    midiString << (MidiMessage::getMidiNoteName (mm.getNoteNumber(), true, true, true));
    midiString << (String (" vel = "));
    midiString << static_cast<int>(mm.getVelocity());
    midiString << "\n";

    midiMonitor.insertTextAtCaret (midiString);
}

//==============================================================================
void MainContentComponent::openDevice (bool isInput, int index)
{
    if (isInput)
    {
        jassert (midiInputs[index]->inDevice == nullptr);
        midiInputs[index]->inDevice = MidiInput::openDevice (index, this);

        if (midiInputs[index]->inDevice == nullptr)
        {
            DBG ("MainContentComponent::openDevice: open input device for index = " << index << " failed!" );
            return;
        }

        midiInputs[index]->inDevice->start();
    }
    else
    {
        jassert (midiOutputs[index]->outDevice == nullptr);
        midiOutputs[index]->outDevice = MidiOutput::openDevice (index);

        if (midiOutputs[index]->outDevice == nullptr)
            DBG ("MainContentComponent::openDevice: open output device for index = " << index << " failed!" );
    }
}

//==============================================================================
void MainContentComponent::closeDevice (bool isInput, int index)
{
    if (isInput)
    {
        jassert (midiInputs[index]->inDevice != nullptr);
        midiInputs[index]->inDevice->stop();
        midiInputs[index]->inDevice = nullptr;
    }
    else
    {
        jassert (midiOutputs[index]->outDevice != nullptr);
        midiOutputs[index]->outDevice = nullptr;
    }
}

//==============================================================================
int MainContentComponent::getNumMidiInputs() const noexcept
{
    return midiInputs.size();
}

//==============================================================================
int MainContentComponent::getNumMidiOutputs() const noexcept
{
    return midiOutputs.size();
}

//==============================================================================
ReferenceCountedObjectPtr<MidiDeviceListEntry>
MainContentComponent::getMidiDevice (int index, bool isInput) const noexcept
{
    return isInput ? midiInputs[index] : midiOutputs[index];
}
