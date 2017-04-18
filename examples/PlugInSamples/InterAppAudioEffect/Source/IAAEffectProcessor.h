#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include <array>


// A simple Inter-App Audio plug-in with a gain control and some meters.
class IAAEffectProcessor  : public AudioProcessor
{
public:
    IAAEffectProcessor();
    ~IAAEffectProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
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

    //==============================================================================
    bool updateCurrentTimeInfoFromHost (AudioPlayHead::CurrentPositionInfo&);

    // Allow an IAAAudioProcessorEditor to register as a listener to receive new
    // meter values directly from the audio thread.
    struct MeterListener
    {
        virtual ~MeterListener() {};

        virtual void handleNewMeterValue (int, float) = 0;
    };

    void addMeterListener    (MeterListener& listener) { meterListeners.add    (&listener); };
    void removeMeterListener (MeterListener& listener) { meterListeners.remove (&listener); };


private:
    //==============================================================================
    AudioProcessorValueTreeState parameters;
    float previousGain = 0.0;
    std::array <float, 2> meterValues = { { 0, 0 } };

    // This keeps a copy of the last set of timing info that was acquired during an
    // audio callback - the UI component will display this.
    AudioPlayHead::CurrentPositionInfo lastPosInfo;

    ListenerList<MeterListener> meterListeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IAAEffectProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
