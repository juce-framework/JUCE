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

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
DspModulePluginDemoAudioProcessor::DspModulePluginDemoAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput ("Input",  AudioChannelSet::stereo(), true)
                       .withOutput ("Output", AudioChannelSet::stereo(), true)),
       lowPassFilter  (dsp::IIR::Coefficients<float>::makeFirstOrderLowPass  (48000.0, 20000.f)),
       highPassFilter (dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (48000.0, 20.0f)),
       waveShapers { {std::tanh}, {dsp::FastMathApproximations::tanh} },
       clipping { clip }
{
    // Oversampling 2 times with IIR filtering
    oversampling = new dsp::Oversampling<float> (2, 1, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false);

    addParameter (inputVolumeParam        = new AudioParameterFloat ("INPUT",  "Input Volume",       { 0.f, 60.f, 0.f, 1.0f },     0.f,     "dB"));
    addParameter (highPassFilterFreqParam = new AudioParameterFloat ("HPFREQ", "Pre Highpass Freq.", { 20.f, 20000.f, 0.f, 0.5f }, 20.f,    "Hz"));
    addParameter (lowPassFilterFreqParam  = new AudioParameterFloat ("LPFREQ", "Post Lowpass Freq.", { 20.f, 20000.f, 0.f, 0.5f }, 20000.f, "Hz"));

    addParameter (stereoParam             = new AudioParameterChoice ("STEREO", "Stereo Processing", { "Always mono", "Yes" },             1));
    addParameter (slopeParam              = new AudioParameterChoice ("SLOPE", "Slope",      { "-6 dB / octave", "-12 dB / octave" },      0));
    addParameter (waveshaperParam         = new AudioParameterChoice ("WVSHP", "Waveshaper", { "std::tanh", "Fast tanh approx." },         0));

    addParameter (cabinetTypeParam        = new AudioParameterChoice ("CABTYPE", "Cabinet Type", { "Guitar amplifier 8'' cabinet ",
                                                                                                   "Cassette recorder cabinet" },          0));

    addParameter (cabinetSimParam         = new AudioParameterBool ("CABSIM", "Cabinet Sim", false));
    addParameter (oversamplingParam       = new AudioParameterBool ("OVERS", "Oversampling", false));

    addParameter (outputVolumeParam       = new AudioParameterFloat ("OUTPUT", "Output Volume", { -40.f, 40.f, 0.f, 1.0f }, 0.f, "dB"));

    cabinetType.set (0);
}

DspModulePluginDemoAudioProcessor::~DspModulePluginDemoAudioProcessor()
{
}

//==============================================================================
bool DspModulePluginDemoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DspModulePluginDemoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto channels = static_cast<uint32> (jmin (getMainBusNumInputChannels(), getMainBusNumOutputChannels()));
    dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), channels };

    lowPassFilter.prepare (spec);
    highPassFilter.prepare (spec);

    inputVolume.prepare (spec);
    outputVolume.prepare (spec);

    convolution.prepare (spec);
    cabinetType.set (-1);

    oversampling->initProcessing (static_cast<size_t> (samplesPerBlock));

    updateParameters();
    reset();
}

void DspModulePluginDemoAudioProcessor::reset()
{
    lowPassFilter.reset();
    highPassFilter.reset();
    convolution.reset();
    oversampling->reset();
}

void DspModulePluginDemoAudioProcessor::releaseResources()
{

}

void DspModulePluginDemoAudioProcessor::process (dsp::ProcessContextReplacing<float> context) noexcept
{
    ScopedNoDenormals noDenormals;

    // Input volume applied with a LinearSmoothedValue
    inputVolume.process (context);

    // Pre-highpass filtering, very useful for distortion audio effects
    // Note : try frequencies around 700 Hz
    highPassFilter.process (context);

    // Upsampling
    dsp::AudioBlock<float> oversampledBlock;

    setLatencySamples (audioCurrentlyOversampled ? roundFloatToInt (oversampling->getLatencyInSamples()) : 0);

    if (audioCurrentlyOversampled)
        oversampledBlock = oversampling->processSamplesUp (context.getInputBlock());

    dsp::ProcessContextReplacing<float> waveshaperContext = audioCurrentlyOversampled ? dsp::ProcessContextReplacing<float> (oversampledBlock) : context;

    // Waveshaper processing, for distortion generation, thanks to the input gain
    // The fast tanh can be used instead of std::tanh to reduce the CPU load
    auto waveshaperIndex = waveshaperParam->getIndex();

    if (isPositiveAndBelow (waveshaperIndex, (int) numWaveShapers) )
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

void DspModulePluginDemoAudioProcessor::processBlock (AudioSampleBuffer& inoutBuffer, MidiBuffer&)
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

//==============================================================================
bool DspModulePluginDemoAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* DspModulePluginDemoAudioProcessor::createEditor()
{
    return new DspModulePluginDemoAudioProcessorEditor (*this);
}

//==============================================================================
bool DspModulePluginDemoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DspModulePluginDemoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

//==============================================================================
void DspModulePluginDemoAudioProcessor::updateParameters()
{
    auto newOversampling = oversamplingParam->get();
    if (newOversampling != audioCurrentlyOversampled)
    {
        audioCurrentlyOversampled = newOversampling;
        oversampling->reset();
    }

    //==============================================================================
    auto inputdB  = Decibels::decibelsToGain (inputVolumeParam->get());
    auto outputdB = Decibels::decibelsToGain (outputVolumeParam->get());

    if (inputVolume.getGainLinear() != inputdB)     inputVolume.setGainLinear (inputdB);
    if (outputVolume.getGainLinear() != outputdB)   outputVolume.setGainLinear (outputdB);

    auto newSlopeType = slopeParam->getIndex();

    if (newSlopeType == 0)
    {
        *lowPassFilter.state  = *dsp::IIR::Coefficients<float>::makeFirstOrderLowPass  (getSampleRate(), lowPassFilterFreqParam->get());
        *highPassFilter.state = *dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (getSampleRate(), highPassFilterFreqParam->get());
    }
    else
    {
        *lowPassFilter.state  = *dsp::IIR::Coefficients<float>::makeLowPass  (getSampleRate(), lowPassFilterFreqParam->get());
        *highPassFilter.state = *dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), highPassFilterFreqParam->get());
    }

    //==============================================================================
    auto type = cabinetTypeParam->getIndex();
    auto currentType = cabinetType.get();

    if (type != currentType)
    {
        cabinetType.set(type);

        auto maxSize = static_cast<size_t> (roundDoubleToInt (8192 * getSampleRate() / 44100));

        if (type == 0)
            convolution.loadImpulseResponse (BinaryData::Impulse1_wav, BinaryData::Impulse1_wavSize, false, true, maxSize);
        else
            convolution.loadImpulseResponse (BinaryData::Impulse2_wav, BinaryData::Impulse2_wavSize, false, true, maxSize);
    }

    cabinetIsBypassed = ! cabinetSimParam->get();

}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DspModulePluginDemoAudioProcessor();
}
