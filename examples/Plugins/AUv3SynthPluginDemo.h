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

 name:                  AUv3SynthPlugin
 version:               1.0.0
 vendor:                JUCE
 website:               http://juce.com
 description:           AUv3 synthesiser audio plugin.

 dependencies:          juce_audio_basics, juce_audio_devices, juce_audio_formats,
                        juce_audio_plugin_client, juce_audio_processors,
                        juce_audio_utils, juce_core, juce_data_structures,
                        juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:             xcode_mac, xcode_iphone

 moduleFlags:           JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:                  AudioProcessor
 mainClass:             AUv3SynthProcessor

 useLocalCopy:          1

 pluginCharacteristics: pluginIsSynth, pluginWantsMidiIn
 extraPluginFormats:    AUv3

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class MaterialLookAndFeel final : public LookAndFeel_V4
{
public:
    //==============================================================================
    MaterialLookAndFeel()
    {
        setColour (ResizableWindow::backgroundColourId, windowBackgroundColour);
        setColour (TextButton::buttonOnColourId,        brightButtonColour);
        setColour (TextButton::buttonColourId,          disabledButtonColour);
    }

    //==============================================================================
    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& /*backgroundColour*/,
                               bool /*isMouseOverButton*/,
                               bool isButtonDown) override
    {
        auto buttonRect = button.getLocalBounds().toFloat();

        if (isButtonDown)
            g.setColour (brightButtonColour.withAlpha (0.7f));
        else if (! button.isEnabled())
            g.setColour (disabledButtonColour);
        else
            g.setColour (brightButtonColour);

        g.fillRoundedRectangle (buttonRect, 5.0f);
    }

    //==============================================================================
    void drawButtonText (Graphics& g, TextButton& button, bool isMouseOverButton, bool isButtonDown) override
    {
        ignoreUnused (isMouseOverButton, isButtonDown);

        Font font (getTextButtonFont (button, button.getHeight()));
        g.setFont (font);

        if (button.isEnabled())
            g.setColour (Colours::white);
        else
            g.setColour (backgroundColour);

        g.drawFittedText (button.getButtonText(), 0, 0,
                          button.getWidth(),
                          button.getHeight(),
                          Justification::centred, 2);
    }

    //==============================================================================
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           Slider::SliderStyle style, Slider& slider) override
    {
        ignoreUnused (style, minSliderPos, maxSliderPos);

        auto r = Rectangle<int> (x + haloRadius, y, width - (haloRadius * 2), height);
        auto backgroundBar = r.withSizeKeepingCentre (r.getWidth(), 2);

        sliderPos = (sliderPos - minSliderPos) / static_cast<float> (width);

        auto knobPos = static_cast<int> (sliderPos * (float) r.getWidth());

        g.setColour (sliderActivePart);
        g.fillRect (backgroundBar.removeFromLeft (knobPos));

        g.setColour (sliderInactivePart);
        g.fillRect (backgroundBar);

        if (slider.isMouseOverOrDragging())
        {
            auto haloBounds = r.withTrimmedLeft (knobPos - haloRadius)
                               .withWidth (haloRadius * 2)
                               .withSizeKeepingCentre (haloRadius * 2, haloRadius * 2);

            g.setColour (sliderActivePart.withAlpha (0.5f));
            g.fillEllipse (haloBounds.toFloat());
        }

        auto knobRadius = slider.isMouseOverOrDragging() ? knobActiveRadius : knobInActiveRadius;
        auto knobBounds = r.withTrimmedLeft (knobPos - knobRadius)
                           .withWidth (knobRadius * 2)
                           .withSizeKeepingCentre (knobRadius * 2, knobRadius * 2);

        g.setColour (sliderActivePart);
        g.fillEllipse (knobBounds.toFloat());
    }

    //==============================================================================
    Font getTextButtonFont (TextButton& button, int buttonHeight) override
    {
        return LookAndFeel_V3::getTextButtonFont (button, buttonHeight).withHeight (buttonFontSize);
    }

    Font getLabelFont (Label& label) override
    {
        return LookAndFeel_V3::getLabelFont (label).withHeight (labelFontSize);
    }

    //==============================================================================
    enum
    {
        labelFontSize  = 12,
        buttonFontSize = 15
    };

    //==============================================================================
    enum
    {
        knobActiveRadius   = 12,
        knobInActiveRadius = 8,
        haloRadius         = 18
    };

    //==============================================================================
    const Colour windowBackgroundColour = Colour (0xff262328);
    const Colour backgroundColour       = Colour (0xff4d4d4d);
    const Colour brightButtonColour     = Colour (0xff80cbc4);
    const Colour disabledButtonColour   = Colour (0xffe4e4e4);
    const Colour sliderInactivePart     = Colour (0xff545d62);
    const Colour sliderActivePart       = Colour (0xff80cbc4);
};

//==============================================================================
class AUv3SynthEditor final : public AudioProcessorEditor,
                              private Timer
{
public:
    //==============================================================================
    AUv3SynthEditor (AudioProcessor& processorIn)
        : AudioProcessorEditor (processorIn),
          roomSizeSlider (Slider::LinearHorizontal, Slider::NoTextBox)
    {
        setLookAndFeel (&materialLookAndFeel);

        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);

        recordButton.onClick = [this] { startRecording(); };
        addAndMakeVisible (recordButton);

        roomSizeSlider.onValueChange = [this] { setParameterValue ("roomSize", (float) roomSizeSlider.getValue()); };
        roomSizeSlider.setRange (0.0, 1.0);
        addAndMakeVisible (roomSizeSlider);

        if (auto fileStream = createAssetInputStream ("proaudio.path"))
        {
            Path proAudioPath;
            proAudioPath.loadPathFromStream (*fileStream);
            proAudioIcon.setPath (proAudioPath);
            addAndMakeVisible (proAudioIcon);

            auto proAudioIconColour = findColour (TextButton::buttonOnColourId);
            proAudioIcon.setFill (FillType (proAudioIconColour));
        }

        setSize (600, 400);
        startTimer (100);
    }

    ~AUv3SynthEditor() override
    {
        setLookAndFeel (nullptr);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto r = getLocalBounds();

        auto guiElementAreaHeight = r.getHeight() / 3;

        proAudioIcon.setTransformToFit (r.removeFromLeft (proportionOfWidth (0.25))
                                         .withSizeKeepingCentre (guiElementAreaHeight, guiElementAreaHeight)
                                         .toFloat(),
                                        RectanglePlacement::fillDestination);

        auto margin = guiElementAreaHeight / 4;
        r.reduce (margin, margin);

        auto buttonHeight = guiElementAreaHeight - margin;

        recordButton  .setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
        roomSizeSlider.setBounds (r.removeFromTop (guiElementAreaHeight).withSizeKeepingCentre (r.getWidth(), buttonHeight));
    }

    //==============================================================================
    void startRecording()
    {
        recordButton.setEnabled (false);
        setParameterValue ("isRecording", 1.0f);
    }

private:
    //==============================================================================
    void timerCallback() override
    {
        auto isRecordingNow = (getParameterValue ("isRecording") >= 0.5f);

        recordButton.setEnabled (! isRecordingNow);
        roomSizeSlider.setValue (getParameterValue ("roomSize"), NotificationType::dontSendNotification);
    }

    //==============================================================================
    AudioProcessorParameter* getParameter (const String& paramId)
    {
        if (auto* audioProcessor = getAudioProcessor())
        {
            auto& params = audioProcessor->getParameters();

            for (auto p : params)
            {
                if (auto* param = dynamic_cast<AudioProcessorParameterWithID*> (p))
                {
                    if (param->paramID == paramId)
                        return param;
                }
            }
        }

        return nullptr;
    }

    //==============================================================================
    float getParameterValue (const String& paramId)
    {
        if (auto* param = getParameter (paramId))
            return param->getValue();

        return 0.0f;
    }

    void setParameterValue (const String& paramId, float value)
    {
        if (auto* param = getParameter (paramId))
            param->setValueNotifyingHost (value);
    }

    //==============================================================================
    MaterialLookAndFeel materialLookAndFeel;

    //==============================================================================
    TextButton recordButton { "Record" };
    Slider roomSizeSlider;
    DrawablePath proAudioIcon;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUv3SynthEditor)
};

//==============================================================================
class AUv3SynthProcessor final : public AudioProcessor
{
public:
    AUv3SynthProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo(), true)),
          currentRecording (1, 1), currentProgram (0)
    {
        // initialize parameters
        addParameter (isRecordingParam = new AudioParameterBool  ({ "isRecording", 1 }, "Is Recording", false));
        addParameter (roomSizeParam    = new AudioParameterFloat ({ "roomSize", 1 }, "Room Size", 0.0f, 1.0f, 0.5f));

        formatManager.registerBasicFormats();

        for (auto i = 0; i < maxNumVoices; ++i)
            synth.addVoice (new SamplerVoice());

        loadNewSample (createAssetInputStream ("singing.ogg"), "ogg");
    }

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return (layouts.getMainOutputChannels() <= 2);
    }

    void prepareToPlay (double sampleRate, int estimatedMaxSizeOfBuffer) override
    {
        ignoreUnused (estimatedMaxSizeOfBuffer);

        lastSampleRate = sampleRate;

        currentRecording.setSize (1, static_cast<int> (std::ceil (maxDurationOfRecording * lastSampleRate)));
        samplesRecorded = 0;

        synth.setCurrentPlaybackSampleRate (lastSampleRate);
        reverb.setSampleRate (lastSampleRate);
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        Reverb::Parameters reverbParameters;
        reverbParameters.roomSize = roomSizeParam->get();

        reverb.setParameters (reverbParameters);
        synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

        if (getMainBusNumOutputChannels() == 1)
            reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
        else if (getMainBusNumOutputChannels() == 2)
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    void releaseResources() override                                            { currentRecording.setSize (1, 1); }

    //==============================================================================
    bool acceptsMidi() const override                                           { return true; }
    bool producesMidi() const override                                          { return false; }
    double getTailLengthSeconds() const override                                { return 0.0; }

    //==============================================================================
    AudioProcessorEditor* createEditor() override                               { return new AUv3SynthEditor (*this); }
    bool hasEditor() const override                                             { return true; }

    //==============================================================================
    const String getName() const override                                       { return "AUv3 Synth"; }
    int getNumPrograms() override                                               { return 4; }
    int getCurrentProgram() override                                            { return currentProgram; }
    void setCurrentProgram (int index) override                                 { currentProgram = index; }

    const String getProgramName (int index) override
    {
        switch (index)
        {
            case 0:  return "Piano";
            case 1:  return "Singing";
            case 2:  return "Pinched Balloon";
            case 3:  return "Gazeebo";
            default: break;
        }

        return "<Unknown>";
    }

    //==============================================================================
    void changeProgramName (int /*index*/, const String& /*name*/) override     {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream stream (destData, true);

        stream.writeFloat (*isRecordingParam);
        stream.writeFloat (*roomSizeParam);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

        isRecordingParam->setValueNotifyingHost (stream.readFloat());
        roomSizeParam->setValueNotifyingHost    (stream.readFloat());

    }

private:
    //==============================================================================
    void loadNewSampleBinary (const void* data, int dataSize, const char* format)
    {
        auto soundBuffer = std::make_unique<MemoryInputStream> (data, static_cast<std::size_t> (dataSize), false);
        loadNewSample (std::move (soundBuffer), format);
    }

    void loadNewSample (std::unique_ptr<InputStream> soundBuffer, const char* format)
    {
        std::unique_ptr<AudioFormatReader> formatReader (formatManager.findFormatForFileExtension (format)->createReaderFor (soundBuffer.release(), true));

        BigInteger midiNotes;
        midiNotes.setRange (0, 126, true);
        SynthesiserSound::Ptr newSound = new SamplerSound ("Voice", *formatReader, midiNotes, 0x40, 0.0, 0.0, 10.0);
        synth.removeSound (0);
        sound = newSound;
        synth.addSound (sound);
    }

    void swapSamples()
    {
        MemoryBlock mb;
        auto* stream = new MemoryOutputStream (mb, true);

        {
            std::unique_ptr<AudioFormatWriter> writer (formatManager.findFormatForFileExtension ("wav")->createWriterFor (stream, lastSampleRate, 1, 16,
                                                                                                                          StringPairArray(), 0));
            writer->writeFromAudioSampleBuffer (currentRecording, 0, currentRecording.getNumSamples());
            writer->flush();
            stream->flush();
        }

        loadNewSampleBinary (mb.getData(), static_cast<int> (mb.getSize()), "wav");
    }

    //==============================================================================
    static constexpr int maxNumVoices = 5;
    static constexpr double maxDurationOfRecording = 1.0;

    //==============================================================================
    AudioFormatManager formatManager;

    int samplesRecorded;
    double lastSampleRate;
    AudioBuffer<float> currentRecording;

    Reverb reverb;
    Synthesiser synth;
    SynthesiserSound::Ptr sound;

    AudioParameterBool* isRecordingParam;
    AudioParameterFloat* roomSizeParam;

    int currentProgram;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUv3SynthProcessor)
};
