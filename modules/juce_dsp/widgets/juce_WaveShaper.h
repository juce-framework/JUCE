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

/**
    Applies waveshaping to audio samples as single samples or AudioBlocks.

    @tags{DSP}
*/
template <typename FloatType, typename Function = FloatType (*) (FloatType)>
struct WaveShaper
{
    Function functionToUse;

    //==============================================================================
    /** Called before processing starts. */
    void prepare (const ProcessSpec&) noexcept   {}

    //==============================================================================
    /** Returns the result of processing a single sample. */
    template <typename SampleType>
    SampleType JUCE_VECTOR_CALLTYPE processSample (SampleType inputSample) const noexcept
    {
        return functionToUse (inputSample);
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) const noexcept
    {
        if (context.isBypassed)
        {
            if (context.usesSeparateInputAndOutputBlocks())
                context.getOutputBlock().copyFrom (context.getInputBlock());
        }
        else
        {
            AudioBlock<FloatType>::process (context.getInputBlock(),
                                            context.getOutputBlock(),
                                            functionToUse);
        }
    }

    void reset() noexcept {}
};

//==============================================================================
#if JUCE_CXX17_IS_AVAILABLE && ! ((JUCE_MAC || JUCE_IOS) && JUCE_CLANG && __clang_major__ < 10)
template <typename Functor>
static WaveShaper<typename std::invoke_result<Functor>, Functor> CreateWaveShaper (Functor functionToUse)   { return {functionToUse}; }
#else
template <typename Functor>
static WaveShaper<typename std::result_of<Functor>, Functor> CreateWaveShaper (Functor functionToUse)   { return {functionToUse}; }
#endif

} // namespace dsp
} // namespace juce
