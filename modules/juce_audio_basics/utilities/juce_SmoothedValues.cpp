/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2018 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_UNIT_TESTS

template <class SmoothedValueType>
class CommonSmoothedValueTests  : public UnitTest
{
public:
    CommonSmoothedValueTests()
        : UnitTest ("CommonSmoothedValueTests", "SmoothedValues")
    {}

    void runTest() override
    {
        beginTest ("Initial state");
        {
            SmoothedValueType lsv;

            auto value = lsv.getCurrentValue();
            expectEquals (lsv.getTargetValue(), value);

            lsv.getNextValue();
            expectEquals (lsv.getCurrentValue(), value);
            expect (! lsv.isSmoothing());
        }

        beginTest ("Resetting");
        {
            auto initialValue = -5.0f;

            SmoothedValueType lsv (-5.0f);
            lsv.reset (3);
            expectEquals (lsv.getCurrentValue(), initialValue);

            auto targetValue = initialValue + 1.0f;
            lsv.setTargetValue (targetValue);
            expectEquals (lsv.getTargetValue(), targetValue);
            expectEquals (lsv.getCurrentValue(), initialValue);
            expect (lsv.isSmoothing());

            auto currentValue = lsv.getNextValue();
            expect (currentValue > initialValue);
            expectEquals (lsv.getCurrentValue(), currentValue);
            expectEquals (lsv.getTargetValue(), targetValue);
            expect (lsv.isSmoothing());

            lsv.reset (5);

            expectEquals (lsv.getCurrentValue(), targetValue);
            expectEquals (lsv.getTargetValue(),  targetValue);
            expect (! lsv.isSmoothing());

            lsv.getNextValue();
            expectEquals (lsv.getCurrentValue(), targetValue);

            lsv.setTargetValue (-15.0f);
            lsv.getNextValue();

            float newStart = -20.0f;
            lsv.setCurrentAndTargetValue (newStart);
            expectEquals (lsv.getNextValue(), newStart);
            expectEquals (lsv.getTargetValue(), newStart);
            expectEquals (lsv.getCurrentValue(), newStart);
            expect (! lsv.isSmoothing());
        }

        beginTest ("Sample rate");
        {
            SmoothedValueType lsvSamples { 3.0f };
            auto lsvTime = lsvSamples;

            auto numSamples = 12;

            lsvSamples.reset (numSamples);
            lsvTime.reset (numSamples * 2, 1.0);

            for (int i = 0; i < numSamples; ++i)
            {
                lsvTime.skip (1);
                expectWithinAbsoluteError (lsvSamples.getNextValue(),
                                           lsvTime.getNextValue(),
                                           1.0e-7f);
            }
        }

        beginTest ("Block processing");
        {
            SmoothedValueType lsv (1.0f);

            lsv.reset (12);
            lsv.setTargetValue (2.0f);

            const auto numSamples = 15;

            AudioBuffer<float> referenceData (1, numSamples);

            for (int i = 0; i < numSamples; ++i)
                referenceData.setSample (0, i, lsv.getNextValue());

            expect (referenceData.getSample (0, 0) > 0);
            expect (referenceData.getSample (0, 10) < lsv.getTargetValue());
            expectWithinAbsoluteError (referenceData.getSample (0, 11),
                                       lsv.getTargetValue(),
                                       1.0e-7f);

            auto getUnitData = [] (int numSamplesToGenerate)
            {
                AudioBuffer<float> result (1, numSamplesToGenerate);

                for (int i = 0; i < numSamplesToGenerate; ++i)
                    result.setSample (0, i, 1.0f);

                return result;
            };

            auto compareData = [this](const AudioBuffer<float>& test,
                                      const AudioBuffer<float>& reference)
            {
                for (int i = 0; i < test.getNumSamples(); ++i)
                    expectWithinAbsoluteError (test.getSample (0, i),
                                               reference.getSample (0, i),
                                               1.0e-7f);
            };

            auto testData = getUnitData (numSamples);
            lsv.setCurrentAndTargetValue (1.0f);
            lsv.setTargetValue (2.0f);
            lsv.applyGain (testData.getWritePointer (0), numSamples);
            compareData (testData, referenceData);

            testData = getUnitData (numSamples);
            AudioBuffer<float> destData (1, numSamples);
            lsv.setCurrentAndTargetValue (1.0f);
            lsv.setTargetValue (2.0f);
            lsv.applyGain (destData.getWritePointer (0),
                           testData.getReadPointer (0),
                           numSamples);
            compareData (destData, referenceData);
            compareData (testData, getUnitData (numSamples));

            testData = getUnitData (numSamples);
            lsv.setCurrentAndTargetValue (1.0f);
            lsv.setTargetValue (2.0f);
            lsv.applyGain (testData, numSamples);
            compareData (testData, referenceData);
        }

        beginTest ("Skip");
        {
            SmoothedValueType lsv;

            lsv.reset (12);
            lsv.setCurrentAndTargetValue (0.0f);
            lsv.setTargetValue (1.0f);

            Array<float> reference;

            for (int i = 0; i < 15; ++i)
                reference.add (lsv.getNextValue());

            lsv.setCurrentAndTargetValue (0.0f);
            lsv.setTargetValue (1.0f);

            expectWithinAbsoluteError (lsv.skip (1), reference[0], 1.0e-7f);
            expectWithinAbsoluteError (lsv.skip (1), reference[1], 1.0e-7f);
            expectWithinAbsoluteError (lsv.skip (2), reference[3], 1.0e-7f);
            lsv.skip (3);
            expectWithinAbsoluteError (lsv.getCurrentValue(), reference[6], 1.0e-7f);
            expectEquals (lsv.skip (300), lsv.getTargetValue());
            expectEquals (lsv.getCurrentValue(), lsv.getTargetValue());
        }

        beginTest ("Moving target");
        {
            SmoothedValueType lsv;

            lsv.reset (12);
            float initialValue = 0.0f;
            lsv.setCurrentAndTargetValue (initialValue);
            lsv.setTargetValue (1.0f);

            auto delta = lsv.getNextValue() - initialValue;

            lsv.skip (6);

            auto newInitialValue = lsv.getCurrentValue();
            lsv.setTargetValue (newInitialValue + 2.0f);
            auto doubleDelta = lsv.getNextValue() - newInitialValue;

            expectWithinAbsoluteError (doubleDelta, delta * 2.0f, 1.0e-7f);
        }
    }
};

static CommonSmoothedValueTests<LinearSmoothedValue<float>> commonLinearSmoothedValueTests;
static CommonSmoothedValueTests<LogSmoothedValue   <float>> commonLogSmoothedValueTests;

class LogSmoothedValueTests  : public UnitTest
{
public:
    LogSmoothedValueTests()
        : UnitTest ("LogSmoothedValueTests", "SmoothedValues")
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
                    LogSmoothedValue<double> slowStart { range.getStart() } , fastStart { range.getEnd() };

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

static LogSmoothedValueTests logSmoothedValueTests;

#endif

} // namespace juce
