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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
    This class handles the audio processing for the DSP module plugin demo.
*/
class DspModulePluginDemoAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    DspModulePluginDemoAudioProcessor();
    ~DspModulePluginDemoAudioProcessor();

    //==============================================================================
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;
    void reset() override;

    //==============================================================================
    bool hasEditor() const override;
    AudioProcessorEditor* createEditor() override;

    //==============================================================================
    bool acceptsMidi() const override;
    bool producesMidi() const override;

    const String getName() const override                                           { return JucePlugin_Name; }
    double getTailLengthSeconds() const override                                    { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                                   { return 1; }
    int getCurrentProgram() override                                                { return 0; }
    void setCurrentProgram (int /*index*/) override                                 {}
    const String getProgramName (int /*index*/) override                            { return {}; }
    void changeProgramName (int /*index*/, const String& /*newName*/) override      {}

    //==============================================================================
    void getStateInformation (MemoryBlock& /*destData*/) override                   {}
    void setStateInformation (const void* /*data*/, int /*sizeInBytes*/) override   {}

    //==============================================================================
    void updateParameters();

    static inline float clip(float x) { return jmax(-1.f, jmin(1.f, x)); }

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
    void process (dsp::ProcessContextReplacing<float>) noexcept;

    //==============================================================================
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> lowPassFilter, highPassFilter;
    dsp::Convolution convolution;

    static constexpr size_t numWaveShapers = 2;
    dsp::WaveShaper<float> waveShapers[numWaveShapers];
    dsp::WaveShaper<float> clipping;

    dsp::Gain<float> inputVolume, outputVolume;

    ScopedPointer<dsp::Oversampling<float>> oversampling;
    bool audioCurrentlyOversampled = false;

    Atomic<int> cabinetType;
    bool cabinetIsBypassed = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemoAudioProcessor)
};
