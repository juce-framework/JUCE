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

#ifndef DOXYGEN
namespace SampleTypeHelpers // Internal classes needed for handling sample type classes
{
    template <typename T, bool = std::is_floating_point<T>::value>
    struct ElementType
    {
        using Type = T;
    };

    template <typename T>
    struct ElementType<const T, false>
    {
        using Type = const typename T::value_type;
    };

    template <typename T>
    struct ElementType<T, false>
    {
        using Type = typename T::value_type;
    };
}
#endif

//==============================================================================
/**
    Minimal and lightweight data-structure which contains a list of pointers to
    channels containing some kind of sample data.

    This class doesn't own any of the data which it points to, it's simply a view
    into data that is owned elsewhere. You can construct one from some raw data
    that you've allocated yourself, or give it a HeapBlock to use, or give it
    an AudioBuffer which it can refer to, but in all cases the user is
    responsible for making sure that the data doesn't get deleted while there's
    still an AudioBlock using it.

    @tags{DSP}
*/
template <typename SampleType>
class AudioBlock
{
private:
    template <typename OtherSampleType>
    using MayUseConvertingConstructor =
        std::enable_if_t<std::is_same<std::remove_const_t<SampleType>,
                                      std::remove_const_t<OtherSampleType>>::value
                             && std::is_const<SampleType>::value
                             && ! std::is_const<OtherSampleType>::value,
                         int>;

public:
    //==============================================================================
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

    //==============================================================================
    /** Create a zero-sized AudioBlock. */
    AudioBlock() noexcept = default;

    /** Creates an AudioBlock from a pointer to an array of channels.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the memory is retained
        throughout the life-time of the AudioBlock and released when no longer needed.
    */
    constexpr AudioBlock (SampleType* const* channelData,
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
    constexpr AudioBlock (SampleType* const* channelData, size_t numberOfChannels,
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
                size_t numberOfChannels, size_t numberOfSamples,
                size_t alignmentInBytes = defaultAlignment) noexcept
        : numChannels (static_cast<ChannelCountType> (numberOfChannels)),
          numSamples (numberOfSamples)
    {
        auto roundedUpNumSamples = (numberOfSamples + elementMask) & ~elementMask;
        auto channelSize = sizeof (SampleType) * roundedUpNumSamples;
        auto channelListBytes = sizeof (SampleType*) * numberOfChannels;
        auto extraBytes = alignmentInBytes - 1;

        heapBlockToUseForAllocation.malloc (channelListBytes + extraBytes + channelSize * numberOfChannels);

        auto* chanArray = unalignedPointerCast<SampleType**> (heapBlockToUseForAllocation.getData());
        channels = chanArray;

        auto* data = unalignedPointerCast<SampleType*> (addBytesToPointer (chanArray, channelListBytes));
        data = snapPointerToAlignment (data, alignmentInBytes);

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
    template <typename OtherSampleType>
    constexpr AudioBlock (AudioBuffer<OtherSampleType>& buffer) noexcept
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
    template <typename OtherSampleType>
    constexpr AudioBlock (const AudioBuffer<OtherSampleType>& buffer) noexcept
        : channels (buffer.getArrayOfReadPointers()),
          numChannels (static_cast<ChannelCountType> (buffer.getNumChannels())),
          numSamples (static_cast<size_t> (buffer.getNumSamples()))
    {
    }

    /** Creates an AudioBlock that points to the data in an AudioBuffer.
        AudioBlock does not copy nor own the memory pointed to by dataToUse.
        Therefore it is the user's responsibility to ensure that the buffer is retained
        throughout the life-time of the AudioBlock without being modified.
    */
    template <typename OtherSampleType>
    AudioBlock (AudioBuffer<OtherSampleType>& buffer, size_t startSampleIndex) noexcept
        : channels (buffer.getArrayOfWritePointers()),
          numChannels (static_cast<ChannelCountType> (buffer.getNumChannels())),
          startSample (startSampleIndex),
          numSamples (static_cast<size_t> (buffer.getNumSamples()) - startSampleIndex)
    {
        jassert (startSample < static_cast<size_t> (buffer.getNumSamples()));
    }

    AudioBlock (const AudioBlock& other) noexcept = default;
    AudioBlock& operator= (const AudioBlock& other) noexcept = default;

    template <typename OtherSampleType, MayUseConvertingConstructor<OtherSampleType> = 0>
    AudioBlock (const AudioBlock<OtherSampleType>& other) noexcept
        : channels { other.channels },
          numChannels { other.numChannels },
          startSample { other.startSample },
          numSamples { other.numSamples }
    {
    }

    template <typename OtherSampleType, MayUseConvertingConstructor<OtherSampleType> = 0>
    AudioBlock& operator= (const AudioBlock<OtherSampleType>& other) noexcept
    {
        AudioBlock blockCopy { other };
        swap (blockCopy);
        return *this;
    }

    void swap (AudioBlock& other) noexcept
    {
        std::swap (other.channels, channels);
        std::swap (other.numChannels, numChannels);
        std::swap (other.startSample, startSample);
        std::swap (other.numSamples, numSamples);
    }

    //==============================================================================
    template <typename OtherSampleType>
    constexpr bool operator== (const AudioBlock<OtherSampleType>& other) const noexcept
    {
        return std::equal (channels,
                           channels + numChannels,
                           other.channels,
                           other.channels + other.numChannels)
               && startSample == other.startSample
               && numSamples == other.numSamples;
    }

    template <typename OtherSampleType>
    constexpr bool operator!= (const AudioBlock<OtherSampleType>& other) const noexcept
    {
        return ! (*this == other);
    }

    //==============================================================================
    /** Returns the number of channels referenced by this block. */
    constexpr size_t getNumChannels() const noexcept          { return static_cast<size_t> (numChannels); }

    /** Returns the number of samples referenced by this block. */
    constexpr size_t getNumSamples()  const noexcept          { return numSamples; }

    /** Returns a raw pointer into one of the channels in this block. */
    SampleType* getChannelPointer (size_t channel) const noexcept
    {
        jassert (channel < numChannels);
        jassert (numSamples > 0);
        return channels[channel] + startSample;
    }

    /** Returns an AudioBlock that represents one of the channels in this block. */
    AudioBlock getSingleChannelBlock (size_t channel) const noexcept
    {
        jassert (channel < numChannels);
        return AudioBlock (channels + channel, 1, startSample, numSamples);
    }

    /** Returns a subset of contiguous channels
        @param channelStart       First channel of the subset
        @param numChannelsToUse   Count of channels in the subset
    */
    AudioBlock getSubsetChannelBlock (size_t channelStart, size_t numChannelsToUse) const noexcept
    {
        jassert (channelStart < numChannels);
        jassert ((channelStart + numChannelsToUse) <= numChannels);

        return AudioBlock (channels + channelStart, numChannelsToUse, startSample, numSamples);
    }

    /** Returns a sample from the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    SampleType getSample (int channel, int sampleIndex) const noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (isPositiveAndBelow (sampleIndex, numSamples));
        return channels[channel][(size_t) startSample + (size_t) sampleIndex];
    }

    /** Modifies a sample in the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    void setSample (int destChannel, int destSample, SampleType newValue) const noexcept
    {
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (isPositiveAndBelow (destSample, numSamples));
        channels[destChannel][(size_t) startSample + (size_t) destSample] = newValue;
    }

    /** Adds a value to a sample in the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    void addSample (int destChannel, int destSample, SampleType valueToAdd) const noexcept
    {
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (isPositiveAndBelow (destSample, numSamples));
        channels[destChannel][(size_t) startSample + (size_t) destSample] += valueToAdd;
    }

    //==============================================================================
    /** Clears the memory referenced by this AudioBlock. */
    AudioBlock&       clear()       noexcept   { clearInternal(); return *this; }
    const AudioBlock& clear() const noexcept   { clearInternal(); return *this; }

    /** Fills the memory referenced by this AudioBlock with value. */
    AudioBlock&       JUCE_VECTOR_CALLTYPE fill (NumericType value)       noexcept   { fillInternal (value); return *this; }
    const AudioBlock& JUCE_VECTOR_CALLTYPE fill (NumericType value) const noexcept   { fillInternal (value); return *this; }

    /** Copies the values in src to this block. */
    template <typename OtherSampleType>
    AudioBlock&       copyFrom (const AudioBlock<OtherSampleType>& src)       noexcept   { copyFromInternal (src); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& copyFrom (const AudioBlock<OtherSampleType>& src) const noexcept   { copyFromInternal (src); return *this; }

    /** Copy the values from an AudioBuffer to this block.

        All indices and sizes are in this AudioBlock's units, i.e. if SampleType is a
        SIMDRegister then incrementing srcPos by one will increase the sample position
        in the AudioBuffer's units by a factor of SIMDRegister<SampleType>::SIMDNumElements.
    */
    template <typename OtherNumericType>
    AudioBlock&       copyFrom (const AudioBuffer<OtherNumericType>& src,
                                size_t srcPos = 0, size_t dstPos = 0,
                                size_t numElements = std::numeric_limits<size_t>::max())         { copyFromInternal (src, srcPos, dstPos, numElements); return *this; }
    template <typename OtherNumericType>
    const AudioBlock& copyFrom (const AudioBuffer<OtherNumericType>& src,
                                size_t srcPos = 0, size_t dstPos = 0,
                                size_t numElements = std::numeric_limits<size_t>::max()) const   { copyFromInternal (src, srcPos, dstPos, numElements); return *this; }


    /** Copies the values from this block to an AudioBuffer.

        All indices and sizes are in this AudioBlock's units, i.e. if SampleType is a
        SIMDRegister then incrementing dstPos by one will increase the sample position
        in the AudioBuffer's units by a factor of SIMDRegister<SampleType>::SIMDNumElements.
    */
    void copyTo (AudioBuffer<typename std::remove_const<NumericType>::type>& dst, size_t srcPos = 0, size_t dstPos = 0,
                 size_t numElements = std::numeric_limits<size_t>::max()) const
    {
        auto dstlen = static_cast<size_t> (dst.getNumSamples()) / sizeFactor;
        auto n = jmin (numSamples - srcPos, dstlen - dstPos, numElements) * sizeFactor;
        auto maxChannels = jmin (static_cast<size_t> (dst.getNumChannels()), static_cast<size_t> (numChannels));

        for (size_t ch = 0; ch < maxChannels; ++ch)
            FloatVectorOperations::copy (dst.getWritePointer ((int) ch, (int) (dstPos * sizeFactor)),
                                         getDataPointer (ch) + (srcPos * sizeFactor),
                                         n);
    }

    /** Move memory within this block from the position srcPos to the position dstPos.
        If numElements is not specified then move will move the maximum amount of memory.
    */
    AudioBlock&       move (size_t srcPos, size_t dstPos,
                            size_t numElements = std::numeric_limits<size_t>::max())       noexcept   { moveInternal (srcPos, dstPos, numElements); return *this; }
    const AudioBlock& move (size_t srcPos, size_t dstPos,
                            size_t numElements = std::numeric_limits<size_t>::max()) const noexcept   { moveInternal (srcPos, dstPos, numElements); return *this; }

    //==============================================================================
    /** Return a new AudioBlock pointing to a sub-block inside this block. This
        function does not copy the memory and you must ensure that the original memory
        pointed to by the receiver remains valid through-out the life-time of the
        returned sub-block.

        @param newOffset   The index of an element inside the receiver which will
                           will become the first element of the return value.
        @param newLength   The number of elements of the newly created sub-block.
    */
    AudioBlock getSubBlock (size_t newOffset, size_t newLength) const noexcept
    {
        jassert (newOffset < numSamples);
        jassert (newOffset + newLength <= numSamples);

        return AudioBlock (channels, numChannels, startSample + newOffset, newLength);
    }

    /** Return a new AudioBlock pointing to a sub-block inside this block. This
        function does not copy the memory and you must ensure that the original memory
        pointed to by the receiver remains valid through-out the life-time of the
        returned sub-block.

        @param newOffset   The index of an element inside the block which will
                           will become the first element of the return value.
                           The return value will include all subsequent elements
                           of the receiver.
    */
    AudioBlock getSubBlock (size_t newOffset) const noexcept
    {
        return getSubBlock (newOffset, getNumSamples() - newOffset);
    }

    //==============================================================================
    /** Adds a fixed value to the elements in this block. */
    AudioBlock&       JUCE_VECTOR_CALLTYPE add (NumericType value)       noexcept   { addInternal (value); return *this; }
    const AudioBlock& JUCE_VECTOR_CALLTYPE add (NumericType value) const noexcept   { addInternal (value); return *this; }

    /** Adds the elements in the src block to the elements in this block. */
    template <typename OtherSampleType>
    AudioBlock&       add (AudioBlock<OtherSampleType> src)       noexcept   { addInternal (src); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& add (AudioBlock<OtherSampleType> src) const noexcept   { addInternal (src); return *this; }

    /** Adds a fixed value to each source value and replaces the contents of this block. */
    template <typename OtherSampleType>
    AudioBlock&       JUCE_VECTOR_CALLTYPE replaceWithSumOf (AudioBlock<OtherSampleType> src, NumericType value)      noexcept    { replaceWithSumOfInternal (src, value); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& JUCE_VECTOR_CALLTYPE replaceWithSumOf (AudioBlock<OtherSampleType> src, NumericType value) const noexcept   { replaceWithSumOfInternal (src, value); return *this; }

    /** Adds each source1 value to the corresponding source2 value and replaces the contents of this block. */
    template <typename Src1SampleType, typename Src2SampleType>
    AudioBlock&       replaceWithSumOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2)       noexcept   { replaceWithSumOfInternal (src1, src2); return *this; }
    template <typename Src1SampleType, typename Src2SampleType>
    const AudioBlock& replaceWithSumOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept   { replaceWithSumOfInternal (src1, src2); return *this; }

    //==============================================================================
    /** Subtracts a fixed value from the elements in this block. */
    AudioBlock&       JUCE_VECTOR_CALLTYPE subtract (NumericType value)       noexcept   { subtractInternal (value); return *this; }
    const AudioBlock& JUCE_VECTOR_CALLTYPE subtract (NumericType value) const noexcept   { subtractInternal (value); return *this; }

    /** Subtracts the source values from the elements in this block. */
    template <typename OtherSampleType>
    AudioBlock&       subtract (AudioBlock<OtherSampleType> src)       noexcept   { subtractInternal (src); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& subtract (AudioBlock<OtherSampleType> src) const noexcept   { subtractInternal (src); return *this; }

    /** Subtracts a fixed value from each source value and replaces the contents of this block. */
    template <typename OtherSampleType>
    AudioBlock&       JUCE_VECTOR_CALLTYPE replaceWithDifferenceOf (AudioBlock<OtherSampleType> src, NumericType value)       noexcept   { replaceWithDifferenceOfInternal (src, value); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& JUCE_VECTOR_CALLTYPE replaceWithDifferenceOf (AudioBlock<OtherSampleType> src, NumericType value) const noexcept   { replaceWithDifferenceOfInternal (src, value); return *this; }

    /** Subtracts each source2 value from the corresponding source1 value and replaces the contents of this block. */
    template <typename Src1SampleType, typename Src2SampleType>
    AudioBlock&       replaceWithDifferenceOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2)       noexcept   { replaceWithDifferenceOfInternal (src1, src2); return *this; }
    template <typename Src1SampleType, typename Src2SampleType>
    const AudioBlock& replaceWithDifferenceOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept   { replaceWithDifferenceOfInternal (src1, src2); return *this; }

    //==============================================================================
    /** Multiplies the elements in this block by a fixed value. */
    AudioBlock&       JUCE_VECTOR_CALLTYPE multiplyBy (NumericType value)       noexcept   { multiplyByInternal (value); return *this; }
    const AudioBlock& JUCE_VECTOR_CALLTYPE multiplyBy (NumericType value) const noexcept   { multiplyByInternal (value); return *this; }

    /** Multiplies the elements in this block by the elements in the src block */
    template <typename OtherSampleType>
    AudioBlock&       multiplyBy (AudioBlock<OtherSampleType> src)       noexcept   { multiplyByInternal (src); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& multiplyBy (AudioBlock<OtherSampleType> src) const noexcept   { multiplyByInternal (src); return *this; }

    /** Replaces the elements in this block with the product of the elements in the source src block and a fixed value. */
    template <typename OtherSampleType>
    AudioBlock&       JUCE_VECTOR_CALLTYPE replaceWithProductOf (AudioBlock<OtherSampleType> src, NumericType value)       noexcept   { replaceWithProductOfInternal (src, value); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& JUCE_VECTOR_CALLTYPE replaceWithProductOf (AudioBlock<OtherSampleType> src, NumericType value) const noexcept   { replaceWithProductOfInternal (src, value); return *this; }

    /** Replaces the elements in this block with the product of the elements in the src1 and scr2 blocks. */
    template <typename Src1SampleType, typename Src2SampleType>
    AudioBlock&       replaceWithProductOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2)       noexcept   { replaceWithProductOfInternal (src1, src2); return *this; }
    template <typename Src1SampleType, typename Src2SampleType>
    const AudioBlock& replaceWithProductOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept   { replaceWithProductOfInternal (src1, src2); return *this; }

    //==============================================================================
    /** Multiplies each channels of this block by a smoothly changing value. */
    template <typename OtherSampleType, typename SmoothingType>
    AudioBlock&       multiplyBy (SmoothedValue<OtherSampleType, SmoothingType>& value)       noexcept   { multiplyByInternal (value); return *this; }
    template <typename OtherSampleType, typename SmoothingType>
    const AudioBlock& multiplyBy (SmoothedValue<OtherSampleType, SmoothingType>& value) const noexcept   { multiplyByInternal (value); return *this; }

    /** Replaces each channel of this block with the product of the src block and a smoothed value. */
    template <typename BlockSampleType, typename SmootherSampleType, typename SmoothingType>
    AudioBlock&       replaceWithProductOf (AudioBlock<BlockSampleType> src, SmoothedValue<SmootherSampleType, SmoothingType>& value)       noexcept   { replaceWithProductOfInternal (src, value); return *this; }
    template <typename BlockSampleType, typename SmootherSampleType, typename SmoothingType>
    const AudioBlock& replaceWithProductOf (AudioBlock<BlockSampleType> src, SmoothedValue<SmootherSampleType, SmoothingType>& value) const noexcept   { replaceWithProductOfInternal (src, value); return *this; }

    //==============================================================================
    /** Multiplies each value in src by a fixed value and adds the result to this block. */
    template <typename OtherSampleType>
    AudioBlock&       JUCE_VECTOR_CALLTYPE addProductOf (AudioBlock<OtherSampleType> src, NumericType factor)       noexcept   { addProductOfInternal (src, factor); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& JUCE_VECTOR_CALLTYPE addProductOf (AudioBlock<OtherSampleType> src, NumericType factor) const noexcept   { addProductOfInternal (src, factor); return *this; }

    /** Multiplies each value in srcA with the corresponding value in srcB and adds the result to this block. */
    template <typename Src1SampleType, typename Src2SampleType>
    AudioBlock&       addProductOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2)       noexcept   { addProductOfInternal (src1, src2); return *this; }
    template <typename Src1SampleType, typename Src2SampleType>
    const AudioBlock& addProductOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept   { addProductOfInternal (src1, src2); return *this; }

    //==============================================================================
    /** Negates each value of this block. */
    AudioBlock&       negate()       noexcept   { negateInternal(); return *this; }
    const AudioBlock& negate() const noexcept   { negateInternal(); return *this; }

    /** Replaces the contents of this block with the negative of the values in the src block. */
    template <typename OtherSampleType>
    AudioBlock&       replaceWithNegativeOf (AudioBlock<OtherSampleType> src)       noexcept   { replaceWithNegativeOfInternal (src); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& replaceWithNegativeOf (AudioBlock<OtherSampleType> src) const noexcept   { replaceWithNegativeOfInternal (src); return *this; }

    /** Replaces the contents of this block with the absolute values of the src block. */
    template <typename OtherSampleType>
    AudioBlock&       replaceWithAbsoluteValueOf (AudioBlock<OtherSampleType> src)       noexcept   { replaceWithAbsoluteValueOfInternal (src); return *this; }
    template <typename OtherSampleType>
    const AudioBlock& replaceWithAbsoluteValueOf (AudioBlock<OtherSampleType> src) const noexcept   { replaceWithAbsoluteValueOfInternal (src); return *this; }

    //==============================================================================
    /** Replaces each element of this block with the minimum of the corresponding element of the source arrays. */
    template <typename Src1SampleType, typename Src2SampleType>
    AudioBlock&       replaceWithMinOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2)       noexcept   { replaceWithMinOfInternal (src1, src2); return *this; }
    template <typename Src1SampleType, typename Src2SampleType>
    const AudioBlock& replaceWithMinOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept   { replaceWithMinOfInternal (src1, src2); return *this; }

    /** Replaces each element of this block with the maximum of the corresponding element of the source arrays. */
    template <typename Src1SampleType, typename Src2SampleType>
    AudioBlock&       replaceWithMaxOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2)       noexcept   { replaceWithMaxOfInternal (src1, src2); return *this; }
    template <typename Src1SampleType, typename Src2SampleType>
    const AudioBlock& replaceWithMaxOf (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept   { replaceWithMaxOfInternal (src1, src2); return *this; }

    //==============================================================================
    /** Finds the minimum and maximum value of the buffer. */
    Range<typename std::remove_const<NumericType>::type> findMinAndMax() const noexcept
    {
        if (numChannels == 0)
            return {};

        auto n = numSamples * sizeFactor;
        auto minmax = FloatVectorOperations::findMinAndMax (getDataPointer (0), n);

        for (size_t ch = 1; ch < numChannels; ++ch)
            minmax = minmax.getUnionWith (FloatVectorOperations::findMinAndMax (getDataPointer (ch), n));

        return minmax;
    }

    //==============================================================================
    // Convenient operator wrappers.
    AudioBlock&       JUCE_VECTOR_CALLTYPE operator+= (NumericType value)       noexcept   { return add (value); }
    const AudioBlock& JUCE_VECTOR_CALLTYPE operator+= (NumericType value) const noexcept   { return add (value); }

    AudioBlock&                            operator+= (AudioBlock src)         noexcept   { return add (src); }
    const AudioBlock&                      operator+= (AudioBlock src)   const noexcept   { return add (src); }

    AudioBlock&       JUCE_VECTOR_CALLTYPE operator-= (NumericType value)       noexcept   { return subtract (value); }
    const AudioBlock& JUCE_VECTOR_CALLTYPE operator-= (NumericType value) const noexcept   { return subtract (value); }

    AudioBlock&                            operator-= (AudioBlock src)         noexcept   { return subtract (src); }
    const AudioBlock&                      operator-= (AudioBlock src)   const noexcept   { return subtract (src); }

    AudioBlock&       JUCE_VECTOR_CALLTYPE operator*= (NumericType value)       noexcept   { return multiplyBy (value); }
    const AudioBlock& JUCE_VECTOR_CALLTYPE operator*= (NumericType value) const noexcept   { return multiplyBy (value); }

    AudioBlock&                            operator*= (AudioBlock src)         noexcept   { return multiplyBy (src); }
    const AudioBlock&                      operator*= (AudioBlock src)   const noexcept   { return multiplyBy (src); }

    template <typename OtherSampleType, typename SmoothingType>
    AudioBlock&       operator*= (SmoothedValue<OtherSampleType, SmoothingType>& value)       noexcept   { return multiplyBy (value); }
    template <typename OtherSampleType, typename SmoothingType>
    const AudioBlock& operator*= (SmoothedValue<OtherSampleType, SmoothingType>& value) const noexcept   { return multiplyBy (value); }

    //==============================================================================
    // This class can only be used with floating point types
    static_assert (std::is_same<std::remove_const_t<SampleType>, float>::value
                    || std::is_same<std::remove_const_t<SampleType>, double>::value
                  #if JUCE_USE_SIMD
                    || std::is_same<std::remove_const_t<SampleType>, SIMDRegister<float>>::value
                    || std::is_same<std::remove_const_t<SampleType>, SIMDRegister<double>>::value
                  #endif
                   , "AudioBlock only supports single or double precision floating point types");

    //==============================================================================
    /** Applies a function to each value in an input block, putting the result into an output block.
        The function supplied must take a SampleType as its parameter, and return a SampleType.
        The two blocks must have the same number of channels and samples.
    */
    template <typename Src1SampleType, typename Src2SampleType, typename FunctionType>
    static void process (AudioBlock<Src1SampleType> inBlock, AudioBlock<Src2SampleType> outBlock, FunctionType&& function)
    {
        auto len = inBlock.getNumSamples();
        auto numChans = inBlock.getNumChannels();

        jassert (len == outBlock.getNumSamples());
        jassert (numChans == outBlock.getNumChannels());

        for (ChannelCountType c = 0; c < numChans; ++c)
        {
            auto* src = inBlock.getChannelPointer (c);
            auto* dst = outBlock.getChannelPointer (c);

            for (size_t i = 0; i < len; ++i)
                dst[i] = function (src[i]);
        }
    }

private:
    NumericType* getDataPointer (size_t channel) const noexcept
    {
        return reinterpret_cast<NumericType*> (getChannelPointer (channel));
    }

    //==============================================================================
    void JUCE_VECTOR_CALLTYPE clearInternal() const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::clear (getDataPointer (ch), n);
    }

    void JUCE_VECTOR_CALLTYPE fillInternal (NumericType value) const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::fill (getDataPointer (ch), value, n);
    }

    template <typename OtherSampleType>
    void copyFromInternal (const AudioBlock<OtherSampleType>& src) const noexcept
    {
        auto maxChannels = jmin (src.numChannels, numChannels);
        auto n = jmin (src.numSamples * src.sizeFactor, numSamples * sizeFactor);

        for (size_t ch = 0; ch < maxChannels; ++ch)
            FloatVectorOperations::copy (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherNumericType>
    void copyFromInternal (const AudioBuffer<OtherNumericType>& src, size_t srcPos, size_t dstPos, size_t numElements) const
    {
        auto srclen = static_cast<size_t> (src.getNumSamples()) / sizeFactor;
        auto n = jmin (srclen - srcPos, numSamples - dstPos, numElements) * sizeFactor;
        auto maxChannels = jmin (static_cast<size_t> (src.getNumChannels()), static_cast<size_t> (numChannels));

        for (size_t ch = 0; ch < maxChannels; ++ch)
            FloatVectorOperations::copy (getDataPointer (ch) + (dstPos * sizeFactor),
                                         src.getReadPointer ((int) ch, (int) (srcPos * sizeFactor)),
                                         n);
    }

    void moveInternal (size_t srcPos, size_t dstPos,
                       size_t numElements = std::numeric_limits<size_t>::max()) const noexcept
    {
        jassert (srcPos <= numSamples && dstPos <= numSamples);
        auto len = jmin (numSamples - srcPos, numSamples - dstPos, numElements) * sizeof (SampleType);

        if (len != 0)
            for (size_t ch = 0; ch < numChannels; ++ch)
                ::memmove (getChannelPointer (ch) + dstPos,
                           getChannelPointer (ch) + srcPos, len);
    }

    //==============================================================================
    void JUCE_VECTOR_CALLTYPE addInternal (NumericType value) const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (getDataPointer (ch), value, n);
    }

    template <typename OtherSampleType>
    void addInternal (AudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE replaceWithSumOfInternal (AudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (getDataPointer (ch), src.getDataPointer (ch), value, n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithSumOfInternal (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::add (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    constexpr void JUCE_VECTOR_CALLTYPE subtractInternal (NumericType value) const noexcept
    {
        addInternal (value * static_cast<NumericType> (-1.0));
    }

    template <typename OtherSampleType>
    void subtractInternal (AudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::subtract (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE replaceWithDifferenceOfInternal (AudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        replaceWithSumOfInternal (src, static_cast<NumericType> (-1.0) * value);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithDifferenceOfInternal (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::subtract (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    void JUCE_VECTOR_CALLTYPE multiplyByInternal (NumericType value) const noexcept
    {
        auto n = numSamples * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (getDataPointer (ch), value, n);
    }

    template <typename OtherSampleType>
    void multiplyByInternal (AudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE replaceWithProductOfInternal (AudioBlock<OtherSampleType> src, NumericType value) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (getDataPointer (ch), src.getDataPointer (ch), value, n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithProductOfInternal (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::multiply (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    template <typename OtherSampleType, typename SmoothingType>
    void multiplyByInternal (SmoothedValue<OtherSampleType, SmoothingType>& value) const noexcept
    {
        if (! value.isSmoothing())
        {
            multiplyByInternal ((NumericType) value.getTargetValue());
        }
        else
        {
            for (size_t i = 0; i < numSamples; ++i)
            {
                const auto scaler = (NumericType) value.getNextValue();

                for (size_t ch = 0; ch < numChannels; ++ch)
                    getDataPointer (ch)[i] *= scaler;
            }
        }
    }

    template <typename BlockSampleType, typename SmootherSampleType, typename SmoothingType>
    void replaceWithProductOfInternal (AudioBlock<BlockSampleType> src, SmoothedValue<SmootherSampleType, SmoothingType>& value) const noexcept
    {
        jassert (numChannels == src.numChannels);

        if (! value.isSmoothing())
        {
            replaceWithProductOfInternal (src, (NumericType) value.getTargetValue());
        }
        else
        {
            auto n = jmin (numSamples, src.numSamples) * sizeFactor;

            for (size_t i = 0; i < n; ++i)
            {
                const auto scaler = (NumericType) value.getNextValue();

                for (size_t ch = 0; ch < numChannels; ++ch)
                    getDataPointer (ch)[i] = scaler * src.getChannelPointer (ch)[i];
            }
        }
    }

    //==============================================================================
    template <typename OtherSampleType>
    void JUCE_VECTOR_CALLTYPE addProductOfInternal (AudioBlock<OtherSampleType> src, NumericType factor) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::addWithMultiply (getDataPointer (ch), src.getDataPointer (ch), factor, n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void addProductOfInternal (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = jmin (numSamples, src1.numSamples, src2.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::addWithMultiply (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    constexpr void negateInternal() const noexcept
    {
        multiplyByInternal (static_cast<NumericType> (-1.0));
    }

    template <typename OtherSampleType>
    void replaceWithNegativeOfInternal (AudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::negate (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    template <typename OtherSampleType>
    void replaceWithAbsoluteValueOfInternal (AudioBlock<OtherSampleType> src) const noexcept
    {
        jassert (numChannels == src.numChannels);
        auto n = jmin (numSamples, src.numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::abs (getDataPointer (ch), src.getDataPointer (ch), n);
    }

    //==============================================================================
    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithMinOfInternal (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = jmin (src1.numSamples, src2.numSamples, numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::min (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    template <typename Src1SampleType, typename Src2SampleType>
    void replaceWithMaxOfInternal (AudioBlock<Src1SampleType> src1, AudioBlock<Src2SampleType> src2) const noexcept
    {
        jassert (numChannels == src1.numChannels && src1.numChannels == src2.numChannels);
        auto n = jmin (src1.numSamples, src2.numSamples, numSamples) * sizeFactor;

        for (size_t ch = 0; ch < numChannels; ++ch)
            FloatVectorOperations::max (getDataPointer (ch), src1.getDataPointer (ch), src2.getDataPointer (ch), n);
    }

    //==============================================================================
    using ChannelCountType = unsigned int;

    //==============================================================================
    static constexpr size_t sizeFactor    = sizeof (SampleType) / sizeof (NumericType);
    static constexpr size_t elementMask   = sizeFactor - 1;
    static constexpr size_t byteMask      = (sizeFactor * sizeof (NumericType)) - 1;

   #if JUCE_USE_SIMD
    static constexpr size_t defaultAlignment = sizeof (SIMDRegister<NumericType>);
   #else
    static constexpr size_t defaultAlignment = sizeof (NumericType);
   #endif

    SampleType* const* channels;
    ChannelCountType numChannels = 0;
    size_t startSample = 0, numSamples = 0;

    template <typename OtherSampleType>
    friend class AudioBlock;
};

} // namespace dsp
} // namespace juce
