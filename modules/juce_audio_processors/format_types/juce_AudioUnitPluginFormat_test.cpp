/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include "juce_AU_Shared.h"

namespace juce
{

class AudioUnitPluginFormatTests final : public UnitTest
{
public:
    AudioUnitPluginFormatTests()
        : UnitTest ("AU Hosting", UnitTestCategories::audioProcessors)
    {
    }

    void runTest() override
    {
        beginTest ("Permissive audio processor produces layout [-1, -2]");
        {
            struct ProcA : public MockAudioProcessor
            {
                ProcA()
                    : MockAudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::stereo())
                                                           .withOutput ("Output", AudioChannelSet::stereo())) {}

                bool isBusesLayoutSupported (const BusesLayout&) const override
                {
                    return true;
                }
            };

            ProcA processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { -1, -2 } });
        }

        beginTest ("Audio processor with matched I/O produces layout [-1, -1]");
        {
            struct ProcB : public MockAudioProcessor
            {
                ProcB()
                    : MockAudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::stereo())
                                                           .withOutput ("Output", AudioChannelSet::stereo())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    return l.getMainInputChannelSet() == l.getMainOutputChannelSet();
                }
            };

            ProcB processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { -1, -1 } });
        }

        beginTest ("Audio processor that supports any input with a two-channel output produces layout [-1, 2]");
        {
            struct ProcC : public MockAudioProcessor
            {
                ProcC()
                    : MockAudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::stereo())
                                                           .withOutput ("Output", AudioChannelSet::stereo())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    return l.getMainOutputChannelSet() == AudioChannelSet::stereo();
                }
            };

            ProcC processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { -1, 2 } });
        }

        beginTest ("Audio processor that supports any output with a 6-channel input produces layout [6, -1]");
        {
            struct ProcD : public MockAudioProcessor
            {
                ProcD()
                    : MockAudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::create5point1())
                                                           .withOutput ("Output", AudioChannelSet::create5point1())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    return l.getMainInputChannelSet() == AudioChannelSet::create5point1();
                }
            };

            ProcD processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { 6, -1 } });
        }

        beginTest ("Audio processor that supports both above layouts produces [-1, 2] and [6, -1]");
        {
            struct ProcE : public MockAudioProcessor
            {
                ProcE()
                    : MockAudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::create5point1())
                                                           .withOutput ("Output", AudioChannelSet::stereo())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    return l.getMainOutputChannelSet() == AudioChannelSet::stereo()
                        || l.getMainInputChannelSet() == AudioChannelSet::create5point1();
                }
            };

            ProcE processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { -1, 2 },
                                          AudioUnitHelpers::Channels { 6, -1 } });
        }

        beginTest ("Audio processor that supports only stereo and 5.1 produces [2, 2], [6, 6], [2, 6], and [6, 2]");
        {
            struct ProcF : public MockAudioProcessor
            {
                ProcF()
                    : MockAudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::create5point1())
                                                           .withOutput ("Output", AudioChannelSet::stereo())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    const std::array supported { AudioChannelSet::stereo(), AudioChannelSet::create5point1() };

                    return std::find (supported.begin(), supported.end(), l.getMainInputChannelSet()) != supported.end()
                        && std::find (supported.begin(), supported.end(), l.getMainOutputChannelSet()) != supported.end();
                }
            };

            ProcF processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { 2, 2 },
                                          AudioUnitHelpers::Channels { 2, 6 },
                                          AudioUnitHelpers::Channels { 6, 2 },
                                          AudioUnitHelpers::Channels { 6, 6 } });
        }

        beginTest ("Complex layout is supported");
        {
            struct ProcG : public MockAudioProcessor
            {
                ProcG()
                    : MockAudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::create9point1point6())
                                                           .withOutput ("Output", AudioChannelSet::create9point1point6())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    using ACS = juce::AudioChannelSet;

                    const auto input = l.getMainInputChannelSet();
                    const auto output = l.getMainOutputChannelSet();

                    if (output == ACS::mono())
                        return input == ACS::mono();

                    if (output == ACS::stereo())
                        return input == ACS::mono() || input == ACS::stereo();

                    if (output == ACS::create9point1point6())
                    {
                        return input == ACS::mono()
                            || input == ACS::stereo()
                            || input == ACS::createLCR()
                            || input == ACS::quadraphonic()
                            || input == ACS::create5point0()
                            || input == ACS::create5point1()
                            || input == ACS::create7point0()
                            || input == ACS::create7point1()
                            || input == ACS::create7point0point2()
                            || input == ACS::create5point1point4()
                            || input == ACS::create7point0point4()
                            || input == ACS::create7point1point4()
                            || input == ACS::create7point0point6()
                            || input == ACS::create7point1point6()
                            || input == ACS::create9point0point6()
                            || input == ACS::create9point1point6();
                    }

                    return false;
                }
            };

            ProcG processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { 1, 1 },
                                          AudioUnitHelpers::Channels { 1, 2 },
                                          AudioUnitHelpers::Channels { 2, 2 },
                                          AudioUnitHelpers::Channels { 1, 16 },
                                          AudioUnitHelpers::Channels { 2, 16 },
                                          AudioUnitHelpers::Channels { 3, 16 },
                                          AudioUnitHelpers::Channels { 4, 16 },
                                          AudioUnitHelpers::Channels { 5, 16 },
                                          AudioUnitHelpers::Channels { 6, 16 },
                                          AudioUnitHelpers::Channels { 7, 16 },
                                          AudioUnitHelpers::Channels { 8, 16 },
                                          AudioUnitHelpers::Channels { 9, 16 },
                                          AudioUnitHelpers::Channels { 10, 16 },
                                          AudioUnitHelpers::Channels { 11, 16 },
                                          AudioUnitHelpers::Channels { 12, 16 },
                                          AudioUnitHelpers::Channels { 13, 16 },
                                          AudioUnitHelpers::Channels { 14, 16 },
                                          AudioUnitHelpers::Channels { 15, 16 },
                                          AudioUnitHelpers::Channels { 16, 16 } });
        }

        beginTest ("Audio processor that supports only stereo out reports [0, 2]");
        {
            struct ProcH : public MockAudioProcessor
            {
                ProcH()
                    : MockAudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    return l.inputBuses.isEmpty() && l.getMainOutputChannelSet() == AudioChannelSet::stereo();
                }
            };

            ProcH processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { 0, 2 } });
        }

        beginTest ("Audio processor that supports any out but no in reports [0, -1]");
        {
            struct ProcI : public MockAudioProcessor
            {
                ProcI()
                    : MockAudioProcessor (BusesProperties().withOutput ("Output", AudioChannelSet::stereo())) {}

                bool isBusesLayoutSupported (const BusesLayout& l) const override
                {
                    return l.inputBuses.isEmpty();
                }
            };

            ProcI processor;
            const auto layouts = AudioUnitHelpers::getAUChannelInfo (processor);
            expect (layouts == std::set { AudioUnitHelpers::Channels { 0, -1 } });
        }
    }

private:
    struct MockAudioProcessor : public AudioProcessor
    {
        using AudioProcessor::AudioProcessor;

        const String getName() const override                         { return "Basic Processor"; }
        double getTailLengthSeconds() const override                  { return {}; }
        bool acceptsMidi() const override                             { return {}; }
        bool producesMidi() const override                            { return {}; }
        AudioProcessorEditor* createEditor() override                 { return {}; }
        bool hasEditor() const override                               { return {}; }
        int getNumPrograms() override                                 { return 1; }
        int getCurrentProgram() override                              { return {}; }
        void setCurrentProgram (int) override                         {}
        const String getProgramName (int) override                    { return {}; }
        void changeProgramName (int, const String&) override          {}
        void getStateInformation (MemoryBlock&) override              {}
        void setStateInformation (const void*, int) override          {}
        void prepareToPlay (double, int) override                     {}
        void releaseResources() override                              {}
        bool supportsDoublePrecisionProcessing() const override       { return {}; }
        bool isMidiEffect() const override                            { return {}; }
        void reset() override                                         {}
        void setNonRealtime (bool) noexcept override                  {}
        void processBlock (AudioBuffer<float>&, MidiBuffer&) override {}
        using AudioProcessor::processBlock;
    };
};

static AudioUnitPluginFormatTests auPluginFormatTests;

} // namespace juce
