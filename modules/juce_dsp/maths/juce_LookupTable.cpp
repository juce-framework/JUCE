/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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

template <typename FloatType>
LookupTable<FloatType>::LookupTable()
{
    data.resize (1);
}

template <typename FloatType>
LookupTable<FloatType>::LookupTable (const std::function<FloatType (size_t)>& functionToApproximate,
                                     size_t numPointsToUse)
{
    initialise (functionToApproximate, numPointsToUse);
}

//==============================================================================
template <typename FloatType>
void LookupTable<FloatType>::initialise (const std::function<FloatType (size_t)>& functionToApproximate,
                                         size_t numPointsToUse)
{
    data.resize (static_cast<int> (getRequiredBufferSize (numPointsToUse)));

    for (size_t i = 0; i < numPointsToUse; ++i)
    {
        auto value = functionToApproximate (i);

        jassert (! std::isnan (value));
        jassert (! std::isinf (value));
        // Make sure functionToApproximate returns a sensible value for the entire specified range.
        // E.g., this won't work for zero:  [] (size_t i) { return 1.0f / i; }

        data.getReference (static_cast<int> (i)) = value;
    }

    prepare();
}

template <typename FloatType>
void LookupTable<FloatType>::prepare() noexcept
{
    auto guardIndex = static_cast<int> (getGuardIndex());
    data.getReference (guardIndex) = data.getUnchecked (guardIndex - 1);
}

template <typename FloatType>
void LookupTableTransform<FloatType>::initialise (const std::function<FloatType (FloatType)>& functionToApproximate,
                                                  FloatType minInputValueToUse,
                                                  FloatType maxInputValueToUse,
                                                  size_t numPoints)
{
    jassert (maxInputValueToUse > minInputValueToUse);

    minInputValue = minInputValueToUse;
    maxInputValue = maxInputValueToUse;
    scaler = FloatType (numPoints - 1) / (maxInputValueToUse - minInputValueToUse);
    offset = -minInputValueToUse * scaler;

    const auto initFn = [functionToApproximate, minInputValueToUse, maxInputValueToUse, numPoints] (size_t i)
    {
        return functionToApproximate (
            jlimit (
                minInputValueToUse, maxInputValueToUse,
                jmap (FloatType (i), FloatType (0), FloatType (numPoints - 1), minInputValueToUse, maxInputValueToUse))
            );
    };

    lookupTable.initialise (initFn, numPoints);
}

//==============================================================================
template <typename FloatType>
double LookupTableTransform<FloatType>::calculateMaxRelativeError (const std::function<FloatType (FloatType)>& functionToApproximate,
                                                                   FloatType minInputValue,
                                                                   FloatType maxInputValue,
                                                                   size_t numPoints,
                                                                   size_t numTestPoints)
{
    jassert (maxInputValue > minInputValue);

    if (numTestPoints == 0)
        numTestPoints = 100 * numPoints;    // use default

    LookupTableTransform transform (functionToApproximate, minInputValue, maxInputValue, numPoints);

    double maxError = 0;

    for (size_t i = 0; i < numTestPoints; ++i)
    {
        auto inputValue = jmap (FloatType (i), FloatType (0), FloatType (numTestPoints - 1), minInputValue, maxInputValue);
        auto approximatedOutputValue = transform.processSample (inputValue);
        auto referenceOutputValue = functionToApproximate (inputValue);

        maxError = jmax (maxError, calculateRelativeDifference ((double) referenceOutputValue, (double) approximatedOutputValue));
    }

    return maxError;
}

//==============================================================================
template <typename FloatType>
double LookupTableTransform<FloatType>::calculateRelativeDifference (double x, double y) noexcept
{
    static const auto eps = std::numeric_limits<double>::min();

    auto absX = std::abs (x);
    auto absY = std::abs (y);
    auto absDiff = std::abs (x - y);

    if (absX < eps)
    {
        if (absY >= eps)
            return absDiff / absY;

        return absDiff;    // return the absolute error if both numbers are too close to zero
    }

    return absDiff / std::min (absX, absY);
}

//==============================================================================
template class LookupTable<float>;
template class LookupTable<double>;

template class LookupTableTransform<float>;
template class LookupTableTransform<double>;

} // namespace dsp
} // namespace juce
