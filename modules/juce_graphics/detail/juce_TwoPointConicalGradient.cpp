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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

namespace juce::detail
{

GradientCircle GradientCircle::transformedBy (const AffineTransform& t) const
{
    // Necessary condition for maintaining a circular shape
    jassert (exactlyEqual (t.mat00, t.mat11));

    GradientCircle result;
    result.c = c.transformedBy (t);
    const auto scaleFactor = std::sqrt (std::abs (t.getDeterminant()));
    result.r = r * scaleFactor;
    return result;
}

//==============================================================================
GradientCircle RadialGradientView::getEndCircle() const
{
    return GradientCircle { isLegacy() ? gradient.point1 : gradient.point2,
                            isLegacy() ? gradient.point1.getDistanceFrom (gradient.point2)
                                       : gradient.endRadius };
}

GradientCircle RadialGradientView::getStartCircle() const
{
    return GradientCircle { gradient.point1, gradient.startRadius };
}

bool RadialGradientView::isLegacy() const noexcept
{
    return gradient.endRadius < 0.0f;
}

//==============================================================================
std::optional<TwoPointConicalGradient> TwoPointConicalGradient::create (const ColourGradient& gradient)
{
    jassert (gradient.isRadial);
    RadialGradientView rg { &gradient };

    const auto s = rg.getStartCircle();
    const auto e = rg.getEndCircle();

    if (! isConicalGradientApplicable (s, e))
        return std::nullopt;

    return TwoPointConicalGradient { s, e };
}

TwoPointConicalGradient::TwoPointConicalGradient (GradientCircle startCircle, GradientCircle endCircle)
    : swapped { approximatelyEqual (endCircle.r, 0.0f) },
      c0 { swapped ? endCircle : startCircle },
      c1 { swapped ? startCircle : endCircle }
{
    jassert (! approximatelyEqual (c0.r, c1.r, inputTolerance));
    f = c0.r / (c0.r - c1.r);
    const auto focalPoint = (1.0f - f) * c0.c + f * c1.c;
    const auto c1FocalSegment = c1.c - focalPoint;

    // We need a transform that moves the focal point to (0, 0) and c1 to (1, 0), and scales
    // uniformly on the two axes.
    const auto angleWithAxisX = std::atan2 (c1FocalSegment.y, c1FocalSegment.x);

    regularisationTransform = AffineTransform::translation (-focalPoint.x,
                                                            -focalPoint.y).rotated (-angleWithAxisX);

    const auto distToFocalPoint = c1FocalSegment.getDistanceFromOrigin();

    // Gradients where the start and end circles are concentric are dispatched to the old gradient
    jassert (! approximatelyEqual (distToFocalPoint, 0.0f, inputTolerance));
    regularisationTransform = regularisationTransform.scaled (1.0f / distToFocalPoint);

    // The math only works if the circles aren't transformed into ellipses
    jassert (exactlyEqual (regularisationTransform.mat00, regularisationTransform.mat11));

    c0 = c0.transformedBy (regularisationTransform);
    c1 = c1.transformedBy (regularisationTransform);
    f  = c0.r / (c0.r - c1.r);
}

std::optional<float> TwoPointConicalGradient::calculateWeight (Point<float> p) const
{
    p = p.transformedBy (regularisationTransform);

    // Geometric meaning: the focus point sits on r1's circumference. The start end and end
    // circles are touching.
    //
    // As r1 approaches 1.0f the accuracy of the solutions visibly deteriorates. We want to
    // switch over to the more stable formula, that assumes r = 1, as soon as possible.
    // At the same time we want the centre positioning error caused by this switch to be << 1px.
    const auto closeToSingular = approximatelyEqual (c1.r, 1.0f, regularisationTolerance);

    static constexpr auto squared = [] (float x) noexcept { return x * x; };

    const auto x_prime = std::invoke (
        [&]
        {
            if (closeToSingular)
                return p.x / 2.0f;

            return c1.r / (squared (c1.r) - 1.0f) * p.x;
        });

    const auto y_prime = std::invoke (
        [&]
        {
            if (closeToSingular)
                return p.y / 2.0f;

            return std::sqrt (std::abs (squared (c1.r) - 1.0f)) / (squared (c1.r) - 1.0f) * p.y;
        });

    const auto x_hat = std::abs (1.0f - f) * x_prime;
    const auto y_hat = std::abs (1.0f - f) * y_prime;

    bool invalid = false;

    const auto x_hat_t = std::invoke (
        [&]
        {
            if (closeToSingular)
                return (squared (x_hat) + squared (y_hat)) / x_hat;

            if (c1.r < 1.0f)
            {
                // Geometric meaning: the start circle is outside the end circle, and we're
                // looking for a solution that's outside the cone defined by them.
                if (squared (x_hat) - squared (y_hat) < 0.0f)
                {
                    invalid = true;
                    return 0.0f;
                };

                if (swapped || (1.0f - f) < 0.0f)
                    return -std::sqrt (squared (x_hat) - squared (y_hat)) - x_hat / c1.r;

                return std::sqrt (squared (x_hat) - squared (y_hat)) - x_hat / c1.r;
            }

            // c1.r > 1.0f
            return std::sqrt (squared (x_hat) + squared (y_hat)) - x_hat / c1.r;
        });

    if (invalid)
        return std::nullopt;

    if (x_hat_t < 0.0f)
        return std::nullopt;

    constexpr auto signum = [] (float v) { return (v > 0.0f) ? 1.0f : ((v < 0.0f) ? -1.0f : 0.0f); };

    const auto t = f + signum (1.0f - f) * x_hat_t;

    if (swapped)
        return 1.0f - t;

    return t;
}

bool TwoPointConicalGradient::isConicalGradientApplicable (const GradientCircle& s,
                                                           const GradientCircle& e)
{
    if (approximatelyEqual (s.c.getDistanceFrom (e.c), 0.0f, inputTolerance))
        return false;

    if (approximatelyEqual (s.r, e.r, inputTolerance))
        return false;

    return true;
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct TwoPointConicalGradientTest final : public UnitTest
{
    TwoPointConicalGradientTest() : UnitTest ("TwoPointConicalGradientTest", UnitTestCategories::graphics)
    {
    }

    static constexpr auto magnitude = 200.0f;
    static constexpr auto safetyFactor = 1.1f;

    // TwoPointConicalGradient::regularisationTolerance is used by the
    // TwoPointConicalGradient class in relation to the internal, normalised space,
    // where the end circle's distance from the focal point is 1.0f.
    inline static const auto tolerance = TwoPointConicalGradient::regularisationTolerance.getAbsolute()
                                         * magnitude
                                         * safetyFactor;

    static Point<float> makeRandomPoint (Random& random)
    {
        return { -magnitude / 2.0f + random.nextFloat() * magnitude,
                 -magnitude / 2.0f + random.nextFloat() * magnitude };
    }

    void runTest() override
    {
        Random random = getRandom();

        beginTest ("Start circle lies completely inside the end circle");
        {
            for (int i = 0; i < 1000; ++i)
            {
                GradientCircle end { makeRandomPoint (random),
                                     std::max (3.0f * tolerance, random.nextFloat() * (magnitude / 2.0f)) };

                const auto startDistanceFromEnd = tolerance + std::max (end.r - 2.0f * tolerance, 0.0f) * random.nextFloat();
                const auto startRadius = (end.r - startDistanceFromEnd - tolerance) * random.nextFloat();
                const auto startAngle = MathConstants<float>::twoPi * random.nextFloat();
                const auto startCentre = Point<float> { end.c.x + std::cos (startAngle) * startDistanceFromEnd,
                                                        end.c.y + std::sin (startAngle) * startDistanceFromEnd };

                GradientCircle start { startCentre, startRadius };

                ColourGradient cg { Colours::red, start.c.x, start.c.y, Colours::blue, end.c.x, end.c.y, true };
                cg.startRadius = start.r;
                cg.endRadius = end.r;

                const auto gradient = TwoPointConicalGradient::create (cg);

                expect (gradient.has_value(),
                        "TwoPointConicalGradient should be appropriate for safely constructed arguments");

                for (int j = 0; j < 100; ++j)
                {
                    const auto p = makeRandomPoint (random);
                    const auto t = gradient->calculateWeight (p);

                    expect (t.has_value(),
                            "TwoPointConicalGradient with a start circle completely inside the end circle "
                            "should be able to calculate a weight for any point");

                    if (t.has_value())
                    {
                        const auto f = *t;
                        const auto pointToCenter = p.getDistanceFrom ((1.0f - f) * start.c + f * end.c);
                        const auto solutionRadius = std::abs ((1.0f - f) * start.r + f * end.r);

                        expect (approximatelyEqual (pointToCenter, solutionRadius, relativeTolerance (0.002f)),
                                "The solution should specify a circle that passes through the rendered pixel");
                    }
                }
            }
        }
    }
};

static TwoPointConicalGradientTest twoPointConicalGradientTest;

#endif

} // namespace juce::detail
