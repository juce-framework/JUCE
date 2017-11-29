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
    This structure is passed into a DSP algorithm's prepare() method, and contains
    information about various aspects of the context in which it can expect to be called.
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

//==============================================================================
/**
    This is a handy base class for the state of a processor (such as parameter values)
    which is typically shared among several procoessors. This is useful to for
    multi-mono filters which share the same state among several mono processors.
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
*/
template <typename ContextSampleType>
struct ProcessContextReplacing
{
public:
    /** The type of a single sample (which may be a vector if multichannel). */
    using SampleType     = ContextSampleType;
    /** The type of audio block that this context handles. */
    using AudioBlockType = AudioBlock<SampleType>;

    /** Creates a ProcessContextReplacing that uses the given audio block.
        Note that the caller must not delete the block while it is still in use by this object!
    */
    ProcessContextReplacing (AudioBlockType& block) noexcept : ioBlock (block) {}

    ProcessContextReplacing (const ProcessContextReplacing&) = default;
    ProcessContextReplacing (ProcessContextReplacing&&) = default;

    /** Returns the audio block to use as the input to a process function. */
    const AudioBlockType& getInputBlock() const noexcept        { return ioBlock; }

    /** Returns the audio block to use as the output to a process function. */
    AudioBlockType& getOutputBlock() const noexcept             { return const_cast<AudioBlockType&> (ioBlock); }

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
};

//==============================================================================
/**
    Contains context information that is passed into an algorithm's process method.

    This context is intended for use in situations where two different blocks are being
    used the input and output to the process algorithm, so the processor must read from
    the block returned by getInputBlock() and write its results to the block returned by
    getOutputBlock().

    @see ProcessContextReplacing
*/
template <typename ContextSampleType>
struct ProcessContextNonReplacing
{
public:
    /** The type of a single sample (which may be a vector if multichannel). */
    using SampleType     = ContextSampleType;
    /** The type of audio block that this context handles. */
    using AudioBlockType = AudioBlock<SampleType>;

    /** Creates a ProcessContextReplacing that uses the given input and output blocks.
        Note that the caller must not delete these blocks while they are still in use by this object!
    */
    ProcessContextNonReplacing (const AudioBlockType& input, AudioBlockType& output) noexcept
        : inputBlock (input), outputBlock (output) {}

    ProcessContextNonReplacing (const ProcessContextNonReplacing&) = default;
    ProcessContextNonReplacing (ProcessContextNonReplacing&&) = default;

    /** Returns the audio block to use as the input to a process function. */
    const AudioBlockType& getInputBlock() const noexcept        { return inputBlock; }

    /** Returns the audio block to use as the output to a process function. */
    AudioBlockType& getOutputBlock() const noexcept             { return const_cast<AudioBlockType&> (outputBlock); }

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
    const AudioBlockType& inputBlock;
    AudioBlockType& outputBlock;
};

} // namespace dsp
} // namespace juce
