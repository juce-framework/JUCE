/*
 ==============================================================================

 Spatializer.cpp
 Created: 23 Nov 2015 3:08:33pm
 Author:  Fabian Renn

 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../GenericEditor.h"

//==============================================================================
/**
 */
class Spatializer  : public AudioProcessor
{
public:

    struct SpeakerPosition
    {
        float radius, phi;
    };

    struct SpeakerLayout
    {
        AudioChannelSet set;
        Array<SpeakerPosition> positions;
    };

    // this needs at least c++11
    static Array<SpeakerLayout> speakerPositions;

    //==============================================================================
    Spatializer() : currentSpeakerLayout (0)
    {
        // clear the default bus arrangements which were created by the base class
        busArrangement.inputBuses .clear();
        busArrangement.outputBuses.clear();

        // add mono in and default out
        busArrangement.inputBuses .add (AudioProcessorBus ("Input",  AudioChannelSet::mono()));
        busArrangement.outputBuses.add (AudioProcessorBus ("Output", speakerPositions[currentSpeakerLayout].set));

        addParameter (radius = new AudioParameterFloat ("radius", "Radius", 0.0f, 1.0f, 0.5f));
        addParameter (phi    = new AudioParameterFloat ("phi",    "Phi",    0.0f, 1.0f, 0.0f));
    }

    ~Spatializer() {}

    //==============================================================================
    bool setPreferredBusArrangement (bool isInputBus, int busIndex,
                                     const AudioChannelSet& preferred) override
    {
        // we only allow mono in
        if (isInputBus && preferred != AudioChannelSet::mono())
            return false;

        // the output must be one of the supported speaker layouts
        if (! isInputBus)
        {
            int i;

            for (i = 0; i < speakerPositions.size(); ++i)
                if (speakerPositions[i].set == preferred) break;

            if (i >= speakerPositions.size())
                return false;

            currentSpeakerLayout = i;
        }

        return AudioProcessor::setPreferredBusArrangement (isInputBus, busIndex, preferred);
    }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        scratchBuffer.setSize (1, samplesPerBlock);
    }

    void releaseResources() override {}

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer&) override
    {
        // copy the input into a scratch buffer
        AudioSampleBuffer scratch (scratchBuffer.getArrayOfWritePointers(), 1, buffer.getNumSamples());
        scratch.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());

        const Array<SpeakerPosition>& positions = speakerPositions.getReference (currentSpeakerLayout).positions;
        const float* inputBuffer  = scratch.getReadPointer (0);
        const float kMaxDistanceGain = -20.0f;

        for (int speakerIdx = 0; speakerIdx < positions.size(); ++speakerIdx)
        {
            const SpeakerPosition& speakerPos = positions.getReference (speakerIdx);
            float fltDistance = distance (polarToCartesian (speakerPos.radius, speakerPos.phi), polarToCartesian (*radius, (*phi) * 2.0f * float_Pi));
            float gainInDb = kMaxDistanceGain * (fltDistance / 2.0f);
            float gain = std::pow (10.0f, (gainInDb / 20.0f));

            busArrangement.getBusBuffer(buffer, false, 0).copyFrom(speakerIdx, 0, inputBuffer, buffer.getNumSamples(), gain);
        }
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override { return new GenericEditor (*this); }
    bool hasEditor() const override               { return true;   }

    //==============================================================================
    const String getName() const override               { return "Gain PlugIn"; }

    bool acceptsMidi() const override                   { return false; }
    bool producesMidi() const override                  { return false; }
    bool silenceInProducesSilenceOut() const override   { return true; }
    double getTailLengthSeconds() const override        { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return String(); }
    void changeProgramName (int , const String& ) override { }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream stream (destData, true);

        stream.writeFloat (*radius);
        stream.writeFloat (*phi);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        MemoryInputStream stream (data, sizeInBytes, false);

        radius->setValueNotifyingHost (stream.readFloat());
        phi->setValueNotifyingHost (stream.readFloat());
    }

private:
    //==============================================================================
    AudioParameterFloat* radius;
    AudioParameterFloat* phi;
    int currentSpeakerLayout;
    AudioSampleBuffer scratchBuffer;

    static Point<float> polarToCartesian (float r, float phi) noexcept
    {
        return Point<float> (r * std::cos (phi), r * std::sin (phi));
    }

    static float distance (Point<float> a, Point<float> b) noexcept
    {
        return std::sqrt (std::pow (a.x - b.x, 2.0f) + std::pow (a.y - b.y, 2.0f));
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spatializer)
};

// this needs at least c++11
Array<Spatializer::SpeakerLayout> Spatializer::speakerPositions =
{
    Spatializer::SpeakerLayout { AudioChannelSet::stereo(),         { SpeakerPosition { 1.0f, -0.25f * float_Pi }, SpeakerPosition { 1.0f, 0.25f * float_Pi } }},
    Spatializer::SpeakerLayout { AudioChannelSet::quadraphonic(),   { SpeakerPosition { 1.0f, -0.25f * float_Pi }, SpeakerPosition { 1.0f, 0.25f * float_Pi }, SpeakerPosition {1.0f, -0.75f * float_Pi}, SpeakerPosition {1.0f, 0.75f * float_Pi}}},
    Spatializer::SpeakerLayout { AudioChannelSet::create5point0(),  {SpeakerPosition { 1.0f, 0.0f}, SpeakerPosition { 1.0f, -0.25f * float_Pi }, SpeakerPosition { 1.0f, 0.25f * float_Pi }, SpeakerPosition {1.0f, -0.75f * float_Pi}, SpeakerPosition {1.0f, 0.75f * float_Pi}}}
};


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Spatializer();
}
