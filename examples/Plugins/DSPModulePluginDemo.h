/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

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
 description:      An audio plugin using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures, juce_dsp,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        DspModulePluginDemoAudioProcessor

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

namespace ID
{
   #define PARAMETER_ID(str) constexpr const char* str { #str };

    PARAMETER_ID (inputGain)
    PARAMETER_ID (outputGain)
    PARAMETER_ID (pan)
    PARAMETER_ID (distortionEnabled)
    PARAMETER_ID (distortionType)
    PARAMETER_ID (distortionOversampler)
    PARAMETER_ID (distortionLowpass)
    PARAMETER_ID (distortionHighpass)
    PARAMETER_ID (distortionInGain)
    PARAMETER_ID (distortionCompGain)
    PARAMETER_ID (distortionMix)
    PARAMETER_ID (convolutionCabEnabled)
    PARAMETER_ID (convolutionReverbEnabled)
    PARAMETER_ID (convolutionReverbMix)
    PARAMETER_ID (multiBandEnabled)
    PARAMETER_ID (multiBandFreq)
    PARAMETER_ID (multiBandLowVolume)
    PARAMETER_ID (multiBandHighVolume)
    PARAMETER_ID (compressorEnabled)
    PARAMETER_ID (compressorThreshold)
    PARAMETER_ID (compressorRatio)
    PARAMETER_ID (compressorAttack)
    PARAMETER_ID (compressorRelease)
    PARAMETER_ID (noiseGateEnabled)
    PARAMETER_ID (noiseGateThreshold)
    PARAMETER_ID (noiseGateRatio)
    PARAMETER_ID (noiseGateAttack)
    PARAMETER_ID (noiseGateRelease)
    PARAMETER_ID (limiterEnabled)
    PARAMETER_ID (limiterThreshold)
    PARAMETER_ID (limiterRelease)
    PARAMETER_ID (directDelayEnabled)
    PARAMETER_ID (directDelayType)
    PARAMETER_ID (directDelayValue)
    PARAMETER_ID (directDelaySmoothing)
    PARAMETER_ID (directDelayMix)
    PARAMETER_ID (delayEffectEnabled)
    PARAMETER_ID (delayEffectType)
    PARAMETER_ID (delayEffectValue)
    PARAMETER_ID (delayEffectSmoothing)
    PARAMETER_ID (delayEffectLowpass)
    PARAMETER_ID (delayEffectFeedback)
    PARAMETER_ID (delayEffectMix)
    PARAMETER_ID (phaserEnabled)
    PARAMETER_ID (phaserRate)
    PARAMETER_ID (phaserDepth)
    PARAMETER_ID (phaserCentreFrequency)
    PARAMETER_ID (phaserFeedback)
    PARAMETER_ID (phaserMix)
    PARAMETER_ID (chorusEnabled)
    PARAMETER_ID (chorusRate)
    PARAMETER_ID (chorusDepth)
    PARAMETER_ID (chorusCentreDelay)
    PARAMETER_ID (chorusFeedback)
    PARAMETER_ID (chorusMix)
    PARAMETER_ID (ladderEnabled)
    PARAMETER_ID (ladderCutoff)
    PARAMETER_ID (ladderResonance)
    PARAMETER_ID (ladderDrive)
    PARAMETER_ID (ladderMode)

   #undef PARAMETER_ID
}

template <typename Func, typename... Items>
constexpr void forEach (Func&& func, Items&&... items)
    noexcept (noexcept (std::initializer_list<int> { (func (std::forward<Items> (items)), 0)... }))
{
    (void) std::initializer_list<int> { ((void) func (std::forward<Items> (items)), 0)... };
}

template <typename... Components>
void addAllAndMakeVisible (Component& target, Components&... children)
{
    forEach ([&] (Component& child) { target.addAndMakeVisible (child); }, children...);
}

template <typename... Processors>
void prepareAll (const dsp::ProcessSpec& spec, Processors&... processors)
{
    forEach ([&] (auto& proc) { proc.prepare (spec); }, processors...);
}

template <typename... Processors>
void resetAll (Processors&... processors)
{
    forEach ([] (auto& proc) { proc.reset(); }, processors...);
}

//==============================================================================
class DspModulePluginDemo  : public AudioProcessor,
                             private ValueTree::Listener
{
public:
    DspModulePluginDemo()
        : DspModulePluginDemo (AudioProcessorValueTreeState::ParameterLayout{}) {}

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        const auto channels = jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());

        if (channels == 0)
            return;

        chain.prepare ({ sampleRate, (uint32) samplesPerBlock, (uint32) channels });

        reset();
    }

    void reset() override
    {
        chain.reset();
        update();
    }

    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        if (jmax (getTotalNumInputChannels(), getTotalNumOutputChannels()) == 0)
            return;

        ScopedNoDenormals noDenormals;

        if (requiresUpdate.load())
            update();

        irSize = dsp::get<convolutionIndex> (chain).reverb.getCurrentIRSize();

        const auto totalNumInputChannels  = getTotalNumInputChannels();
        const auto totalNumOutputChannels = getTotalNumOutputChannels();

        setLatencySamples (dsp::get<convolutionIndex> (chain).getLatency()
                           + (dsp::isBypassed<distortionIndex> (chain) ? 0 : roundToInt (dsp::get<distortionIndex> (chain).getLatency())));

        const auto numChannels = jmax (totalNumInputChannels, totalNumOutputChannels);

        auto inoutBlock = dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
        chain.process (dsp::ProcessContextReplacing<float> (inoutBlock));
    }

    void processBlock (AudioBuffer<double>&, MidiBuffer&) override {}

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    //==============================================================================
    const String getName() const override { return "DSPModulePluginDemo"; }

    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms()    override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const String getProgramName (int) override { return {}; }

    void changeProgramName (int, const String&) override {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        copyXmlToBinary (*apvts.copyState().createXml(), destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        apvts.replaceState (ValueTree::fromXml (*getXmlFromBinary (data, sizeInBytes)));
    }

    int getCurrentIRSize() const { return irSize; }

    using Parameter = AudioProcessorValueTreeState::Parameter;

    // This struct holds references to the raw parameters, so that we don't have to search
    // the APVTS (involving string comparisons and map lookups!) every time a parameter
    // changes.
    struct ParameterReferences
    {
        template <typename Param>
        static Param& addToLayout (AudioProcessorValueTreeState::ParameterLayout& layout,
                                   std::unique_ptr<Param> param)
        {
            auto& ref = *param;
            layout.add (std::move (param));
            return ref;
        }

        static String valueToTextFunction (float x) { return String (x, 2); }
        static float textToValueFunction (const String& str) { return str.getFloatValue(); }

        static String valueToTextPanFunction (float x) { return getPanningTextForValue ((x + 100.0f) / 200.0f); }
        static float textToValuePanFunction (const String& str) { return getPanningValueForText (str) * 200.0f - 100.0f; }

        // Creates parameters, adds them to the layout, and stores references to the parameters
        // in this struct.
        explicit ParameterReferences (AudioProcessorValueTreeState::ParameterLayout& layout)
            : inputGain (addToLayout (layout,
                                      std::make_unique<Parameter> (ID::inputGain,
                                                                   "Input",
                                                                   "dB",
                                                                   NormalisableRange<float> (-40.0f, 40.0f),
                                                                   0.0f,
                                                                   valueToTextFunction,
                                                                   textToValueFunction))),
              outputGain (addToLayout (layout,
                                       std::make_unique<Parameter> (ID::outputGain,
                                                                    "Output",
                                                                    "dB",
                                                                    NormalisableRange<float> (-40.0f, 40.0f),
                                                                    0.0f,
                                                                    valueToTextFunction,
                                                                    textToValueFunction))),
              pan (addToLayout (layout,
                                std::make_unique<Parameter> (ID::pan,
                                                             "Panning",
                                                             "",
                                                             NormalisableRange<float> (-100.0f, 100.0f),
                                                             0.0f,
                                                             valueToTextPanFunction,
                                                             textToValuePanFunction))),
              distortionEnabled (addToLayout (layout,
                                              std::make_unique<AudioParameterBool> (ID::distortionEnabled,
                                                                                    "Distortion",
                                                                                    true,
                                                                                    ""))),
              distortionType (addToLayout (layout,
                                           std::make_unique<AudioParameterChoice> (ID::distortionType,
                                                                                   "Waveshaper",
                                                                                   StringArray { "std::tanh", "Approx. tanh" },
                                                                                   0))),
              distortionInGain (addToLayout (layout,
                                             std::make_unique<Parameter> (ID::distortionInGain,
                                                                          "Gain",
                                                                          "dB",
                                                                          NormalisableRange<float> (-40.0f, 40.0f),
                                                                          0.0f,
                                                                          valueToTextFunction,
                                                                          textToValueFunction))),
              distortionLowpass (addToLayout (layout,
                                              std::make_unique<Parameter> (ID::distortionLowpass,
                                                                           "Post Low-pass",
                                                                           "Hz",
                                                                           NormalisableRange<float> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                                           22000.0f,
                                                                           valueToTextFunction,
                                                                           textToValueFunction))),
              distortionHighpass (addToLayout (layout,
                                               std::make_unique<Parameter> (ID::distortionHighpass,
                                                                            "Pre High-pass",
                                                                            "Hz",
                                                                            NormalisableRange<float> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                                            20.0f,
                                                                            valueToTextFunction,
                                                                            textToValueFunction))),
              distortionCompGain (addToLayout (layout,
                                               std::make_unique<Parameter> (ID::distortionCompGain,
                                                                            "Compensat.",
                                                                            "dB",
                                                                            NormalisableRange<float> (-40.0f, 40.0f),
                                                                            0.0f,
                                                                            valueToTextFunction,
                                                                            textToValueFunction))),
              distortionMix (addToLayout (layout,
                                          std::make_unique<Parameter> (ID::distortionMix,
                                                                       "Mix",
                                                                       "%",
                                                                       NormalisableRange<float> (0.0f, 100.0f),
                                                                       100.0f,
                                                                       valueToTextFunction,
                                                                       textToValueFunction))),
              distortionOversampler (addToLayout (layout,
                                                  std::make_unique<AudioParameterChoice> (ID::distortionOversampler,
                                                                                          "Oversampling",
                                                                                          StringArray { "2X", "4X", "8X", "2X compensated", "4X compensated", "8X compensated" },
                                                                                          1))),
              multiBandEnabled (addToLayout (layout,
                                             std::make_unique<AudioParameterBool> (ID::multiBandEnabled,
                                                                                   "Multi-band",
                                                                                   false,
                                                                                   ""))),
              multiBandFreq (addToLayout (layout,
                                          std::make_unique<Parameter> (ID::multiBandFreq,
                                                                       "Sep. Freq.",
                                                                       "Hz",
                                                                       NormalisableRange<float> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                                       2000.0f,
                                                                       valueToTextFunction,
                                                                       textToValueFunction))),
              multiBandLowVolume (addToLayout (layout,
                                               std::make_unique<Parameter> (ID::multiBandLowVolume,
                                                                            "Low volume",
                                                                            "dB",
                                                                            NormalisableRange<float> (-40.0f, 40.0f),
                                                                            0.0f,
                                                                            valueToTextFunction,
                                                                            textToValueFunction))),
              multiBandHighVolume (addToLayout (layout,
                                                std::make_unique<Parameter> (ID::multiBandHighVolume,
                                                                             "High volume",
                                                                             "dB",
                                                                             NormalisableRange<float> (-40.0f, 40.0f),
                                                                             0.0f,
                                                                             valueToTextFunction,
                                                                             textToValueFunction))),
              convolutionCabEnabled (addToLayout (layout,
                                                  std::make_unique<AudioParameterBool> (ID::convolutionCabEnabled,
                                                                                        "Cabinet",
                                                                                        false,
                                                                                        ""))),
              convolutionReverbEnabled (addToLayout (layout,
                                                     std::make_unique<AudioParameterBool> (ID::convolutionReverbEnabled,
                                                                                           "Reverb",
                                                                                           false,
                                                                                           ""))),
              convolutionReverbMix (addToLayout (layout,
                                                 std::make_unique<Parameter> (ID::convolutionReverbMix,
                                                                              "Reverb Mix",
                                                                              "%",
                                                                              NormalisableRange<float> (0.0f, 100.0f),
                                                                              50.0f,
                                                                              valueToTextFunction,
                                                                              textToValueFunction))),
              compressorEnabled (addToLayout (layout,
                                              std::make_unique<AudioParameterBool> (ID::compressorEnabled,
                                                                                    "Comp.",
                                                                                    false,
                                                                                    ""))),
              compressorThreshold (addToLayout (layout,
                                                std::make_unique<Parameter> (ID::compressorThreshold,
                                                                             "Threshold",
                                                                             "dB",
                                                                             NormalisableRange<float> (-100.0f, 0.0f),
                                                                             0.0f,
                                                                             valueToTextFunction,
                                                                             textToValueFunction))),
              compressorRatio (addToLayout (layout,
                                            std::make_unique<Parameter> (ID::compressorRatio,
                                                                         "Ratio",
                                                                         ":1",
                                                                         NormalisableRange<float> (1.0f, 100.0f, 0.0f, 0.25f),
                                                                         1.0f,
                                                                         valueToTextFunction,
                                                                         textToValueFunction))),
              compressorAttack (addToLayout (layout,
                                             std::make_unique<Parameter> (ID::compressorAttack,
                                                                          "Attack",
                                                                          "ms",
                                                                          NormalisableRange<float> (0.01f, 1000.0f, 0.0f, 0.25f),
                                                                          1.0f,
                                                                          valueToTextFunction,
                                                                          textToValueFunction))),
              compressorRelease (addToLayout (layout,
                                              std::make_unique<Parameter> (ID::compressorRelease,
                                                                           "Release",
                                                                           "ms",
                                                                           NormalisableRange<float> (10.0f, 10000.0f, 0.0f, 0.25f),
                                                                           100.0f,
                                                                           valueToTextFunction,
                                                                           textToValueFunction))),
              noiseGateEnabled (addToLayout (layout,
                                             std::make_unique<AudioParameterBool> (ID::noiseGateEnabled,
                                                                                   "Gate",
                                                                                   false,
                                                                                   ""))),
              noiseGateThreshold (addToLayout (layout,
                                               std::make_unique<Parameter> (ID::noiseGateThreshold,
                                                                            "Threshold",
                                                                            "dB",
                                                                            NormalisableRange<float> (-100.0f, 0.0f),
                                                                            -100.0f,
                                                                            valueToTextFunction,
                                                                            textToValueFunction))),
              noiseGateRatio (addToLayout (layout,
                                           std::make_unique<Parameter> (ID::noiseGateRatio,
                                                                        "Ratio",
                                                                        ":1",
                                                                        NormalisableRange<float> (1.0f, 100.0f, 0.0f, 0.25f),
                                                                        10.0f,
                                                                        valueToTextFunction,
                                                                        textToValueFunction))),
              noiseGateAttack (addToLayout (layout,
                                            std::make_unique<Parameter> (ID::noiseGateAttack,
                                                                         "Attack",
                                                                         "ms",
                                                                         NormalisableRange<float> (0.01f, 1000.0f, 0.0f, 0.25f),
                                                                         1.0f,
                                                                         valueToTextFunction,
                                                                         textToValueFunction))),
              noiseGateRelease (addToLayout (layout,
                                             std::make_unique<Parameter> (ID::noiseGateRelease,
                                                                          "Release",
                                                                          "ms",
                                                                          NormalisableRange<float> (10.0f, 10000.0f, 0.0f, 0.25f),
                                                                          100.0f,
                                                                          valueToTextFunction,
                                                                          textToValueFunction))),
              limiterEnabled (addToLayout (layout,
                                           std::make_unique<AudioParameterBool> (ID::limiterEnabled,
                                                                                 "Limiter",
                                                                                 false,
                                                                                 ""))),
              limiterThreshold (addToLayout (layout,
                                             std::make_unique<Parameter> (ID::limiterThreshold,
                                                                          "Threshold",
                                                                          "dB",
                                                                          NormalisableRange<float> (-40.0f, 0.0f),
                                                                          0.0f,
                                                                          valueToTextFunction,
                                                                          textToValueFunction))),
              limiterRelease (addToLayout (layout,
                                           std::make_unique<Parameter> (ID::limiterRelease,
                                                                        "Release",
                                                                        "ms",
                                                                        NormalisableRange<float> (10.0f, 10000.0f, 0.0f, 0.25f),
                                                                        100.0f,
                                                                        valueToTextFunction,
                                                                        textToValueFunction))),
              directDelayEnabled (addToLayout (layout,
                                               std::make_unique<AudioParameterBool> (ID::directDelayEnabled,
                                                                                     "DL Dir.",
                                                                                     false,
                                                                                     ""))),
              directDelayType (addToLayout (layout,
                                            std::make_unique<AudioParameterChoice> (ID::directDelayType,
                                                                                    "DL Type",
                                                                                    StringArray { "None", "Linear", "Lagrange", "Thiran" },
                                                                                    1))),
              directDelayValue (addToLayout (layout,
                                             std::make_unique<Parameter> (ID::directDelayValue,
                                                                          "Delay",
                                                                          "smps",
                                                                          NormalisableRange<float> (0.0f, 44100.0f),
                                                                          0.0f,
                                                                          valueToTextFunction,
                                                                          textToValueFunction))),
              directDelaySmoothing (addToLayout (layout,
                                                 std::make_unique<Parameter> (ID::directDelaySmoothing,
                                                                              "Smooth",
                                                                              "ms",
                                                                              NormalisableRange<float> (20.0f, 10000.0f, 0.0f, 0.25f),
                                                                              200.0f,
                                                                              valueToTextFunction,
                                                                              textToValueFunction))),
              directDelayMix (addToLayout (layout,
                                           std::make_unique<Parameter> (ID::directDelayMix,
                                                                        "Delay Mix",
                                                                        "%",
                                                                        NormalisableRange<float> (0.0f, 100.0f),
                                                                        50.0f,
                                                                        valueToTextFunction,
                                                                        textToValueFunction))),
              delayEffectEnabled (addToLayout (layout,
                                               std::make_unique<AudioParameterBool> (ID::delayEffectEnabled,
                                                                                     "DL Effect",
                                                                                     false,
                                                                                     ""))),
              delayEffectType (addToLayout (layout,
                                            std::make_unique<AudioParameterChoice> (ID::delayEffectType,
                                                                                    "DL Type",
                                                                                    StringArray { "None", "Linear", "Lagrange", "Thiran" },
                                                                                    1))),
              delayEffectValue (addToLayout (layout,
                                             std::make_unique<Parameter> (ID::delayEffectValue,
                                                                          "Delay",
                                                                          "ms",
                                                                          NormalisableRange<float> (0.01f, 1000.0f),
                                                                          100.0f,
                                                                          valueToTextFunction,
                                                                          textToValueFunction))),
              delayEffectSmoothing (addToLayout (layout,
                                                 std::make_unique<Parameter> (ID::delayEffectSmoothing,
                                                                              "Smooth",
                                                                              "ms",
                                                                              NormalisableRange<float> (20.0f, 10000.0f, 0.0f, 0.25f),
                                                                              400.0f,
                                                                              valueToTextFunction,
                                                                              textToValueFunction))),
              delayEffectLowpass (addToLayout (layout,
                                               std::make_unique<Parameter> (ID::delayEffectLowpass,
                                                                            "Low-pass",
                                                                            "Hz",
                                                                            NormalisableRange<float> (20.0f, 22000.0f, 0.0f, 0.25f),
                                                                            22000.0f,
                                                                            valueToTextFunction,
                                                                            textToValueFunction))),
              delayEffectMix (addToLayout (layout,
                                           std::make_unique<Parameter> (ID::delayEffectMix,
                                                                        "Delay Mix",
                                                                        "%",
                                                                        NormalisableRange<float> (0.0f, 100.0f),
                                                                        50.0f,
                                                                        valueToTextFunction,
                                                                        textToValueFunction))),
              delayEffectFeedback (addToLayout (layout,
                                                std::make_unique<Parameter> (ID::delayEffectFeedback,
                                                                             "Feedback",
                                                                             "dB",
                                                                             NormalisableRange<float> (-100.0f, 0.0f),
                                                                             -100.0f,
                                                                             valueToTextFunction,
                                                                             textToValueFunction))),
              phaserEnabled (addToLayout (layout,
                                          std::make_unique<AudioParameterBool> (ID::phaserEnabled,
                                                                                "Phaser",
                                                                                false,
                                                                                ""))),
              phaserRate (addToLayout (layout,
                                       std::make_unique<Parameter> (ID::phaserRate,
                                                                    "Rate",
                                                                    "Hz",
                                                                    NormalisableRange<float> (0.05f, 20.0f, 0.0f, 0.25f),
                                                                    1.0f,
                                                                    valueToTextFunction,
                                                                    textToValueFunction))),
              phaserDepth (addToLayout (layout,
                                        std::make_unique<Parameter> (ID::phaserDepth,
                                                                     "Depth",
                                                                     "%",
                                                                     NormalisableRange<float> (0.0f, 100.0f),
                                                                     50.0f,
                                                                     valueToTextFunction,
                                                                     textToValueFunction))),
              phaserCentreFrequency (addToLayout (layout,
                                                  std::make_unique<Parameter> (ID::phaserCentreFrequency,
                                                                               "Center",
                                                                               "Hz",
                                                                               NormalisableRange<float> (20.0f, 20000.0f, 0.0f, 0.25f),
                                                                               600.0f,
                                                                               valueToTextFunction,
                                                                               textToValueFunction))),
              phaserFeedback (addToLayout (layout,
                                           std::make_unique<Parameter> (ID::phaserFeedback,
                                                                        "Feedback",
                                                                        "%",
                                                                        NormalisableRange<float> (0.0f, 100.0f),
                                                                        50.0f,
                                                                        valueToTextFunction,
                                                                        textToValueFunction))),
              phaserMix (addToLayout (layout,
                                      std::make_unique<Parameter> (ID::phaserMix,
                                                                   "Mix",
                                                                   "%",
                                                                   NormalisableRange<float> (0.0f, 100.0f),
                                                                   50.0f,
                                                                   valueToTextFunction,
                                                                   textToValueFunction))),
              chorusEnabled (addToLayout (layout,
                                          std::make_unique<AudioParameterBool> (ID::chorusEnabled,
                                                                                "Chorus",
                                                                                false,
                                                                                ""))),
              chorusRate (addToLayout (layout,
                                       std::make_unique<Parameter> (ID::chorusRate,
                                                                    "Rate",
                                                                    "Hz",
                                                                    NormalisableRange<float> (0.05f, 20.0f, 0.0f, 0.25f),
                                                                    1.0f,
                                                                    valueToTextFunction,
                                                                    textToValueFunction))),
              chorusDepth (addToLayout (layout,
                                        std::make_unique<Parameter> (ID::chorusDepth,
                                                                     "Depth",
                                                                     "%",
                                                                     NormalisableRange<float> (0.0f, 100.0f),
                                                                     50.0f,
                                                                     valueToTextFunction,
                                                                     textToValueFunction))),
              chorusCentreDelay (addToLayout (layout,
                                              std::make_unique<Parameter> (ID::chorusCentreDelay,
                                                                           "Center",
                                                                           "ms",
                                                                           NormalisableRange<float> (1.0f, 100.0f, 0.0f, 0.25f),
                                                                           7.0f,
                                                                           valueToTextFunction,
                                                                           textToValueFunction))),
              chorusFeedback (addToLayout (layout,
                                           std::make_unique<Parameter> (ID::chorusFeedback,
                                                                        "Feedback",
                                                                        "%",
                                                                        NormalisableRange<float> (0.0f, 100.0f),
                                                                        50.0f,
                                                                        valueToTextFunction,
                                                                        textToValueFunction))),
              chorusMix (addToLayout (layout,
                                      std::make_unique<Parameter> (ID::chorusMix,
                                                                   "Mix",
                                                                   "%",
                                                                   NormalisableRange<float> (0.0f, 100.0f),
                                                                   50.0f,
                                                                   valueToTextFunction,
                                                                   textToValueFunction))),
              ladderEnabled (addToLayout (layout,
                                          std::make_unique<AudioParameterBool> (ID::ladderEnabled,
                                                                                "Ladder",
                                                                                false,
                                                                                ""))),
              ladderMode (addToLayout (layout,
                                       std::make_unique<AudioParameterChoice> (ID::ladderMode,
                                                                               "Mode",
                                                                               StringArray { "LP12", "LP24", "HP12", "HP24", "BP12", "BP24" },
                                                                               1))),
              ladderCutoff (addToLayout (layout,
                                         std::make_unique<Parameter> (ID::ladderCutoff,
                                                                      "Frequency",
                                                                      "Hz",
                                                                      NormalisableRange<float> (10.0f, 22000.0f, 0.0f, 0.25f),
                                                                      1000.0f,
                                                                      valueToTextFunction,
                                                                      textToValueFunction))),
              ladderResonance (addToLayout (layout,
                                            std::make_unique<Parameter> (ID::ladderResonance,
                                                                         "Resonance",
                                                                         "%",
                                                                         NormalisableRange<float> (0.0f, 100.0f),
                                                                         0.0f,
                                                                         valueToTextFunction,
                                                                         textToValueFunction))),
              ladderDrive (addToLayout (layout,
                                        std::make_unique<Parameter> (ID::ladderDrive,
                                                                     "Drive",
                                                                     "dB",
                                                                     NormalisableRange<float> (0.0f, 40.0f),
                                                                     0.0f,
                                                                     valueToTextFunction,
                                                                     textToValueFunction)))
        {}

        Parameter& inputGain;
        Parameter& outputGain;
        Parameter& pan;

        AudioParameterBool& distortionEnabled;
        AudioParameterChoice& distortionType;
        Parameter& distortionInGain;
        Parameter& distortionLowpass;
        Parameter& distortionHighpass;
        Parameter& distortionCompGain;
        Parameter& distortionMix;
        AudioParameterChoice& distortionOversampler;

        AudioParameterBool& multiBandEnabled;
        Parameter& multiBandFreq;
        Parameter& multiBandLowVolume;
        Parameter& multiBandHighVolume;

        AudioParameterBool& convolutionCabEnabled;
        AudioParameterBool& convolutionReverbEnabled;
        Parameter& convolutionReverbMix;

        AudioParameterBool& compressorEnabled;
        Parameter& compressorThreshold;
        Parameter& compressorRatio;
        Parameter& compressorAttack;
        Parameter& compressorRelease;

        AudioParameterBool& noiseGateEnabled;
        Parameter& noiseGateThreshold;
        Parameter& noiseGateRatio;
        Parameter& noiseGateAttack;
        Parameter& noiseGateRelease;

        AudioParameterBool& limiterEnabled;
        Parameter& limiterThreshold;
        Parameter& limiterRelease;

        AudioParameterBool& directDelayEnabled;
        AudioParameterChoice& directDelayType;
        Parameter& directDelayValue;
        Parameter& directDelaySmoothing;
        Parameter& directDelayMix;

        AudioParameterBool& delayEffectEnabled;
        AudioParameterChoice& delayEffectType;
        Parameter& delayEffectValue;
        Parameter& delayEffectSmoothing;
        Parameter& delayEffectLowpass;
        Parameter& delayEffectMix;
        Parameter& delayEffectFeedback;

        AudioParameterBool& phaserEnabled;
        Parameter& phaserRate;
        Parameter& phaserDepth;
        Parameter& phaserCentreFrequency;
        Parameter& phaserFeedback;
        Parameter& phaserMix;

        AudioParameterBool& chorusEnabled;
        Parameter& chorusRate;
        Parameter& chorusDepth;
        Parameter& chorusCentreDelay;
        Parameter& chorusFeedback;
        Parameter& chorusMix;

        AudioParameterBool& ladderEnabled;
        AudioParameterChoice& ladderMode;
        Parameter& ladderCutoff;
        Parameter& ladderResonance;
        Parameter& ladderDrive;
    };

    const ParameterReferences& getParameterValues() const noexcept { return parameters; }

    //==============================================================================
    // We store this here so that the editor retains its state if it is closed and reopened
    int indexTab = 0;

private:
    struct LayoutAndReferences
    {
        AudioProcessorValueTreeState::ParameterLayout layout;
        ParameterReferences references;
    };

    explicit DspModulePluginDemo (AudioProcessorValueTreeState::ParameterLayout layout)
        : AudioProcessor (BusesProperties().withInput ("In",   AudioChannelSet::stereo())
                                           .withOutput ("Out", AudioChannelSet::stereo())),
          parameters { layout },
          apvts { *this, nullptr, "state", std::move (layout) }
    {
        apvts.state.addListener (this);

        forEach ([] (dsp::Gain<float>& gain) { gain.setRampDurationSeconds (0.05); },
                 dsp::get<inputGainIndex>  (chain),
                 dsp::get<outputGainIndex> (chain));

        dsp::get<pannerIndex> (chain).setRule (dsp::PannerRule::linear);
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override
    {
        requiresUpdate.store (true);
    }

    //==============================================================================
    void update()
    {
        {
            DistortionProcessor& distortion = dsp::get<distortionIndex> (chain);

            if (distortion.currentIndexOversampling != parameters.distortionOversampler.getIndex())
            {
                distortion.currentIndexOversampling = parameters.distortionOversampler.getIndex();
                prepareToPlay (getSampleRate(), getBlockSize());
                return;
            }

            distortion.currentIndexWaveshaper = parameters.distortionType.getIndex();
            distortion.lowpass .setCutoffFrequency (parameters.distortionLowpass.get());
            distortion.highpass.setCutoffFrequency (parameters.distortionHighpass.get());
            distortion.distGain.setGainDecibels (parameters.distortionInGain.get());
            distortion.compGain.setGainDecibels (parameters.distortionCompGain.get());
            distortion.mixer.setWetMixProportion (parameters.distortionMix.get() / 100.0f);
            dsp::setBypassed<distortionIndex> (chain, ! parameters.distortionEnabled);
        }

        {
            ConvolutionProcessor& convolution = dsp::get<convolutionIndex> (chain);
            convolution.cabEnabled    = parameters.convolutionCabEnabled;
            convolution.reverbEnabled = parameters.convolutionReverbEnabled;
            convolution.mixer.setWetMixProportion (parameters.convolutionReverbMix.get() / 100.0f);
        }

        dsp::get<inputGainIndex>  (chain).setGainDecibels (parameters.inputGain.get());
        dsp::get<outputGainIndex> (chain).setGainDecibels (parameters.outputGain.get());
        dsp::get<pannerIndex> (chain).setPan (parameters.pan.get() / 100.0f);

        {
            MultiBandProcessor& multiband = dsp::get<multiBandIndex> (chain);
            const auto multibandFreq = parameters.multiBandFreq.get();
            multiband.lowpass .setCutoffFrequency (multibandFreq);
            multiband.highpass.setCutoffFrequency (multibandFreq);
            const bool enabled = parameters.multiBandEnabled;
            multiband.lowVolume .setGainDecibels (enabled ? parameters.multiBandLowVolume .get() : 0.0f);
            multiband.highVolume.setGainDecibels (enabled ? parameters.multiBandHighVolume.get() : 0.0f);
            dsp::setBypassed<multiBandIndex> (chain, ! enabled);
        }

        {
            dsp::Compressor<float>& compressor = dsp::get<compressorIndex> (chain);
            compressor.setThreshold (parameters.compressorThreshold.get());
            compressor.setRatio     (parameters.compressorRatio.get());
            compressor.setAttack    (parameters.compressorAttack.get());
            compressor.setRelease   (parameters.compressorRelease.get());
            dsp::setBypassed<compressorIndex> (chain, ! parameters.compressorEnabled);
        }

        {
            dsp::NoiseGate<float>& noiseGate = dsp::get<noiseGateIndex> (chain);
            noiseGate.setThreshold (parameters.noiseGateThreshold.get());
            noiseGate.setRatio     (parameters.noiseGateRatio.get());
            noiseGate.setAttack    (parameters.noiseGateAttack.get());
            noiseGate.setRelease   (parameters.noiseGateRelease.get());
            dsp::setBypassed<noiseGateIndex> (chain, ! parameters.noiseGateEnabled);
        }

        {
            dsp::Limiter<float>& limiter = dsp::get<limiterIndex> (chain);
            limiter.setThreshold (parameters.limiterThreshold.get());
            limiter.setRelease   (parameters.limiterRelease.get());
            dsp::setBypassed<limiterIndex> (chain, ! parameters.limiterEnabled);
        }

        {
            DirectDelayProcessor& delay = dsp::get<directDelayIndex> (chain);
            delay.delayLineDirectType = parameters.directDelayType.getIndex();

            std::fill (delay.delayDirectValue.begin(),
                       delay.delayDirectValue.end(),
                       (double) parameters.directDelayValue.get());

            delay.smoothFilter.setCutoffFrequency (1000.0 / parameters.directDelaySmoothing.get());
            delay.mixer.setWetMixProportion (parameters.directDelayMix.get() / 100.0f);
            dsp::setBypassed<directDelayIndex> (chain, ! parameters.directDelayEnabled);
        }

        {
            DelayEffectProcessor& delay = dsp::get<delayEffectIndex> (chain);
            delay.delayEffectType = parameters.delayEffectType.getIndex();

            std::fill (delay.delayEffectValue.begin(),
                       delay.delayEffectValue.end(),
                       (double) parameters.delayEffectValue.get() / 1000.0 * getSampleRate());

            const auto feedbackGain = Decibels::decibelsToGain (parameters.delayEffectFeedback.get(), -100.0f);

            for (auto& volume : delay.delayFeedbackVolume)
                volume.setTargetValue (feedbackGain);

            delay.smoothFilter.setCutoffFrequency (1000.0 / parameters.delayEffectSmoothing.get());
            delay.lowpass.setCutoffFrequency (parameters.delayEffectLowpass.get());
            delay.mixer.setWetMixProportion (parameters.delayEffectMix.get() / 100.0f);
            dsp::setBypassed<delayEffectIndex> (chain, ! parameters.delayEffectEnabled);
        }

        {
            dsp::Phaser<float>& phaser = dsp::get<phaserIndex> (chain);
            phaser.setRate            (parameters.phaserRate.get());
            phaser.setDepth           (parameters.phaserDepth.get() / 100.0f);
            phaser.setCentreFrequency (parameters.phaserCentreFrequency.get());
            phaser.setFeedback        (parameters.phaserFeedback.get() / 100.0f * 0.95f);
            phaser.setMix             (parameters.phaserMix.get() / 100.0f);
            dsp::setBypassed<phaserIndex> (chain, ! parameters.phaserEnabled);
        }

        {
            dsp::Chorus<float>& chorus = dsp::get<chorusIndex> (chain);
            chorus.setRate        (parameters.chorusRate.get());
            chorus.setDepth       (parameters.chorusDepth.get() / 100.0f);
            chorus.setCentreDelay (parameters.chorusCentreDelay.get());
            chorus.setFeedback    (parameters.chorusFeedback.get() / 100.0f * 0.95f);
            chorus.setMix         (parameters.chorusMix.get() / 100.0f);
            dsp::setBypassed<chorusIndex> (chain, ! parameters.chorusEnabled);
        }

        {
            dsp::LadderFilter<float>& ladder = dsp::get<ladderIndex> (chain);

            ladder.setCutoffFrequencyHz (parameters.ladderCutoff.get());
            ladder.setResonance         (parameters.ladderResonance.get() / 100.0f);
            ladder.setDrive (Decibels::decibelsToGain (parameters.ladderDrive.get()));

            ladder.setMode ([&]
            {
                switch (parameters.ladderMode.getIndex())
                {
                    case 0: return dsp::LadderFilterMode::LPF12;
                    case 1: return dsp::LadderFilterMode::LPF24;
                    case 2: return dsp::LadderFilterMode::HPF12;
                    case 3: return dsp::LadderFilterMode::HPF24;
                    case 4: return dsp::LadderFilterMode::BPF12;

                    default: break;
                }

                return dsp::LadderFilterMode::BPF24;
            }());

            dsp::setBypassed<ladderIndex> (chain, ! parameters.ladderEnabled);
        }

        requiresUpdate.store (false);
    }

    //==============================================================================
    static String getPanningTextForValue (float value)
    {
        if (value == 0.5f)
            return "center";

        if (value < 0.5f)
            return String (roundToInt ((0.5f - value) * 200.0f)) + "%L";

        return String (roundToInt ((value - 0.5f) * 200.0f)) + "%R";
    }

    static float getPanningValueForText (String strText)
    {
        if (strText.compareIgnoreCase ("center") == 0 || strText.compareIgnoreCase ("c") == 0)
            return 0.5f;

        strText = strText.trim();

        if (strText.indexOfIgnoreCase ("%L") != -1)
        {
            auto percentage = (float) strText.substring (0, strText.indexOf ("%")).getDoubleValue();
            return (100.0f - percentage) / 100.0f * 0.5f;
        }

        if (strText.indexOfIgnoreCase ("%R") != -1)
        {
            auto percentage = (float) strText.substring (0, strText.indexOf ("%")).getDoubleValue();
            return percentage / 100.0f * 0.5f + 0.5f;
        }

        return 0.5f;
    }

    //==============================================================================
    struct DistortionProcessor
    {
        DistortionProcessor()
        {
            forEach ([] (dsp::Gain<float>& gain) { gain.setRampDurationSeconds (0.05); },
                     distGain,
                     compGain);

            lowpass.setType  (dsp::FirstOrderTPTFilterType::lowpass);
            highpass.setType (dsp::FirstOrderTPTFilterType::highpass);
            mixer.setMixingRule (dsp::DryWetMixingRule::linear);
        }

        void prepare (const dsp::ProcessSpec& spec)
        {
            for (auto& oversampler : oversamplers)
                oversampler.initProcessing (spec.maximumBlockSize);

            prepareAll (spec, lowpass, highpass, distGain, compGain, mixer);
        }

        void reset()
        {
            for (auto& oversampler : oversamplers)
                oversampler.reset();

            resetAll (lowpass, highpass, distGain, compGain, mixer);
        }

        float getLatency() const
        {
            return oversamplers[size_t (currentIndexOversampling)].getLatencyInSamples();
        }

        template <typename Context>
        void process (Context& context)
        {
            if (context.isBypassed)
                return;

            const auto& inputBlock = context.getInputBlock();

            mixer.setWetLatency (getLatency());
            mixer.pushDrySamples (inputBlock);

            distGain.process (context);
            highpass.process (context);

            auto ovBlock = oversamplers[size_t (currentIndexOversampling)].processSamplesUp (inputBlock);

            dsp::ProcessContextReplacing<float> waveshaperContext (ovBlock);

            if (isPositiveAndBelow (currentIndexWaveshaper, waveShapers.size()))
            {
                waveShapers[size_t (currentIndexWaveshaper)].process (waveshaperContext);

                if (currentIndexWaveshaper == 1)
                    clipping.process (waveshaperContext);

                waveshaperContext.getOutputBlock() *= 0.7f;
            }

            auto& outputBlock = context.getOutputBlock();
            oversamplers[size_t (currentIndexOversampling)].processSamplesDown (outputBlock);

            lowpass.process (context);
            compGain.process (context);
            mixer.mixWetSamples (outputBlock);
        }

        std::array<dsp::Oversampling<float>, 6> oversamplers
        { {
            { 2, 1, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false },
            { 2, 2, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false },
            { 2, 3, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false },

            { 2, 1, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true },
            { 2, 2, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true },
            { 2, 3, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true },
        } };

        static float clip (float in) { return juce::jlimit (-1.0f, 1.0f, in); }

        dsp::FirstOrderTPTFilter<float> lowpass, highpass;
        dsp::Gain<float> distGain, compGain;
        dsp::DryWetMixer<float> mixer { 10 };
        std::array<dsp::WaveShaper<float>, 2> waveShapers { { { std::tanh },
                                                              { dsp::FastMathApproximations::tanh } } };
        dsp::WaveShaper<float> clipping { clip };
        int currentIndexOversampling = 0;
        int currentIndexWaveshaper   = 0;
    };

    struct ConvolutionProcessor
    {
        ConvolutionProcessor()
        {
            loadImpulseResponse (cabinet, "guitar_amp.wav");
            loadImpulseResponse (reverb,  "reverb_ir.wav");
            mixer.setMixingRule (dsp::DryWetMixingRule::balanced);
        }

        void prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, cabinet, reverb, mixer);
        }

        void reset()
        {
            resetAll (cabinet, reverb, mixer);
        }

        template <typename Context>
        void process (Context& context)
        {
            auto contextConv = context;
            contextConv.isBypassed = (! cabEnabled) || context.isBypassed;
            cabinet.process (contextConv);

            if (cabEnabled)
                context.getOutputBlock().multiplyBy (4.0f);

            if (reverbEnabled)
                mixer.pushDrySamples (context.getInputBlock());

            contextConv.isBypassed = (! reverbEnabled) || context.isBypassed;
            reverb.process (contextConv);

            if (reverbEnabled)
            {
                const auto& outputBlock = context.getOutputBlock();
                outputBlock.multiplyBy (4.0f);
                mixer.mixWetSamples (outputBlock);
            }
        }

        int getLatency() const
        {
            auto latency = 0;

            if (cabEnabled)
                latency += cabinet.getLatency();

            if (reverbEnabled)
                latency += reverb.getLatency();

            return latency;
        }

        dsp::ConvolutionMessageQueue queue;
        dsp::Convolution cabinet { dsp::Convolution::NonUniform { 512 }, queue };
        dsp::Convolution reverb { dsp::Convolution::NonUniform { 512 }, queue };
        dsp::DryWetMixer<float> mixer;
        bool cabEnabled = false, reverbEnabled = false;

    private:
        static void loadImpulseResponse (dsp::Convolution& convolution, const char* filename)
        {
            auto stream = createAssetInputStream (filename);

            if (stream == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioFormatManager manager;
            manager.registerBasicFormats();
            std::unique_ptr<AudioFormatReader> reader { manager.createReaderFor (std::move (stream)) };

            if (reader == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioBuffer<float> buffer (static_cast<int> (reader->numChannels),
                                       static_cast<int> (reader->lengthInSamples));
            reader->read (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), 0, buffer.getNumSamples());

            convolution.loadImpulseResponse (std::move (buffer),
                                             reader->sampleRate,
                                             dsp::Convolution::Stereo::yes,
                                             dsp::Convolution::Trim::yes,
                                             dsp::Convolution::Normalise::yes);
        }
    };

    struct MultiBandProcessor
    {
        MultiBandProcessor()
        {
            forEach ([] (dsp::Gain<float>& gain) { gain.setRampDurationSeconds (0.05); },
                     lowVolume,
                     highVolume);

            lowpass .setType (dsp::LinkwitzRileyFilterType::lowpass);
            highpass.setType (dsp::LinkwitzRileyFilterType::highpass);
        }

        void prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, lowpass, highpass, lowVolume, highVolume);
            bufferSeparation.setSize (4, int (spec.maximumBlockSize), false, false, true);
        }

        void reset()
        {
            resetAll (lowpass, highpass, lowVolume, highVolume);
        }

        template <typename Context>
        void process (Context& context)
        {
            const auto& inputBlock = context.getInputBlock();

            const auto numSamples  = inputBlock.getNumSamples();
            const auto numChannels = inputBlock.getNumChannels();

            auto sepBlock = dsp::AudioBlock<float> (bufferSeparation).getSubBlock (0, (size_t) numSamples);

            auto sepLowBlock  = sepBlock.getSubsetChannelBlock (0, (size_t) numChannels);
            auto sepHighBlock = sepBlock.getSubsetChannelBlock (2, (size_t) numChannels);

            sepLowBlock .copyFrom (inputBlock);
            sepHighBlock.copyFrom (inputBlock);

            auto contextLow = dsp::ProcessContextReplacing<float> (sepLowBlock);
            contextLow.isBypassed = context.isBypassed;
            lowpass  .process (contextLow);
            lowVolume.process (contextLow);

            auto contextHigh = dsp::ProcessContextReplacing<float> (sepHighBlock);
            contextHigh.isBypassed = context.isBypassed;
            highpass  .process (contextHigh);
            highVolume.process (contextHigh);

            if (! context.isBypassed)
            {
                sepLowBlock.add (sepHighBlock);
                context.getOutputBlock().copyFrom (sepLowBlock);
            }
        }

        dsp::LinkwitzRileyFilter<float> lowpass, highpass;
        dsp::Gain<float> lowVolume, highVolume;
        AudioBuffer<float> bufferSeparation;
    };

    struct DirectDelayProcessor
    {
        DirectDelayProcessor()
        {
            smoothFilter.setType (dsp::FirstOrderTPTFilterType::lowpass);
            mixer.setMixingRule (dsp::DryWetMixingRule::linear);
        }

        void prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, noInterpolation, linear, lagrange, thiran, smoothFilter, mixer);
        }

        void reset()
        {
            resetAll (noInterpolation, linear, lagrange, thiran, smoothFilter, mixer);
        }

        template <typename Context>
        void process (Context& context)
        {
            if (context.isBypassed)
                return;

            const auto& inputBlock  = context.getInputBlock();
            const auto& outputBlock = context.getOutputBlock();

            mixer.pushDrySamples (inputBlock);

            const auto numChannels = inputBlock.getNumChannels();
            const auto numSamples  = inputBlock.getNumSamples();

            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto* samplesIn  = inputBlock .getChannelPointer (channel);
                auto* samplesOut = outputBlock.getChannelPointer (channel);

                for (size_t i = 0; i < numSamples; ++i)
                {
                    const auto delay = smoothFilter.processSample (int (channel), delayDirectValue[channel]);

                    samplesOut[i] = [&]
                    {
                        switch (delayLineDirectType)
                        {
                            case 0:
                                noInterpolation.pushSample (int (channel), samplesIn[i]);
                                noInterpolation.setDelay ((float) delay);
                                return noInterpolation.popSample (int (channel));

                            case 1:
                                linear.pushSample (int (channel), samplesIn[i]);
                                linear.setDelay ((float) delay);
                                return linear.popSample (int (channel));

                            case 2:
                                lagrange.pushSample (int (channel), samplesIn[i]);
                                lagrange.setDelay ((float) delay);
                                return lagrange.popSample (int (channel));

                            case 3:
                                thiran.pushSample (int (channel), samplesIn[i]);
                                thiran.setDelay ((float) delay);
                                return thiran.popSample (int (channel));

                            default:
                                break;
                        }

                        jassertfalse;
                        return 0.0f;
                    }();
                }
            }

            mixer.mixWetSamples (outputBlock);
        }

        static constexpr auto directDelayBufferSize = 44100;
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::None>        noInterpolation { directDelayBufferSize };
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Linear>      linear          { directDelayBufferSize };
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd> lagrange        { directDelayBufferSize };
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Thiran>      thiran          { directDelayBufferSize };

        // Double precision to avoid some approximation issues
        dsp::FirstOrderTPTFilter<double> smoothFilter;

        dsp::DryWetMixer<float> mixer;
        std::array<double, 2> delayDirectValue { {} };

        int delayLineDirectType = 1;
    };

    struct DelayEffectProcessor
    {
        DelayEffectProcessor()
        {
            smoothFilter.setType (dsp::FirstOrderTPTFilterType::lowpass);
            lowpass.setType      (dsp::FirstOrderTPTFilterType::lowpass);
            mixer.setMixingRule (dsp::DryWetMixingRule::linear);
        }

        void prepare (const dsp::ProcessSpec& spec)
        {
            prepareAll (spec, noInterpolation, linear, lagrange, thiran, smoothFilter, lowpass, mixer);

            for (auto& volume : delayFeedbackVolume)
                volume.reset (spec.sampleRate, 0.05);
        }

        void reset()
        {
            resetAll (noInterpolation, linear, lagrange, thiran, smoothFilter, lowpass, mixer);
            std::fill (lastDelayEffectOutput.begin(), lastDelayEffectOutput.end(), 0.0f);
        }

        template <typename Context>
        void process (Context& context)
        {
            if (context.isBypassed)
                return;

            const auto& inputBlock  = context.getInputBlock();
            const auto& outputBlock = context.getOutputBlock();
            const auto numSamples  = inputBlock.getNumSamples();
            const auto numChannels = inputBlock.getNumChannels();

            mixer.pushDrySamples (inputBlock);

            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto* samplesIn  = inputBlock .getChannelPointer (channel);
                auto* samplesOut = outputBlock.getChannelPointer (channel);

                for (size_t i = 0; i < numSamples; ++i)
                {
                    auto input = samplesIn[i] - lastDelayEffectOutput[channel];

                    auto delay = smoothFilter.processSample (int (channel), delayEffectValue[channel]);

                    const auto output = [&]
                    {
                        switch (delayEffectType)
                        {
                            case 0:
                                noInterpolation.pushSample (int (channel), input);
                                noInterpolation.setDelay ((float) delay);
                                return noInterpolation.popSample (int (channel));

                            case 1:
                                linear.pushSample (int (channel), input);
                                linear.setDelay ((float) delay);
                                return linear.popSample (int (channel));

                            case 2:
                                lagrange.pushSample (int (channel), input);
                                lagrange.setDelay ((float) delay);
                                return lagrange.popSample (int (channel));

                            case 3:
                                thiran.pushSample (int (channel), input);
                                thiran.setDelay ((float) delay);
                                return thiran.popSample (int (channel));

                            default:
                                break;
                        }

                        jassertfalse;
                        return 0.0f;
                    }();

                    const auto processed = lowpass.processSample (int (channel), output);

                    samplesOut[i] = processed;
                    lastDelayEffectOutput[channel] = processed * delayFeedbackVolume[channel].getNextValue();
                }
            }

            mixer.mixWetSamples (outputBlock);
        }

        static constexpr auto effectDelaySamples = 192000;
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::None>        noInterpolation { effectDelaySamples };
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Linear>      linear          { effectDelaySamples };
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Lagrange3rd> lagrange        { effectDelaySamples };
        dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Thiran>      thiran          { effectDelaySamples };

        // Double precision to avoid some approximation issues
        dsp::FirstOrderTPTFilter<double> smoothFilter;

        std::array<double, 2> delayEffectValue;

        std::array<LinearSmoothedValue<float>, 2> delayFeedbackVolume;
        dsp::FirstOrderTPTFilter<float> lowpass;
        dsp::DryWetMixer<float> mixer;
        std::array<float, 2> lastDelayEffectOutput;

        int delayEffectType = 1;
    };

    ParameterReferences parameters;
    AudioProcessorValueTreeState apvts;

    using Chain = dsp::ProcessorChain<dsp::NoiseGate<float>,
                                      dsp::Gain<float>,
                                      DirectDelayProcessor,
                                      MultiBandProcessor,
                                      dsp::Compressor<float>,
                                      dsp::Phaser<float>,
                                      dsp::Chorus<float>,
                                      DistortionProcessor,
                                      dsp::LadderFilter<float>,
                                      DelayEffectProcessor,
                                      ConvolutionProcessor,
                                      dsp::Limiter<float>,
                                      dsp::Gain<float>,
                                      dsp::Panner<float>>;
    Chain chain;

    // We use this enum to index into the chain above
    enum ProcessorIndices
    {
        noiseGateIndex,
        inputGainIndex,
        directDelayIndex,
        multiBandIndex,
        compressorIndex,
        phaserIndex,
        chorusIndex,
        distortionIndex,
        ladderIndex,
        delayEffectIndex,
        convolutionIndex,
        limiterIndex,
        outputGainIndex,
        pannerIndex
    };

    //==============================================================================
    std::atomic<bool> requiresUpdate { true };
    std::atomic<int> irSize { 0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemo)
};

//==============================================================================
class DspModulePluginDemoEditor  : public AudioProcessorEditor
{
public:
    explicit DspModulePluginDemoEditor (DspModulePluginDemo& p)
        : AudioProcessorEditor (&p),
          proc (p)
    {
        comboEffect.addSectionHeading ("Main");
        comboEffect.addItem ("Distortion", TabDistortion);
        comboEffect.addItem ("Convolution", TabConvolution);
        comboEffect.addItem ("Multi-band", TabMultiBand);

        comboEffect.addSectionHeading ("Dynamics");
        comboEffect.addItem ("Compressor", TabCompressor);
        comboEffect.addItem ("Noise gate", TabNoiseGate);
        comboEffect.addItem ("Limiter", TabLimiter);

        comboEffect.addSectionHeading ("Delay");
        comboEffect.addItem ("Delay line direct", TabDelayLineDirect);
        comboEffect.addItem ("Delay line effect", TabDelayLineEffect);

        comboEffect.addSectionHeading ("Others");
        comboEffect.addItem ("Phaser", TabPhaser);
        comboEffect.addItem ("Chorus", TabChorus);
        comboEffect.addItem ("Ladder filter", TabLadder);

        comboEffect.setSelectedId (proc.indexTab + 1, dontSendNotification);
        comboEffect.onChange = [this]
        {
            proc.indexTab = comboEffect.getSelectedId() - 1;
            updateVisibility();
        };

        addAllAndMakeVisible (*this,
                              comboEffect,
                              labelEffect,
                              basicControls,
                              distortionControls,
                              convolutionControls,
                              multibandControls,
                              compressorControls,
                              noiseGateControls,
                              limiterControls,
                              directDelayControls,
                              delayEffectControls,
                              phaserControls,
                              chorusControls,
                              ladderControls);
        labelEffect.setJustificationType (Justification::centredRight);
        labelEffect.attachToComponent (&comboEffect, true);

        updateVisibility();

        setSize (800, 430);
        setResizable (false, false);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        auto rect = getLocalBounds();

        auto rectTop    = rect.removeFromTop (topSize);
        auto rectBottom = rect.removeFromBottom (bottomSize);

        auto rectEffects = rect.removeFromBottom (tabSize);
        auto rectChoice  = rect.removeFromBottom (midSize);

        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.fillRect (rect);

        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).brighter (0.2f));
        g.fillRect (rectEffects);

        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker (0.2f));
        g.fillRect (rectTop);
        g.fillRect (rectBottom);
        g.fillRect (rectChoice);

        g.setColour (Colours::white);
        g.setFont (Font (20.0f).italicised().withExtraKerningFactor (0.1f));
        g.drawFittedText ("DSP MODULE DEMO", rectTop.reduced (10, 0), Justification::centredLeft, 1);

        g.setFont (Font (14.0f));
        String strText = "IR length (reverb): " + String (proc.getCurrentIRSize()) + " samples";
        g.drawFittedText (strText, rectBottom.reduced (10, 0), Justification::centredRight, 1);
    }

    void resized() override
    {
        auto rect = getLocalBounds();
        rect.removeFromTop (topSize);
        rect.removeFromBottom (bottomSize);

        auto rectEffects = rect.removeFromBottom (tabSize);
        auto rectChoice  = rect.removeFromBottom (midSize);

        comboEffect.setBounds (rectChoice.withSizeKeepingCentre (200, 24));

        rect.reduce (80, 0);
        rectEffects.reduce (20, 0);

        basicControls.setBounds (rect);

        forEach ([&] (Component& comp) { comp.setBounds (rectEffects); },
                 distortionControls,
                 convolutionControls,
                 multibandControls,
                 compressorControls,
                 noiseGateControls,
                 limiterControls,
                 directDelayControls,
                 delayEffectControls,
                 phaserControls,
                 chorusControls,
                 ladderControls);
    }

private:
    class AttachedSlider  : public Component
    {
    public:
        explicit AttachedSlider (RangedAudioParameter& param)
            : label ("", param.name),
              attachment (param, slider)
        {
            addAllAndMakeVisible (*this, slider, label);

            slider.setTextValueSuffix (" " + param.label);

            label.attachToComponent (&slider, false);
            label.setJustificationType (Justification::centred);
        }

        void resized() override { slider.setBounds (getLocalBounds().reduced (0, 40)); }

    private:
        Slider slider { Slider::RotaryVerticalDrag, Slider::TextBoxBelow };
        Label label;
        SliderParameterAttachment attachment;
    };

    class AttachedToggle  : public Component
    {
    public:
        explicit AttachedToggle (RangedAudioParameter& param)
            : toggle (param.name),
              attachment (param, toggle)
        {
            addAndMakeVisible (toggle);
        }

        void resized() override { toggle.setBounds (getLocalBounds()); }

    private:
        ToggleButton toggle;
        ButtonParameterAttachment attachment;
    };

    class AttachedCombo  : public Component
    {
    public:
        explicit AttachedCombo (RangedAudioParameter& param)
            : combo (param),
              label ("", param.name),
              attachment (param, combo)
        {
            addAllAndMakeVisible (*this, combo, label);

            label.attachToComponent (&combo, false);
            label.setJustificationType (Justification::centred);
        }

        void resized() override
        {
            combo.setBounds (getLocalBounds().withSizeKeepingCentre (jmin (getWidth(), 150), 24));
        }

    private:
        struct ComboWithItems : public ComboBox
        {
            explicit ComboWithItems (RangedAudioParameter& param)
            {
                // Adding the list here in the constructor means that the combo
                // is already populated when we construct the attachment below
                addItemList (dynamic_cast<AudioParameterChoice&> (param).choices, 1);
            }
        };

        ComboWithItems combo;
        Label label;
        ComboBoxParameterAttachment attachment;
    };

    //==============================================================================
    void updateVisibility()
    {
        const auto indexEffect = comboEffect.getSelectedId();

        const auto op = [&] (const std::tuple<Component&, int>& tup)
        {
            Component& comp    = std::get<0> (tup);
            const int tabIndex = std::get<1> (tup);
            comp.setVisible (tabIndex == indexEffect);
        };

        forEach (op,
                 std::forward_as_tuple (distortionControls,  TabDistortion),
                 std::forward_as_tuple (convolutionControls, TabConvolution),
                 std::forward_as_tuple (multibandControls,   TabMultiBand),
                 std::forward_as_tuple (compressorControls,  TabCompressor),
                 std::forward_as_tuple (noiseGateControls,   TabNoiseGate),
                 std::forward_as_tuple (limiterControls,     TabLimiter),
                 std::forward_as_tuple (directDelayControls, TabDelayLineDirect),
                 std::forward_as_tuple (delayEffectControls, TabDelayLineEffect),
                 std::forward_as_tuple (phaserControls,      TabPhaser),
                 std::forward_as_tuple (chorusControls,      TabChorus),
                 std::forward_as_tuple (ladderControls,      TabLadder));
    }

    enum EffectsTabs
    {
        TabDistortion = 1,
        TabConvolution,
        TabMultiBand,
        TabCompressor,
        TabNoiseGate,
        TabLimiter,
        TabDelayLineDirect,
        TabDelayLineEffect,
        TabPhaser,
        TabChorus,
        TabLadder
    };

    //==============================================================================
    ComboBox comboEffect;
    Label labelEffect { "Audio effect: " };

    struct GetTrackInfo
    {
        // Combo boxes need a lot of room
        Grid::TrackInfo operator() (AttachedCombo&)             const { return 120_px; }

        // Toggles are a bit smaller
        Grid::TrackInfo operator() (AttachedToggle&)            const { return 80_px; }

        // Sliders take up as much room as they can
        Grid::TrackInfo operator() (AttachedSlider&)            const { return 1_fr; }
    };

    template <typename... Components>
    static void performLayout (const Rectangle<int>& bounds, Components&... components)
    {
        Grid grid;
        using Track = Grid::TrackInfo;

        grid.autoColumns     = Track (1_fr);
        grid.autoRows        = Track (1_fr);
        grid.columnGap       = Grid::Px (10);
        grid.rowGap          = Grid::Px (0);
        grid.autoFlow        = Grid::AutoFlow::column;

        grid.templateColumns = { GetTrackInfo{} (components)... };
        grid.items           = { GridItem (components)... };

        grid.performLayout (bounds);
    }

    struct BasicControls : public Component
    {
        explicit BasicControls (const DspModulePluginDemo::ParameterReferences& state)
            : pan       (state.pan),
              input     (state.inputGain),
              output    (state.outputGain)
        {
            addAllAndMakeVisible (*this, pan, input, output);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), input, output, pan);
        }

        AttachedSlider pan, input, output;
    };

    struct DistortionControls : public Component
    {
        explicit DistortionControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle       (state.distortionEnabled),
              lowpass      (state.distortionLowpass),
              highpass     (state.distortionHighpass),
              mix          (state.distortionMix),
              gain         (state.distortionInGain),
              compv        (state.distortionCompGain),
              type         (state.distortionType),
              oversampling (state.distortionOversampler)
        {
            addAllAndMakeVisible (*this, toggle, type, lowpass, highpass, mix, gain, compv, oversampling);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, type, gain, highpass, lowpass, compv, mix, oversampling);
        }

        AttachedToggle toggle;
        AttachedSlider lowpass, highpass, mix, gain, compv;
        AttachedCombo type, oversampling;
    };

    struct ConvolutionControls : public Component
    {
        explicit ConvolutionControls (const DspModulePluginDemo::ParameterReferences& state)
            : cab    (state.convolutionCabEnabled),
              reverb (state.convolutionReverbEnabled),
              mix    (state.convolutionReverbMix)
        {
            addAllAndMakeVisible (*this, cab, reverb, mix);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), cab, reverb, mix);
        }

        AttachedToggle cab, reverb;
        AttachedSlider mix;
    };

    struct MultiBandControls : public Component
    {
        explicit MultiBandControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle (state.multiBandEnabled),
              low    (state.multiBandLowVolume),
              high   (state.multiBandHighVolume),
              lRFreq (state.multiBandFreq)
        {
            addAllAndMakeVisible (*this, toggle, low, high, lRFreq);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, lRFreq, low, high);
        }

        AttachedToggle toggle;
        AttachedSlider low, high, lRFreq;
    };

    struct CompressorControls : public Component
    {
        explicit CompressorControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle    (state.compressorEnabled),
              threshold (state.compressorThreshold),
              ratio     (state.compressorRatio),
              attack    (state.compressorAttack),
              release   (state.compressorRelease)
        {
            addAllAndMakeVisible (*this, toggle, threshold, ratio, attack, release);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, threshold, ratio, attack, release);
        }

        AttachedToggle toggle;
        AttachedSlider threshold, ratio, attack, release;
    };

    struct NoiseGateControls : public Component
    {
        explicit NoiseGateControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle    (state.noiseGateEnabled),
              threshold (state.noiseGateThreshold),
              ratio     (state.noiseGateRatio),
              attack    (state.noiseGateAttack),
              release   (state.noiseGateRelease)
        {
            addAllAndMakeVisible (*this, toggle, threshold, ratio, attack, release);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, threshold, ratio, attack, release);
        }

        AttachedToggle toggle;
        AttachedSlider threshold, ratio, attack, release;
    };

    struct LimiterControls : public Component
    {
        explicit LimiterControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle    (state.limiterEnabled),
              threshold (state.limiterThreshold),
              release   (state.limiterRelease)
        {
            addAllAndMakeVisible (*this, toggle, threshold, release);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, threshold, release);
        }

        AttachedToggle toggle;
        AttachedSlider threshold, release;
    };

    struct DirectDelayControls : public Component
    {
        explicit DirectDelayControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle (state.directDelayEnabled),
              type   (state.directDelayType),
              delay  (state.directDelayValue),
              smooth (state.directDelaySmoothing),
              mix    (state.directDelayMix)
        {
            addAllAndMakeVisible (*this, toggle, type, delay, smooth, mix);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, type, delay, smooth, mix);
        }

        AttachedToggle toggle;
        AttachedCombo type;
        AttachedSlider delay, smooth, mix;
    };

    struct DelayEffectControls : public Component
    {
        explicit DelayEffectControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle   (state.delayEffectEnabled),
              type     (state.delayEffectType),
              value    (state.delayEffectValue),
              smooth   (state.delayEffectSmoothing),
              lowpass  (state.delayEffectLowpass),
              feedback (state.delayEffectFeedback),
              mix      (state.delayEffectMix)
        {
            addAllAndMakeVisible (*this, toggle, type, value, smooth, lowpass, feedback, mix);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, type, value, smooth, lowpass, feedback, mix);
        }

        AttachedToggle toggle;
        AttachedCombo type;
        AttachedSlider value, smooth, lowpass, feedback, mix;
    };

    struct PhaserControls : public Component
    {
        explicit PhaserControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle   (state.phaserEnabled),
              rate     (state.phaserRate),
              depth    (state.phaserDepth),
              centre   (state.phaserCentreFrequency),
              feedback (state.phaserFeedback),
              mix      (state.phaserMix)
        {
            addAllAndMakeVisible (*this, toggle, rate, depth, centre, feedback, mix);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, rate, depth, centre, feedback, mix);
        }

        AttachedToggle toggle;
        AttachedSlider rate, depth, centre, feedback, mix;
    };

    struct ChorusControls : public Component
    {
        explicit ChorusControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle   (state.chorusEnabled),
              rate     (state.chorusRate),
              depth    (state.chorusDepth),
              centre   (state.chorusCentreDelay),
              feedback (state.chorusFeedback),
              mix      (state.chorusMix)
        {
            addAllAndMakeVisible (*this, toggle, rate, depth, centre, feedback, mix);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, rate, depth, centre, feedback, mix);
        }

        AttachedToggle toggle;
        AttachedSlider rate, depth, centre, feedback, mix;
    };

    struct LadderControls : public Component
    {
        explicit LadderControls (const DspModulePluginDemo::ParameterReferences& state)
            : toggle    (state.ladderEnabled),
              mode      (state.ladderMode),
              freq      (state.ladderCutoff),
              resonance (state.ladderResonance),
              drive     (state.ladderDrive)
        {
            addAllAndMakeVisible (*this, toggle, mode, freq, resonance, drive);
        }

        void resized() override
        {
            performLayout (getLocalBounds(), toggle, mode, freq, resonance, drive);
        }

        AttachedToggle toggle;
        AttachedCombo mode;
        AttachedSlider freq, resonance, drive;
    };

    //==============================================================================
    static constexpr auto topSize    = 40,
                          bottomSize = 40,
                          midSize    = 40,
                          tabSize    = 155;

    //==============================================================================
    DspModulePluginDemo& proc;

    BasicControls       basicControls       { proc.getParameterValues() };
    DistortionControls  distortionControls  { proc.getParameterValues() };
    ConvolutionControls convolutionControls { proc.getParameterValues() };
    MultiBandControls   multibandControls   { proc.getParameterValues() };
    CompressorControls  compressorControls  { proc.getParameterValues() };
    NoiseGateControls   noiseGateControls   { proc.getParameterValues() };
    LimiterControls     limiterControls     { proc.getParameterValues() };
    DirectDelayControls directDelayControls { proc.getParameterValues() };
    DelayEffectControls delayEffectControls { proc.getParameterValues() };
    PhaserControls      phaserControls      { proc.getParameterValues() };
    ChorusControls      chorusControls      { proc.getParameterValues() };
    LadderControls      ladderControls      { proc.getParameterValues() };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspModulePluginDemoEditor)
};

struct DspModulePluginDemoAudioProcessor  : public DspModulePluginDemo
{
    AudioProcessorEditor* createEditor() override
    {
        return new DspModulePluginDemoEditor (*this);
    }

    bool hasEditor() const override { return true; }
};
