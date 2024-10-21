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

std::function<float (float)> Easings::createCubicBezier (float x1, float y1, float x2, float y2)
{
    // The x axis represents time, it's important this always stays in the range 0 - 1
    jassert (isPositiveAndNotGreaterThan (x1, 1.0f));
    jassert (isPositiveAndNotGreaterThan (x2, 1.0f));

    chromium::gfx::CubicBezier cubicBezier { (double) x1, (double) y1, (double) x2, (double) y2 };
    return [bezier = std::move (cubicBezier)] (float v) { return (float) bezier.Solve (v); };
}

std::function<float (float)> Easings::createCubicBezier (Point<float> controlPoint1,
                                                         Point<float> controlPoint2)
{
    return createCubicBezier (controlPoint1.getX(),
                              controlPoint1.getY(),
                              controlPoint2.getX(),
                              controlPoint2.getY());
}

std::function<float (float)> Easings::createEase()
{
    const static auto f = createCubicBezier (0.25f, 0.1f, 0.25f, 1.0f);
    return f;
}

std::function<float (float)> Easings::createEaseIn()
{
    const static auto f = createCubicBezier (0.42f, 0.0f, 1.0f, 1.0f);
    return f;
}

std::function<float (float)> Easings::createEaseOut()
{
    const static auto f = createCubicBezier (0.0f, 0.0f, 0.58f, 1.0f);;
    return f;
}

std::function<float (float)> Easings::createEaseInOut()
{
    const static auto f = createCubicBezier (0.42f, 0.0f, 0.58f, 1.0f);
    return f;
}

std::function<float (float)> Easings::createLinear()
{
    return [] (auto x){ return x; };
}

std::function<float (float)> Easings::createEaseOutBack()
{
    const static auto f = createCubicBezier (0.34f, 1.56f, 0.64f, 1.0f);
    return f;
}

std::function<float (float)> Easings::createEaseInOutCubic()
{
    const static auto f = createCubicBezier (0.65f, 0.0f, 0.35f, 1.0f);
    return f;
}

std::function<float (float)> Easings::createSpring (const SpringEasingOptions& options)
{
    return [=] (float v)
    {
        const auto t = std::clamp (v, 0.0f, 1.0f);
        const auto omega = 2.0f * MathConstants<float>::pi * options.getFrequency();
        const auto physicalValue = 1.0f - std::exp (-options.getAttenuation() * t) * std::cos (omega * t);
        const auto squish = 1.0f / options.getExtraAttenuationRange();
        const auto shift = 1.0f - options.getExtraAttenuationRange();
        const auto weight = std::clamp (std::pow (squish * (std::max (t - shift, 0.0f)), 2.0f), 0.0f, 1.0f);
        return weight + (1.0f - weight) * physicalValue;
    };
}

std::function<float (float)> Easings::createBounce (int numBounces)
{
    jassert (numBounces >= 0);
    numBounces = std::max (0, numBounces);

    const auto alpha = std::pow (0.05f, 1.0f / (float) numBounces);

    const auto fallTime = [] (float h)
    {
        return std::sqrt (2.0f * h);
    };

    std::vector<float> bounceTimes;
    bounceTimes.reserve ((size_t) (numBounces + 1));
    bounceTimes.push_back (fallTime (1.0f));

    for (int i = 1; i < numBounces + 1; ++i)
        bounceTimes.push_back (bounceTimes.back() + 2.0f * fallTime (std::pow (alpha, (float) i)));

    for (auto& bounce : bounceTimes)
        bounce /= bounceTimes.back();

    return [alpha, times = std::move (bounceTimes)] (float v)
    {
        v = std::clamp (v, 0.0f, 1.0f);

        const auto boundIt = std::lower_bound (times.begin(), times.end(), v);

        if (boundIt == times.end())
            return 1.0f;

        const auto i = (size_t) std::distance (times.begin(), boundIt);
        const auto height = i == 0 ? 1.0f : std::pow (alpha, (float) i);
        const auto center = i == 0 ? 0.0f : (times[i] + times[i - 1]) / 2.0f;
        const auto distToZero = i == 0 ? times[i] : (times[i] - times[i - 1]) / 2.0f;
        return 1.0f - height * (1.0f - std::pow (1.0f / distToZero * (v - center), 2.0f));
    };
}

std::function<float (float)> Easings::createOnOffRamp()
{
    return [] (float x) { return 1.0f - std::abs (2.0f * (x - 0.5f)); };
}

} // namespace juce
