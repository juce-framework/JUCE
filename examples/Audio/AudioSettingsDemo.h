/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioSettingsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays information about audio devices.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioSettingsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class AudioSettingsDemo final : public Component,
                                public ChangeListener
{
public:
    AudioSettingsDemo()
    {
        setOpaque (true);

       #ifndef JUCE_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (bool granted)
                                     {
                                         int numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });
       #endif

        audioSetupComp.reset (new AudioDeviceSelectorComponent (audioDeviceManager,
                                                                0, 256, 0, 256, true, true, true, false));
        addAndMakeVisible (audioSetupComp.get());

        addAndMakeVisible (diagnosticsBox);
        diagnosticsBox.setMultiLine (true);
        diagnosticsBox.setReturnKeyStartsNewLine (true);
        diagnosticsBox.setReadOnly (true);
        diagnosticsBox.setScrollbarsShown (true);
        diagnosticsBox.setCaretVisible (false);
        diagnosticsBox.setPopupMenuEnabled (true);

        audioDeviceManager.addChangeListener (this);

        logMessage ("Audio device diagnostics:\n");
        dumpDeviceInfo();

        setSize (500, 600);
    }

    ~AudioSettingsDemo() override
    {
        audioDeviceManager.removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto r =  getLocalBounds().reduced (4);
        audioSetupComp->setBounds (r.removeFromTop (proportionOfHeight (0.65f)));
        diagnosticsBox.setBounds (r);
    }

    void dumpDeviceInfo()
    {
        logMessage ("--------------------------------------");
        logMessage ("Current audio device type: " + (audioDeviceManager.getCurrentDeviceTypeObject() != nullptr
                                                     ? audioDeviceManager.getCurrentDeviceTypeObject()->getTypeName()
                                                     : "<none>"));

        if (AudioIODevice* device = audioDeviceManager.getCurrentAudioDevice())
        {
            logMessage ("Current audio device: "   + device->getName().quoted());
            logMessage ("Sample rate: "    + String (device->getCurrentSampleRate()) + " Hz");
            logMessage ("Block size: "     + String (device->getCurrentBufferSizeSamples()) + " samples");
            logMessage ("Output Latency: " + String (device->getOutputLatencyInSamples())   + " samples");
            logMessage ("Input Latency: "  + String (device->getInputLatencyInSamples())    + " samples");
            logMessage ("Bit depth: "      + String (device->getCurrentBitDepth()));
            logMessage ("Input channel names: "    + device->getInputChannelNames().joinIntoString (", "));
            logMessage ("Active input channels: "  + getListOfActiveBits (device->getActiveInputChannels()));
            logMessage ("Output channel names: "   + device->getOutputChannelNames().joinIntoString (", "));
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
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager() };
   #endif

    std::unique_ptr<AudioDeviceSelectorComponent> audioSetupComp;
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
