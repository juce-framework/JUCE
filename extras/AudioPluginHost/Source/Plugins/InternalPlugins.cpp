/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

#include "InternalPlugins.h"
#include "PluginGraph.h"

#define PIP_DEMO_UTILITIES_INCLUDED 1

// An alternative version of createAssetInputStream from the demo utilities header
// that fetches resources from embedded binary data instead of files
static std::unique_ptr<InputStream> createAssetInputStream (const char* resourcePath)
{
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        if (String (BinaryData::originalFilenames[i]) == String (resourcePath))
        {
            int dataSizeInBytes;
            auto* resource = BinaryData::getNamedResource (BinaryData::namedResourceList[i], dataSizeInBytes);
            return std::make_unique<MemoryInputStream> (resource, dataSizeInBytes, false);
        }
    }

    return {};
}

#include "../../../../examples/Plugins/AUv3SynthPluginDemo.h"
#include "../../../../examples/Plugins/ArpeggiatorPluginDemo.h"
#include "../../../../examples/Plugins/AudioPluginDemo.h"
#include "../../../../examples/Plugins/DSPModulePluginDemo.h"
#include "../../../../examples/Plugins/GainPluginDemo.h"
#include "../../../../examples/Plugins/MidiLoggerPluginDemo.h"
#include "../../../../examples/Plugins/MultiOutSynthPluginDemo.h"
#include "../../../../examples/Plugins/NoiseGatePluginDemo.h"
#include "../../../../examples/Plugins/SamplerPluginDemo.h"
#include "../../../../examples/Plugins/SurroundPluginDemo.h"

//==============================================================================
class InternalPlugin   : public AudioPluginInstance
{
public:
    explicit InternalPlugin (std::unique_ptr<AudioProcessor> innerIn)
        : inner (std::move (innerIn))
    {
        jassert (inner != nullptr);

        for (auto isInput : { true, false })
            matchChannels (isInput);

        setBusesLayout (inner->getBusesLayout());
    }

    //==============================================================================
    const String getName() const override                                         { return inner->getName(); }
    StringArray getAlternateDisplayNames() const override                         { return inner->getAlternateDisplayNames(); }
    double getTailLengthSeconds() const override                                  { return inner->getTailLengthSeconds(); }
    bool acceptsMidi() const override                                             { return inner->acceptsMidi(); }
    bool producesMidi() const override                                            { return inner->producesMidi(); }
    AudioProcessorEditor* createEditor() override                                 { return inner->createEditor(); }
    bool hasEditor() const override                                               { return inner->hasEditor(); }
    int getNumPrograms() override                                                 { return inner->getNumPrograms(); }
    int getCurrentProgram() override                                              { return inner->getCurrentProgram(); }
    void setCurrentProgram (int i) override                                       { inner->setCurrentProgram (i); }
    const String getProgramName (int i) override                                  { return inner->getProgramName (i); }
    void changeProgramName (int i, const String& n) override                      { inner->changeProgramName (i, n); }
    void getStateInformation (juce::MemoryBlock& b) override                      { inner->getStateInformation (b); }
    void setStateInformation (const void* d, int s) override                      { inner->setStateInformation (d, s); }
    void getCurrentProgramStateInformation (juce::MemoryBlock& b) override        { inner->getCurrentProgramStateInformation (b); }
    void setCurrentProgramStateInformation (const void* d, int s) override        { inner->setCurrentProgramStateInformation (d, s); }
    void prepareToPlay (double sr, int bs) override                               { inner->setRateAndBufferSizeDetails (sr, bs); inner->prepareToPlay (sr, bs); }
    void releaseResources() override                                              { inner->releaseResources(); }
    void memoryWarningReceived() override                                         { inner->memoryWarningReceived(); }
    void processBlock (AudioBuffer<float>& a, MidiBuffer& m) override             { inner->processBlock (a, m); }
    void processBlock (AudioBuffer<double>& a, MidiBuffer& m) override            { inner->processBlock (a, m); }
    void processBlockBypassed (AudioBuffer<float>& a, MidiBuffer& m) override     { inner->processBlockBypassed (a, m); }
    void processBlockBypassed (AudioBuffer<double>& a, MidiBuffer& m) override    { inner->processBlockBypassed (a, m); }
    bool supportsDoublePrecisionProcessing() const override                       { return inner->supportsDoublePrecisionProcessing(); }
    bool supportsMPE() const override                                             { return inner->supportsMPE(); }
    bool isMidiEffect() const override                                            { return inner->isMidiEffect(); }
    void reset() override                                                         { inner->reset(); }
    void setNonRealtime (bool b) noexcept override                                { inner->setNonRealtime (b); }
    void refreshParameterList() override                                          { inner->refreshParameterList(); }
    void numChannelsChanged() override                                            { inner->numChannelsChanged(); }
    void numBusesChanged() override                                               { inner->numBusesChanged(); }
    void processorLayoutsChanged() override                                       { inner->processorLayoutsChanged(); }
    void setPlayHead (AudioPlayHead* p) override                                  { inner->setPlayHead (p); }
    void updateTrackProperties (const TrackProperties& p) override                { inner->updateTrackProperties (p); }
    bool isBusesLayoutSupported (const BusesLayout& layout) const override        { return inner->checkBusesLayoutSupported (layout); }
    bool applyBusLayouts (const BusesLayout& layouts) override                    { return inner->setBusesLayout (layouts) && AudioPluginInstance::applyBusLayouts (layouts); }

    bool canAddBus (bool) const override                                          { return true; }
    bool canRemoveBus (bool) const override                                       { return true; }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& description) const override
    {
        description = getPluginDescription (*inner);
    }

private:
    static PluginDescription getPluginDescription (const AudioProcessor& proc)
    {
        const auto ins                  = proc.getTotalNumInputChannels();
        const auto outs                 = proc.getTotalNumOutputChannels();
        const auto identifier           = proc.getName();
        const auto registerAsGenerator  = ins == 0;
        const auto acceptsMidi          = proc.acceptsMidi();

        PluginDescription descr;

        descr.name              = identifier;
        descr.descriptiveName   = identifier;
        descr.pluginFormatName  = InternalPluginFormat::getIdentifier();
        descr.category          = (registerAsGenerator ? (acceptsMidi ? "Synth" : "Generator") : "Effect");
        descr.manufacturerName  = "JUCE";
        descr.version           = ProjectInfo::versionString;
        descr.fileOrIdentifier  = identifier;
        descr.isInstrument      = (acceptsMidi && registerAsGenerator);
        descr.numInputChannels  = ins;
        descr.numOutputChannels = outs;

        descr.uniqueId = descr.deprecatedUid = identifier.hashCode();

        return descr;
    }

    void matchChannels (bool isInput)
    {
        const auto inBuses = inner->getBusCount (isInput);

        while (getBusCount (isInput) < inBuses)
            addBus (isInput);

        while (inBuses < getBusCount (isInput))
            removeBus (isInput);
    }

    std::unique_ptr<AudioProcessor> inner;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InternalPlugin)
};

//==============================================================================
class SineWaveSynth : public AudioProcessor
{
public:
    SineWaveSynth()
        : AudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo()))
    {
        const int numVoices = 8;

        // Add some voices...
        for (int i = numVoices; --i >= 0;)
            synth.addVoice (new SineWaveVoice());

        // ..and give the synth a sound to play
        synth.addSound (new SineWaveSound());
    }

    static String getIdentifier()
    {
        return "Sine Wave Synth";
    }

    //==============================================================================
    void prepareToPlay (double newSampleRate, int) override
    {
        synth.setCurrentPlaybackSampleRate (newSampleRate);
    }

    void releaseResources() override {}

    //==============================================================================
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        const int numSamples = buffer.getNumSamples();

        buffer.clear();
        synth.renderNextBlock (buffer, midiMessages, 0, numSamples);
        buffer.applyGain (0.8f);
    }

    using AudioProcessor::processBlock;

    const String getName() const override                                           { return getIdentifier(); }
    double getTailLengthSeconds() const override                                    { return 0.0; }
    bool acceptsMidi() const override                                               { return true; }
    bool producesMidi() const override                                              { return true; }
    AudioProcessorEditor* createEditor() override                                   { return nullptr; }
    bool hasEditor() const override                                                 { return false; }
    int getNumPrograms() override                                                   { return 1; }
    int getCurrentProgram() override                                                { return 0; }
    void setCurrentProgram (int) override                                           {}
    const String getProgramName (int) override                                      { return {}; }
    void changeProgramName (int, const String&) override                            {}
    void getStateInformation (juce::MemoryBlock&) override                          {}
    void setStateInformation (const void*, int) override                            {}

private:
    //==============================================================================
    struct SineWaveSound  : public SynthesiserSound
    {
        SineWaveSound() = default;

        bool appliesToNote (int /*midiNoteNumber*/) override    { return true; }
        bool appliesToChannel (int /*midiChannel*/) override    { return true; }
    };

    struct SineWaveVoice  : public SynthesiserVoice
    {
        SineWaveVoice() = default;

        bool canPlaySound (SynthesiserSound* sound) override
        {
            return dynamic_cast<SineWaveSound*> (sound) != nullptr;
        }

        void startNote (int midiNoteNumber, float velocity,
                        SynthesiserSound* /*sound*/,
                        int /*currentPitchWheelPosition*/) override
        {
            currentAngle = 0.0;
            level = velocity * 0.15;
            tailOff = 0.0;

            double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
            double cyclesPerSample = cyclesPerSecond / getSampleRate();

            angleDelta = cyclesPerSample * 2.0 * MathConstants<double>::pi;
        }

        void stopNote (float /*velocity*/, bool allowTailOff) override
        {
            if (allowTailOff)
            {
                // start a tail-off by setting this flag. The render callback will pick up on
                // this and do a fade out, calling clearCurrentNote() when it's finished.

                if (approximatelyEqual (tailOff, 0.0)) // we only need to begin a tail-off if it's not already doing so - the
                                                       // stopNote method could be called more than once.
                    tailOff = 1.0;
            }
            else
            {
                // we're being told to stop playing immediately, so reset everything..

                clearCurrentNote();
                angleDelta = 0.0;
            }
        }

        void pitchWheelMoved (int /*newValue*/) override
        {
            // not implemented for the purposes of this demo!
        }

        void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override
        {
            // not implemented for the purposes of this demo!
        }

        void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
        {
            if (! approximatelyEqual (angleDelta, 0.0))
            {
                if (tailOff > 0)
                {
                    while (--numSamples >= 0)
                    {
                        const float currentSample = (float) (sin (currentAngle) * level * tailOff);

                        for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample (i, startSample, currentSample);

                        currentAngle += angleDelta;
                        ++startSample;

                        tailOff *= 0.99;

                        if (tailOff <= 0.005)
                        {
                            // tells the synth that this voice has stopped
                            clearCurrentNote();

                            angleDelta = 0.0;
                            break;
                        }
                    }
                }
                else
                {
                    while (--numSamples >= 0)
                    {
                        const float currentSample = (float) (sin (currentAngle) * level);

                        for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample (i, startSample, currentSample);

                        currentAngle += angleDelta;
                        ++startSample;
                    }
                }
            }
        }

        using SynthesiserVoice::renderNextBlock;

    private:
        double currentAngle = 0, angleDelta = 0, level = 0, tailOff = 0;
    };

    //==============================================================================
    Synthesiser synth;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineWaveSynth)
};

//==============================================================================
class ReverbPlugin : public AudioProcessor
{
public:
    ReverbPlugin()
        : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo())
                                           .withOutput ("Output", AudioChannelSet::stereo()))
    {}

    static String getIdentifier()
    {
        return "Reverb";
    }

    void prepareToPlay (double newSampleRate, int) override
    {
        reverb.setSampleRate (newSampleRate);
    }

    void reset() override
    {
        reverb.reset();
    }

    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        auto numChannels = buffer.getNumChannels();

        if (numChannels == 1)
            reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
        else
            reverb.processStereo (buffer.getWritePointer (0),
                                  buffer.getWritePointer (1),
                                  buffer.getNumSamples());

        for (int ch = 2; ch < numChannels; ++ch)
            buffer.clear (ch, 0, buffer.getNumSamples());
    }

    using AudioProcessor::processBlock;

    const String getName() const override                                           { return getIdentifier(); }
    double getTailLengthSeconds() const override                                    { return 0.0; }
    bool acceptsMidi() const override                                               { return false; }
    bool producesMidi() const override                                              { return false; }
    AudioProcessorEditor* createEditor() override                                   { return nullptr; }
    bool hasEditor() const override                                                 { return false; }
    int getNumPrograms() override                                                   { return 1; }
    int getCurrentProgram() override                                                { return 0; }
    void setCurrentProgram (int) override                                           {}
    const String getProgramName (int) override                                      { return {}; }
    void changeProgramName (int, const String&) override                            {}
    void getStateInformation (juce::MemoryBlock&) override                          {}
    void setStateInformation (const void*, int) override                            {}

private:
    Reverb reverb;
};

//==============================================================================

InternalPluginFormat::InternalPluginFactory::InternalPluginFactory (const std::initializer_list<Constructor>& constructorsIn)
    : constructors (constructorsIn),
      descriptions ([&]
      {
          std::vector<PluginDescription> result;

          for (const auto& constructor : constructors)
              result.push_back (constructor()->getPluginDescription());

          return result;
      }())
{}

std::unique_ptr<AudioPluginInstance> InternalPluginFormat::InternalPluginFactory::createInstance (const String& name) const
{
    const auto begin = descriptions.begin();
    const auto it = std::find_if (begin,
                                  descriptions.end(),
                                  [&] (const PluginDescription& desc) { return name.equalsIgnoreCase (desc.name); });

    if (it == descriptions.end())
        return nullptr;

    const auto index = (size_t) std::distance (begin, it);
    return constructors[index]();
}

InternalPluginFormat::InternalPluginFormat()
    : factory {
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode); },
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode); },
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode); },
        [] { return std::make_unique<AudioProcessorGraph::AudioGraphIOProcessor> (AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode); },

        [] { return std::make_unique<InternalPlugin> (std::make_unique<SineWaveSynth>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<ReverbPlugin>()); },

        [] { return std::make_unique<InternalPlugin> (std::make_unique<AUv3SynthProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<Arpeggiator>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<DspModulePluginDemoAudioProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<GainProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<JuceDemoPluginAudioProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<MidiLoggerPluginDemoProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<MultiOutSynth>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<NoiseGate>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<SamplerAudioProcessor>()); },
        [] { return std::make_unique<InternalPlugin> (std::make_unique<SurroundProcessor>()); }
    }
{
}

std::unique_ptr<AudioPluginInstance> InternalPluginFormat::createInstance (const String& name)
{
    return factory.createInstance (name);
}

void InternalPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                 double /*initialSampleRate*/, int /*initialBufferSize*/,
                                                 PluginCreationCallback callback)
{
    if (auto p = createInstance (desc.name))
        callback (std::move (p), {});
    else
        callback (nullptr, NEEDS_TRANS ("Invalid internal plugin name"));
}

bool InternalPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const
{
    return false;
}

const std::vector<PluginDescription>& InternalPluginFormat::getAllTypes() const
{
    return factory.getDescriptions();
}
