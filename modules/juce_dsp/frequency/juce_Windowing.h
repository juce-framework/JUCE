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
