#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

#include <map>

class PluginDemoPlaybackRenderer : public juce::ARAPlaybackRenderer
{
public:
    //==============================================================================
    PluginDemoPlaybackRenderer(juce::ARADocumentController* documentController, bool useBuffering = true)
        : juce::ARAPlaybackRenderer(documentController), useBufferedAudioSourceReader (useBuffering) {}

    //==============================================================================
    void prepareToPlay (const ProcessSpec& spec) override;
    void releaseResources() override;

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    bool processBlock (juce::AudioBuffer<float>& buffer, const ProcessContext& context) noexcept override;

private:
    //==============================================================================
    // We're subclassing here only to provide a proper default c'tor for our shared ressource
    class SharedTimeSliceThread   : public juce::TimeSliceThread
    {
        public:
            SharedTimeSliceThread()
                : TimeSliceThread (juce::String (JucePlugin_Name) + " ARA Sample Reading Thread")
            {
                startThread (7);   // above "default" priority so playback is fluent, but below realtime
            }
    };
    juce::SharedResourcePointer<SharedTimeSliceThread> sharedTimesliceThread;

    //==============================================================================
    ProcessSpec processSpec { 44100.0, 4096, 2 };

    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<juce::ARAAudioSource*, std::unique_ptr<juce::AudioFormatReader>> audioSourceReaders;

    // temp buffers to use for summing signals if rendering multiple regions
    std::unique_ptr<juce::AudioBuffer<float>> tempBuffer;

    const bool useBufferedAudioSourceReader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginDemoPlaybackRenderer)
};
