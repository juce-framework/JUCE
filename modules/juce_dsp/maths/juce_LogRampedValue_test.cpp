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

static CommonSmoothedValueTests <LogRampedValue <float>> commonLogRampedValueTests;

class LogRampedValueTests  : public UnitTest
{
public:
    LogRampedValueTests()
        : UnitTest ("LogRampedValueTests", "DSP")
    {}

    void runTest() override
    {
        beginTest ("Curve");
        {
            Array<double> levels = { -0.12243, -1.21245, -12.2342, -22.4683, -30.0, -61.18753 };

            for (auto level : levels)
            {
                Array<Range<double>> ranges = { Range<double> (0.0,    1.0),
                                                Range<double> (-2.345, 0.0),
                                                Range<double> (-2.63,  3.56),
                                                Range<double> (3.3,    -0.2) };

                for (auto range : ranges)
                {
                    LogRampedValue<double> slowStart { range.getStart() } , fastStart { range.getEnd() };

                    auto numSamples = 12;
                    slowStart.reset (numSamples);
                    fastStart.reset (numSamples);

                    slowStart.setLogParameters (level, true);
                    fastStart.setLogParameters (level, false);

                    slowStart.setTargetValue (range.getEnd());
                    fastStart.setTargetValue (range.getStart());

                    AudioBuffer<double> results (2, numSamples + 1);

                    results.setSample (0, 0, slowStart.getCurrentValue());
                    results.setSample (1, 0, fastStart.getCurrentValue());

                    for (int i = 1; i < results.getNumSamples(); ++i)
                    {
                        results.setSample (0, i, slowStart.getNextValue());
                        results.setSample (1, i, fastStart.getNextValue());
                    }

                    for (int i = 0; i < results.getNumSamples(); ++i)
                        expectWithinAbsoluteError (results.getSample (0, i),
                                                   results.getSample (1, results.getNumSamples() - (i + 1)),
                                                   1.0e-7);

                    auto expectedMidpoint = range.getStart() + (range.getLength() * Decibels::decibelsToGain (level));
                    expectWithinAbsoluteError (results.getSample (0, numSamples / 2),
                                               expectedMidpoint,
                                               1.0e-7);
                }
            }
        }
    }
};

static LogRampedValueTests LogRampedValueTests;

} // namespace dsp
} // namespace juce
