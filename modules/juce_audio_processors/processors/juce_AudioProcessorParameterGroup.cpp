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

namespace juce
{

#if JUCE_UNIT_TESTS

class ParameterGroupTests   : public UnitTest
{
public:
    ParameterGroupTests() : UnitTest ("ParameterGroups", "Parameters") {}

    void runTest() override
    {
        beginTest ("ParameterGroups");

        auto g1 = std::make_unique<AudioProcessorParameterGroup> ("g1", "g1", " - ");

        auto* p1 = new AudioParameterFloat ("p1", "p1", { 0.0f, 2.0f }, 0.5f);
        auto* p2 = new AudioParameterFloat ("p2", "p2", { 0.0f, 2.0f }, 0.5f);
        auto* p3 = new AudioParameterFloat ("p3", "p3", { 0.0f, 2.0f }, 0.5f);

        g1->addChild (std::unique_ptr<AudioParameterFloat> (p1));
        g1->addChild (std::unique_ptr<AudioParameterFloat> (p2),
                      std::unique_ptr<AudioParameterFloat> (p3));

        auto p4 = std::make_unique<AudioParameterFloat> ("p4", "p4", NormalisableRange<float> (0.0f, 2.0f), 0.5f);
        auto p5 = std::make_unique<AudioParameterFloat> ("p5", "p5", NormalisableRange<float> (0.0f, 2.0f), 0.5f);
        auto p6 = std::make_unique<AudioParameterFloat> ("p6", "p6", NormalisableRange<float> (0.0f, 2.0f), 0.5f);

        g1->addChild (std::move (p4));
        g1->addChild (std::move (p5),
                      std::move (p6));

        {
            auto topLevelParams = g1->getParameters (false);
            auto params = g1->getParameters (true);
            expect (topLevelParams == params);
            expectEquals (params.size(), 6);

            expect (params[0] == (AudioProcessorParameter*) p1);
            expect (params[1] == (AudioProcessorParameter*) p2);
            expect (params[2] == (AudioProcessorParameter*) p3);

            expect (dynamic_cast<AudioParameterFloat*> (params[3])->name == "p4");
            expect (dynamic_cast<AudioParameterFloat*> (params[4])->name == "p5");
            expect (dynamic_cast<AudioParameterFloat*> (params[5])->name == "p6");
        }

        auto* p7 = new AudioParameterFloat ("p7", "p7", { 0.0f, 2.0f }, 0.5f);
        auto* p8 = new AudioParameterFloat ("p8", "p8", { 0.0f, 2.0f }, 0.5f);
        auto* p9 = new AudioParameterFloat ("p9", "p9", { 0.0f, 2.0f }, 0.5f);

        auto p10 = std::make_unique<AudioParameterFloat> ("p10", "p10", NormalisableRange<float> (0.0f, 2.0f), 0.5f);
        auto p11 = std::make_unique<AudioParameterFloat> ("p11", "p11", NormalisableRange<float> (0.0f, 2.0f), 0.5f);
        auto p12 = std::make_unique<AudioParameterFloat> ("p12", "p12", NormalisableRange<float> (0.0f, 2.0f), 0.5f);

        auto g2 = std::make_unique<AudioProcessorParameterGroup> ("g2", "g2", " | ", std::unique_ptr<AudioParameterFloat> (p7));
        auto g3 = std::make_unique<AudioProcessorParameterGroup> ("g3", "g3", " | ", std::unique_ptr<AudioParameterFloat> (p8), std::unique_ptr<AudioParameterFloat> (p9));
        auto g4 = std::make_unique<AudioProcessorParameterGroup> ("g4", "g4", " | ", std::move (p10));
        auto g5 = std::make_unique<AudioProcessorParameterGroup> ("g5", "g5", " | ", std::move (p11), std::move (p12));

        g1->addChild (std::move (g2));
        g4->addChild (std::move (g5));
        g1->addChild (std::move (g3), std::move (g4));

        {
            auto topLevelParams = g1->getParameters (false);
            auto params = g1->getParameters (true);
            expectEquals (topLevelParams.size(), 6);
            expectEquals (params.size(), 12);

            expect (params[0] == (AudioProcessorParameter*) p1);
            expect (params[1] == (AudioProcessorParameter*) p2);
            expect (params[2] == (AudioProcessorParameter*) p3);

            expect (dynamic_cast<AudioParameterFloat*> (params[3])->name == "p4");
            expect (dynamic_cast<AudioParameterFloat*> (params[4])->name == "p5");
            expect (dynamic_cast<AudioParameterFloat*> (params[5])->name == "p6");

            expect (params[6] == (AudioProcessorParameter*) p7);
            expect (params[7] == (AudioProcessorParameter*) p8);
            expect (params[8] == (AudioProcessorParameter*) p9);

            expect (dynamic_cast<AudioParameterFloat*> (params[9]) ->name == "p10");
            expect (dynamic_cast<AudioParameterFloat*> (params[10])->name == "p11");
            expect (dynamic_cast<AudioParameterFloat*> (params[11])->name == "p12");
        }

        g1->addChild (std::make_unique<AudioProcessorParameterGroup> ("g6", "g6", " | ",
                                                                      std::make_unique<AudioParameterFloat> ("p11", "p11", NormalisableRange<float> (0.0f, 2.0f), 0.5f),
                                                                      std::make_unique<AudioProcessorParameterGroup> ("g7", "g7", " | ",
                                                                                                                      std::make_unique<AudioParameterFloat> ("p12", "p12", NormalisableRange<float> (0.0f, 2.0f), 0.5f)),
                                                                      std::make_unique<AudioParameterFloat> ("p13", "p13", NormalisableRange<float> (0.0f, 2.0f), 0.5f)));

        TestAudioProcessor processor;

        processor.addParameter (new AudioParameterFloat ("pstart", "pstart", NormalisableRange<float> (0.0f, 2.0f), 0.5f));
        auto groupParams = g1->getParameters (true);
        processor.addParameterGroup (std::move (g1));
        processor.addParameter (new AudioParameterFloat ("pend", "pend", NormalisableRange<float> (0.0f, 2.0f), 0.5f));

        auto& processorParams = processor.getParameters();
        expect (dynamic_cast<AudioParameterFloat*> (processorParams.getFirst())->name == "pstart");
        expect (dynamic_cast<AudioParameterFloat*> (processorParams.getLast()) ->name == "pend");

        auto numParams = processorParams.size();

        for (int i = 1; i < numParams - 1; ++i)
            expect (processorParams[i] == groupParams[i - 1]);

    }
private:
    struct TestAudioProcessor   : public AudioProcessor
    {
        const String getName() const override { return "ap"; }
        void prepareToPlay (double, int) override {}
        void releaseResources() override {}
        void processBlock (AudioBuffer<float>&, MidiBuffer&) override {}
        double getTailLengthSeconds() const override { return 0.0; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        int getNumPrograms() override { return 0; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram (int) override {}
        const String getProgramName (int) override { return {}; }
        void changeProgramName (int, const String&) override {}
        void getStateInformation (MemoryBlock&) override {}
        void setStateInformation (const void*, int) override {}
    };
};

static ParameterGroupTests parameterGroupTests;

#endif

} // namespace juce
