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

template <int k>
struct LagrangeResampleHelper
{
    static forcedinline void calc (float& a, float b) noexcept   { a *= b * (1.0f / k); }
};

template <>
struct LagrangeResampleHelper<0>
{
    static forcedinline void calc (float&, float) noexcept {}
};

template <int k>
static float calcCoefficient (float input, float offset) noexcept
{
    LagrangeResampleHelper<0 - k>::calc (input, -2.0f - offset);
    LagrangeResampleHelper<1 - k>::calc (input, -1.0f - offset);
    LagrangeResampleHelper<2 - k>::calc (input,  0.0f - offset);
    LagrangeResampleHelper<3 - k>::calc (input,  1.0f - offset);
    LagrangeResampleHelper<4 - k>::calc (input,  2.0f - offset);
    return input;
}

float Interpolators::LagrangeTraits::valueAtOffset (const float* inputs, float offset, int index) noexcept
{
    float result = 0.0f;

    result += calcCoefficient<0> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<1> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<2> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<3> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<4> (inputs[index], offset);

    return result;
}

} // namespace juce
