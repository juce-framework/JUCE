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
/**
    Helper class for using linear interpolation between a begin and an end value.

    The ValueType could be any numerical type, or a std::tuple containing numerical types. This
    class is mainly intended to be used with the latter.

    This way you can interpolate multiple values by supplying a single float value, which you can
    access in an Animator's value change callback.

    E.g.
    @code
    const auto boundsToTuple = [] (auto b)
    {
        return std::make_tuple (b.getX(), b.getY(), b.getWidth(), b.getHeight());
    };

    const auto begin = boundsToTuple (component.getBoundsInParent());
    const auto end   = boundsToTuple (targetBounds);
    const auto limits = makeAnimationLimits (begin, end);

    // This is the value change callback of an Animator, where you will transition a Component from
    // one bounds to the next. See the AnimatorsDemo for a more detailed example.
    const auto valueChanged = [&component, limits] (auto v)
    {
        const auto [x, y, w, h] = limits.lerp (v);
        component.setBounds (x, y, w, h);
    };
    @endcode

    @see ValueAnimatorBuilder::ValueChangedCallback

    @tags{Animations}
*/
template <typename ValueType>
class JUCE_API  StaticAnimationLimits
{
public:
    /** Constructor. You can use it to interpolate between a 0 initialised numerical value or tuple
        and the provided end state.
    */
    explicit StaticAnimationLimits (const ValueType& endIn)
        : StaticAnimationLimits ({}, endIn) {}

    /** Constructor. Creates an object that will interpolate between the two provided beginning and
        end states. The ValueType can be a numerical type or a std::tuple containing numerical
        types.
    */
    StaticAnimationLimits (const ValueType& beginIn, const ValueType& endIn)
        : begin (beginIn), end (endIn) {}

    /** Evaluation operator. Returns a value that is a linear interpolation of the beginning and end
        state. It's a shorthand for the lerp() function.
    */
    ValueType operator() (float value) const
    {
        return lerp (value);
    }

    /** Returns a value that is a linear interpolation of the beginning and end state.
    */
    ValueType lerp (float value) const
    {
        using namespace detail::ArrayAndTupleOps;

        if constexpr (std::is_integral_v<ValueType>)
            return (ValueType) std::round ((float) begin + ((float) (end - begin) * value));
        else
            return (ValueType) (begin + ((end - begin) * value));
    }

private:
    ValueType begin{}, end{};
};

/** Creates an instance of StaticAnimationLimits, deducing ValueType from
    the function argument.
*/
template <typename ValueType>
StaticAnimationLimits<ValueType> makeAnimationLimits (const ValueType& end)
{
    return StaticAnimationLimits<ValueType> (end);
}

/** Creates an instance of StaticAnimationLimits, deducing ValueType from
    the function arguments.
*/
template <typename ValueType>
StaticAnimationLimits<ValueType> makeAnimationLimits (const ValueType& begin, const ValueType& end)
{
    return StaticAnimationLimits<ValueType> (begin, end);
}

} // namespace juce
