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
namespace dsp
{

class AudioBlockUnitTests   : public UnitTest
{
public:
    AudioBlockUnitTests()
        : UnitTest ("AudioBlock", UnitTestCategories::dsp)
    {}

    void runTest() override
    {
        beginTest ("Equality");
        {
            expect (block == block);
            expect (block != otherBlock);
        }

        beginTest ("Constructors");
        {
            expect (block == AudioBlock<float> (data.getArrayOfWritePointers(), (size_t) data.getNumChannels(), (size_t) data.getNumSamples()));
            expect (block == AudioBlock<float> (data.getArrayOfWritePointers(), (size_t) data.getNumChannels(), (size_t) 0, (size_t) data.getNumSamples()));
            expect (block == AudioBlock<float> (block));

            expect (block == AudioBlock<const float> (data.getArrayOfWritePointers(), (size_t) data.getNumChannels(), (size_t) data.getNumSamples()));
            expect (block == AudioBlock<const float> (data.getArrayOfWritePointers(), (size_t) data.getNumChannels(), (size_t) 0, (size_t) data.getNumSamples()));
            expect (block == AudioBlock<const float> (block));
        }

        beginTest ("Swap");
        {
            resetBlocks();

            expect (block != otherBlock);
            expectEquals (block.getSample (0, 0), 1.0f);
            expectEquals (block.getSample (0, 4), 5.0f);
            expectEquals (otherBlock.getSample (0, 0), -1.0f);
            expectEquals (otherBlock.getSample (0, 3), -4.0f);

            block.swap (otherBlock);

            expect (block != otherBlock);
            expectEquals (otherBlock.getSample (0, 0), 1.0f);
            expectEquals (otherBlock.getSample (0, 4), 5.0f);
            expectEquals (block.getSample (0, 0), -1.0f);
            expectEquals (block.getSample (0, 3), -4.0f);
        }

        beginTest ("Getters and setters");
        {
            resetBlocks();

            expectEquals ((int) block.getNumChannels(), data.getNumChannels());
            expectEquals ((int) block.getNumSamples(),  data.getNumSamples());

            expectEquals (block.getChannelPointer (0)[2], 3.0f);
            block.getChannelPointer (0)[2] = 999.0f;
            expectEquals (block.getChannelPointer (0)[2], 999.0f);

            expectEquals (block.getSample (0, 4), 5.0f);
            expectEquals (block.getSample (1, 4), 11.0f);

            expectEquals (block.getSingleChannelBlock (1).getSample (0, 3), block.getSample (1, 3));

            expectEquals (block.getSubsetChannelBlock (0, 2).getSample (1, 3), block.getSample (1, 3));
            expectEquals (block.getSubsetChannelBlock (1, 1).getSample (0, 3), block.getSample (1, 3));

            block.setSample (1, 1, 777.0f);
            expectEquals (block.getSample (1, 1), 777.0f);

            block.addSample (1, 1, 1.0f);
            expectEquals (block.getSample (1, 1), 778.0f);
        }

        beginTest ("Copying");
        {
            block.clear();
            expectEquals (block.getSample (0, 2), 0.0f);
            expectEquals (block.getSample (1, 4), 0.0f);

            block.fill (456.0f);
            expectEquals (block.getSample (0, 2), 456.0f);
            expectEquals (block.getSample (1, 4), 456.0f);

            block.copyFrom (otherBlock);
            expect (block != otherBlock);
            expectEquals (block.getSample (0, 2), otherBlock.getSample (0, 2));
            expectEquals (block.getSample (1, 4), otherBlock.getSample (1, 4));

            resetBlocks();

            AudioBuffer<float> otherBuffer ((int) block.getNumChannels(), (int) block.getNumSamples());
            otherBlock.copyTo (otherBuffer);
            expectEquals (otherBlock.getSample (0, 2), otherBuffer.getSample (0, 2));
            expectEquals (otherBlock.getSample (1, 4), otherBuffer.getSample (1, 4));

            block.copyFrom (otherBuffer);
            expectEquals (block.getSample (0, 2), otherBlock.getSample (0, 2));
            expectEquals (block.getSample (1, 4), otherBlock.getSample (1, 4));

            float testSample1 = block.getSample (0, 2);
            float testSample2 = block.getSample (1, 3);
            expect (testSample1 != block.getSample (0, 4));
            expect (testSample2 != block.getSample (1, 5));
            block.move (0, 2);
            expectEquals (block.getSample (0, 4), testSample1);
            expectEquals (block.getSample (1, 5), testSample2);
        }

        beginTest ("Addition");
        {
            resetBlocks();

            block.add (15.0f);
            expectEquals (block.getSample (0, 4), 20.0f);
            expectEquals (block.getSample (1, 4), 26.0f);

            block.add (otherBlock);
            expectEquals (block.getSample (0, 4), 15.0f);
            expectEquals (block.getSample (1, 4), 15.0f);

            block.replaceWithSumOf (otherBlock, 9.0f);
            expectEquals (block.getSample (0, 4), 4.0f);
            expectEquals (block.getSample (1, 4), -2.0f);

            resetBlocks();

            block.replaceWithSumOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), 0.0f);
            expectEquals (block.getSample (1, 4), 0.0f);
        }

        beginTest ("Subtraction");
        {
            resetBlocks();

            block.subtract (15.0f);
            expectEquals (block.getSample (0, 4), -10.0f);
            expectEquals (block.getSample (1, 4), -4.0f);

            block.subtract (otherBlock);
            expectEquals (block.getSample (0, 4), -5.0f);
            expectEquals (block.getSample (1, 4), 7.0f);

            block.replaceWithDifferenceOf (otherBlock, 9.0f);
            expectEquals (block.getSample (0, 4), -14.0f);
            expectEquals (block.getSample (1, 4), -20.0f);

            resetBlocks();

            block.replaceWithDifferenceOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), 10.0f);
            expectEquals (block.getSample (1, 4), 22.0f);
        }

        beginTest ("Multiplication");
        {
            resetBlocks();

            block.multiplyBy (10.0f);
            expectEquals (block.getSample (0, 4), 50.0f);
            expectEquals (block.getSample (1, 4), 110.0f);

            block.multiplyBy (otherBlock);
            expectEquals (block.getSample (0, 4), -250.0f);
            expectEquals (block.getSample (1, 4), -1210.0f);

            block.replaceWithProductOf (otherBlock, 3.0f);
            expectEquals (block.getSample (0, 4), -15.0f);
            expectEquals (block.getSample (1, 4), -33.0f);

            resetBlocks();

            block.replaceWithProductOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), -25.0f);
            expectEquals (block.getSample (1, 4), -121.0f);
        }

        beginTest ("Smoothing");
        {
            block.fill (1.0f);
            SmoothedValue<float> sv { 1.0f };
            sv.reset (1, 4);
            sv.setTargetValue (0.0f);

            block.multiplyBy (sv);
            expect (block.getSample (0, 2) < 1.0f);
            expect (block.getSample (1, 2) < 1.0f);
            expect (block.getSample (0, 2) > 0.0f);
            expect (block.getSample (1, 2) > 0.0f);
            expectEquals (block.getSample (0, 5), 0.0f);
            expectEquals (block.getSample (1, 5), 0.0f);

            sv.setCurrentAndTargetValue (-1.0f);
            sv.setTargetValue (0.0f);
            otherBlock.fill (-1.0f);
            block.replaceWithProductOf (otherBlock, sv);
            expect (block.getSample (0, 2) < 1.0f);
            expect (block.getSample (1, 2) < 1.0f);
            expect (block.getSample (0, 2) > 0.0f);
            expect (block.getSample (1, 2) > 0.0f);
            expectEquals (block.getSample (0, 5), 0.0f);
            expectEquals (block.getSample (1, 5), 0.0f);
        }

        beginTest ("Multiply add");
        {
            resetBlocks();

            block.addProductOf (otherBlock, -1.0f);
            expectEquals (block.getSample (0, 4), 10.0f);
            expectEquals (block.getSample (1, 4), 22.0f);

            block.addProductOf (otherBlock, otherBlock);
            expectEquals (block.getSample (0, 4), 35.0f);
            expectEquals (block.getSample (1, 4), 143.0f);
        }

        beginTest ("Negative abs min max");
        {
            resetBlocks();
            otherBlock.negate();

            block.add (otherBlock);
            expectEquals (block.getSample (0, 4), 10.0f);
            expectEquals (block.getSample (1, 4), 22.0f);

            block.replaceWithNegativeOf (otherBlock);
            expectEquals (block.getSample (0, 4), -5.0f);
            expectEquals (block.getSample (1, 4), -11.0f);

            block.clear();
            otherBlock.negate();
            block.replaceWithAbsoluteValueOf (otherBlock);
            expectEquals (block.getSample (0, 4), 5.0f);
            expectEquals (block.getSample (1, 4), 11.0f);

            resetBlocks();
            block.replaceWithMinOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), -5.0f);
            expectEquals (block.getSample (1, 4), -11.0f);

            resetBlocks();
            block.replaceWithMaxOf (block, otherBlock);
            expectEquals (block.getSample (0, 4), 5.0f);
            expectEquals (block.getSample (1, 4), 11.0f);

            resetBlocks();
            auto range = block.findMinAndMax();
            expectEquals (range.getStart(), 1.0f);
            expectEquals (range.getEnd(), 12.0f);
        }

        beginTest ("Operators");
        {
            resetBlocks();
            block += 10.0f;
            expectEquals (block.getSample (0, 4), 15.0f);
            expectEquals (block.getSample (1, 4), 21.0f);
            block += otherBlock;
            expectEquals (block.getSample (0, 4), 10.0f);
            expectEquals (block.getSample (1, 4), 10.0f);

            resetBlocks();
            block -= 10.0f;
            expectEquals (block.getSample (0, 4), -5.0f);
            expectEquals (block.getSample (1, 4), 1.0f);
            block -= otherBlock;
            expectEquals (block.getSample (0, 4), 0.0f);
            expectEquals (block.getSample (1, 4), 12.0f);

            resetBlocks();
            block *= 10.0f;
            expectEquals (block.getSample (0, 4), 50.0f);
            expectEquals (block.getSample (1, 4), 110.0f);
            block *= otherBlock;
            expectEquals (block.getSample (0, 4), -250.0f);
            expectEquals (block.getSample (1, 4), -1210.0f);
        }

        beginTest ("Process");
        {
            resetBlocks();
            AudioBlock<float>::process (block, otherBlock, [](float x) { return x + 1.0f; });
            expectEquals (otherBlock.getSample (0, 4), 6.0f);
            expectEquals (otherBlock.getSample (1, 4), 12.0f);
        }
    }

private:
    AudioBuffer<float> data { 2, 6 }, otherData { 2, 6 };
    AudioBlock<float> block { data }, otherBlock { otherData };

    void resetBlocks()
    {
        auto value = 1.0f;

        for (size_t c = 0; c < block.getNumChannels(); ++c)
        {
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                block.setSample ((int) c, (int) i, value);
                value += 1.0f;
            }
        }

        otherBlock.replaceWithNegativeOf (block);
    }
};

static AudioBlockUnitTests audioBlockUnitTests;

} // namespace dsp
} // namespace juce
