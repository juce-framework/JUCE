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

namespace juce::dsp
{

/**
    A class which provides multiple windowing functions useful for filter design
    and spectrum analyzers.

    The different functions provided here can be used by creating either a
    WindowingFunction object, or a static function to fill an array with the
    windowing method samples.

    @tags{DSP}
*/
template <typename FloatType>
class JUCE_API  WindowingFunction
{
public:
    //==============================================================================
    /** The windowing methods available. */
    enum WindowingMethod
    {
        rectangular = 0,
        triangular,
        hann,
        hamming,
        blackman,
        blackmanHarris,
        flatTop,
        kaiser,
        numWindowingMethods
    };

    //==============================================================================
    /** This constructor automatically fills a buffer of the specified size using
        the fillWindowingTables function and the specified arguments.

        @see fillWindowingTables
    */
    WindowingFunction (size_t size, WindowingMethod,
                       bool normalise = true, FloatType beta = 0);

    //==============================================================================
    /** Fills the content of the object array with a given windowing method table.

        @param size         the size of the destination buffer allocated in the object
        @param type         the type of windowing method being used
        @param normalise    if the result must be normalised, creating a DC amplitude
                            response of one
        @param beta         an optional argument useful only for Kaiser's method
                            which must be positive and sets the properties of the
                            method (bandwidth and attenuation increases with beta)
    */
    void fillWindowingTables (size_t size, WindowingMethod type,
                              bool normalise = true, FloatType beta = 0) noexcept;

    /** Fills the content of an array with a given windowing method table.

        @param samples      the destination buffer pointer
        @param size         the size of the destination buffer allocated in the object
        @param normalise    if the result must be normalised, creating a DC amplitude
                            response of one
        @param beta         an optional argument useful only for Kaiser's method,
                            which must be positive and sets the properties of the
                            method (bandwidth and attenuation increases with beta)
    */
    static void fillWindowingTables (FloatType* samples, size_t size, WindowingMethod,
                                     bool normalise = true, FloatType beta = 0) noexcept;

    /** Multiplies the content of a buffer with the given window. */
    void multiplyWithWindowingTable (FloatType* samples, size_t size) const noexcept;

    /** Returns the name of a given windowing method. */
    static const char* getWindowingMethodName (WindowingMethod) noexcept;


private:
    //==============================================================================
    Array<FloatType> windowTable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowingFunction)
};

} // namespace juce::dsp
