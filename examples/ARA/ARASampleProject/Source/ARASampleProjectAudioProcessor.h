#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <map>

#if ! JucePlugin_Enable_ARA
    #error "bad project configuration, JucePlugin_Enable_ARA is required for compiling this class"
#endif

//==============================================================================
/**
    Processor class for ARA sample project
    In this simple demo we're using a buffered ARA sample reader to pull audio samples
    from the host and render them without any modifications, effectively making this
    an ARA enabled pass-through renderer.
*/
class ARASampleProjectAudioProcessor    : public AudioProcessor,
                                          public AudioProcessorARAExtension
{
public:
    //==============================================================================
    ARASampleProjectAudioProcessor();
    ~ARASampleProjectAudioProcessor();

    /** Additional configuration for the AudioProcessor if used for internal rendering (waveform display).
        Like bus layout or other crucial renderer configuration, this may not be changed between
        prepareToPlay() and releaseResources().
        If \p isAlwaysNonRealtime is true, the plug-in skips the internal multi-threaded buffering
        of any non-realtime ressources needed for rendering, such as audio source samples.
    */
    void setAlwaysNonRealtime (bool isAlwaysNonRealtime) noexcept   { alwaysNonRealtime = isAlwaysNonRealtime; }
    bool isAlwaysNonRealtime() const noexcept                       { return alwaysNonRealtime; }

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
    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<ARAAudioSource*, std::unique_ptr<AudioFormatReader>> audioSourceReaders;

    // temp buffers to use for summing signals if rendering multiple regions
    std::unique_ptr<AudioBuffer<float>> tempBuffer;

    bool alwaysNonRealtime { false };

    bool lastProcessBlockSucceeded { true };

    AudioPlayHead::CurrentPositionInfo lastPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessor)
};
