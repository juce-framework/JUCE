#pragma once

#include "JuceHeader.h"
#include <map>

#if ! JucePlugin_Enable_ARA
    #error "bad project configuration, JucePlugin_Enable_ARA is required for compiling this class"
#endif

//==============================================================================
/**
    Processor class for ARAPluginDemo
    In this simple demo we're using a buffered ARA sample reader to pull audio samples
    from the host and render them without any modifications, effectively making this
    an ARA enabled pass-through renderer.
*/
class ARAPluginDemoAudioProcessor     : public AudioProcessor,
                                        public AudioProcessorARAExtension
{
public:
    //==============================================================================
    // Default construction as done by the plug-in client wrappers via createPluginFilterOfType()
    // Since these may be used in realtime contexts, they must use internal buffering when reading
    // audio source samples, thus this c'tor sets useBufferedAudioSourceReader to true.
    ARAPluginDemoAudioProcessor();

    // Explicit construction when used internally, see PlaybackRegionView.cpp for an example
    // In typical UI use cases, these internal processors are used inside an ARAPlaybackRegionReader
    // on a separate background thread which already implements buffering - to prevent unnecessary
    // double buffering, set useBufferedAudioSourceReader to false in such cases.
    explicit ARAPluginDemoAudioProcessor(bool useBufferedAudioSourceReader);

    // Getter of current playback state for the UI
    const AudioPlayHead::CurrentPositionInfo& getLastKnownPositionInfo() { return lastPositionInfo; }

    //==============================================================================
    void prepareToPlay (double newSampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    bool didProcessBlockSucceed() override { return lastProcessBlockSucceeded; }

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:

    // We're subclassing here only to provide a proper default c'tor for our shared ressource
    class SharedTimeSliceThread   : public TimeSliceThread
    {
        public:
            SharedTimeSliceThread()
                : TimeSliceThread (String (JucePlugin_Name) + " ARA Sample Reading Thread")
            {
                startThread (7);   // above "default" priority so playback is fluent, but below realtime
            }
    };
    SharedResourcePointer<SharedTimeSliceThread> sharedTimesliceThread;

    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<ARAAudioSource*, std::unique_ptr<AudioFormatReader>> audioSourceReaders;

    // temp buffers to use for summing signals if rendering multiple regions
    std::unique_ptr<AudioBuffer<float>> tempBuffer;

    const bool useBufferedAudioSourceReader;

    bool lastProcessBlockSucceeded { true };

    AudioPlayHead::CurrentPositionInfo lastPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginDemoAudioProcessor)
};
