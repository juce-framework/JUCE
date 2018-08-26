/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             MidiDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Handles incoming and outcoming midi messages.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, xcode_iphone, androidstudio

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MidiDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
struct MidiDeviceListEntry : ReferenceCountedObject
{
    MidiDeviceListEntry (const String& deviceName) : name (deviceName) {}

    String name;
    std::unique_ptr<MidiInput> inDevice;
    std::unique_ptr<MidiOutput> outDevice;

    using Ptr = ReferenceCountedObjectPtr<MidiDeviceListEntry>;
};


//==============================================================================
class MidiDemo  : public Component,
                  private Timer,
                  private MidiKeyboardStateListener,
                  private MidiInputCallback,
                  private AsyncUpdater
{
public:
    //==============================================================================
    MidiDemo()
        : midiKeyboard       (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
          midiInputSelector  (new MidiDeviceListBox ("Midi Input Selector",  *this, true)),
          midiOutputSelector (new MidiDeviceListBox ("Midi Output Selector", *this, false))
    {
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
        midiMonitor.setText ({});
        addAndMakeVisible (midiMonitor);

        if (! BluetoothMidiDevicePairingDialogue::isAvailable())
            pairButton.setEnabled (false);

        addAndMakeVisible (pairButton);
        pairButton.onClick = []
        {
            RuntimePermissions::request (RuntimePermissions::bluetoothMidi,
                                         [] (bool wasGranted)
                                         {
                                             if (wasGranted)
                                                 BluetoothMidiDevicePairingDialogue::open();
                                         });
        };
        keyboardState.addListener (this);

        addAndMakeVisible (midiInputSelector .get());
        addAndMakeVisible (midiOutputSelector.get());

        setSize (732, 520);

        startTimer (500);
    }

    ~MidiDemo()
    {
        stopTimer();
        midiInputs .clear();
        midiOutputs.clear();
        keyboardState.removeListener (this);

        midiInputSelector .reset();
        midiOutputSelector.reset();
    }

    //==============================================================================
    void timerCallback() override
    {
        updateDeviceList (true);
        updateDeviceList (false);
    }

    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        MidiMessage m (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
        m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
        sendToOutputs (m);
    }

    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        MidiMessage m (MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
        m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
        sendToOutputs (m);
    }

    void paint (Graphics&) override {}

    void resized() override
    {
        auto margin = 10;

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

        outgoingMidiLabel.setBounds (margin, getHeight() / 2, getWidth() - (2 * margin), 24);
        midiKeyboard.setBounds (margin, (getHeight() / 2) + (24 + margin), getWidth() - (2 * margin), 64);

        incomingMidiLabel.setBounds (margin, (getHeight() / 2) + (24 + (2 * margin) + 64),
                                     getWidth() - (2 * margin), 24);

        auto y = (getHeight() / 2) + ((2 * 24) + (3 * margin) + 64);
        midiMonitor.setBounds (margin, y,
                               getWidth() - (2 * margin), getHeight() - y - margin);
    }

    void openDevice (bool isInput, int index)
    {
        if (isInput)
        {
            jassert (midiInputs[index]->inDevice.get() == nullptr);
            midiInputs[index]->inDevice.reset (MidiInput::openDevice (index, this));

            if (midiInputs[index]->inDevice.get() == nullptr)
            {
                DBG ("MidiDemo::openDevice: open input device for index = " << index << " failed!");
                return;
            }

            midiInputs[index]->inDevice->start();
        }
        else
        {
            jassert (midiOutputs[index]->outDevice.get() == nullptr);
            midiOutputs[index]->outDevice.reset (MidiOutput::openDevice (index));

            if (midiOutputs[index]->outDevice.get() == nullptr)
            {
                DBG ("MidiDemo::openDevice: open output device for index = " << index << " failed!");
            }
        }
    }

    void closeDevice (bool isInput, int index)
    {
        if (isInput)
        {
            jassert (midiInputs[index]->inDevice.get() != nullptr);
            midiInputs[index]->inDevice->stop();
            midiInputs[index]->inDevice.reset();
        }
        else
        {
            jassert (midiOutputs[index]->outDevice.get() != nullptr);
            midiOutputs[index]->outDevice.reset();
        }
    }

    int getNumMidiInputs() const noexcept
    {
        return midiInputs.size();
    }

    int getNumMidiOutputs() const noexcept
    {
        return midiOutputs.size();
    }

    ReferenceCountedObjectPtr<MidiDeviceListEntry> getMidiDevice (int index, bool isInput) const noexcept
    {
        return isInput ? midiInputs[index] : midiOutputs[index];
    }

private:
    //==============================================================================
    struct MidiDeviceListBox : public ListBox,
                               private ListBoxModel
    {
        MidiDeviceListBox (const String& name,
                           MidiDemo& contentComponent,
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

        void paintListBoxItem (int rowNumber, Graphics& g,
                               int width, int height, bool rowIsSelected) override
        {
            auto textColour = getLookAndFeel().findColour (ListBox::textColourId);

            if (rowIsSelected)
                g.fillAll (textColour.interpolatedWith (getLookAndFeel().findColour (ListBox::backgroundColourId), 0.5));


            g.setColour (textColour);
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
            auto newSelectedItems = getSelectedRows();
            if (newSelectedItems != lastSelectedItems)
            {
                for (auto i = 0; i < lastSelectedItems.size(); ++i)
                {
                    if (! newSelectedItems.contains (lastSelectedItems[i]))
                        parent.closeDevice (isInput, lastSelectedItems[i]);
                }

                for (auto i = 0; i < newSelectedItems.size(); ++i)
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
            for (auto i = 0; i < midiDevices.size(); ++i)
                if (midiDevices[i]->inDevice.get() != nullptr || midiDevices[i]->outDevice.get() != nullptr)
                    selectedRows.addRange (Range<int> (i, i + 1));

            lastSelectedItems = selectedRows;
            updateContent();
            setSelectedRows (selectedRows, dontSendNotification);
        }

    private:
        //==============================================================================
        MidiDemo& parent;
        bool isInput;
        SparseSet<int> lastSelectedItems;
    };

    //==============================================================================
    void handleIncomingMidiMessage (MidiInput* /*source*/, const MidiMessage& message) override
    {
        // This is called on the MIDI thread
        const ScopedLock sl (midiMonitorLock);
        incomingMessages.add (message);
        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        // This is called on the message loop
        Array<MidiMessage> messages;

        {
            const ScopedLock sl (midiMonitorLock);
            messages.swapWith (incomingMessages);
        }

        String messageText;

        for (auto& m : messages)
            messageText << m.getDescription() << "\n";

        midiMonitor.insertTextAtCaret (messageText);
    }

    void sendToOutputs (const MidiMessage& msg)
    {
        for (auto midiOutput : midiOutputs)
            if (midiOutput->outDevice.get() != nullptr)
                midiOutput->outDevice->sendMessageNow (msg);
    }

    //==============================================================================
    bool hasDeviceListChanged (const StringArray& deviceNames, bool isInputDevice)
    {
        ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                                : midiOutputs;

        if (deviceNames.size() != midiDevices.size())
            return true;

        for (auto i = 0; i < deviceNames.size(); ++i)
            if (deviceNames[i] != midiDevices[i]->name)
                return true;

        return false;
    }

    ReferenceCountedObjectPtr<MidiDeviceListEntry> findDeviceWithName (const String& name, bool isInputDevice) const
    {
        const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                                      : midiOutputs;

        for (auto midiDevice : midiDevices)
            if (midiDevice->name == name)
                return midiDevice;

        return nullptr;
    }

    void closeUnpluggedDevices (StringArray& currentlyPluggedInDevices, bool isInputDevice)
    {
        ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
                                                                                : midiOutputs;

        for (auto i = midiDevices.size(); --i >= 0;)
        {
            auto& d = *midiDevices[i];

            if (! currentlyPluggedInDevices.contains (d.name))
            {
                if (isInputDevice ? d.inDevice .get() != nullptr
                                  : d.outDevice.get() != nullptr)
                    closeDevice (isInputDevice, i);

                midiDevices.remove (i);
            }
        }
    }

    void updateDeviceList (bool isInputDeviceList)
    {
        auto newDeviceNames = isInputDeviceList ? MidiInput::getDevices()
                                                : MidiOutput::getDevices();

        if (hasDeviceListChanged (newDeviceNames, isInputDeviceList))
        {

            ReferenceCountedArray<MidiDeviceListEntry>& midiDevices
                = isInputDeviceList ? midiInputs : midiOutputs;

            closeUnpluggedDevices (newDeviceNames, isInputDeviceList);

            ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;

            // add all currently plugged-in devices to the device list
            for (auto newDeviceName : newDeviceNames)
            {
                MidiDeviceListEntry::Ptr entry = findDeviceWithName (newDeviceName, isInputDeviceList);

                if (entry == nullptr)
                    entry = new MidiDeviceListEntry (newDeviceName);

                newDeviceList.add (entry);
            }

            // actually update the device list
            midiDevices = newDeviceList;

            // update the selection status of the combo-box
            if (auto* midiSelector = isInputDeviceList ? midiInputSelector.get() : midiOutputSelector.get())
                midiSelector->syncSelectedItemsWithDeviceList (midiDevices);
        }
    }

    //==============================================================================
    void addLabelAndSetStyle (Label& label)
    {
        label.setFont (Font (15.00f, Font::plain));
        label.setJustificationType (Justification::centredLeft);
        label.setEditable (false, false, false);
        label.setColour (TextEditor::textColourId, Colours::black);
        label.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (label);
    }

    //==============================================================================
    Label midiInputLabel    { "Midi Input Label",  "MIDI Input:" };
    Label midiOutputLabel   { "Midi Output Label", "MIDI Output:" };
    Label incomingMidiLabel { "Incoming Midi Label", "Received MIDI messages:" };
    Label outgoingMidiLabel { "Outgoing Midi Label", "Play the keyboard to send MIDI messages..." };
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent midiKeyboard;
    TextEditor midiMonitor  { "MIDI Monitor" };
    TextButton pairButton   { "MIDI Bluetooth devices..." };

    std::unique_ptr<MidiDeviceListBox> midiInputSelector, midiOutputSelector;
    ReferenceCountedArray<MidiDeviceListEntry> midiInputs, midiOutputs;

    CriticalSection midiMonitorLock;
    Array<MidiMessage> incomingMessages;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiDemo)
};
