/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A multi-channel buffer containing floating point audio samples.

    @tags{Audio}
*/
template <typename Type>
class AudioBuffer
{
public:
    //==============================================================================
    /** Creates an empty buffer with 0 channels and 0 length. */
    AudioBuffer() noexcept
       : channels (static_cast<Type**> (preallocatedChannelSpace))
    {
    }

    //==============================================================================
    /** Creates a buffer with a specified number of channels and samples.

        The contents of the buffer will initially be undefined, so use clear() to
        set all the samples to zero.

        The buffer will allocate its memory internally, and this will be released
        when the buffer is deleted. If the memory can't be allocated, this will
        throw a std::bad_alloc exception.
    */
    AudioBuffer (int numChannelsToAllocate,
                 int numSamplesToAllocate)
       : numChannels (numChannelsToAllocate),
         size (numSamplesToAllocate)
    {
        jassert (size >= 0 && numChannels >= 0);
        allocateData();
    }

    /** Creates a buffer using a pre-allocated block of memory.

        Note that if the buffer is resized or its number of channels is changed, it
        will re-allocate memory internally and copy the existing data to this new area,
        so it will then stop directly addressing this memory.

        @param dataToReferTo    a pre-allocated array containing pointers to the data
                                for each channel that should be used by this buffer. The
                                buffer will only refer to this memory, it won't try to delete
                                it when the buffer is deleted or resized.
        @param numChannelsToUse the number of channels to use - this must correspond to the
                                number of elements in the array passed in
        @param numSamples       the number of samples to use - this must correspond to the
                                size of the arrays passed in
    */
    AudioBuffer (Type* const* dataToReferTo,
                 int numChannelsToUse,
                 int numSamples)
        : numChannels (numChannelsToUse),
          size (numSamples)
    {
        jassert (dataToReferTo != nullptr);
        jassert (numChannelsToUse >= 0 && numSamples >= 0);
        allocateChannels (dataToReferTo, 0);
    }

    /** Creates a buffer using a pre-allocated block of memory.

        Note that if the buffer is resized or its number of channels is changed, it
        will re-allocate memory internally and copy the existing data to this new area,
        so it will then stop directly addressing this memory.

        @param dataToReferTo    a pre-allocated array containing pointers to the data
                                for each channel that should be used by this buffer. The
                                buffer will only refer to this memory, it won't try to delete
                                it when the buffer is deleted or resized.
        @param numChannelsToUse the number of channels to use - this must correspond to the
                                number of elements in the array passed in
        @param startSample      the offset within the arrays at which the data begins
        @param numSamples       the number of samples to use - this must correspond to the
                                size of the arrays passed in
    */
    AudioBuffer (Type* const* dataToReferTo,
                 int numChannelsToUse,
                 int startSample,
                 int numSamples)
        : numChannels (numChannelsToUse),
          size (numSamples)
    {
        jassert (dataToReferTo != nullptr);
        jassert (numChannelsToUse >= 0 && startSample >= 0 && numSamples >= 0);
        allocateChannels (dataToReferTo, startSample);
    }

    /** Copies another buffer.

        This buffer will make its own copy of the other's data, unless the buffer was created
        using an external data buffer, in which case both buffers will just point to the same
        shared block of data.
    */
    AudioBuffer (const AudioBuffer& other)
       : numChannels (other.numChannels),
         size (other.size),
         allocatedBytes (other.allocatedBytes)
    {
        if (allocatedBytes == 0)
        {
            allocateChannels (other.channels, 0);
        }
        else
        {
            allocateData();

            if (other.isClear)
            {
                clear();
            }
            else
            {
                for (int i = 0; i < numChannels; ++i)
                    FloatVectorOperations::copy (channels[i], other.channels[i], size);
            }
        }
    }

    /** Copies another buffer onto this one.

        This buffer's size will be changed to that of the other buffer.
    */
    AudioBuffer& operator= (const AudioBuffer& other)
    {
        if (this != &other)
        {
            setSize (other.getNumChannels(), other.getNumSamples(), false, false, false);

            if (other.isClear)
            {
                clear();
            }
            else
            {
                isClear = false;

                for (int i = 0; i < numChannels; ++i)
                    FloatVectorOperations::copy (channels[i], other.channels[i], size);
            }
        }

        return *this;
    }

    /** Destructor.

        This will free any memory allocated by the buffer.
    */
    ~AudioBuffer() = default;

    /** Move constructor. */
    AudioBuffer (AudioBuffer&& other) noexcept
        : numChannels (other.numChannels),
          size (other.size),
          allocatedBytes (other.allocatedBytes),
          allocatedData (std::move (other.allocatedData)),
          isClear (other.isClear)
    {
        if (numChannels < (int) numElementsInArray (preallocatedChannelSpace))
        {
            channels = preallocatedChannelSpace;

            for (int i = 0; i < numChannels; ++i)
                preallocatedChannelSpace[i] = other.channels[i];
        }
        else
        {
            channels = other.channels;
        }

        other.numChannels = 0;
        other.size = 0;
        other.allocatedBytes = 0;
    }

    /** Move assignment. */
    AudioBuffer& operator= (AudioBuffer&& other) noexcept
    {
        numChannels = other.numChannels;
        size = other.size;
        allocatedBytes = other.allocatedBytes;
        allocatedData = std::move (other.allocatedData);
        isClear = other.isClear;

        if (numChannels < (int) numElementsInArray (preallocatedChannelSpace))
        {
            channels = preallocatedChannelSpace;

            for (int i = 0; i < numChannels; ++i)
                preallocatedChannelSpace[i] = other.channels[i];
        }
        else
        {
            channels = other.channels;
        }

        other.numChannels = 0;
        other.size = 0;
        other.allocatedBytes = 0;
        return *this;
    }

    //==============================================================================
    /** Returns the number of channels of audio data that this buffer contains.

        @see getNumSamples, getReadPointer, getWritePointer
    */
    int getNumChannels() const noexcept                             { return numChannels; }

    /** Returns the number of samples allocated in each of the buffer's channels.

        @see getNumChannels, getReadPointer, getWritePointer
    */
    int getNumSamples() const noexcept                              { return size; }

    /** Returns a pointer to an array of read-only samples in one of the buffer's channels.

        For speed, this doesn't check whether the channel number is out of range,
        so be careful when using it!

        If you need to write to the data, do NOT call this method and const_cast the
        result! Instead, you must call getWritePointer so that the buffer knows you're
        planning on modifying the data.
    */
    const Type* getReadPointer (int channelNumber) const noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        return channels[channelNumber];
    }

    /** Returns a pointer to an array of read-only samples in one of the buffer's channels.

        For speed, this doesn't check whether the channel number or index are out of range,
        so be careful when using it!

        If you need to write to the data, do NOT call this method and const_cast the
        result! Instead, you must call getWritePointer so that the buffer knows you're
        planning on modifying the data.
    */
    const Type* getReadPointer (int channelNumber, int sampleIndex) const noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        jassert (isPositiveAndBelow (sampleIndex, size));
        return channels[channelNumber] + sampleIndex;
    }

    /** Returns a writeable pointer to one of the buffer's channels.

        For speed, this doesn't check whether the channel number is out of range,
        so be careful when using it!

        Note that if you're not planning on writing to the data, you should always
        use getReadPointer instead.

        This will mark the buffer as not cleared and the hasBeenCleared method will return
        false after this call. If you retain this write pointer and write some data to
        the buffer after calling its clear method, subsequent clear calls will do nothing.
        To avoid this either call this method each time you need to write data, or use the
        setNotClear method to force the internal cleared flag to false.

        @see setNotClear
    */
    Type* getWritePointer (int channelNumber) noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        isClear = false;
        return channels[channelNumber];
    }

    /** Returns a writeable pointer to one of the buffer's channels.

        For speed, this doesn't check whether the channel number or index are out of range,
        so be careful when using it!

        Note that if you're not planning on writing to the data, you should
        use getReadPointer instead.

        This will mark the buffer as not cleared and the hasBeenCleared method will return
        false after this call. If you retain this write pointer and write some data to
        the buffer after calling its clear method, subsequent clear calls will do nothing.
        To avoid this either call this method each time you need to write data, or use the
        setNotClear method to force the internal cleared flag to false.

        @see setNotClear
    */
    Type* getWritePointer (int channelNumber, int sampleIndex) noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        jassert (isPositiveAndBelow (sampleIndex, size));
        isClear = false;
        return channels[channelNumber] + sampleIndex;
    }

    /** Returns an array of pointers to the channels in the buffer.

        Don't modify any of the pointers that are returned, and bear in mind that
        these will become invalid if the buffer is resized.
    */
    const Type* const* getArrayOfReadPointers() const noexcept            { return channels; }

    /** Returns an array of pointers to the channels in the buffer.

        Don't modify any of the pointers that are returned, and bear in mind that
        these will become invalid if the buffer is resized.

        This will mark the buffer as not cleared and the hasBeenCleared method will return
        false after this call. If you retain this write pointer and write some data to
        the buffer after calling its clear method, subsequent clear calls will do nothing.
        To avoid this either call this method each time you need to write data, or use the
        setNotClear method to force the internal cleared flag to false.

        @see setNotClear
    */
    Type* const* getArrayOfWritePointers() noexcept                       { isClear = false; return channels; }

    //==============================================================================
    /** Changes the buffer's size or number of channels.

        This can expand or contract the buffer's length, and add or remove channels.

        Note that if keepExistingContent and avoidReallocating are both true, then it will
        only avoid reallocating if neither the channel count or length in samples increase.

        If the required memory can't be allocated, this will throw a std::bad_alloc exception.

        @param newNumChannels       the new number of channels.
        @param newNumSamples        the new number of samples.
        @param keepExistingContent  if this is true, it will try to preserve as much of the
                                    old data as it can in the new buffer.
        @param clearExtraSpace      if this is true, then any extra channels or space that is
                                    allocated will be also be cleared. If false, then this space is left
                                    uninitialised.
        @param avoidReallocating    if this is true, then changing the buffer's size won't reduce the
                                    amount of memory that is currently allocated (but it will still
                                    increase it if the new size is bigger than the amount it currently has).
                                    If this is false, then a new allocation will be done so that the buffer
                                    uses takes up the minimum amount of memory that it needs.
    */
    void setSize (int newNumChannels,
                  int newNumSamples,
                  bool keepExistingContent = false,
                  bool clearExtraSpace = false,
                  bool avoidReallocating = false)
    {
        jassert (newNumChannels >= 0);
        jassert (newNumSamples >= 0);

        if (newNumSamples != size || newNumChannels != numChannels)
        {
            auto allocatedSamplesPerChannel = ((size_t) newNumSamples + 3) & ~3u;
            auto channelListSize = ((static_cast<size_t> (1 + newNumChannels) * sizeof (Type*)) + 15) & ~15u;
            auto newTotalBytes = ((size_t) newNumChannels * (size_t) allocatedSamplesPerChannel * sizeof (Type))
                                    + channelListSize + 32;

            if (keepExistingContent)
            {
                if (avoidReallocating && newNumChannels <= numChannels && newNumSamples <= size)
                {
                    // no need to do any remapping in this case, as the channel pointers will remain correct!
                }
                else
                {
                    HeapBlock<char, true> newData;
                    newData.allocate (newTotalBytes, clearExtraSpace || isClear);

                    auto numSamplesToCopy = (size_t) jmin (newNumSamples, size);

                    auto newChannels = unalignedPointerCast<Type**> (newData.get());
                    auto newChan     = unalignedPointerCast<Type*> (newData + channelListSize);

                    for (int j = 0; j < newNumChannels; ++j)
                    {
                        newChannels[j] = newChan;
                        newChan += allocatedSamplesPerChannel;
                    }

                    if (! isClear)
                    {
                        auto numChansToCopy = jmin (numChannels, newNumChannels);

                        for (int i = 0; i < numChansToCopy; ++i)
                            FloatVectorOperations::copy (newChannels[i], channels[i], (int) numSamplesToCopy);
                    }

                    allocatedData.swapWith (newData);
                    allocatedBytes = newTotalBytes;
                    channels = newChannels;
                }
            }
            else
            {
                if (avoidReallocating && allocatedBytes >= newTotalBytes)
                {
                    if (clearExtraSpace || isClear)
                        allocatedData.clear (newTotalBytes);
                }
                else
                {
                    allocatedBytes = newTotalBytes;
                    allocatedData.allocate (newTotalBytes, clearExtraSpace || isClear);
                    channels = unalignedPointerCast<Type**> (allocatedData.get());
                }

                auto* chan = unalignedPointerCast<Type*> (allocatedData + channelListSize);

                for (int i = 0; i < newNumChannels; ++i)
                {
                    channels[i] = chan;
                    chan += allocatedSamplesPerChannel;
                }
            }

            channels[newNumChannels] = nullptr;
            size = newNumSamples;
            numChannels = newNumChannels;
        }
    }

    /** Makes this buffer point to a pre-allocated set of channel data arrays.

        There's also a constructor that lets you specify arrays like this, but this
        lets you change the channels dynamically.

        Note that if the buffer is resized or its number of channels is changed, it
        will re-allocate memory internally and copy the existing data to this new area,
        so it will then stop directly addressing this memory.

        The hasBeenCleared method will return false after this call.

        @param dataToReferTo    a pre-allocated array containing pointers to the data
                                for each channel that should be used by this buffer. The
                                buffer will only refer to this memory, it won't try to delete
                                it when the buffer is deleted or resized.
        @param newNumChannels   the number of channels to use - this must correspond to the
                                number of elements in the array passed in
        @param newStartSample   the offset within the arrays at which the data begins
        @param newNumSamples    the number of samples to use - this must correspond to the
                                size of the arrays passed in
    */
    void setDataToReferTo (Type* const* dataToReferTo,
                           int newNumChannels,
                           int newStartSample,
                           int newNumSamples)
    {
        jassert (dataToReferTo != nullptr);
        jassert (newNumChannels >= 0 && newNumSamples >= 0);

        if (allocatedBytes != 0)
        {
            allocatedBytes = 0;
            allocatedData.jfree();
        }

        numChannels = newNumChannels;
        size = newNumSamples;

        allocateChannels (dataToReferTo, newStartSample);
        jassert (! isClear);
    }

    /** Makes this buffer point to a pre-allocated set of channel data arrays.

        There's also a constructor that lets you specify arrays like this, but this
        lets you change the channels dynamically.

        Note that if the buffer is resized or its number of channels is changed, it
        will re-allocate memory internally and copy the existing data to this new area,
        so it will then stop directly addressing this memory.

        The hasBeenCleared method will return false after this call.

        @param dataToReferTo    a pre-allocated array containing pointers to the data
                                for each channel that should be used by this buffer. The
                                buffer will only refer to this memory, it won't try to delete
                                it when the buffer is deleted or resized.
        @param newNumChannels   the number of channels to use - this must correspond to the
                                number of elements in the array passed in
        @param newNumSamples    the number of samples to use - this must correspond to the
                                size of the arrays passed in
    */
    void setDataToReferTo (Type* const* dataToReferTo,
                           int newNumChannels,
                           int newNumSamples)
    {
        setDataToReferTo (dataToReferTo, newNumChannels, 0, newNumSamples);
    }

    /** Resizes this buffer to match the given one, and copies all of its content across.

        The source buffer can contain a different floating point type, so this can be used to
        convert between 32 and 64 bit float buffer types.

        The hasBeenCleared method will return false after this call if the other buffer
        contains data.
    */
    template <typename OtherType>
    void makeCopyOf (const AudioBuffer<OtherType>& other, bool avoidReallocating = false)
    {
        setSize (other.getNumChannels(), other.getNumSamples(), false, false, avoidReallocating);

        if (other.hasBeenCleared())
        {
            clear();
        }
        else
        {
            isClear = false;

            for (int chan = 0; chan < numChannels; ++chan)
            {
                auto* dest = channels[chan];
                auto* src = other.getReadPointer (chan);

                for (int i = 0; i < size; ++i)
                    dest[i] = static_cast<Type> (src[i]);
            }
        }
    }

    //==============================================================================
    /** Clears all the samples in all channels and marks the buffer as cleared.

        This method will do nothing if the buffer has been marked as cleared (i.e. the
        hasBeenCleared method returns true.)

        @see hasBeenCleared, setNotClear
    */
    void clear() noexcept
    {
        if (! isClear)
        {
            for (int i = 0; i < numChannels; ++i)
            {
                JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4661)
                FloatVectorOperations::clear (channels[i], size);
                JUCE_END_IGNORE_WARNINGS_MSVC
            }

            isClear = true;
        }
    }

    /** Clears a specified region of all the channels.

        This will mark the buffer as cleared if the entire buffer contents are cleared.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!

        This method will do nothing if the buffer has been marked as cleared (i.e. the
        hasBeenCleared method returns true.)

        @see hasBeenCleared, setNotClear
    */
    void clear (int startSample, int numSamples) noexcept
    {
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

        if (! isClear)
        {
            for (int i = 0; i < numChannels; ++i)
                FloatVectorOperations::clear (channels[i] + startSample, numSamples);

            isClear = (startSample == 0 && numSamples == size);
        }
    }

    /** Clears a specified region of just one channel.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!

        This method will do nothing if the buffer has been marked as cleared (i.e. the
        hasBeenCleared method returns true.)

        @see hasBeenCleared, setNotClear
    */
    void clear (int channel, int startSample, int numSamples) noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

        if (! isClear)
            FloatVectorOperations::clear (channels[channel] + startSample, numSamples);
    }

    /** Returns true if the buffer has been entirely cleared.

        Note that this does not actually measure the contents of the buffer - it simply
        returns a flag that is set when the buffer is cleared, and which is reset whenever
        functions like getWritePointer are invoked. That means the method is quick, but it
        may return false negatives when in fact the buffer is still empty.
    */
    bool hasBeenCleared() const noexcept                            { return isClear; }

    /** Forces the internal cleared flag of the buffer to false.

        This may be useful in the case where you are holding on to a write pointer and call
        the clear method before writing some data. You can then use this method to mark the
        buffer as containing data so that subsequent clear calls will succeed. However a
        better solution is to call getWritePointer each time you need to write data.
    */
    void setNotClear() noexcept                                     { isClear = false; }

    //==============================================================================
    /** Returns a sample from the buffer.

        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    Type getSample (int channel, int sampleIndex) const noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (isPositiveAndBelow (sampleIndex, size));
        return *(channels[channel] + sampleIndex);
    }

    /** Sets a sample in the buffer.

        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.

        The hasBeenCleared method will return false after this call.
    */
    void setSample (int destChannel, int destSample, Type newValue) noexcept
    {
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (isPositiveAndBelow (destSample, size));
        *(channels[destChannel] + destSample) = newValue;
        isClear = false;
    }

    /** Adds a value to a sample in the buffer.

        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.

        The hasBeenCleared method will return false after this call.
    */
    void addSample (int destChannel, int destSample, Type valueToAdd) noexcept
    {
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (isPositiveAndBelow (destSample, size));
        *(channels[destChannel] + destSample) += valueToAdd;
        isClear = false;
    }

    /** Applies a gain multiple to a region of one channel.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!
    */
    void applyGain (int channel, int startSample, int numSamples, Type gain) noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

        if (! approximatelyEqual (gain, Type (1)) && ! isClear)
        {
            auto* d = channels[channel] + startSample;

            if (approximatelyEqual (gain, Type()))
                FloatVectorOperations::clear (d, numSamples);
            else
                FloatVectorOperations::multiply (d, gain, numSamples);
        }
    }

    /** Applies a gain multiple to a region of all the channels.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGain (int startSample, int numSamples, Type gain) noexcept
    {
        for (int i = 0; i < numChannels; ++i)
            applyGain (i, startSample, numSamples, gain);
    }

    /** Applies a gain multiple to all the audio data. */
    void applyGain (Type gain) noexcept
    {
        applyGain (0, size, gain);
    }

    /** Applies a range of gains to a region of a channel.

        The gain that is applied to each sample will vary from
        startGain on the first sample to endGain on the last Sample,
        so it can be used to do basic fades.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGainRamp (int channel, int startSample, int numSamples,
                        Type startGain, Type endGain) noexcept
    {
        if (! isClear)
        {
            if (approximatelyEqual (startGain, endGain))
            {
                applyGain (channel, startSample, numSamples, startGain);
            }
            else
            {
                jassert (isPositiveAndBelow (channel, numChannels));
                jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

                const auto increment = (endGain - startGain) / (float) numSamples;
                auto* d = channels[channel] + startSample;

                while (--numSamples >= 0)
                {
                    *d++ *= startGain;
                    startGain += increment;
                }
            }
        }
    }

    /** Applies a range of gains to a region of all channels.

        The gain that is applied to each sample will vary from
        startGain on the first sample to endGain on the last Sample,
        so it can be used to do basic fades.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGainRamp (int startSample, int numSamples,
                        Type startGain, Type endGain) noexcept
    {
        for (int i = 0; i < numChannels; ++i)
            applyGainRamp (i, startSample, numSamples, startGain, endGain);
    }

    /** Adds samples from another buffer to this one.

        The hasBeenCleared method will return false after this call if samples have
        been added.

        @param destChannel          the channel within this buffer to add the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to add from
        @param sourceChannel        the channel within the source buffer to read from
        @param sourceStartSample    the offset within the source buffer's channel to start reading samples from
        @param numSamples           the number of samples to process
        @param gainToApplyToSource  an optional gain to apply to the source samples before they are
                                    added to this buffer's samples

        @see copyFrom
    */
    void addFrom (int destChannel,
                  int destStartSample,
                  const AudioBuffer& source,
                  int sourceChannel,
                  int sourceStartSample,
                  int numSamples,
                  Type gainToApplyToSource = Type (1)) noexcept
    {
        jassert (&source != this
                 || sourceChannel != destChannel
                 || sourceStartSample + numSamples <= destStartSample
                 || destStartSample + numSamples <= sourceStartSample);
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (destStartSample >= 0 && numSamples >= 0 && destStartSample + numSamples <= size);
        jassert (isPositiveAndBelow (sourceChannel, source.numChannels));
        jassert (sourceStartSample >= 0 && sourceStartSample + numSamples <= source.size);

        if (! approximatelyEqual (gainToApplyToSource, (Type) 0) && numSamples > 0 && ! source.isClear)
        {
            auto* d = channels[destChannel] + destStartSample;
            auto* s = source.channels[sourceChannel] + sourceStartSample;

            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4661)

            if (isClear)
            {
                isClear = false;

                if (! approximatelyEqual (gainToApplyToSource, Type (1)))
                    FloatVectorOperations::copyWithMultiply (d, s, gainToApplyToSource, numSamples);
                else
                    FloatVectorOperations::copy (d, s, numSamples);
            }
            else
            {
                if (! approximatelyEqual (gainToApplyToSource, Type (1)))
                    FloatVectorOperations::addWithMultiply (d, s, gainToApplyToSource, numSamples);
                else
                    FloatVectorOperations::add (d, s, numSamples);
            }

            JUCE_END_IGNORE_WARNINGS_MSVC
        }
    }

    /** Adds samples from an array of floats to one of the channels.

        The hasBeenCleared method will return false after this call if samples have
        been added.

        @param destChannel          the channel within this buffer to add the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source data to use
        @param numSamples           the number of samples to process
        @param gainToApplyToSource  an optional gain to apply to the source samples before they are
                                    added to this buffer's samples

        @see copyFrom
    */
    void addFrom (int destChannel,
                  int destStartSample,
                  const Type* source,
                  int numSamples,
                  Type gainToApplyToSource = Type (1)) noexcept
    {
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (destStartSample >= 0 && numSamples >= 0 && destStartSample + numSamples <= size);
        jassert (source != nullptr);

        if (! approximatelyEqual (gainToApplyToSource, Type()) && numSamples > 0)
        {
            auto* d = channels[destChannel] + destStartSample;

            if (isClear)
            {
                isClear = false;

                if (! approximatelyEqual (gainToApplyToSource, Type (1)))
                    FloatVectorOperations::copyWithMultiply (d, source, gainToApplyToSource, numSamples);
                else
                    FloatVectorOperations::copy (d, source, numSamples);
            }
            else
            {
                if (! approximatelyEqual (gainToApplyToSource, Type (1)))
                    FloatVectorOperations::addWithMultiply (d, source, gainToApplyToSource, numSamples);
                else
                    FloatVectorOperations::add (d, source, numSamples);
            }
        }
    }


    /** Adds samples from an array of floats, applying a gain ramp to them.

        The hasBeenCleared method will return false after this call if samples have
        been added.

        @param destChannel          the channel within this buffer to add the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source data to use
        @param numSamples           the number of samples to process
        @param startGain            the gain to apply to the first sample (this is multiplied with
                                    the source samples before they are added to this buffer)
        @param endGain              The gain that would apply to the sample after the final sample.
                                    The gain that applies to the final sample is
                                    (numSamples - 1) / numSamples * (endGain - startGain). This
                                    ensures a continuous ramp when supplying the same value in
                                    endGain and startGain in subsequent blocks. The gain is linearly
                                    interpolated between the first and last samples.
    */
    void addFromWithRamp (int destChannel,
                          int destStartSample,
                          const Type* source,
                          int numSamples,
                          Type startGain,
                          Type endGain) noexcept
    {
        if (approximatelyEqual (startGain, endGain))
        {
            addFrom (destChannel, destStartSample, source, numSamples, startGain);
        }
        else
        {
            jassert (isPositiveAndBelow (destChannel, numChannels));
            jassert (destStartSample >= 0 && numSamples >= 0 && destStartSample + numSamples <= size);
            jassert (source != nullptr);

            if (numSamples > 0)
            {
                isClear = false;
                const auto increment = (endGain - startGain) / (Type) numSamples;
                auto* d = channels[destChannel] + destStartSample;

                while (--numSamples >= 0)
                {
                    *d++ += startGain * *source++;
                    startGain += increment;
                }
            }
        }
    }

    /** Copies samples from another buffer to this one.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param sourceChannel        the channel within the source buffer to read from
        @param sourceStartSample    the offset within the source buffer's channel to start reading samples from
        @param numSamples           the number of samples to process

        @see addFrom
    */
    void copyFrom (int destChannel,
                   int destStartSample,
                   const AudioBuffer& source,
                   int sourceChannel,
                   int sourceStartSample,
                   int numSamples) noexcept
    {
        jassert (&source != this
                 || sourceChannel != destChannel
                 || sourceStartSample + numSamples <= destStartSample
                 || destStartSample + numSamples <= sourceStartSample);
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
        jassert (isPositiveAndBelow (sourceChannel, source.numChannels));
        jassert (sourceStartSample >= 0 && numSamples >= 0 && sourceStartSample + numSamples <= source.size);

        if (numSamples > 0)
        {
            if (source.isClear)
            {
                if (! isClear)
                    FloatVectorOperations::clear (channels[destChannel] + destStartSample, numSamples);
            }
            else
            {
                isClear = false;
                FloatVectorOperations::copy (channels[destChannel] + destStartSample,
                                             source.channels[sourceChannel] + sourceStartSample,
                                             numSamples);
            }
        }
    }

    /** Copies samples from an array of floats into one of the channels.

        The hasBeenCleared method will return false after this call if samples have
        been copied.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process

        @see addFrom
    */
    void copyFrom (int destChannel,
                   int destStartSample,
                   const Type* source,
                   int numSamples) noexcept
    {
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (destStartSample >= 0 && numSamples >= 0 && destStartSample + numSamples <= size);
        jassert (source != nullptr);

        if (numSamples > 0)
        {
            isClear = false;
            FloatVectorOperations::copy (channels[destChannel] + destStartSample, source, numSamples);
        }
    }

    /** Copies samples from an array of floats into one of the channels, applying a gain to it.

        The hasBeenCleared method will return false after this call if samples have
        been copied.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process
        @param gain                 the gain to apply

        @see addFrom
    */
    void copyFrom (int destChannel,
                   int destStartSample,
                   const Type* source,
                   int numSamples,
                   Type gain) noexcept
    {
        jassert (isPositiveAndBelow (destChannel, numChannels));
        jassert (destStartSample >= 0 && numSamples >= 0 && destStartSample + numSamples <= size);
        jassert (source != nullptr);

        if (numSamples > 0)
        {
            auto* d = channels[destChannel] + destStartSample;

            if (! approximatelyEqual (gain, Type (1)))
            {
                if (approximatelyEqual (gain, Type()))
                {
                    if (! isClear)
                        FloatVectorOperations::clear (d, numSamples);
                }
                else
                {
                    isClear = false;
                    FloatVectorOperations::copyWithMultiply (d, source, gain, numSamples);
                }
            }
            else
            {
                isClear = false;
                FloatVectorOperations::copy (d, source, numSamples);
            }
        }
    }

    /** Copies samples from an array of floats into one of the channels, applying a gain ramp.

        The hasBeenCleared method will return false after this call if samples have
        been copied.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process
        @param startGain            the gain to apply to the first sample (this is multiplied with
                                    the source samples before they are copied to this buffer)
        @param endGain              The gain that would apply to the sample after the final sample.
                                    The gain that applies to the final sample is
                                    (numSamples - 1) / numSamples * (endGain - startGain). This
                                    ensures a continuous ramp when supplying the same value in
                                    endGain and startGain in subsequent blocks. The gain is linearly
                                    interpolated between the first and last samples.

        @see addFrom
    */
    void copyFromWithRamp (int destChannel,
                           int destStartSample,
                           const Type* source,
                           int numSamples,
                           Type startGain,
                           Type endGain) noexcept
    {
        if (approximatelyEqual (startGain, endGain))
        {
            copyFrom (destChannel, destStartSample, source, numSamples, startGain);
        }
        else
        {
            jassert (isPositiveAndBelow (destChannel, numChannels));
            jassert (destStartSample >= 0 && numSamples >= 0 && destStartSample + numSamples <= size);
            jassert (source != nullptr);

            if (numSamples > 0)
            {
                isClear = false;
                const auto increment = (endGain - startGain) / (Type) numSamples;
                auto* d = channels[destChannel] + destStartSample;

                while (--numSamples >= 0)
                {
                    *d++ = startGain * *source++;
                    startGain += increment;
                }
            }
        }
    }

    /** Returns a Range indicating the lowest and highest sample values in a given section.

        @param channel      the channel to read from
        @param startSample  the start sample within the channel
        @param numSamples   the number of samples to check
    */
    Range<Type> findMinMax (int channel, int startSample, int numSamples) const noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

        if (isClear)
            return { Type (0), Type (0) };

        return FloatVectorOperations::findMinAndMax (channels[channel] + startSample, numSamples);
    }

    /** Finds the highest absolute sample value within a region of a channel. */
    Type getMagnitude (int channel, int startSample, int numSamples) const noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

        if (isClear)
            return Type (0);

        auto r = findMinMax (channel, startSample, numSamples);

        return jmax (r.getStart(), -r.getStart(), r.getEnd(), -r.getEnd());
    }

    /** Finds the highest absolute sample value within a region on all channels. */
    Type getMagnitude (int startSample, int numSamples) const noexcept
    {
        Type mag (0);

        if (! isClear)
            for (int i = 0; i < numChannels; ++i)
                mag = jmax (mag, getMagnitude (i, startSample, numSamples));

        return mag;
    }

    /** Returns the root mean squared level for a region of a channel. */
    Type getRMSLevel (int channel, int startSample, int numSamples) const noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

        if (numSamples <= 0 || channel < 0 || channel >= numChannels || isClear)
            return Type (0);

        auto* data = channels[channel] + startSample;
        double sum = 0.0;

        for (int i = 0; i < numSamples; ++i)
        {
            auto sample = data[i];
            sum += sample * sample;
        }

        return static_cast<Type> (std::sqrt (sum / numSamples));
    }

    /** Reverses a part of a channel. */
    void reverse (int channel, int startSample, int numSamples) const noexcept
    {
        jassert (isPositiveAndBelow (channel, numChannels));
        jassert (startSample >= 0 && numSamples >= 0 && startSample + numSamples <= size);

        if (! isClear)
            std::reverse (channels[channel] + startSample,
                          channels[channel] + startSample + numSamples);
    }

    /** Reverses a part of the buffer. */
    void reverse (int startSample, int numSamples) const noexcept
    {
        for (int i = 0; i < numChannels; ++i)
            reverse (i, startSample, numSamples);
    }

    //==============================================================================
    /** This allows templated code that takes an AudioBuffer to access its sample type. */
    using SampleType = Type;

private:
    //==============================================================================
    void allocateData()
    {
       #if (! JUCE_GCC || (__GNUC__ * 100 + __GNUC_MINOR__) >= 409)
        static_assert (alignof (Type) <= maxAlignment,
                       "AudioBuffer cannot hold types with alignment requirements larger than that guaranteed by malloc");
       #endif
        jassert (size >= 0);

        auto channelListSize = (size_t) (numChannels + 1) * sizeof (Type*);
        auto requiredSampleAlignment = std::alignment_of_v<Type>;
        size_t alignmentOverflow = channelListSize % requiredSampleAlignment;

        if (alignmentOverflow != 0)
            channelListSize += requiredSampleAlignment - alignmentOverflow;

        allocatedBytes = (size_t) numChannels * (size_t) size * sizeof (Type) + channelListSize + 32;
        allocatedData.jmalloc (allocatedBytes);
        channels = unalignedPointerCast<Type**> (allocatedData.get());
        auto chan = unalignedPointerCast<Type*> (allocatedData + channelListSize);

        for (int i = 0; i < numChannels; ++i)
        {
            channels[i] = chan;
            chan += size;
        }

        channels[numChannels] = nullptr;
        isClear = false;
    }

    void allocateChannels (Type* const* dataToReferTo, int offset)
    {
        jassert (offset >= 0);

        // (try to avoid doing a malloc here, as that'll blow up things like Pro-Tools)
        if (numChannels < (int) numElementsInArray (preallocatedChannelSpace))
        {
            channels = static_cast<Type**> (preallocatedChannelSpace);
        }
        else
        {
            allocatedData.jmalloc (numChannels + 1, sizeof (Type*));
            channels = unalignedPointerCast<Type**> (allocatedData.get());
        }

        for (int i = 0; i < numChannels; ++i)
        {
            // you have to pass in the same number of valid pointers as numChannels
            jassert (dataToReferTo[i] != nullptr);
            channels[i] = dataToReferTo[i] + offset;
        }

        channels[numChannels] = nullptr;
        isClear = false;
    }

    /*  On iOS/arm7 the alignment of `double` is greater than the alignment of
        `std::max_align_t`, so we can't trust max_align_t. Instead, we query
        lots of primitive types and use the maximum alignment of all of them.
    */
    static constexpr size_t getMaxAlignment() noexcept
    {
        constexpr size_t alignments[] { alignof (std::max_align_t),
                                        alignof (void*),
                                        alignof (float),
                                        alignof (double),
                                        alignof (long double),
                                        alignof (short int),
                                        alignof (int),
                                        alignof (long int),
                                        alignof (long long int),
                                        alignof (bool),
                                        alignof (char),
                                        alignof (char16_t),
                                        alignof (char32_t),
                                        alignof (wchar_t) };

        size_t max = 0;

        for (const auto elem : alignments)
            max = jmax (max, elem);

        return max;
    }

    int numChannels = 0, size = 0;
    size_t allocatedBytes = 0;
    Type** channels;
    HeapBlock<char, true> allocatedData;
    Type* preallocatedChannelSpace[32];
    bool isClear = false;
    static constexpr size_t maxAlignment = getMaxAlignment();

    JUCE_LEAK_DETECTOR (AudioBuffer)
};

//==============================================================================
template <typename Type>
bool operator== (const AudioBuffer<Type>& a, const AudioBuffer<Type>& b)
{
    if (a.getNumChannels() != b.getNumChannels())
        return false;

    for (auto c = 0; c < a.getNumChannels(); ++c)
    {
        const auto begin = [c] (auto& x) { return x.getReadPointer (c); };
        const auto end = [c] (auto& x) { return x.getReadPointer (c) + x.getNumSamples(); };

        if (! std::equal (begin (a), end (a), begin (b), end (b)))
            return false;
    }

    return true;
}

template <typename Type>
bool operator!= (const AudioBuffer<Type>& a, const AudioBuffer<Type>& b)
{
    return ! (a == b);
}

//==============================================================================
/**
    A multi-channel buffer of 32-bit floating point audio samples.

    This type is here for backwards compatibility with the older AudioSampleBuffer
    class, which was fixed for 32-bit data, but is otherwise the same as the new
    templated AudioBuffer class.

    @see AudioBuffer
*/
using AudioSampleBuffer = AudioBuffer<float>;

} // namespace juce
