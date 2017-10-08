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

#ifndef DOXYGEN
namespace SampleTypeHelpers // Internal classes needed for handling sample type classes
{
    template <typename Container> struct ElementType { using Type = typename Container::value_type; };
    template <> struct ElementType<float>            { using Type = float;  };
    template <> struct ElementType<double>           { using Type = double; };
    template <> struct ElementType<long double>      { using Type = long double; };
}
#endif

//==============================================================================
/**
    Minimal and lightweight data-structure which contains a list of pointers to
    channels containing some kind of sample data.

    This class doesn't own any of the data which it points to, it's simply a view
    into data that is owned elsewhere. You can construct one from some raw data
    that you've allocated yourself, or give it a HeapBlock to use, or give it
    an AudioSampleBuffer which it can refer to, but in all cases the user is
    responsible for making sure that the data doesn't get deleted while there's
    still an AudioBlock using it.
*/
template <typename SampleType>
class AudioBlock
{
public:
    //==============================================================================
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

    //==============================================================================
    /** Create a zero-sized AudioBlock. */
    forcedinline AudioBlock() noexcept {}

    /** Creates an AudioBlock from a pointer to an array of channels.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the memory is retained
        throughout the life-time of the AudioBlock and released when no longer needed.
    */
    forcedinline AudioBlock (SampleType* const* channelData,
                             size_t numberOfChannels, size_t numberOfSamples) noexcept
        : channels (channelData),
          numChannels (static_cast<ChannelCountType> (numberOfChannels)),
          numSamples (numberOfSamples)
    {
    }

    /** Creates an AudioBlock from a pointer to an array of channels.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the memory is retained
        throughout the life-time of the AudioBlock and released when no longer needed.
    */
    forcedinline AudioBlock (SampleType* const* channelData, size_t numberOfChannels,
                             size_t startSampleIndex, size_t numberOfSamples) noexcept
        : channels (channelData),
          numChannels (static_cast<ChannelCountType> (numberOfChannels)),
          startSample (startSampleIndex),
          numSamples (numberOfSamples)
    {
    }

    /** Allocates a suitable amount of space in a HeapBlock, and initialises this object
        to point into it.
        The HeapBlock must of course not be freed or re-allocated while this object is still in
        use, because it will be referencing its data.
    */
    AudioBlock (HeapBlock<char>& heapBlockToUseForAllocation,
                size_t numberOfChannels, size_t numberOfSamples) noexcept
        : numChannels (static_cast<ChannelCountType> (numberOfChannels)),
          numSamples (numberOfSamples)
    {
        auto roundedUpNumSamples = (numberOfSamples + elementMask) & ~elementMask;
        auto channelSize = sizeof (SampleType) * roundedUpNumSamples;
        auto channelListBytes = sizeof (SampleType*) * numberOfChannels;
        auto extraBytes = sizeof (SampleType) - 1;

        heapBlockToUseForAllocation.malloc (channelListBytes + extraBytes + channelSize * numberOfChannels);

        auto* chanArray = reinterpret_cast<SampleType**> (heapBlockToUseForAllocation.getData());
        channels = chanArray;

        auto* data = reinterpret_cast<SampleType*> (addBytesToPointer (chanArray, channelListBytes));
        data = snapPointerToAlignment (data, sizeof (SampleType));

        for (ChannelCountType i = 0; i < numChannels; ++i)
        {
            chanArray[i] = data;
            data += roundedUpNumSamples;
        }
    }

    /** Creates an AudioBlock that points to the data in an AudioBuffer.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the buffer is retained
        throughout the life-time of the AudioBlock without being modified.
    */
    AudioBlock (AudioBuffer<SampleType>& buffer) noexcept
        : channels (buffer.getArrayOfWritePointers()),
          numChannels (static_cast<ChannelCountType> (buffer.getNumChannels())),
          numSamples (static_cast<size_t> (buffer.getNumSamples()))
    {
    }

    /** Creates an AudioBlock that points to the data in an AudioBuffer.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the buffer is retained
        throughout the life-time of the AudioBlock without being modified.
    */
    AudioBlock (AudioBuffer<SampleType>& buffer, size_t startSampleIndex) noexcept
        : channels (buffer.getArrayOfWritePointers()),
          numChannels (static_cast<ChannelCountType> (buffer.getNumChannels())),
          startSample (startSampleIndex),
          numSamples (static_cast<size_t> (buffer.getNumSamples()))
    {
        jassert (startSample < numSamples);
    }

    AudioBlock (const AudioBlock& other) noexcept = default;
    AudioBlock& operator= (const AudioBlock& other) noexcept = default;

    //==============================================================================
    forcedinline size_t getNumSamples() const noexcept           { return numSamples; }
    forcedinline size_t getNumChannels() const noexcept          { return static_cast<size_t> (numChannels); }

    /** Returns a raw pointer into one of the channels in this block. */
    forcedinline const SampleType* getChannelPointer (size_t channel) const noexcept
    {
        jassert (channel < numChannels);
        jassert (numSamples > 0);
        return *(channels + channel) + startSample;
    }

    /** Returns a raw pointer into one of the channels in this block. */
    forcedinline SampleType* getChannelPointer (size_t channel) noexcept
    {
        jassert (channel < numChannels);
        jassert (numSamples > 0);
        return *(channels + channel) + startSample;
    }

    /** Returns an AudioBlock that represents one of the channels in this block. */
    forcedinline AudioBlock<SampleType> getSingleChannelBlock (size_t channel) const noexcept
    {
        jassert (channel < numChannels);
        return AudioBlock (channels + channel, 1, startSample, numSamples);
    }

    /** Returns a subset of continguous channels
        @param channelStart       First channel of the subset
        @param numChannelsToUse   Count of channels in the subset
    */
    forcedinline AudioBlock<SampleType> getSubsetChannelBlock (size_t channelStart, size_t numChannelsToUse) noexcept
    {
        jassert (channelStart < numChannels);
        jassert ((channelStart + numChannelsToUse) <= numChannels);

        return AudioBlock (channels + channelStart, numChannelsToUse, startSample, numSamples);
    }

    //==============================================================================
    /** Clear the memory described by this AudioBlock. */
    forcedinline AudioBlock& clear() noexcept
    {
        auto n = static_cast<int> (numSamples * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::clear (channelPtr (ch), n);

        return *this;
    }

    /** Fill memory with value. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE fill (SampleType value) noexcept
    {
        auto n = static_cast<int> (numSamples * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::fill (channelPtr (ch), value, n);

        return *this;
    }

    /** Copy the values in src to the receiver. */
    forcedinline AudioBlock& copy (const AudioBlock& src) noexcept
    {
        auto maxChannels = jmin (src.numChannels, numChannels);
        auto n = static_cast<int> (jmin (src.numSamples, numSamples) * sizeFactor);

        for (size_t ch = 0; ch < maxChannels; ++ch)
            FloatVectorOperations::copy (channelPtr (ch), src.channelPtr (ch), n);

        return *this;
    }

    /** Move memory within the receiver from the position srcPos to the position dstPos.
        If numElements is not specified then move will move the maximum amount of memory.
    */
    forcedinline AudioBlock& move (size_t srcPos, size_t dstPos,
                                   size_t numElements = std::numeric_limits<size_t>::max()) noexcept
    {
        jassert (srcPos <= numSamples && dstPos <= numSamples);
        auto len = jmin (numSamples - srcPos, numSamples - dstPos, numElements) * sizeof (SampleType);

        if (len != 0)
            for (size_t ch = 0; ch < numChannels; ++ch)
                ::memmove (getChannelPointer (ch) + dstPos,
                           getChannelPointer (ch) + srcPos, len);

        return *this;
    }

    //==============================================================================
    /** Return a new AudioBlock pointing to a sub-block inside the receiver. This
        function does not copy the memory and you must ensure that the original memory
        pointed to by the receiver remains valid through-out the life-time of the
        returned sub-block.

        @param newOffset   The index of an element inside the reciever which will
                           will become the first element of the return value.
        @param newLength   The number of elements of the newly created sub-block.
    */
    inline AudioBlock getSubBlock (size_t newOffset, size_t newLength) const noexcept
    {
        jassert (newOffset < numSamples);
        jassert (newOffset + newLength <= numSamples);

        return AudioBlock (channels, numChannels, startSample + newOffset, newLength);
    }

    /** Return a new AudioBlock pointing to a sub-block inside the receiver. This
        function does not copy the memory and you must ensure that the original memory
        pointed to by the receiver remains valid through-out the life-time of the
        returned sub-block.

        @param newOffset   The index of an element inside the reciever which will
                           will become the first element of the return value.
                           The return value will include all subsequent elements
                           of the receiver.
    */
    inline AudioBlock getSubBlock (size_t newOffset) const noexcept
    {
        return getSubBlock (newOffset, getNumSamples() - newOffset);
    }

    //==============================================================================
    /** Adds a fixed value to the receiver. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE add (SampleType value) noexcept
    {
        auto n = static_cast<int> (numSamples * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (channelPtr (ch), value, n);

        return *this;
    }

    /** Adds the source values to the receiver. */
    forcedinline AudioBlock& add (const AudioBlock& src) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (channelPtr (ch), src.channelPtr (ch), n);

        return *this;
    }

    /** Adds a fixed value to each source value and stores it in the destination array of the receiver. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE add (const AudioBlock& src, SampleType value) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (channelPtr (ch), src.channelPtr (ch), value, n);

        return *this;
    }

    /** Adds each source1 value to the corresponding source2 value and stores it in the destination array of the receiver. */
    forcedinline AudioBlock& add (const AudioBlock& src1, const AudioBlock& src2) noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (channelPtr (ch), src1.channelPtr (ch), src2.getChannelPointer (ch), n);

        return *this;
    }

    /** Subtracts a fixed value from the receiver. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE subtract (SampleType value) noexcept
    {
        return add (value * static_cast<SampleType> (-1.0));
    }

    /** Subtracts the source values from the receiver. */
    forcedinline AudioBlock& subtract (const AudioBlock& src) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::subtract (channelPtr (ch), src.channelPtr (ch), n);

        return *this;
    }

    /** Subtracts a fixed value from each source value and stores it in the destination array of the receiver. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE subtract (const AudioBlock& src, SampleType value) noexcept
    {
        return add (src, static_cast<SampleType> (-1.0) * value);
    }

    /** Subtracts each source2 value from the corresponding source1 value and stores it in the destination array of the receiver. */
    forcedinline AudioBlock& subtract (const AudioBlock& src1, const AudioBlock& src2) noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::subtract (channelPtr (ch), src1.channelPtr (ch), src2.channelPtr (ch), n);
        return *this;
    }

    /** Multiplies a fixed value to the receiver. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE multiply (SampleType value) noexcept
    {
        auto n = static_cast<int> (numSamples * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (channelPtr (ch), value, n);

        return *this;
    }

    /** Multiplies the source values to the receiver. */
    forcedinline AudioBlock& multiply (const AudioBlock& src) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (channelPtr (ch), src.channelPtr (ch), n);

        return *this;
    }

    /** Multiplies a fixed value to each source value and stores it in the destination array of the receiver. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE multiply (const AudioBlock& src, SampleType value) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (channelPtr (ch), src.channelPtr (ch), value, n);

        return *this;
    }

    /** Multiplies each source1 value to the corresponding source2 value and stores it in the destination array of the receiver. */
    forcedinline AudioBlock& multiply (const AudioBlock& src1, const AudioBlock& src2) noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (channelPtr (ch), src1.channelPtr (ch), src2.channelPtr (ch), n);
        return *this;
    }

    /** Multiplies each value in src with factor and adds the result to the receiver. */
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE addWithMultiply (const AudioBlock& src, SampleType factor) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::addWithMultiply (channelPtr (ch), src.channelPtr (ch), factor, n);

        return *this;
    }

    /** Multiplies each value in srcA with the corresponding value in srcB and adds the result to the receiver. */
    forcedinline AudioBlock& addWithMultiply (const AudioBlock& src1, const AudioBlock& src2) noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::addWithMultiply (channelPtr (ch), src1.channelPtr (ch), src2.channelPtr (ch), n);

        return *this;
    }

    /** Negates each value of the receiver. */
    forcedinline AudioBlock& negate() noexcept
    {
        return multiply (static_cast<SampleType> (-1.0));
    }

    /** Negates each value of source and stores it in the receiver. */
    forcedinline AudioBlock& replaceWithNegativeOf (const AudioBlock& src) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::negate (channelPtr (ch), src.channelPtr (ch), n);

        return *this;
    }

    /** Takes the absolute value of each element of src and stores it inside the receiver. */
    forcedinline AudioBlock& replaceWithAbsoluteValueOf (const AudioBlock& src) noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = static_cast<int> (jmin (numSamples, src.numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::abs (channelPtr (ch), src.channelPtr (ch), n);

        return *this;
    }

    /** Each element of receiver will be the minimum of the corresponding element of the source arrays. */
    forcedinline AudioBlock& min (const AudioBlock& src1, const AudioBlock& src2) noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = static_cast<int> (jmin (src1.numSamples, src2.numSamples, numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::min (channelPtr (ch), src1.channelPtr (ch), src2.channelPtr (ch), n);

        return *this;
    }

    /** Each element of the receiver will be the maximum of the corresponding element of the source arrays. */
    forcedinline AudioBlock& max (AudioBlock src1, AudioBlock src2) noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = static_cast<int> (jmin (src1.numSamples, src2.numSamples, numSamples) * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::max (channelPtr (ch), src1.channelPtr (ch), src2.channelPtr (ch), n);

        return *this;
    }

    /** Finds the minimum and maximum value of the buffer. */
    forcedinline Range<NumericType> findMinAndMax() const noexcept
    {
        Range<NumericType> minmax;
        auto n = static_cast<int> (numSamples * sizeFactor);

        for (size_t ch = 0; ch < numChannels; ++ch)
            minmax = minmax.getUnionWith (FloatVectorOperations::findMinAndMax (channelPtr (ch), n));

        return minmax;
    }

    //==============================================================================
    // convenient operator wrappers
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE operator+= (SampleType src) noexcept   { return add (src); }
    forcedinline AudioBlock&                      operator+= (AudioBlock src) noexcept   { return add (src); }
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE operator-= (SampleType src) noexcept   { return subtract (src); }
    forcedinline AudioBlock&                      operator-= (AudioBlock src) noexcept   { return subtract (src); }
    forcedinline AudioBlock& JUCE_VECTOR_CALLTYPE operator*= (SampleType src) noexcept   { return multiply (src); }
    forcedinline AudioBlock&                      operator*= (AudioBlock src) noexcept   { return multiply (src); }

    //==============================================================================
    // This class can only be used with floating point types
    static_assert (std::is_same<SampleType, float>::value                || std::is_same<SampleType, double>::value
                  #if JUCE_USE_SIMD
                   || std::is_same<SampleType, SIMDRegister<float> >::value || std::is_same<SampleType, SIMDRegister<double> >::value
                  #endif
                   , "AudioBlock only supports single or double precision floating point types");

    //==============================================================================
    template <typename FunctionType>
    static void process (const AudioBlock<SampleType>& inBlock,
                         AudioBlock<SampleType>& outBlock,
                         const FunctionType& function)
    {
        auto len = inBlock.getNumSamples();
        auto numChans = inBlock.getNumChannels();

        for (ChannelCountType c = 0; c < numChans; ++c)
        {
            auto* src = inBlock.getChannelPointer(c);
            auto* dst = outBlock.getChannelPointer(c);

            for (size_t i = 0; i < len; ++i)
                dst[i] = function (src[i]);
        }
    }

private:
    //==============================================================================
    NumericType*       channelPtr (size_t ch) noexcept          { return reinterpret_cast<NumericType*>       (getChannelPointer (ch)); }
    const NumericType* channelPtr (size_t ch) const noexcept    { return reinterpret_cast<const NumericType*> (getChannelPointer (ch)); }

    //==============================================================================
    using ChannelCountType = unsigned int;

    //==============================================================================
    static constexpr size_t sizeFactor  = sizeof (SampleType) / sizeof (NumericType);
    static constexpr size_t elementMask = sizeFactor - 1;
    static constexpr size_t byteMask    = (sizeFactor * sizeof (NumericType)) - 1;

    SampleType* const* channels;
    ChannelCountType numChannels = 0;
    size_t startSample = 0, numSamples = 0;
};

} // namespace dsp
} // namespace juce
