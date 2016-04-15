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
        diagnosticsBox.setColour (TextEditor::textColourId, Colours::white);
        diagnosticsBox.setColour (TextEditor::backgroundColourId, Colour (0x32ffffff));
        diagnosticsBox.setColour (TextEditor::outlineColourId, Colour (0x1c000000));
        diagnosticsBox.setColour (TextEditor::shadowColourId, Colour (0x16000000));

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
        fillStandardDemoBackground (g);
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

    static String getListOfActiveBits (const BitArray& b)
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
