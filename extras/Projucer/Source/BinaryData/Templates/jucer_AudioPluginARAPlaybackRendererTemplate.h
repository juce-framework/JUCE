/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA playback renderer implementation. 

  ==============================================================================
*/

#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

//==============================================================================
/**
*/
class %%araplaybackrenderer_class_name%%  : public juce::ARAPlaybackRenderer
{
public:
    //==============================================================================
    using juce::ARAPlaybackRenderer::ARAPlaybackRenderer;

    //==============================================================================
    void prepareToPlay (double sampleRate, int maximumSamplesPerBlock, int numChannels) override;
    void releaseResources() override;

    //==============================================================================
    bool processBlock (juce::AudioBuffer<float>& buffer, bool isNonRealtime, const juce::AudioPlayHead::CurrentPositionInfo& positionInfo) noexcept override;

private:
    //==============================================================================
    double sampleRate { 44100.0 };
    int maximumSamplesPerBlock { 4096 };
    int numChannels { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%araplaybackrenderer_class_name%%)
};
