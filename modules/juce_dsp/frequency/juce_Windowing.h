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
    A class which provides multiple windowing functions useful for filter design
    and spectrum analyzers
*/
template <typename FloatType>
struct WindowingFunction
{
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
    WindowingFunction (size_t size, WindowingMethod,
                       bool normalize = true, FloatType beta = 0);

    //==============================================================================
    /** Fills the content of an array with a given windowing method table */
    void fillWindowingTables (size_t size, WindowingMethod type,
                              bool normalize = true, FloatType beta = 0) noexcept;

    /** Fills the content of an array with a given windowing method table */
    static void fillWindowingTables (FloatType* samples, size_t size, WindowingMethod,
                                     bool normalize = true, FloatType beta = 0) noexcept;

    /** Multiply the content of a buffer with the given window */
    void multiplyWithWindowingTable (FloatType* samples, size_t size) noexcept;

    /** Returns the name of a given windowing method */
    static const char* getWindowingMethodName (WindowingMethod) noexcept;


private:
    //==============================================================================
    Array<FloatType> windowTable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowingFunction)
};

} // namespace dsp
} // namespace juce
