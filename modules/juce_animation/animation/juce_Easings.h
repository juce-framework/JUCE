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

//==============================================================================
/** A selection of options available for customising a spring style easing function.

    @tags{Animations}
*/
class SpringEasingOptions
{
public:
    /** Specifies the number of oscillations the easing would undergo.

        This also affects the speed of the movement.

        @see getFrequency, withAttenuation, withExtraAttenuationRange
    */
    [[nodiscard]] auto withFrequency (float newFrequency) const
    {
        return withMember (*this, &SpringEasingOptions::frequency, newFrequency);
    }

    /** Affects how quickly the oscillations die down.

        @see getAttenuation, withFrequency, withExtraAttenuationRange
    */
    [[nodiscard]] auto withAttenuation (float newAttenuation) const
    {
        return withMember (*this, &SpringEasingOptions::attenuation, newAttenuation);
    }

    /** Specifies the input value at which an extra non-physical attenuation begins to be applied.
        The value must be in the range [0.05f, 0.98f].

        This ensures that the easing always reaches an output value of 1.0f when the input value is
        1.0f. If the attenuation is set sufficiently high this won't have a visible effect.

        @see getExtraAttenuationRange, withFrequency, withAttenuation
    */
    [[nodiscard]] auto withExtraAttenuationRange (float newExtraAttenuationRange) const
    {
        return withMember (*this, &SpringEasingOptions::extraAttenuationRange, std::clamp (newExtraAttenuationRange, 0.05f, 0.98f));
    }

    /** Returns the value specified by withFrequency.

        If no value was specified the default value is 3.0f.

        @see withFrequency
    */
    [[nodiscard]] auto getFrequency() const { return frequency; }

    /** Returns the value specified by withAttenuation.

        If no value was specified the default value is 3.0f.

        @see withAttenuation
    */
    [[nodiscard]] auto getAttenuation() const { return attenuation; }

    /** Returns the value specified by withExtraAttenuationRange

        If no value was specified the default value is 0.25f.

        @see withExtraAttenuationRange
    */
    [[nodiscard]] auto getExtraAttenuationRange() const { return extraAttenuationRange; }

private:
    float frequency = 3.0f;
    float attenuation = 3.0f;
    float extraAttenuationRange = 0.25f;
};

//==============================================================================
/**
    Holds a number of easing functions that you can pass into ValueAnimatorBuilder::withEasing to
    transform the linear progression of animations.

    Using createSpring() for example would transform a rigid movement into one that is reminiscent
    of a weight attached to a spring.

    For examples of all the easing functions see the AnimationEasingDemo in the DemoRunner.

    @tags{Animations}
*/
struct Easings
{
    /** Returns a cubic Bezier function with the control points (x1, y1), (x2, y2). These points are
        the two middle points of a cubic Bezier function's four control points, the first and last
        being (0, 0), and (1, 1).
    */
    static std::function<float (float)> createCubicBezier (float x1, float y1, float x2, float y2);

    /** Returns a cubic Bezier function with two control points. These points are the two middle
        points of a cubic Bezier function's four control points, the first and last being (0, 0),
        and (1, 1).
    */
    static std::function<float (float)> createCubicBezier (Point<float> controlPoint1,
                                                           Point<float> controlPoint2);

    /** Returns the easing function createCubicBezier (0.25f, 0.1f, 0.25f, 1.0f). This indicates
        that the interpolation starts slowly, accelerates sharply, and then slows gradually towards
        the end. It is similar to createEaseInOut(), though it accelerates more sharply at the
        beginning.

        This is equivalent to using the "ease" keyword when specifying a timing-function in CSS.

        This is the default easing used by the ValueAnimatorBuilder class if no other easing
        function is specified.

        @see createCubicBezier, createEaseIn, createEaseOut, createEaseInOut
    */
    static std::function<float (float)> createEase();

    /** Returns the easing function createCubicBezier (0.42f, 0.0f, 1.0f, 1.0f). This indicates that
        the interpolation starts slowly, then progressively speeds up until the end, at which point
        it stops abruptly.

        This is equivalent to using the "ease-in" keyword when specifying a timing-function in CSS.

        @see createCubicBezier, createEase, createEaseOut, createEaseInOut
    */
    static std::function<float (float)> createEaseIn();

    /** Returns the easing function createCubicBezier (0.0f, 0.0f, 0.58f, 1.0f). This indicates that
        the interpolation starts abruptly and then progressively slows down towards the end.

        This is equivalent to using the "ease-out" keyword when specifying a timing-function in CSS.

        @see createCubicBezier, createEase, createEaseIn, createEaseInOut, createEaseOutBack
    */
    static std::function<float (float)> createEaseOut();

    /** Returns the easing function createCubicBezier (0.34f, 1.56f, 0.64f, 1.0f). This indicates
        that the interpolation starts abruptly, quickly decelerating before overshooting the target
        value by approximately 10% and changing direction to slowly head back towards the target
        value.

        Like createSpring() this will overshoot causing it to return float values exceeding 1.0f.

        This is equivalent to easeOutBack as specified on https://easings.net/#easeOutBack.

        @see createCubicBezier, createEaseOutBack, createSpring
    */
    static std::function<float (float)> createEaseOutBack();

    /** Returns the easing function createCubicBezier (0.42f, 0.0f, 0.58f, 1.0f). This indicates
        that the interpolation starts slowly, speeds up, and then slows down towards the end. At the
        beginning, it behaves like createEaseIn(); at the end, it behaves like createEaseOut().

        This is equivalent to using the "ease-in-out" keyword when specifying a timing-function in
        CSS.

        @see createCubicBezier, createEase, createEaseIn, createEaseInOut
    */
    static std::function<float (float)> createEaseInOut();

    /** Returns the easing function createCubicBezier (0.65f, 0.0f, 0.35f, 1.0f). This indicates
        that the interpolation starts slowly, speeds up, and then slows down towards the end. It
        behaves similar to createEaseInOut() but is more exaggerated and has a more symmetrical
        curve.

        This is equivalent to easeInOutCubic as specified on https://easings.net/#easeInOutCubic.

        @see createCubicBezier, createEaseInOut
    */
    static std::function<float (float)> createEaseInOutCubic();

    /** Returns an easing function with a constant rate of interpolation, with no change in the rate
        of progress throughout the duration (that is, no acceleration or deceleration).
    */
    static std::function<float (float)> createLinear();

    /** Returns an easing function that behaves like a spring with a weight attached.

        Like createEaseOutBack() this might overshoot causing it to return float values exceeding
        1.0f.

        @see createEaseOutBack
    */
    static std::function<float (float)> createSpring (const SpringEasingOptions& options = {});

    /** Returns an easing function that behaves like a bouncy ball dropped on the ground.

        The function will bounce numBounces times on the input range of [0, 1] before coming to
        stop, each bounce is less pronounced than the previous.

        This is equivalent to easeInOutCubic as specified on https://easings.net/#easeOutBounce.
    */
    static std::function<float (float)> createBounce (int numBounces = 3);

    /** Returns an easing function that reaches 1.0f when the input value is 0.5f, before returning
        to 0.0f when the input values reaches 1.0f.

        This is useful for making a repeating pulsation.
    */
    static std::function<float (float)> createOnOffRamp();
};

} // namespace juce
