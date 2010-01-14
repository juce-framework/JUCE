/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioDeviceSelectorComponent.h"
#include "../buttons/juce_TextButton.h"
#include "../menus/juce_PopupMenu.h"
#include "../windows/juce_AlertWindow.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../text/juce_LocalisedStrings.h"


//==============================================================================
class SimpleDeviceManagerInputLevelMeter  : public Component,
                                            public Timer
{
public:
    SimpleDeviceManagerInputLevelMeter (AudioDeviceManager* const manager_)
        : manager (manager_),
          level (0)
    {
        startTimer (50);
        manager->enableInputLevelMeasurement (true);
    }

    ~SimpleDeviceManagerInputLevelMeter()
    {
        manager->enableInputLevelMeasurement (false);
    }

    void timerCallback()
    {
        const float newLevel = (float) manager->getCurrentInputLevel();

        if (fabsf (level - newLevel) > 0.005f)
        {
            level = newLevel;
            repaint();
        }
    }

    void paint (Graphics& g)
    {
        getLookAndFeel().drawLevelMeter (g, getWidth(), getHeight(),
                                         (float) exp (log (level) / 3.0)); // (add a bit of a skew to make the level more obvious)
    }

private:
    AudioDeviceManager* const manager;
    float level;
};


//==============================================================================
class MidiInputSelectorComponentListBox  : public ListBox,
                                           public ListBoxModel
{
public:
    //==============================================================================
    MidiInputSelectorComponentListBox (AudioDeviceManager& deviceManager_,
                                       const String& noItemsMessage_,
                                       const int minNumber_,
                                       const int maxNumber_)
        : ListBox (String::empty, 0),
          deviceManager (deviceManager_),
          noItemsMessage (noItemsMessage_),
          minNumber (minNumber_),
          maxNumber (maxNumber_)
    {
        items = MidiInput::getDevices();

        setModel (this);
        setOutlineThickness (1);
    }

    ~MidiInputSelectorComponentListBox()
    {
    }

    int getNumRows()
    {
        return items.size();
    }

    void paintListBoxItem (int row,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected)
    {
        if (((unsigned int) row) < (unsigned int) items.size())
        {
            if (rowIsSelected)
                g.fillAll (findColour (TextEditor::highlightColourId)
                               .withMultipliedAlpha (0.3f));

            const String item (items [row]);
            bool enabled = deviceManager.isMidiInputEnabled (item);

            const int x = getTickX();
            const int tickW = height - height / 4;

            getLookAndFeel().drawTickBox (g, *this, x - tickW, (height - tickW) / 2, tickW, tickW,
                                          enabled, true, true, false);

            g.setFont (height * 0.6f);
            g.setColour (findColour (ListBox::textColourId, true).withMultipliedAlpha (enabled ? 1.0f : 0.6f));
            g.drawText (item, x, 0, width - x - 2, height, Justification::centredLeft, true);
        }
    }

    void listBoxItemClicked (int row, const MouseEvent& e)
    {
        selectRow (row);

        if (e.x < getTickX())
            flipEnablement (row);
    }

    void listBoxItemDoubleClicked (int row, const MouseEvent&)
    {
        flipEnablement (row);
    }

    void returnKeyPressed (int row)
    {
        flipEnablement (row);
    }

    void paint (Graphics& g)
    {
        ListBox::paint (g);

        if (items.size() == 0)
        {
            g.setColour (Colours::grey);
            g.setFont (13.0f);
            g.drawText (noItemsMessage,
                        0, 0, getWidth(), getHeight() / 2,
                        Justification::centred, true);
        }
    }

    int getBestHeight (const int preferredHeight)
    {
        const int extra = getOutlineThickness() * 2;

        return jmax (getRowHeight() * 2 + extra,
                     jmin (getRowHeight() * getNumRows() + extra,
                           preferredHeight));
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioDeviceManager& deviceManager;
    const String noItemsMessage;
    StringArray items;
    int minNumber, maxNumber;

    void flipEnablement (const int row)
    {
        if (((unsigned int) row) < (unsigned int) items.size())
        {
            const String item (items [row]);
            deviceManager.setMidiInputEnabled (item, ! deviceManager.isMidiInputEnabled (item));
        }
    }

    int getTickX() const
    {
        return getRowHeight() + 5;
    }

    MidiInputSelectorComponentListBox (const MidiInputSelectorComponentListBox&);
    const MidiInputSelectorComponentListBox& operator= (const MidiInputSelectorComponentListBox&);
};


//==============================================================================
class AudioDeviceSettingsPanel : public Component,
                                 public ComboBoxListener,
                                 public ChangeListener,
                                 public ButtonListener
{
public:
    AudioDeviceSettingsPanel (AudioIODeviceType* type_,
                              AudioIODeviceType::DeviceSetupDetails& setup_,
                              const bool hideAdvancedOptionsWithButton)
        : type (type_),
          setup (setup_)
    {
        sampleRateDropDown = 0;
        sampleRateLabel = 0;
        bufferSizeDropDown = 0;
        bufferSizeLabel = 0;
        outputDeviceDropDown = 0;
        outputDeviceLabel = 0;
        inputDeviceDropDown = 0;
        inputDeviceLabel = 0;
        testButton = 0;
        inputLevelMeter = 0;
        showUIButton = 0;
        inputChanList = 0;
        outputChanList = 0;
        inputChanLabel = 0;
        outputChanLabel = 0;
        showAdvancedSettingsButton = 0;

        if (hideAdvancedOptionsWithButton)
        {
            addAndMakeVisible (showAdvancedSettingsButton = new TextButton (TRANS("Show advanced settings...")));
            showAdvancedSettingsButton->addButtonListener (this);
        }

        type->scanForDevices();

        setup.manager->addChangeListener (this);
        changeListenerCallback (0);
    }

    ~AudioDeviceSettingsPanel()
    {
        setup.manager->removeChangeListener (this);

        deleteAndZero (outputDeviceLabel);
        deleteAndZero (inputDeviceLabel);
        deleteAndZero (sampleRateLabel);
        deleteAndZero (bufferSizeLabel);
        deleteAndZero (showUIButton);
        deleteAndZero (inputChanLabel);
        deleteAndZero (outputChanLabel);
        deleteAndZero (showAdvancedSettingsButton);

        deleteAllChildren();
    }

    void resized()
    {
        const int lx = proportionOfWidth (0.35f);
        const int w = proportionOfWidth (0.4f);
        const int h = 24;
        const int space = 6;
        const int dh = h + space;
        int y = 0;

        if (outputDeviceDropDown != 0)
        {
            outputDeviceDropDown->setBounds (lx, y, w, h);

            if (testButton != 0)
                testButton->setBounds (proportionOfWidth (0.77f),
                                       outputDeviceDropDown->getY(),
                                       proportionOfWidth (0.18f),
                                       h);
            y += dh;
        }

        if (inputDeviceDropDown != 0)
        {
            inputDeviceDropDown->setBounds (lx, y, w, h);

            inputLevelMeter->setBounds (proportionOfWidth (0.77f),
                                        inputDeviceDropDown->getY(),
                                        proportionOfWidth (0.18f),
                                        h);
            y += dh;
        }

        const int maxBoxHeight = 100;//(getHeight() - y - dh * 2) / numBoxes;

        if (outputChanList != 0)
        {
            const int bh = outputChanList->getBestHeight (maxBoxHeight);
            outputChanList->setBounds (lx, y, proportionOfWidth (0.55f), bh);
            y += bh + space;
        }

        if (inputChanList != 0)
        {
            const int bh = inputChanList->getBestHeight (maxBoxHeight);
            inputChanList->setBounds (lx, y, proportionOfWidth (0.55f), bh);
            y += bh + space;
        }

        y += space * 2;

        if (showAdvancedSettingsButton != 0)
        {
            showAdvancedSettingsButton->changeWidthToFitText (h);
            showAdvancedSettingsButton->setTopLeftPosition (lx, y);
        }

        if (sampleRateDropDown != 0)
        {
            sampleRateDropDown->setVisible (showAdvancedSettingsButton == 0
                                             || ! showAdvancedSettingsButton->isVisible());

            sampleRateDropDown->setBounds (lx, y, w, h);
            y += dh;
        }

        if (bufferSizeDropDown != 0)
        {
            bufferSizeDropDown->setVisible (showAdvancedSettingsButton == 0
                                             || ! showAdvancedSettingsButton->isVisible());
            bufferSizeDropDown->setBounds (lx, y, w, h);
            y += dh;
        }

        if (showUIButton != 0)
        {
            showUIButton->setVisible (showAdvancedSettingsButton == 0
                                        || ! showAdvancedSettingsButton->isVisible());
            showUIButton->changeWidthToFitText (h);
            showUIButton->setTopLeftPosition (lx, y);
        }
    }

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged)
    {
        if (comboBoxThatHasChanged == 0)
            return;

        AudioDeviceManager::AudioDeviceSetup config;
        setup.manager->getAudioDeviceSetup (config);
        String error;

        if (comboBoxThatHasChanged == outputDeviceDropDown
              || comboBoxThatHasChanged == inputDeviceDropDown)
        {
            if (outputDeviceDropDown != 0)
                config.outputDeviceName = outputDeviceDropDown->getSelectedId() < 0 ? String::empty
                                                                                    : outputDeviceDropDown->getText();

            if (inputDeviceDropDown != 0)
                config.inputDeviceName = inputDeviceDropDown->getSelectedId() < 0 ? String::empty
                                                                                  : inputDeviceDropDown->getText();

            if (! type->hasSeparateInputsAndOutputs())
                config.inputDeviceName = config.outputDeviceName;

            if (comboBoxThatHasChanged == inputDeviceDropDown)
                config.useDefaultInputChannels = true;
            else
                config.useDefaultOutputChannels = true;

            error = setup.manager->setAudioDeviceSetup (config, true);

            showCorrectDeviceName (inputDeviceDropDown, true);
            showCorrectDeviceName (outputDeviceDropDown, false);

            updateControlPanelButton();
            resized();
        }
        else if (comboBoxThatHasChanged == sampleRateDropDown)
        {
            if (sampleRateDropDown->getSelectedId() > 0)
            {
                config.sampleRate = sampleRateDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }
        else if (comboBoxThatHasChanged == bufferSizeDropDown)
        {
            if (bufferSizeDropDown->getSelectedId() > 0)
            {
                config.bufferSize = bufferSizeDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }

        if (error.isNotEmpty())
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         T("Error when trying to open audio device!"),
                                         error);
        }
    }

    void buttonClicked (Button* button)
    {
        if (button == showAdvancedSettingsButton)
        {
            showAdvancedSettingsButton->setVisible (false);
            resized();
        }
        else if (button == showUIButton)
        {
            AudioIODevice* const device = setup.manager->getCurrentAudioDevice();

            if (device != 0 && device->showControlPanel())
            {
                setup.manager->closeAudioDevice();
                setup.manager->restartLastAudioDevice();
                getTopLevelComponent()->toFront (true);
            }
        }
        else if (button == testButton && testButton != 0)
        {
            setup.manager->playTestSound();
        }
    }

    void updateControlPanelButton()
    {
        AudioIODevice* const currentDevice = setup.manager->getCurrentAudioDevice();

        deleteAndZero (showUIButton);

        if (currentDevice != 0 && currentDevice->hasControlPanel())
        {
            addAndMakeVisible (showUIButton = new TextButton (TRANS ("show this device's control panel"),
                                                              TRANS ("opens the device's own control panel")));
            showUIButton->addButtonListener (this);
        }

        resized();
    }

    void changeListenerCallback (void*)
    {
        AudioIODevice* const currentDevice = setup.manager->getCurrentAudioDevice();

        if (setup.maxNumOutputChannels > 0 || ! type->hasSeparateInputsAndOutputs())
        {
            if (outputDeviceDropDown == 0)
            {
                outputDeviceDropDown = new ComboBox (String::empty);
                outputDeviceDropDown->addListener (this);
                addAndMakeVisible (outputDeviceDropDown);

                outputDeviceLabel = new Label (String::empty,
                                               type->hasSeparateInputsAndOutputs() ? TRANS ("output:")
                                                                                   : TRANS ("device:"));
                outputDeviceLabel->attachToComponent (outputDeviceDropDown, true);

                if (setup.maxNumOutputChannels > 0)
                {
                    addAndMakeVisible (testButton = new TextButton (TRANS ("Test")));
                    testButton->addButtonListener (this);
                }
            }

            addNamesToDeviceBox (*outputDeviceDropDown, false);
        }

        if (setup.maxNumInputChannels > 0 && type->hasSeparateInputsAndOutputs())
        {
            if (inputDeviceDropDown == 0)
            {
                inputDeviceDropDown = new ComboBox (String::empty);
                inputDeviceDropDown->addListener (this);
                addAndMakeVisible (inputDeviceDropDown);

                inputDeviceLabel = new Label (String::empty, TRANS ("input:"));
                inputDeviceLabel->attachToComponent (inputDeviceDropDown, true);

                addAndMakeVisible (inputLevelMeter
                    = new SimpleDeviceManagerInputLevelMeter (setup.manager));
            }

            addNamesToDeviceBox (*inputDeviceDropDown, true);
        }

        updateControlPanelButton();
        showCorrectDeviceName (inputDeviceDropDown, true);
        showCorrectDeviceName (outputDeviceDropDown, false);

        if (currentDevice != 0)
        {
            if (setup.maxNumOutputChannels > 0
                 && setup.minNumOutputChannels < setup.manager->getCurrentAudioDevice()->getOutputChannelNames().size())
            {
                if (outputChanList == 0)
                {
                    addAndMakeVisible (outputChanList
                        = new ChannelSelectorListBox (setup, ChannelSelectorListBox::audioOutputType,
                                                      TRANS ("(no audio output channels found)")));
                    outputChanLabel = new Label (String::empty, TRANS ("active output channels:"));
                    outputChanLabel->attachToComponent (outputChanList, true);
                }

                outputChanList->refresh();
            }
            else
            {
                deleteAndZero (outputChanLabel);
                deleteAndZero (outputChanList);
            }

            if (setup.maxNumInputChannels > 0
                 && setup.minNumInputChannels < setup.manager->getCurrentAudioDevice()->getInputChannelNames().size())
            {
                if (inputChanList == 0)
                {
                    addAndMakeVisible (inputChanList
                        = new ChannelSelectorListBox (setup, ChannelSelectorListBox::audioInputType,
                                                      TRANS ("(no audio input channels found)")));
                    inputChanLabel = new Label (String::empty, TRANS ("active input channels:"));
                    inputChanLabel->attachToComponent (inputChanList, true);
                }

                inputChanList->refresh();
            }
            else
            {
                deleteAndZero (inputChanLabel);
                deleteAndZero (inputChanList);
            }

            // sample rate..
            {
                if (sampleRateDropDown == 0)
                {
                    addAndMakeVisible (sampleRateDropDown = new ComboBox (String::empty));
                    sampleRateDropDown->addListener (this);

                    delete sampleRateLabel;
                    sampleRateLabel = new Label (String::empty, TRANS ("sample rate:"));
                    sampleRateLabel->attachToComponent (sampleRateDropDown, true);
                }
                else
                {
                    sampleRateDropDown->clear();
                    sampleRateDropDown->removeListener (this);
                }

                const int numRates = currentDevice->getNumSampleRates();

                for (int i = 0; i < numRates; ++i)
                {
                    const int rate = roundToInt (currentDevice->getSampleRate (i));
                    sampleRateDropDown->addItem (String (rate) + T(" Hz"), rate);
                }

                sampleRateDropDown->setSelectedId (roundToInt (currentDevice->getCurrentSampleRate()), true);
                sampleRateDropDown->addListener (this);
            }

            // buffer size
            {
                if (bufferSizeDropDown == 0)
                {
                    addAndMakeVisible (bufferSizeDropDown = new ComboBox (String::empty));
                    bufferSizeDropDown->addListener (this);

                    delete bufferSizeLabel;
                    bufferSizeLabel = new Label (String::empty, TRANS ("audio buffer size:"));
                    bufferSizeLabel->attachToComponent (bufferSizeDropDown, true);
                }
                else
                {
                    bufferSizeDropDown->clear();
                }

                const int numBufferSizes = currentDevice->getNumBufferSizesAvailable();
                double currentRate = currentDevice->getCurrentSampleRate();
                if (currentRate == 0)
                    currentRate = 48000.0;

                for (int i = 0; i < numBufferSizes; ++i)
                {
                    const int bs = currentDevice->getBufferSizeSamples (i);
                    bufferSizeDropDown->addItem (String (bs)
                                                  + T(" samples (")
                                                  + String (bs * 1000.0 / currentRate, 1)
                                                  + T(" ms)"),
                                                 bs);
                }

                bufferSizeDropDown->setSelectedId (currentDevice->getCurrentBufferSizeSamples(), true);
            }
        }
        else
        {
            jassert (setup.manager->getCurrentAudioDevice() == 0); // not the correct device type!

            deleteAndZero (sampleRateLabel);
            deleteAndZero (bufferSizeLabel);
            deleteAndZero (sampleRateDropDown);
            deleteAndZero (bufferSizeDropDown);

            if (outputDeviceDropDown != 0)
                outputDeviceDropDown->setSelectedId (-1, true);

            if (inputDeviceDropDown != 0)
                inputDeviceDropDown->setSelectedId (-1, true);
        }

        resized();
        setSize (getWidth(), getLowestY() + 4);
    }

private:
    AudioIODeviceType* const type;
    const AudioIODeviceType::DeviceSetupDetails setup;

    ComboBox* outputDeviceDropDown;
    ComboBox* inputDeviceDropDown;
    ComboBox* sampleRateDropDown;
    ComboBox* bufferSizeDropDown;
    Label* outputDeviceLabel;
    Label* inputDeviceLabel;
    Label* sampleRateLabel;
    Label* bufferSizeLabel;
    Label* inputChanLabel;
    Label* outputChanLabel;
    TextButton* testButton;
    Component* inputLevelMeter;
    TextButton* showUIButton;
    TextButton* showAdvancedSettingsButton;

    void showCorrectDeviceName (ComboBox* const box, const bool isInput)
    {
        if (box != 0)
        {
            AudioIODevice* const currentDevice = dynamic_cast <AudioIODevice*> (setup.manager->getCurrentAudioDevice());

            const int index = type->getIndexOfDevice (currentDevice, isInput);

            box->setSelectedId (index + 1, true);

            if (testButton != 0 && ! isInput)
                testButton->setEnabled (index >= 0);
        }
    }

    void addNamesToDeviceBox (ComboBox& combo, bool isInputs)
    {
        const StringArray devs (type->getDeviceNames (isInputs));

        combo.clear (true);

        for (int i = 0; i < devs.size(); ++i)
            combo.addItem (devs[i], i + 1);

        combo.addItem (TRANS("<< none >>"), -1);
        combo.setSelectedId (-1, true);
    }

    int getLowestY() const
    {
        int y = 0;

        for (int i = getNumChildComponents(); --i >= 0;)
            y = jmax (y, getChildComponent (i)->getBottom());

        return y;
    }

public:
    //==============================================================================
    class ChannelSelectorListBox  : public ListBox,
                                    public ListBoxModel
    {
    public:
        enum BoxType
        {
            audioInputType,
            audioOutputType
        };

        //==============================================================================
        ChannelSelectorListBox (const AudioIODeviceType::DeviceSetupDetails& setup_,
                                const BoxType type_,
                                const String& noItemsMessage_)
            : ListBox (String::empty, 0),
              setup (setup_),
              type (type_),
              noItemsMessage (noItemsMessage_)
        {
            refresh();
            setModel (this);
            setOutlineThickness (1);
        }

        ~ChannelSelectorListBox()
        {
        }

        void refresh()
        {
            items.clear();

            AudioIODevice* const currentDevice = setup.manager->getCurrentAudioDevice();

            if (currentDevice != 0)
            {
                if (type == audioInputType)
                    items = currentDevice->getInputChannelNames();
                else if (type == audioOutputType)
                    items = currentDevice->getOutputChannelNames();

                if (setup.useStereoPairs)
                {
                    StringArray pairs;

                    for (int i = 0; i < items.size(); i += 2)
                    {
                        String name (items[i]);
                        String name2 (items[i + 1]);

                        String commonBit;

                        for (int j = 0; j < name.length(); ++j)
                            if (name.substring (0, j).equalsIgnoreCase (name2.substring (0, j)))
                                commonBit = name.substring (0, j);

                        pairs.add (name.trim()
                                    + " + "
                                    + name2.substring (commonBit.length()).trim());
                    }

                    items = pairs;
                }
            }

            updateContent();
            repaint();
        }

        int getNumRows()
        {
            return items.size();
        }

        void paintListBoxItem (int row,
                               Graphics& g,
                               int width, int height,
                               bool rowIsSelected)
        {
            if (((unsigned int) row) < (unsigned int) items.size())
            {
                if (rowIsSelected)
                    g.fillAll (findColour (TextEditor::highlightColourId)
                                   .withMultipliedAlpha (0.3f));

                const String item (items [row]);
                bool enabled = false;

                AudioDeviceManager::AudioDeviceSetup config;
                setup.manager->getAudioDeviceSetup (config);

                if (setup.useStereoPairs)
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels [row * 2] || config.inputChannels [row * 2 + 1];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels [row * 2] || config.outputChannels [row * 2 + 1];
                }
                else
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels [row];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels [row];
                }

                const int x = getTickX();
                const int tickW = height - height / 4;

                getLookAndFeel().drawTickBox (g, *this, x - tickW, (height - tickW) / 2, tickW, tickW,
                                              enabled, true, true, false);

                g.setFont (height * 0.6f);
                g.setColour (findColour (ListBox::textColourId, true).withMultipliedAlpha (enabled ? 1.0f : 0.6f));
                g.drawText (item, x, 0, width - x - 2, height, Justification::centredLeft, true);
            }
        }

        void listBoxItemClicked (int row, const MouseEvent& e)
        {
            selectRow (row);

            if (e.x < getTickX())
                flipEnablement (row);
        }

        void listBoxItemDoubleClicked (int row, const MouseEvent&)
        {
            flipEnablement (row);
        }

        void returnKeyPressed (int row)
        {
            flipEnablement (row);
        }

        void paint (Graphics& g)
        {
            ListBox::paint (g);

            if (items.size() == 0)
            {
                g.setColour (Colours::grey);
                g.setFont (13.0f);
                g.drawText (noItemsMessage,
                            0, 0, getWidth(), getHeight() / 2,
                            Justification::centred, true);
            }
        }

        int getBestHeight (int maxHeight)
        {
            return getRowHeight() * jlimit (2, jmax (2, maxHeight / getRowHeight()),
                                            getNumRows())
                       + getOutlineThickness() * 2;
        }

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        const AudioIODeviceType::DeviceSetupDetails setup;
        const BoxType type;
        const String noItemsMessage;
        StringArray items;

        void flipEnablement (const int row)
        {
            jassert (type == audioInputType || type == audioOutputType);

            if (((unsigned int) row) < (unsigned int) items.size())
            {
                AudioDeviceManager::AudioDeviceSetup config;
                setup.manager->getAudioDeviceSetup (config);

                if (setup.useStereoPairs)
                {
                    BitArray bits;
                    BitArray& original = (type == audioInputType ? config.inputChannels
                                                                 : config.outputChannels);

                    int i;
                    for (i = 0; i < 256; i += 2)
                        bits.setBit (i / 2, original [i] || original [i + 1]);

                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (bits, row, setup.minNumInputChannels / 2, setup.maxNumInputChannels / 2);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (bits, row, setup.minNumOutputChannels / 2, setup.maxNumOutputChannels / 2);
                    }

                    for (i = 0; i < 256; ++i)
                        original.setBit (i, bits [i / 2]);
                }
                else
                {
                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (config.inputChannels, row, setup.minNumInputChannels, setup.maxNumInputChannels);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (config.outputChannels, row, setup.minNumOutputChannels, setup.maxNumOutputChannels);
                    }
                }

                String error (setup.manager->setAudioDeviceSetup (config, true));

                if (! error.isEmpty())
                {
                    //xxx
                }
            }
        }

        static void flipBit (BitArray& chans, int index, int minNumber, int maxNumber)
        {
            const int numActive = chans.countNumberOfSetBits();

            if (chans [index])
            {
                if (numActive > minNumber)
                    chans.setBit (index, false);
            }
            else
            {
                if (numActive >= maxNumber)
                {
                    const int firstActiveChan = chans.findNextSetBit();

                    chans.setBit (index > firstActiveChan
                                     ? firstActiveChan : chans.getHighestBit(),
                                  false);
                }

                chans.setBit (index, true);
            }
        }

        int getTickX() const
        {
            return getRowHeight() + 5;
        }

        ChannelSelectorListBox (const ChannelSelectorListBox&);
        const ChannelSelectorListBox& operator= (const ChannelSelectorListBox&);
    };

private:
    ChannelSelectorListBox* inputChanList;
    ChannelSelectorListBox* outputChanList;

    AudioDeviceSettingsPanel (const AudioDeviceSettingsPanel&);
    const AudioDeviceSettingsPanel& operator= (const AudioDeviceSettingsPanel&);
};


//==============================================================================
AudioDeviceSelectorComponent::AudioDeviceSelectorComponent (AudioDeviceManager& deviceManager_,
                                                            const int minInputChannels_,
                                                            const int maxInputChannels_,
                                                            const int minOutputChannels_,
                                                            const int maxOutputChannels_,
                                                            const bool showMidiInputOptions,
                                                            const bool showMidiOutputSelector,
                                                            const bool showChannelsAsStereoPairs_,
                                                            const bool hideAdvancedOptionsWithButton_)
    : deviceManager (deviceManager_),
      deviceTypeDropDown (0),
      deviceTypeDropDownLabel (0),
      audioDeviceSettingsComp (0),
      minOutputChannels (minOutputChannels_),
      maxOutputChannels (maxOutputChannels_),
      minInputChannels (minInputChannels_),
      maxInputChannels (maxInputChannels_),
      showChannelsAsStereoPairs (showChannelsAsStereoPairs_),
      hideAdvancedOptionsWithButton (hideAdvancedOptionsWithButton_)
{
    jassert (minOutputChannels >= 0 && minOutputChannels <= maxOutputChannels);
    jassert (minInputChannels >= 0 && minInputChannels <= maxInputChannels);

    if (deviceManager_.getAvailableDeviceTypes().size() > 1)
    {
        deviceTypeDropDown = new ComboBox (String::empty);

        for (int i = 0; i < deviceManager_.getAvailableDeviceTypes().size(); ++i)
        {
            deviceTypeDropDown
                ->addItem (deviceManager_.getAvailableDeviceTypes().getUnchecked(i)->getTypeName(),
                           i + 1);
        }

        addAndMakeVisible (deviceTypeDropDown);
        deviceTypeDropDown->addListener (this);

        deviceTypeDropDownLabel = new Label (String::empty, TRANS ("audio device type:"));
        deviceTypeDropDownLabel->setJustificationType (Justification::centredRight);
        deviceTypeDropDownLabel->attachToComponent (deviceTypeDropDown, true);
    }

    if (showMidiInputOptions)
    {
        addAndMakeVisible (midiInputsList
                            = new MidiInputSelectorComponentListBox (deviceManager,
                                                                     TRANS("(no midi inputs available)"),
                                                                     0, 0));

        midiInputsLabel = new Label (String::empty, TRANS ("active midi inputs:"));
        midiInputsLabel->setJustificationType (Justification::topRight);
        midiInputsLabel->attachToComponent (midiInputsList, true);
    }
    else
    {
        midiInputsList = 0;
        midiInputsLabel = 0;
    }

    if (showMidiOutputSelector)
    {
        addAndMakeVisible (midiOutputSelector = new ComboBox (String::empty));
        midiOutputSelector->addListener (this);

        midiOutputLabel = new Label ("lm", TRANS("Midi Output:"));
        midiOutputLabel->attachToComponent (midiOutputSelector, true);
    }
    else
    {
        midiOutputSelector = 0;
        midiOutputLabel = 0;
    }

    deviceManager_.addChangeListener (this);
    changeListenerCallback (0);
}

AudioDeviceSelectorComponent::~AudioDeviceSelectorComponent()
{
    deviceManager.removeChangeListener (this);
    deleteAllChildren();
}

void AudioDeviceSelectorComponent::resized()
{
    const int lx = proportionOfWidth (0.35f);
    const int w = proportionOfWidth (0.4f);
    const int h = 24;
    const int space = 6;
    const int dh = h + space;
    int y = 15;

    if (deviceTypeDropDown != 0)
    {
        deviceTypeDropDown->setBounds (lx, y, proportionOfWidth (0.3f), h);
        y += dh + space * 2;
    }

    if (audioDeviceSettingsComp != 0)
    {
        audioDeviceSettingsComp->setBounds (0, y, getWidth(), audioDeviceSettingsComp->getHeight());
        y += audioDeviceSettingsComp->getHeight() + space;
    }

    if (midiInputsList != 0)
    {
        const int bh = midiInputsList->getBestHeight (jmin (h * 8, getHeight() - y - space - h));
        midiInputsList->setBounds (lx, y, w, bh);
        y += bh + space;
    }

    if (midiOutputSelector != 0)
        midiOutputSelector->setBounds (lx, y, w, h);
}

void AudioDeviceSelectorComponent::childBoundsChanged (Component* child)
{
    if (child == audioDeviceSettingsComp)
        resized();
}

void AudioDeviceSelectorComponent::buttonClicked (Button*)
{
    AudioIODevice* const device = deviceManager.getCurrentAudioDevice();

    if (device != 0 && device->hasControlPanel())
    {
        if (device->showControlPanel())
            deviceManager.restartLastAudioDevice();

        getTopLevelComponent()->toFront (true);
    }
}

void AudioDeviceSelectorComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == deviceTypeDropDown)
    {
        AudioIODeviceType* const type = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown->getSelectedId() - 1];

        if (type != 0)
        {
            deleteAndZero (audioDeviceSettingsComp);

            deviceManager.setCurrentAudioDeviceType (type->getTypeName(), true);

            changeListenerCallback (0); // needed in case the type hasn't actally changed
        }
    }
    else if (comboBoxThatHasChanged == midiOutputSelector)
    {
        deviceManager.setDefaultMidiOutput (midiOutputSelector->getText());
    }
}

void AudioDeviceSelectorComponent::changeListenerCallback (void*)
{
    if (deviceTypeDropDown != 0)
    {
        deviceTypeDropDown->setText (deviceManager.getCurrentAudioDeviceType(), false);
    }

    if (audioDeviceSettingsComp == 0
         || audioDeviceSettingsCompType != deviceManager.getCurrentAudioDeviceType())
    {
        audioDeviceSettingsCompType = deviceManager.getCurrentAudioDeviceType();

        deleteAndZero (audioDeviceSettingsComp);

        AudioIODeviceType* const type
            = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown == 0
                                                        ? 0 : deviceTypeDropDown->getSelectedId() - 1];

        if (type != 0)
        {
            AudioIODeviceType::DeviceSetupDetails details;
            details.manager = &deviceManager;
            details.minNumInputChannels = minInputChannels;
            details.maxNumInputChannels = maxInputChannels;
            details.minNumOutputChannels = minOutputChannels;
            details.maxNumOutputChannels = maxOutputChannels;
            details.useStereoPairs = showChannelsAsStereoPairs;

            audioDeviceSettingsComp = new AudioDeviceSettingsPanel (type, details, hideAdvancedOptionsWithButton);

            if (audioDeviceSettingsComp != 0)
            {
                addAndMakeVisible (audioDeviceSettingsComp);
                audioDeviceSettingsComp->resized();
            }
        }
    }

    if (midiInputsList != 0)
    {
        midiInputsList->updateContent();
        midiInputsList->repaint();
    }

    if (midiOutputSelector != 0)
    {
        midiOutputSelector->clear();

        const StringArray midiOuts (MidiOutput::getDevices());

        midiOutputSelector->addItem (TRANS("<< none >>"), -1);
        midiOutputSelector->addSeparator();

        for (int i = 0; i < midiOuts.size(); ++i)
            midiOutputSelector->addItem (midiOuts[i], i + 1);

        int current = -1;

        if (deviceManager.getDefaultMidiOutput() != 0)
            current = 1 + midiOuts.indexOf (deviceManager.getDefaultMidiOutputName());

        midiOutputSelector->setSelectedId (current, true);
    }

    resized();
}


END_JUCE_NAMESPACE
