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
    This structure is passed into a DSP algorithm's prepare() method, and contains
    information about various aspects of the context in which it can expect to be called.

    @tags{DSP}
*/
struct ProcessSpec
{
    /** The sample rate that will be used for the data that is sent to the processor. */
    double sampleRate;

    /** The maximum number of samples that will be in the blocks sent to process() method. */
    uint32 maximumBlockSize;

    /** The number of channels that the process() method will be expected to handle. */
    uint32 numChannels;
};

constexpr bool operator== (const ProcessSpec& a, const ProcessSpec& b)
{
    const auto tie = [] (const ProcessSpec& p)
    {
        return std::tie (p.sampleRate, p.maximumBlockSize, p.numChannels);
    };

    return tie (a) == tie (b);
}

constexpr bool operator!= (const ProcessSpec& a, const ProcessSpec& b) { return ! (a == b); }

//==============================================================================
/**
    This is a handy base class for the state of a processor (such as parameter values)
    which is typically shared among several processors. This is useful for multi-mono
    filters which share the same state among several mono processors.

    @tags{DSP}
*/
struct ProcessorState  : public ReferenceCountedObject
{
    /** The ProcessorState structure is ref-counted, so this is a handy type that can be used
        as a pointer to one.
    */
    using Ptr = ReferenceCountedObjectPtr<ProcessorState>;
};

//==============================================================================
/**
    Contains context information that is passed into an algorithm's process method.

    This context is intended for use in situations where a single block is being used
    for both the input and output, so it will return the same object for both its
    getInputBlock() and getOutputBlock() methods.

    @see ProcessContextNonReplacing

    @tags{DSP}
*/
template <typename ContextSampleType>
struct ProcessContextReplacing
{
public:
    /** The type of a single sample (which may be a vector if multichannel). */
    using SampleType     = ContextSampleType;
    /** The type of audio block that this context handles. */
    using AudioBlockType = AudioBlock<SampleType>;
    using ConstAudioBlockType = AudioBlock<const SampleType>;

    /** Creates a ProcessContextReplacing that uses the given audio block.
        Note that the caller must not delete the block while it is still in use by this object!
    */
    ProcessContextReplacing (AudioBlockType& block) noexcept : ioBlock (block) {}

    ProcessContextReplacing (const ProcessContextReplacing&) = default;
    ProcessContextReplacing (ProcessContextReplacing&&) = default;

    /** Returns the audio block to use as the input to a process function. */
    const ConstAudioBlockType& getInputBlock() const noexcept   { return constBlock; }

    /** Returns the audio block to use as the output to a process function. */
    AudioBlockType& getOutputBlock() const noexcept             { return ioBlock; }

    /** All process context classes will define this constant method so that templated
        code can determine whether the input and output blocks refer to the same buffer,
        or to two different ones.
    */
    static constexpr bool usesSeparateInputAndOutputBlocks()    { return false; }

    /** If set to true, then a processor's process() method is expected to do whatever
        is appropriate for it to be in a bypassed state.
    */
    bool isBypassed = false;

private:
    AudioBlockType& ioBlock;
    ConstAudioBlockType constBlock { ioBlock };
};

//==============================================================================
/**
    Contains context information that is passed into an algorithm's process method.

    This context is intended for use in situations where two different blocks are being
    used the input and output to the process algorithm, so the processor must read from
    the block returned by getInputBlock() and write its results to the block returned by
    getOutputBlock().

    @see ProcessContextReplacing

    @tags{DSP}
*/
template <typename ContextSampleType>
struct ProcessContextNonReplacing
{
public:
    /** The type of a single sample (which may be a vector if multichannel). */
    using SampleType     = ContextSampleType;
    /** The type of audio block that this context handles. */
    using AudioBlockType = AudioBlock<SampleType>;
    using ConstAudioBlockType = AudioBlock<const SampleType>;

    /** Creates a ProcessContextNonReplacing that uses the given input and output blocks.
        Note that the caller must not delete these blocks while they are still in use by this object!
    */
    ProcessContextNonReplacing (const ConstAudioBlockType& input, AudioBlockType& output) noexcept
        : inputBlock (input), outputBlock (output)
    {
        // If the input and output blocks are the same then you should use
        // ProcessContextReplacing instead.
        jassert (input != output);
    }

    ProcessContextNonReplacing (const ProcessContextNonReplacing&) = default;
    ProcessContextNonReplacing (ProcessContextNonReplacing&&) = default;

    /** Returns the audio block to use as the input to a process function. */
    const ConstAudioBlockType& getInputBlock() const noexcept   { return inputBlock; }

    /** Returns the audio block to use as the output to a process function. */
    AudioBlockType& getOutputBlock() const noexcept             { return outputBlock; }

    /** All process context classes will define this constant method so that templated
        code can determine whether the input and output blocks refer to the same buffer,
        or to two different ones.
    */
    static constexpr bool usesSeparateInputAndOutputBlocks()    { return true; }

    /** If set to true, then a processor's process() method is expected to do whatever
        is appropriate for it to be in a bypassed state.
    */
    bool isBypassed = false;

private:
    ConstAudioBlockType inputBlock;
    AudioBlockType& outputBlock;
};

} // namespace dsp
} // namespace juce
