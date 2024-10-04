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

namespace juce
{

#if JUCE_UNIT_TESTS

static CommonSmoothedValueTests <SmoothedValue<float, ValueSmoothingTypes::Linear>> commonLinearSmoothedValueTests;
static CommonSmoothedValueTests <SmoothedValue<float, ValueSmoothingTypes::Multiplicative>> commonMultiplicativeSmoothedValueTests;

class SmoothedValueTests final : public UnitTest
{
public:
    SmoothedValueTests()
        : UnitTest ("SmoothedValueTests", UnitTestCategories::smoothedValues)
    {}

    void runTest() override
    {
        beginTest ("Linear moving target");
        {
            SmoothedValue<float, ValueSmoothingTypes::Linear> sv;

            sv.reset (12);
            float initialValue = 0.0f;
            sv.setCurrentAndTargetValue (initialValue);
            sv.setTargetValue (1.0f);

            auto delta = sv.getNextValue() - initialValue;

            sv.skip (6);

            auto newInitialValue = sv.getCurrentValue();
            sv.setTargetValue (newInitialValue + 2.0f);
            auto doubleDelta = sv.getNextValue() - newInitialValue;

            expectWithinAbsoluteError (doubleDelta, delta * 2.0f, 1.0e-7f);
        }

        beginTest ("Multiplicative curve");
        {
            SmoothedValue<double, ValueSmoothingTypes::Multiplicative> sv;

            auto numSamples = 12;
            AudioBuffer<double> values (2, numSamples + 1);

            sv.reset (numSamples);
            sv.setCurrentAndTargetValue (1.0);
            sv.setTargetValue (2.0f);

            values.setSample (0, 0, sv.getCurrentValue());

            for (int i = 1; i < values.getNumSamples(); ++i)
                values.setSample (0, i, sv.getNextValue());

            sv.setTargetValue (1.0f);
            values.setSample (1, values.getNumSamples() - 1, sv.getCurrentValue());

            for (int i = values.getNumSamples() - 2; i >= 0 ; --i)
                values.setSample (1, i, sv.getNextValue());

            for (int i = 0; i < values.getNumSamples(); ++i)
                expectWithinAbsoluteError (values.getSample (0, i), values.getSample (1, i), 1.0e-9);
        }
    }
};

static SmoothedValueTests smoothedValueTests;

#endif

} // namespace juce
