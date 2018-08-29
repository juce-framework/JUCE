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

/**
    Class for efficiently approximating expensive arithmetic operations.

    The approximation is based on linear interpolation between pre-calculated values.
    The approximated function should be passed as a callable object to the constructor
    along with the number of data points to be pre-calculated. The accuracy of the
    approximation can be increased by using more points at the cost of a larger memory
    footprint.

    Consider using LookupTableTransform as an easy-to-use alternative.

    Example:

        LookupTable<float> lut ([] (size_t i) { return std::sqrt ((float) i); }, 64);
        auto outValue = lut[17];

    @see LookupTableTransform

    @tags{DSP}
*/
template <typename FloatType>
class LookupTable
{
public:
    /** Creates an uninitialised LookupTable object.

        You need to call initialise() before using the object. Prefer using the
        non-default constructor instead.

        @see initialise
    */
    LookupTable();

    /** Creates and initialises a LookupTable object.

        @param functionToApproximate The function to be approximated. This should be a
                                     mapping from the integer range [0, numPointsToUse - 1].
        @param numPointsToUse        The number of pre-calculated values stored.
    */
    LookupTable (const std::function<FloatType (size_t)>& functionToApproximate, size_t numPointsToUse);

    /** Initialises or changes the parameters of a LookupTable object.

        This function can be used to change what function is approximated by an already
        constructed LookupTable along with the number of data points used. If the function
        to be approximated won't ever change, prefer using the non-default constructor.

        @param functionToApproximate The function to be approximated. This should be a
                                     mapping from the integer range [0, numPointsToUse - 1].
        @param numPointsToUse        The number of pre-calculated values stored.
    */
    void initialise (const std::function<FloatType (size_t)>& functionToApproximate, size_t numPointsToUse);

    //==============================================================================
    /** Calculates the approximated value for the given index without range checking.

        Use this if you can guarantee that the index is non-negative and less than numPoints.
        Otherwise use get().

        @param index The approximation is calculated for this non-integer index.
        @return      The approximated value at the given index.

        @see get, operator[]
    */
    FloatType getUnchecked (FloatType index) const noexcept
    {
        jassert (isInitialised());  // Use the non-default constructor or call initialise() before first use
        jassert (isPositiveAndBelow (index, FloatType (getNumPoints())));

        auto i = truncatePositiveToUnsignedInt (index);
        auto f = index - FloatType (i);
        jassert (isPositiveAndBelow (f, FloatType (1)));

        auto x0 = data.getUnchecked (static_cast<int> (i));
        auto x1 = data.getUnchecked (static_cast<int> (i + 1));

        return jmap (f, x0, x1);
    }

    //==============================================================================
    /** Calculates the approximated value for the given index with range checking.

        This can be called with any input indices. If the provided index is out-of-range
        either the bottom or the top element of the LookupTable is returned.

        If the index is guaranteed to be in range use the faster getUnchecked() instead.

        @param index The approximation is calculated for this non-integer index.
        @return      The approximated value at the given index.

        @see getUnchecked, operator[]
    */
    FloatType get (FloatType index) const noexcept
    {
        if (index >= getNumPoints())
            index = static_cast<FloatType> (getGuardIndex());
        else if (index < 0)
            index = {};

        return getUnchecked (index);
    }

    //==============================================================================
    /** @see getUnchecked */
    FloatType operator[] (FloatType index) const noexcept       { return getUnchecked (index); }

    /** Returns the size of the LookupTable, i.e., the number of pre-calculated data points. */
    size_t getNumPoints() const noexcept                        { return static_cast<size_t> (data.size()) - 1; }

    /** Returns true if the LookupTable is initialised and ready to be used. */
    bool isInitialised() const noexcept                         { return data.size() > 1; }

private:
    //==============================================================================
    Array<FloatType> data;

    void prepare() noexcept;
    static size_t getRequiredBufferSize (size_t numPointsToUse) noexcept { return numPointsToUse + 1; }
    size_t getGuardIndex() const noexcept                                { return getRequiredBufferSize (getNumPoints()) - 1; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookupTable)
};


//==============================================================================
/** Class for approximating expensive arithmetic operations.

    Once initialised, this class can be used just like the function it approximates
    via operator().

    Example:

        LookupTableTransform<float> tanhApprox ([] (float x) { return std::tanh (x); }, -5.0f, 5.0f, 64);
        auto outValue = tanhApprox (4.2f);

    Note: If you try to call the function with an input outside the provided
    range, it will return either the first or the last recorded LookupTable value.

    @see LookupTable

    @tags{DSP}
*/
template <typename FloatType>
class LookupTableTransform
{
public:
    //==============================================================================
    /** Creates an uninitialised LookupTableTransform object.

        You need to call initialise() before using the object. Prefer using the
        non-default constructor instead.

        @see initialise
    */
    LookupTableTransform()
    {}

    //==============================================================================
    /** Creates and initialises a LookupTableTransform object.

        @param functionToApproximate The function to be approximated. This should be a
                                     mapping from a FloatType to FloatType.
        @param minInputValueToUse    The lowest input value used. The approximation will
                                     fail for values lower than this.
        @param maxInputValueToUse    The highest input value used. The approximation will
                                     fail for values higher than this.
        @param numPoints             The number of pre-calculated values stored.
    */
    LookupTableTransform (const std::function<FloatType (FloatType)>& functionToApproximate,
                          FloatType minInputValueToUse,
                          FloatType maxInputValueToUse,
                          size_t numPoints)
    {
        initialise (functionToApproximate, minInputValueToUse, maxInputValueToUse, numPoints);
    }

    //==============================================================================
    /** Initialises or changes the parameters of a LookupTableTransform object.

        @param functionToApproximate The function to be approximated. This should be a
                                     mapping from a FloatType to FloatType.
        @param minInputValueToUse    The lowest input value used. The approximation will
                                     fail for values lower than this.
        @param maxInputValueToUse    The highest input value used. The approximation will
                                     fail for values higher than this.
        @param numPoints             The number of pre-calculated values stored.
    */
    void initialise (const std::function<FloatType (FloatType)>& functionToApproximate,
                     FloatType minInputValueToUse,
                     FloatType maxInputValueToUse,
                     size_t numPoints);

    //==============================================================================
    /** Calculates the approximated value for the given input value without range checking.

        Use this if you can guarantee that the input value is within the range specified
        in the constructor or initialise(), otherwise use processSample().

        @param value The approximation is calculated for this input value.
        @return      The approximated value for the provided input value.

        @see processSample, operator(), operator[]
    */
    FloatType processSampleUnchecked (FloatType value) const noexcept
    {
        jassert (value >= minInputValue && value <= maxInputValue);
        return lookupTable[scaler * value + offset];
    }

    //==============================================================================
    /** Calculates the approximated value for the given input value with range checking.

        This can be called with any input values. Out-of-range input values will be
        clipped to the specified input range.

        If the index is guaranteed to be in range use the faster processSampleUnchecked()
        instead.

        @param value The approximation is calculated for this input value.
        @return      The approximated value for the provided input value.

        @see processSampleUnchecked, operator(), operator[]
    */
    FloatType processSample (FloatType value) const noexcept
    {
        auto index = scaler * jlimit (minInputValue, maxInputValue, value) + offset;
        jassert (isPositiveAndBelow (index, FloatType (lookupTable.getNumPoints())));

        return lookupTable[index];
    }

    //==============================================================================
    /** @see processSampleUnchecked */
    FloatType operator[] (FloatType index) const noexcept       { return processSampleUnchecked (index); }

    /** @see processSample */
    FloatType operator() (FloatType index) const noexcept       { return processSample (index); }

    //==============================================================================
    /** Processes an array of input values without range checking
        @see process
    */
    void processUnchecked (const FloatType* input, FloatType* output, size_t numSamples) const noexcept
    {
        for (size_t i = 0; i < numSamples; ++i)
            output[i] = processSampleUnchecked (input[i]);
    }

    //==============================================================================
    /** Processes an array of input values with range checking
        @see processUnchecked
    */
    void process (const FloatType* input, FloatType* output, size_t numSamples) const noexcept
    {
        for (size_t i = 0; i < numSamples; ++i)
            output[i] = processSample (input[i]);
    }

    //==============================================================================
    /** Calculates the maximum relative error of the approximation for the specified
        parameter set.

        The closer the returned value is to zero the more accurate the approximation
        is.

        This function compares the approximated output of this class to the function
        it approximates at a range of points and returns the maximum relative error.
        This can be used to determine if the approximation is suitable for the given
        problem. The accuracy of the approximation can generally be improved by
        increasing numPoints.

        @param functionToApproximate The approximated function. This should be a
                                     mapping from a FloatType to FloatType.
        @param minInputValue         The lowest input value used.
        @param maxInputValue         The highest input value used.
        @param numPoints             The number of pre-calculated values stored.
        @param numTestPoints         The number of input values used for error
                                     calculation. Higher numbers can increase the
                                     accuracy of the error calculation. If it's zero
                                     then 100 * numPoints will be used.
    */
    static double calculateMaxRelativeError (const std::function<FloatType (FloatType)>& functionToApproximate,
                                             FloatType minInputValue,
                                             FloatType maxInputValue,
                                             size_t numPoints,
                                             size_t numTestPoints = 0);
private:
    //==============================================================================
    static double calculateRelativeDifference (double, double) noexcept;

    //==============================================================================
    LookupTable<FloatType> lookupTable;

    FloatType minInputValue, maxInputValue;
    FloatType scaler, offset;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookupTableTransform)
};

} // namespace dsp
} // namespace juce
