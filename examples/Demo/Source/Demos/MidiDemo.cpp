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

#include "../JuceDemoHeader.h"

/** Simple list box that just displays a StringArray. */
class MidiLogListBoxModel   : public ListBoxModel
{
public:
    MidiLogListBoxModel (const Array<MidiMessage>& list)
        : midiMessageList (list)
    {
    }

    int getNumRows() override    { return midiMessageList.size(); }

    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (Colours::blue.withAlpha (0.2f));

        if (isPositiveAndBelow (row, midiMessageList.size()))
        {
            g.setColour (Colours::black);

            const MidiMessage& message = midiMessageList.getReference (row);
            double time = message.getTimeStamp();

            g.drawText (String::formatted ("%02d:%02d:%02d",
                                           ((int) (time / 3600.0)) % 24,
                                           ((int) (time / 60.0)) % 60,
                                           ((int) time) % 60)
                            + "  -  " + message.getDescription(),
                        Rectangle<int> (width, height).reduced (4, 0),
                        Justification::centredLeft, true);
        }
    }

private:
    const Array<MidiMessage>& midiMessageList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiLogListBoxModel)
};

//==============================================================================
class MidiDemo  : public Component,
                  private ComboBox::Listener,
                  private MidiInputCallback,
                  private MidiKeyboardStateListener,
                  private AsyncUpdater
{
public:
    MidiDemo()
        : deviceManager (MainAppWindow::getSharedAudioDeviceManager()),
          lastInputIndex (0), isAddingFromMidiInput (false),
          keyboardComponent (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
          midiLogListBoxModel (midiMessageList)
    {
        setOpaque (true);

        // MIDI Inputs
        addAndMakeVisible (midiInputListLabel);
        midiInputListLabel.setText ("MIDI Input:", dontSendNotification);
        midiInputListLabel.attachToComponent (&midiInputList, true);

        addAndMakeVisible (midiInputList);
        midiInputList.setTextWhenNoChoicesAvailable ("No MIDI Inputs Enabled");
        const StringArray midiInputs (MidiInput::getDevices());
        midiInputList.addItemList (midiInputs, 1);
        midiInputList.addListener (this);

        // find the first enabled device and use that by default
        for (int i = 0; i < midiInputs.size(); ++i)
        {
            if (deviceManager.isMidiInputEnabled (midiInputs[i]))
            {
                setMidiInput (i);
                break;
            }
        }

        // if no enabled devices were found just use the first one in the list
        if (midiInputList.getSelectedId() == 0)
            setMidiInput (0);


        // MIDI Outputs
        addAndMakeVisible (midiOutputListLabel);
        midiOutputListLabel.setText ("MIDI Output:", dontSendNotification);
        midiOutputListLabel.attachToComponent (&midiOutputList, true);

        addAndMakeVisible (midiOutputList);
        midiOutputList.setTextWhenNoChoicesAvailable ("No MIDI Output Enabled");
        midiOutputList.addItemList (MidiOutput::getDevices(), 1);
        midiOutputList.addListener (this);

        addAndMakeVisible (keyboardComponent);
        keyboardState.addListener (this);

        addAndMakeVisible (messageListBox);
        messageListBox.setModel (&midiLogListBoxModel);
        messageListBox.setColour (ListBox::backgroundColourId, Colour (0x32ffffff));
        messageListBox.setColour (ListBox::outlineColourId, Colours::black);
    }

    ~MidiDemo()
    {
        keyboardState.removeListener (this);
        deviceManager.removeMidiInputCallback (MidiInput::getDevices()[midiInputList.getSelectedItemIndex()], this);
        midiInputList.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        midiInputList.setBounds (area.removeFromTop (36).removeFromRight (getWidth() - 150).reduced (8));
        midiOutputList.setBounds (area.removeFromTop (36).removeFromRight (getWidth() - 150).reduced (8));
        keyboardComponent.setBounds (area.removeFromTop (80).reduced(8));
        messageListBox.setBounds (area.reduced (8));
    }

private:
    AudioDeviceManager& deviceManager;
    ComboBox midiInputList, midiOutputList;
    Label midiInputListLabel, midiOutputListLabel;
    int lastInputIndex;
    bool isAddingFromMidiInput;

    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboardComponent;

    ListBox messageListBox;
    Array<MidiMessage> midiMessageList;
    MidiLogListBoxModel midiLogListBoxModel;
    ScopedPointer<MidiOutput> currentMidiOutput;

    //==============================================================================
    /** Starts listening to a MIDI input device, enabling it if necessary. */
    void setMidiInput (int index)
    {
        const StringArray list (MidiInput::getDevices());

        deviceManager.removeMidiInputCallback (list[lastInputIndex], this);

        const String newInput (list[index]);

        if (! deviceManager.isMidiInputEnabled (newInput))
            deviceManager.setMidiInputEnabled (newInput, true);

        deviceManager.addMidiInputCallback (newInput, this);
        midiInputList.setSelectedId (index + 1, dontSendNotification);

        lastInputIndex = index;
    }

    //==============================================================================
    void setMidiOutput (int index)
    {
        currentMidiOutput = nullptr;

        if (MidiOutput::getDevices() [index].isNotEmpty())
        {
            currentMidiOutput = MidiOutput::openDevice (index);
            jassert (currentMidiOutput);
        }
    }

    void comboBoxChanged (ComboBox* box) override
    {
        if (box == &midiInputList)    setMidiInput  (midiInputList.getSelectedItemIndex());
        if (box == &midiOutputList)   setMidiOutput (midiOutputList.getSelectedItemIndex());
    }

    // These methods handle callbacks from the midi device + on-screen keyboard..
    void handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override
    {
        const ScopedValueSetter<bool> scopedInputFlag (isAddingFromMidiInput, true);
        keyboardState.processNextMidiEvent (message);
        postMessageToList (message);
    }

    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        if (! isAddingFromMidiInput)
        {
            MidiMessage m (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
            m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
            postMessageToList (m);
        }
    }

    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        if (! isAddingFromMidiInput)
        {
            MidiMessage m (MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
            m.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
            postMessageToList (m);
        }
    }

    // This is used to dispach an incoming message to the message thread
    struct IncomingMessageCallback   : public CallbackMessage
    {
        IncomingMessageCallback (MidiDemo* d, const MidiMessage& m)
            : demo (d), message (m) {}

        void messageCallback() override
        {
            if (demo != nullptr)
                demo->addMessageToList (message);
        }

        Component::SafePointer<MidiDemo> demo;
        MidiMessage message;
    };

    void postMessageToList (const MidiMessage& message)
    {
        if (currentMidiOutput != nullptr)
            currentMidiOutput->sendMessageNow (message);

        (new IncomingMessageCallback (this, message))->post();
    }

    void addMessageToList (const MidiMessage& message)
    {
        midiMessageList.add (message);
        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        messageListBox.updateContent();
        messageListBox.scrollToEnsureRowIsOnscreen (midiMessageList.size() - 1);
        messageListBox.repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<MidiDemo> demo ("32 Audio: MIDI i/o");
