/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
// Although clang supports C++17, their standard library still has no invoke_result
// support. Remove the "|| JUCE_CLANG" once clang supports this properly!
#if (! JUCE_CXX17_IS_AVAILABLE) || JUCE_CLANG
template <typename Functor>
static WaveShaper<typename std::result_of<Functor>, Functor> CreateWaveShaper (Functor functionToUse)   { return {functionToUse}; }
#else
template <typename Functor>
static WaveShaper<typename std::invoke_result<Functor>, Functor> CreateWaveShaper (Functor functionToUse)   { return {functionToUse}; }
#endif

} // namespace dsp
} // namespace juce
