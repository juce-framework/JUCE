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

#include "../JuceDemoHeader.h"

//==============================================================================
class AudioSettingsDemo  : public Component,
                           public ChangeListener
{
public:
    AudioSettingsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (audioSetupComp
                            = new AudioDeviceSelectorComponent (MainAppWindow::getSharedAudioDeviceManager(),
                                                                0, 256, 0, 256, true, true, true, false));

        addAndMakeVisible (diagnosticsBox);
        diagnosticsBox.setMultiLine (true);
        diagnosticsBox.setReturnKeyStartsNewLine (true);
        diagnosticsBox.setReadOnly (true);
        diagnosticsBox.setScrollbarsShown (true);
        diagnosticsBox.setCaretVisible (false);
        diagnosticsBox.setPopupMenuEnabled (true);

        MainAppWindow::getSharedAudioDeviceManager().addChangeListener (this);

        logMessage ("Audio device diagnostics:\n");
        dumpDeviceInfo();
    }

    ~AudioSettingsDemo()
    {
        MainAppWindow::getSharedAudioDeviceManager().removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (4));
        audioSetupComp->setBounds (r.removeFromTop (proportionOfHeight (0.65f)));
        diagnosticsBox.setBounds (r);
    }

    void dumpDeviceInfo()
    {
        AudioDeviceManager& dm = MainAppWindow::getSharedAudioDeviceManager();

        logMessage ("--------------------------------------");
        logMessage ("Current audio device type: " + (dm.getCurrentDeviceTypeObject() != nullptr
                                                        ? dm.getCurrentDeviceTypeObject()->getTypeName()
                                                        : "<none>"));

        if (AudioIODevice* device = dm.getCurrentAudioDevice())
        {
            logMessage ("Current audio device: " + device->getName().quoted());
            logMessage ("Sample rate: " + String (device->getCurrentSampleRate()) + " Hz");
            logMessage ("Block size: " + String (device->getCurrentBufferSizeSamples()) + " samples");
            logMessage ("Output Latency: " + String (device->getOutputLatencyInSamples()) + " samples");
            logMessage ("Input Latency: " + String (device->getInputLatencyInSamples()) + " samples");
            logMessage ("Bit depth: " + String (device->getCurrentBitDepth()));
            logMessage ("Input channel names: " + device->getInputChannelNames().joinIntoString (", "));
            logMessage ("Active input channels: " + getListOfActiveBits (device->getActiveInputChannels()));
            logMessage ("Output channel names: " + device->getOutputChannelNames().joinIntoString (", "));
            logMessage ("Active output channels: " + getListOfActiveBits (device->getActiveOutputChannels()));
        }
        else
        {
            logMessage ("No audio device open");
        }
    }

    void logMessage (const String& m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret (m + newLine);
    }

private:
    ScopedPointer<AudioDeviceSelectorComponent> audioSetupComp;
    TextEditor diagnosticsBox;

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        dumpDeviceInfo();
    }

    void lookAndFeelChanged() override
    {
        diagnosticsBox.applyFontToAllText (diagnosticsBox.getFont());
    }

    static String getListOfActiveBits (const BigInteger& b)
    {
        StringArray bits;

        for (int i = 0; i <= b.getHighestBit(); ++i)
            if (b[i])
                bits.add (String (i));

        return bits.joinIntoString (", ");
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<AudioSettingsDemo> demo ("30 Audio: Settings");
