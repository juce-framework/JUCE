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

 name:             DSPModulePluginDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Audio plugin using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures, juce_dsp,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        DspModulePluginDemoAudioProcessor

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct ParameterSlider    : public Slider,
                            public Timer
{
    ParameterSlider (AudioProcessorParameter& p)
        : Slider (p.getName (256)), param (p)
    {
        setRange (0.0, 1.0, 0.0);
        startTimerHz (30);
        updateSliderPos();
    }

    void valueChanged() override
    {
        if (isMouseButtonDown())
            param.setValueNotifyingHost ((float) Slider::getValue());
        else
            param.setValue ((float) Slider::getValue());
    }

    void timerCallback() override       { updateSliderPos(); }

    void startedDragging() override     { param.beginChangeGesture(); }
    void stoppedDragging() override     { param.endChangeGesture();   }

    double getValueFromText (const String& text) override   { return param.getValueForText (text); }
    String getTextFromValue (double value) override         { return param.getText ((float) value, 1024) + " " + param.getLabel(); }

    void updateSliderPos()
    {
        auto newValue = param.getValue();

        if (newValue != (float) Slider::getValue() && ! isMouseButtonDown())
            Slider::setValue (newValue);
    }

    AudioProcessorParameter& param;
};

//==============================================================================
/**
    This class handles the audio processing for the DSP module plugin demo.
*/
class DspModulePluginDemoAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    DspModulePluginDemoAudioProcessor()
         : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
                                            .withOutput ("Output", AudioChannelSet::stereo(), true)),
           lowPassFilter  (dsp::IIR::Coefficients<float>::makeFirstOrderLowPass  (48000.0, 20000.0f)),
           highPassFilter (dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (48000.0, 20.0f)),
           waveShapers    { { std::tanh }, { dsp::FastMathApproximations::tanh } },
           clipping       { clip }
    {
        // Oversampling 2 times with IIR filtering
        oversampling.reset (new dsp::Oversampling<float> (2, 1, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false));

        addParameter (inputVolumeParam        = new AudioParameterFloat  ("INPUT",   "Input Volume",       { 0.0f,  60.0f,    0.0f, 1.0f }, 0.0f,     "dB"));
        addParameter (highPassFilterFreqParam = new AudioParameterFloat  ("HPFREQ",  "Pre Highpass Freq.", { 20.0f, 20000.0f, 0.0f, 0.5f }, 20.0f,    "Hz"));
        addParameter (lowPassFilterFreqParam  = new AudioParameterFloat  ("LPFREQ",  "Post Lowpass Freq.", { 20.0f, 20000.0f, 0.0f, 0.5f }, 20000.0f, "Hz"));

        addParameter (stereoParam             = new AudioParameterChoice ("STEREO",  "Stereo Processing",  { "Always mono", "Yes" },                 1));
        addParameter (slopeParam              = new AudioParameterChoice ("SLOPE",   "Slope",              { "-6 dB / octave", "-12 dB / octave" },  0));
        addParameter (waveshaperParam         = new AudioParameterChoice ("WVSHP",   "Waveshaper",         { "std::tanh", "Fast tanh approx." },     0));

        addParameter (cabinetTypeParam        = new AudioParameterChoice ("CABTYPE", "Cabinet Type",       { "Guitar amplifier 8'' cabinet ",
                                                                                                             "Cassette recorder cabinet" },          0));

        addParameter (cabinetSimParam         = new AudioParameterBool   ("CABSIM",  "Cabinet Sim",  false));
        addParameter (oversamplingParam       = new AudioParameterBool   ("OVERS",   "Oversampling", false));

        addParameter (outputVolumeParam       = new AudioParameterFloat  ("OUTPUT",  "Output Volume",      { -40.0f, 40.0f, 0.0f, 1.0f }, 0.0f, "dB"));

        cabinetType.set (0);
    }

    ~DspModulePluginDemoAudioProcessor() {}

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        // This is the place where you check if the layout is supported.
        // In this template code we only support mono or stereo.
        if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
            return false;

        // This checks if the input layout matches the output layout
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;

        return true;
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        auto channels = static_cast<uint32> (jmin (getMainBusNumInputChannels(), getMainBusNumOutputChannels()));
        dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), channels };

        lowPassFilter .prepare (spec);
        highPassFilter.prepare (spec);

        inputVolume .prepare (spec);
        outputVolume.prepare (spec);

        convolution.prepare (spec);
        cabinetType.set (-1);

        oversampling->initProcessing (static_cast<size_t> (samplesPerBlock));

        updateParameters();
        reset();
    }

    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& inoutBuffer, MidiBuffer&) override
    {
        auto totalNumInputChannels  = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        auto numSamples = inoutBuffer.getNumSamples();

        for (auto i = jmin (2, totalNumInputChannels); i < totalNumOutputChannels; ++i)
            inoutBuffer.clear (i, 0, numSamples);

        updateParameters();

        dsp::AudioBlock<float> block (inoutBuffer);

        if (stereoParam->getIndex() == 1)
        {
            // Stereo processing mode:
            if (block.getNumChannels() > 2)
                block = block.getSubsetChannelBlock (0, 2);

            process (dsp::ProcessContextReplacing<float> (block));
        }
        else
        {
            // Mono processing mode:
            auto firstChan = block.getSingleChannelBlock (0);

            process (dsp::ProcessContextReplacing<float> (firstChan));

            for (size_t chan = 1; chan < block.getNumChannels(); ++chan)
                block.getSingleChannelBlock (chan).copy (firstChan);
        }
    }

    void reset() override
    {
        lowPassFilter .reset();
        highPassFilter.reset();
        convolution   .reset();
        oversampling->reset();
    }

    //==============================================================================
    bool hasEditor() const override                                       { return true; }

    AudioProcessorEditor* createEditor() override
    {
        return new DspModulePluginDemoAudioProcessorEditor (*this);
    }

    //==============================================================================
    bool acceptsMidi() const override                                     { return false; }
    bool producesMidi() const override                                    { return false; }
    const String getName() const override                                 { return JucePlugin_Name; }
    double getTailLengthSeconds() const override                          { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                         { return 1; }
    int getCurrentProgram() override                                      { return 0; }
    void setCurrentProgram (int) override                                 {}
    const String getProgramName (int) override                            { return {}; }
    void changeProgramName (int, const String&) override                  {}

    //==============================================================================
    void getStateInformation (MemoryBlock&) override                      {}
    void setStateInformation (const void*, int) override                  {}

    //==============================================================================
    void updateParameters()
    {
        auto newOversampling = oversamplingParam->get();
        if (newOversampling != audioCurrentlyOversampled)
        {
            audioCurrentlyOversampled = newOversampling;
            oversampling->reset();
        }

        //==============================================================================
        auto inputdB  = Decibels::decibelsToGain  (inputVolumeParam->get());
        auto outputdB = Decibels::decibelsToGain (outputVolumeParam->get());

        if (inputVolume .getGainLinear() != inputdB)     inputVolume.setGainLinear (inputdB);
        if (outputVolume.getGainLinear() != outputdB)   outputVolume.setGainLinear (outputdB);

        auto newSlopeType = slopeParam->getIndex();

        if (newSlopeType == 0)
        {
            *lowPassFilter .state = *dsp::IIR::Coefficients<float>::makeFirstOrderLowPass  (getSampleRate(),  lowPassFilterFreqParam->get());
            *highPassFilter.state = *dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (getSampleRate(), highPassFilterFreqParam->get());
        }
        else
        {
            *lowPassFilter .state = *dsp::IIR::Coefficients<float>::makeLowPass  (getSampleRate(),  lowPassFilterFreqParam->get());
            *highPassFilter.state = *dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), highPassFilterFreqParam->get());
        }

        //==============================================================================
        auto type = cabinetTypeParam->getIndex();
        auto currentType = cabinetType.get();

        if (type != currentType)
        {
            cabinetType.set (type);

            auto maxSize = static_cast<size_t> (roundToInt (getSampleRate() * (8192.0 / 44100.0)));
            auto assetName = (type == 0 ? "Impulse1.wav" : "Impulse2.wav");

            std::unique_ptr<InputStream> assetInputStream (createAssetInputStream (assetName));

            if (assetInputStream != nullptr)
            {
                currentCabinetData.reset();
                assetInputStream->readIntoMemoryBlock (currentCabinetData);

                convolution.loadImpulseResponse (currentCabinetData.getData(), currentCabinetData.getSize(),
                                                 false, true, maxSize);
            }
        }

        cabinetIsBypassed = ! cabinetSimParam->get();

    }

    static inline float clip (float x) { return jmax (-1.0f, jmin (1.0f, x)); }

    //==============================================================================
    AudioParameterFloat* inputVolumeParam;
    AudioParameterFloat* outputVolumeParam;
    AudioParameterFloat* lowPassFilterFreqParam;
    AudioParameterFloat* highPassFilterFreqParam;

    AudioParameterChoice* stereoParam;
    AudioParameterChoice* slopeParam;
    AudioParameterChoice* waveshaperParam;
    AudioParameterChoice* cabinetTypeParam;

    AudioParameterBool* cabinetSimParam;
    AudioParameterBool* oversamplingParam;

private:
    //==============================================================================
    /**
        This is the editor component that will be displayed.
    */
    class DspModulePluginDemoAudioProcessorEditor  : public AudioProcessorEditor
    {
    public:
        //==============================================================================
        DspModulePluginDemoAudioProcessorEditor (DspModulePluginDemoAudioProcessor& p)
            : AudioProcessorEditor    (&p),
              processor               (p),
              inputVolumeLabel        ({}, processor.inputVolumeParam->name),
              outputVolumeLabel       ({}, processor.outputVolumeParam->name),
              lowPassFilterFreqLabel  ({}, processor.lowPassFilterFreqParam->name),
              highPassFilterFreqLabel ({}, processor.highPassFilterFreqParam->name),
              stereoLabel             ({}, processor.stereoParam->name),
              slopeLabel              ({}, processor.slopeParam->name),
              waveshaperLabel         ({}, processor.waveshaperParam->name),
              cabinetTypeLabel        ({}, processor.cabinetTypeParam->name)
        {
            //==============================================================================
            inputVolumeSlider       .reset (new ParameterSlider (*processor.inputVolumeParam));
            outputVolumeSlider      .reset (new ParameterSlider (*processor.outputVolumeParam));
            lowPassFilterFreqSlider .reset (new ParameterSlider (*processor.lowPassFilterFreqParam));
            highPassFilterFreqSlider.reset (new ParameterSlider (*processor.highPassFilterFreqParam));

            addAndMakeVisible (inputVolumeSlider       .get());
            addAndMakeVisible (outputVolumeSlider      .get());
            addAndMakeVisible (lowPassFilterFreqSlider .get());
            addAndMakeVisible (highPassFilterFreqSlider.get());

            addAndMakeVisible (inputVolumeLabel);
            inputVolumeLabel.setJustificationType (Justification::centredLeft);
            inputVolumeLabel.attachToComponent (inputVolumeSlider.get(), true);

            addAndMakeVisible (outputVolumeLabel);
            outputVolumeLabel.setJustificationType (Justification::centredLeft);
            outputVolumeLabel.attachToComponent (outputVolumeSlider.get(), true);

            addAndMakeVisible (lowPassFilterFreqLabel);
            lowPassFilterFreqLabel.setJustificationType (Justification::centredLeft);
            lowPassFilterFreqLabel.attachToComponent (lowPassFilterFreqSlider.get(), true);

            addAndMakeVisible (highPassFilterFreqLabel);
            highPassFilterFreqLabel.setJustificationType (Justification::centredLeft);
            highPassFilterFreqLabel.attachToComponent (highPassFilterFreqSlider.get(), true);

            //==============================================================================
            addAndMakeVisible (stereoBox);

            auto i = 1;
            for (auto choice : processor.stereoParam->choices)
                stereoBox.addItem (choice, i++);

            stereoBox.onChange = [this] { processor.stereoParam->operator= (stereoBox.getSelectedItemIndex()); };
            stereoBox.setSelectedId (processor.stereoParam->getIndex() + 1);

            addAndMakeVisible (stereoLabel);
            stereoLabel.setJustificationType (Justification::centredLeft);
            stereoLabel.attachToComponent (&stereoBox, true);

            //==============================================================================
            addAndMakeVisible(slopeBox);

            i = 1;
            for (auto choice : processor.slopeParam->choices)
                slopeBox.addItem(choice, i++);

            slopeBox.onChange = [this] { processor.slopeParam->operator= (slopeBox.getSelectedItemIndex()); };
            slopeBox.setSelectedId(processor.slopeParam->getIndex() + 1);

            addAndMakeVisible(slopeLabel);
            slopeLabel.setJustificationType(Justification::centredLeft);
            slopeLabel.attachToComponent(&slopeBox, true);

            //==============================================================================
            addAndMakeVisible (waveshaperBox);

            i = 1;
            for (auto choice : processor.waveshaperParam->choices)
                waveshaperBox.addItem (choice, i++);

            waveshaperBox.onChange = [this] { processor.waveshaperParam->operator= (waveshaperBox.getSelectedItemIndex()); };
            waveshaperBox.setSelectedId (processor.waveshaperParam->getIndex() + 1);

            addAndMakeVisible (waveshaperLabel);
            waveshaperLabel.setJustificationType (Justification::centredLeft);
            waveshaperLabel.attachToComponent (&waveshaperBox, true);

            //==============================================================================
            addAndMakeVisible (cabinetTypeBox);

            i = 1;
            for (auto choice : processor.cabinetTypeParam->choices)
                cabinetTypeBox.addItem (choice, i++);

            cabinetTypeBox.onChange = [this] { processor.cabinetTypeParam->operator= (cabinetTypeBox.getSelectedItemIndex()); };
            cabinetTypeBox.setSelectedId (processor.cabinetTypeParam->getIndex() + 1);

            addAndMakeVisible (cabinetTypeLabel);
            cabinetTypeLabel.setJustificationType (Justification::centredLeft);
            cabinetTypeLabel.attachToComponent (&cabinetTypeBox, true);

            //==============================================================================
            addAndMakeVisible (cabinetSimButton);
            cabinetSimButton.onClick = [this] { processor.cabinetSimParam->operator= (cabinetSimButton.getToggleState()); };
            cabinetSimButton.setButtonText  (processor.cabinetSimParam->name);
            cabinetSimButton.setToggleState (processor.cabinetSimParam->get(), NotificationType::dontSendNotification);

            addAndMakeVisible (oversamplingButton);
            oversamplingButton.onClick = [this] { processor.oversamplingParam->operator= (oversamplingButton.getToggleState()); };
            oversamplingButton.setButtonText  (processor.oversamplingParam->name);
            oversamplingButton.setToggleState (processor.oversamplingParam->get(), NotificationType::dontSendNotification);

            //==============================================================================
            setSize (600, 400);
        }

        ~DspModulePluginDemoAudioProcessorEditor() {}

        //==============================================================================
        void paint (Graphics& g) override
        {
            g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
            g.fillAll();
        }

        void resized() override
        {
            auto bounds = getLocalBounds().reduced (10);
            bounds.removeFromTop (10);
            bounds.removeFromLeft (125);

            //==============================================================================
            inputVolumeSlider->setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (5);

            outputVolumeSlider->setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (15);

            highPassFilterFreqSlider->setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (5);

            lowPassFilterFreqSlider->setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (15);

            //==============================================================================
            stereoBox.setBounds (bounds.removeFromTop(30));
            bounds.removeFromTop (5);

            slopeBox.setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (5);

            waveshaperBox.setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (5);

            cabinetTypeBox.setBounds (bounds.removeFromTop (30));
            bounds.removeFromTop (15);

            //==============================================================================
            auto buttonSlice = bounds.removeFromTop (30);
            cabinetSimButton.setSize (200, buttonSlice.getHeight());
            cabinetSimButton.setCentrePosition (buttonSlice.getCentre());
            bounds.removeFromTop(5);

            buttonSlice = bounds.removeFromTop (30);
            oversamplingButton.setSize(200, buttonSlice.getHeight());
            oversamplingButton.setCentrePosition(buttonSlice.getCentre());
        }

    private:
        //==============================================================================
        DspModulePluginDemoAudioProcessor& processor;

        std::unique_ptr<ParameterSlider> inputVolumeSlider, outputVolumeSlider,
                                         lowPassFilterFreqSlider, highPassFilterFreqSlider;
        ComboBox stereoBox, slopeBox, waveshaperBox, cabinetTypeBox;
        ToggleButton cabinetSimButton, oversamplingButton;

        Label inputVolumeLabel, outputVolumeLabel, lowPassFilterFreqLabel,
              highPassFilterFreqLabel, stereoLabel, slopeLabel, waveshaperLabel,
              cabinetTypeLabel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemoAudioProcessorEditor)
    };

    //==============================================================================
    void process (dsp::ProcessContextReplacing<float> context) noexcept
    {
        ScopedNoDenormals noDenormals;

        // Input volume applied with a LinearSmoothedValue
        inputVolume.process (context);

        // Pre-highpass filtering, very useful for distortion audio effects
        // Note : try frequencies around 700 Hz
        highPassFilter.process (context);

        // Upsampling
        dsp::AudioBlock<float> oversampledBlock;

        setLatencySamples (audioCurrentlyOversampled ? roundToInt (oversampling->getLatencyInSamples()) : 0);

        if (audioCurrentlyOversampled)
            oversampledBlock = oversampling->processSamplesUp (context.getInputBlock());

        auto waveshaperContext = audioCurrentlyOversampled ? dsp::ProcessContextReplacing<float> (oversampledBlock)
                                                           : context;

        // Waveshaper processing, for distortion generation, thanks to the input gain
        // The fast tanh can be used instead of std::tanh to reduce the CPU load
        auto waveshaperIndex = waveshaperParam->getIndex();

        if (isPositiveAndBelow (waveshaperIndex, numWaveShapers) )
        {
            waveShapers[waveshaperIndex].process (waveshaperContext);

            if (waveshaperIndex == 1)
                clipping.process (waveshaperContext);

            waveshaperContext.getOutputBlock() *= 0.7f;
        }

        // Downsampling
        if (audioCurrentlyOversampled)
            oversampling->processSamplesDown (context.getOutputBlock());

        // Post-lowpass filtering
        lowPassFilter.process (context);

        // Convolution with the impulse response of a guitar cabinet
        auto wasBypassed = context.isBypassed;
        context.isBypassed = context.isBypassed || cabinetIsBypassed;
        convolution.process (context);
        context.isBypassed = wasBypassed;

        // Output volume applied with a LinearSmoothedValue
        outputVolume.process (context);
    }

    //==============================================================================
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> lowPassFilter, highPassFilter;
    dsp::Convolution convolution;
    MemoryBlock currentCabinetData;

    static constexpr size_t numWaveShapers = 2;
    dsp::WaveShaper<float> waveShapers[numWaveShapers];
    dsp::WaveShaper<float> clipping;

    dsp::Gain<float> inputVolume, outputVolume;

    std::unique_ptr<dsp::Oversampling<float>> oversampling;
    bool audioCurrentlyOversampled = false;

    Atomic<int> cabinetType;
    bool cabinetIsBypassed = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemoAudioProcessor)
};
