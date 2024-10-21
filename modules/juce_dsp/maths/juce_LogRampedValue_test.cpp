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

static CommonSmoothedValueTests <LogRampedValue <float>> commonLogRampedValueTests;

class LogRampedValueTests final : public UnitTest
{
public:
    LogRampedValueTests()
        : UnitTest ("LogRampedValueTests", UnitTestCategories::dsp)
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

} // namespace juce::dsp
