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

namespace juce::dsp
{

class ProcessorChainTest final : public UnitTest
{
    template <int AddValue>
    struct MockProcessor
    {
        void prepare (const ProcessSpec&) noexcept { isPrepared = true; }
        void reset() noexcept { isReset = true; }

        template <typename Context>
        void process (const Context& context) noexcept
        {
            bufferWasClear = approximatelyEqual (context.getInputBlock().getSample (0, 0), 0.0f);

            if (! context.isBypassed)
                context.getOutputBlock().add (AddValue);
        }

        bool isPrepared     = false;
        bool isReset        = false;
        bool bufferWasClear = false;
    };

public:
    ProcessorChainTest()
        : UnitTest ("ProcessorChain", UnitTestCategories::dsp) {}

    void runTest() override
    {
        beginTest ("After calling setBypass, processor is bypassed");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            setBypassed<0> (chain, true);
            expect (isBypassed<0> (chain));
            setBypassed<0> (chain, false);
            expect (! isBypassed<0> (chain));

            setBypassed<1> (chain, true);
            expect (isBypassed<1> (chain));
            setBypassed<1> (chain, false);
            expect (! isBypassed<1> (chain));
        }

        beginTest ("After calling prepare, all processors are prepared");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            expect (! get<0> (chain).isPrepared);
            expect (! get<1> (chain).isPrepared);

            chain.prepare (ProcessSpec{});

            expect (get<0> (chain).isPrepared);
            expect (get<1> (chain).isPrepared);
        }

        beginTest ("After calling reset, all processors are reset");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            expect (! get<0> (chain).isReset);
            expect (! get<1> (chain).isReset);

            chain.reset();

            expect (get<0> (chain).isReset);
            expect (get<1> (chain).isReset);
        }

        beginTest ("After calling process, all processors contribute to processing");
        {
            ProcessorChain<MockProcessor<1>, MockProcessor<2>> chain;

            AudioBuffer<float> buffer (1, 1);
            AudioBlock<float> block (buffer);
            ProcessContextReplacing<float> context (block);

            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 3.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (! get<1> (chain).bufferWasClear);

            setBypassed<0> (chain, true);
            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 2.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (get<1> (chain).bufferWasClear);

            setBypassed<1> (chain, true);
            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 0.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (get<1> (chain).bufferWasClear);

            setBypassed<0> (chain, false);
            block.clear();
            chain.process (context);
            expectEquals (buffer.getSample (0, 0), 1.0f);
            expect (get<0> (chain).bufferWasClear);
            expect (! get<1> (chain).bufferWasClear);
        }

        beginTest ("Chains with trailing items that only support replacing contexts can be built");
        {
            AudioBuffer<float> inBuf (1, 1), outBuf (1, 1);
            juce::dsp::AudioBlock<float> in (inBuf), out (outBuf);

            struct OnlyReplacing
            {
                void prepare (const juce::dsp::ProcessSpec&) {}
                void process (const juce::dsp::ProcessContextReplacing<float>& c)
                {
                    c.getOutputBlock().multiplyBy (2.0f);
                }
                void reset() {}
            };

            {
                juce::dsp::ProcessorChain<juce::dsp::Gain<float>, OnlyReplacing, OnlyReplacing> c;
                juce::dsp::ProcessContextNonReplacing<float> context (in, out);
                get<0> (c).setGainLinear (1.0f);
                c.prepare (ProcessSpec{});
                inBuf.setSample (0, 0, 1.0f);
                c.process (context);
                expectEquals (outBuf.getSample (0, 0), 4.0f);
            }
        }
    }
};

static ProcessorChainTest processorChainUnitTest;

} // namespace juce::dsp
