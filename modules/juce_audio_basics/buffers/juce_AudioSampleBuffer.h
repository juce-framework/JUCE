/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_AUDIOSAMPLEBUFFER_H_INCLUDED
#define JUCE_AUDIOSAMPLEBUFFER_H_INCLUDED


//==============================================================================
/**
    A multi-channel buffer of 32-bit floating point audio samples.

*/
class JUCE_API  AudioSampleBuffer
{
public:
    //==============================================================================
    /** Creates an empty buffer with 0 channels and 0 length. */
    AudioSampleBuffer() noexcept;

    //==============================================================================
    /** Creates a buffer with a specified number of channels and samples.

        The contents of the buffer will initially be undefined, so use clear() to
        set all the samples to zero.

        The buffer will allocate its memory internally, and this will be released
        when the buffer is deleted. If the memory can't be allocated, this will
        throw a std::bad_alloc exception.
    */
    AudioSampleBuffer (int numChannels,
                       int numSamples) noexcept;

    /** Creates a buffer using a pre-allocated block of memory.

        Note that if the buffer is resized or its number of channels is changed, it
        will re-allocate memory internally and copy the existing data to this new area,
        so it will then stop directly addressing this memory.

        @param dataToReferTo    a pre-allocated array containing pointers to the data
                                for each channel that should be used by this buffer. The
                                buffer will only refer to this memory, it won't try to delete
                                it when the buffer is deleted or resized.
        @param numChannels      the number of channels to use - this must correspond to the
                                number of elements in the array passed in
        @param numSamples       the number of samples to use - this must correspond to the
                                size of the arrays passed in
    */
    AudioSampleBuffer (float* const* dataToReferTo,
                       int numChannels,
                       int numSamples) noexcept;

    /** Creates a buffer using a pre-allocated block of memory.

        Note that if the buffer is resized or its number of channels is changed, it
        will re-allocate memory internally and copy the existing data to this new area,
        so it will then stop directly addressing this memory.

        @param dataToReferTo    a pre-allocated array containing pointers to the data
                                for each channel that should be used by this buffer. The
                                buffer will only refer to this memory, it won't try to delete
                                it when the buffer is deleted or resized.
        @param numChannels      the number of channels to use - this must correspond to the
                                number of elements in the array passed in
        @param startSample      the offset within the arrays at which the data begins
        @param numSamples       the number of samples to use - this must correspond to the
                                size of the arrays passed in
    */
    AudioSampleBuffer (float* const* dataToReferTo,
                       int numChannels,
                       int startSample,
                       int numSamples) noexcept;

    /** Copies another buffer.

        This buffer will make its own copy of the other's data, unless the buffer was created
        using an external data buffer, in which case boths buffers will just point to the same
        shared block of data.
    */
    AudioSampleBuffer (const AudioSampleBuffer&) noexcept;

    /** Copies another buffer onto this one.
        This buffer's size will be changed to that of the other buffer.
    */
    AudioSampleBuffer& operator= (const AudioSampleBuffer&) noexcept;

    /** Destructor.
        This will free any memory allocated by the buffer.
    */
    ~AudioSampleBuffer() noexcept;

    //==============================================================================
    /** Returns the number of channels of audio data that this buffer contains.
        @see getSampleData
    */
    int getNumChannels() const noexcept     { return numChannels; }

    /** Returns the number of samples allocated in each of the buffer's channels.
        @see getSampleData
    */
    int getNumSamples() const noexcept      { return size; }

    /** Returns a pointer to an array of read-only samples in one of the buffer's channels.
        For speed, this doesn't check whether the channel number is out of range,
        so be careful when using it!
        If you need to write to the data, do NOT call this method and const_cast the
        result! Instead, you must call getWritePointer so that the buffer knows you're
        planning on modifying the data.
    */
    const float* getReadPointer (int channelNumber) const noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        return channels [channelNumber];
    }

    /** Returns a pointer to an array of read-only samples in one of the buffer's channels.
        For speed, this doesn't check whether the channel number or index are out of range,
        so be careful when using it!
        If you need to write to the data, do NOT call this method and const_cast the
        result! Instead, you must call getWritePointer so that the buffer knows you're
        planning on modifying the data.
    */
    const float* getReadPointer (int channelNumber, int sampleIndex) const noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        jassert (isPositiveAndBelow (sampleIndex, size));
        return channels [channelNumber] + sampleIndex;
    }

    /** Returns a writeable pointer to one of the buffer's channels.
        For speed, this doesn't check whether the channel number is out of range,
        so be careful when using it!
        Note that if you're not planning on writing to the data, you should always
        use getReadPointer instead.
    */
    float* getWritePointer (int channelNumber) noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        isClear = false;
        return channels [channelNumber];
    }

    /** Returns a writeable pointer to one of the buffer's channels.
        For speed, this doesn't check whether the channel number or index are out of range,
        so be careful when using it!
        Note that if you're not planning on writing to the data, you should
        use getReadPointer instead.
    */
    float* getWritePointer (int channelNumber, int sampleIndex) noexcept
    {
        jassert (isPositiveAndBelow (channelNumber, numChannels));
        jassert (isPositiveAndBelow (sampleIndex, size));
        isClear = false;
        return channels [channelNumber] + sampleIndex;
    }

    /** Returns an array of pointers to the channels in the buffer.

        Don't modify any of the pointers that are returned, and bear in mind that
        these will become invalid if the buffer is resized.
    */
    const float** getArrayOfReadPointers() const noexcept           { return const_cast<const float**> (channels); }

    /** Returns an array of pointers to the channels in the buffer.

        Don't modify any of the pointers that are returned, and bear in mind that
        these will become invalid if the buffer is resized.
    */
    float** getArrayOfWritePointers() noexcept                      { isClear = false; return channels; }

    //==============================================================================
    /** Changes the buffer's size or number of channels.

        This can expand or contract the buffer's length, and add or remove channels.

        If keepExistingContent is true, it will try to preserve as much of the
        old data as it can in the new buffer.

        If clearExtraSpace is true, then any extra channels or space that is
        allocated will be also be cleared. If false, then this space is left
        uninitialised.

        If avoidReallocating is true, then changing the buffer's size won't reduce the
        amount of memory that is currently allocated (but it will still increase it if
        the new size is bigger than the amount it currently has). If this is false, then
        a new allocation will be done so that the buffer uses takes up the minimum amount
        of memory that it needs.

        If the required memory can't be allocated, this will throw a std::bad_alloc exception.
    */
    void setSize (int newNumChannels,
                  int newNumSamples,
                  bool keepExistingContent = false,
                  bool clearExtraSpace = false,
                  bool avoidReallocating = false) noexcept;


    /** Makes this buffer point to a pre-allocated set of channel data arrays.

        There's also a constructor that lets you specify arrays like this, but this
        lets you change the channels dynamically.

        Note that if the buffer is resized or its number of channels is changed, it
        will re-allocate memory internally and copy the existing data to this new area,
        so it will then stop directly addressing this memory.

        @param dataToReferTo    a pre-allocated array containing pointers to the data
                                for each channel that should be used by this buffer. The
                                buffer will only refer to this memory, it won't try to delete
                                it when the buffer is deleted or resized.
        @param numChannels      the number of channels to use - this must correspond to the
                                number of elements in the array passed in
        @param numSamples       the number of samples to use - this must correspond to the
                                size of the arrays passed in
    */
    void setDataToReferTo (float** dataToReferTo,
                           int numChannels,
                           int numSamples) noexcept;

    //==============================================================================
    /** Clears all the samples in all channels. */
    void clear() noexcept;

    /** Clears a specified region of all the channels.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!
    */
    void clear (int startSample,
                int numSamples) noexcept;

    /** Clears a specified region of just one channel.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!
    */
    void clear (int channel,
                int startSample,
                int numSamples) noexcept;

    /** Returns true if the buffer has been entirely cleared.
        Note that this does not actually measure the contents of the buffer - it simply
        returns a flag that is set when the buffer is cleared, and which is reset whenever
        functions like getWritePointer() are invoked. That means the method does not take
        any time, but it may return false negatives when in fact the buffer is still empty.
    */
    bool hasBeenCleared() const noexcept                            { return isClear; }

    //==============================================================================
    /** Returns a sample from the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    float getSample (int channel, int sampleIndex) const noexcept;

    /** Sets a sample in the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    void setSample (int destChannel, int destSample, float newValue) noexcept;

    /** Adds a value to a sample in the buffer.
        The channel and index are not checked - they are expected to be in-range. If not,
        an assertion will be thrown, but in a release build, you're into 'undefined behaviour'
        territory.
    */
    void addSample (int destChannel, int destSample, float valueToAdd) noexcept;

    /** Applies a gain multiple to a region of one channel.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!
    */
    void applyGain (int channel,
                    int startSample,
                    int numSamples,
                    float gain) noexcept;

    /** Applies a gain multiple to a region of all the channels.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGain (int startSample,
                    int numSamples,
                    float gain) noexcept;

    /** Applies a gain multiple to all the audio data. */
    void applyGain (float gain) noexcept;

    /** Applies a range of gains to a region of a channel.

        The gain that is applied to each sample will vary from
        startGain on the first sample to endGain on the last Sample,
        so it can be used to do basic fades.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGainRamp (int channel,
                        int startSample,
                        int numSamples,
                        float startGain,
                        float endGain) noexcept;

    /** Applies a range of gains to a region of all channels.

        The gain that is applied to each sample will vary from
        startGain on the first sample to endGain on the last Sample,
        so it can be used to do basic fades.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGainRamp (int startSample,
                        int numSamples,
                        float startGain,
                        float endGain) noexcept;

    /** Adds samples from another buffer to this one.

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
                  const AudioSampleBuffer& source,
                  int sourceChannel,
                  int sourceStartSample,
                  int numSamples,
                  float gainToApplyToSource = 1.0f) noexcept;

    /** Adds samples from an array of floats to one of the channels.

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
                  const float* source,
                  int numSamples,
                  float gainToApplyToSource = 1.0f) noexcept;

    /** Adds samples from an array of floats, applying a gain ramp to them.

        @param destChannel          the channel within this buffer to add the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source data to use
        @param numSamples           the number of samples to process
        @param startGain            the gain to apply to the first sample (this is multiplied with
                                    the source samples before they are added to this buffer)
        @param endGain              the gain to apply to the final sample. The gain is linearly
                                    interpolated between the first and last samples.
    */
    void addFromWithRamp (int destChannel,
                          int destStartSample,
                          const float* source,
                          int numSamples,
                          float startGain,
                          float endGain) noexcept;

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
                   const AudioSampleBuffer& source,
                   int sourceChannel,
                   int sourceStartSample,
                   int numSamples) noexcept;

    /** Copies samples from an array of floats into one of the channels.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process

        @see addFrom
    */
    void copyFrom (int destChannel,
                   int destStartSample,
                   const float* source,
                   int numSamples) noexcept;

    /** Copies samples from an array of floats into one of the channels, applying a gain to it.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process
        @param gain                 the gain to apply

        @see addFrom
    */
    void copyFrom (int destChannel,
                   int destStartSample,
                   const float* source,
                   int numSamples,
                   float gain) noexcept;

    /** Copies samples from an array of floats into one of the channels, applying a gain ramp.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process
        @param startGain            the gain to apply to the first sample (this is multiplied with
                                    the source samples before they are copied to this buffer)
        @param endGain              the gain to apply to the final sample. The gain is linearly
                                    interpolated between the first and last samples.

        @see addFrom
    */
    void copyFromWithRamp (int destChannel,
                           int destStartSample,
                           const float* source,
                           int numSamples,
                           float startGain,
                           float endGain) noexcept;


    /** Returns a Range indicating the lowest and highest sample values in a given section.

        @param channel      the channel to read from
        @param startSample  the start sample within the channel
        @param numSamples   the number of samples to check
    */
    Range<float> findMinMax (int channel,
                             int startSample,
                             int numSamples) const noexcept;

    /** Finds the highest absolute sample value within a region of a channel. */
    float getMagnitude (int channel,
                        int startSample,
                        int numSamples) const noexcept;

    /** Finds the highest absolute sample value within a region on all channels. */
    float getMagnitude (int startSample,
                        int numSamples) const noexcept;

    /** Returns the root mean squared level for a region of a channel. */
    float getRMSLevel (int channel,
                       int startSample,
                       int numSamples) const noexcept;

    /** Reverses a part of a channel. */
    void reverse (int channel, int startSample, int numSamples) const noexcept;

    /** Reverses a part of the buffer. */
    void reverse (int startSample, int numSamples) const noexcept;

    //==============================================================================
   #ifndef DOXYGEN
    // Note that these methods have now been replaced by getReadPointer() and getWritePointer()
    JUCE_DEPRECATED_WITH_BODY (const float* getSampleData (int channel) const,            { return getReadPointer (channel); })
    JUCE_DEPRECATED_WITH_BODY (const float* getSampleData (int channel, int index) const, { return getReadPointer (channel, index); })
    JUCE_DEPRECATED_WITH_BODY (float* getSampleData (int channel),                        { return getWritePointer (channel); })
    JUCE_DEPRECATED_WITH_BODY (float* getSampleData (int channel, int index),             { return getWritePointer (channel, index); })

    // These have been replaced by getArrayOfReadPointers() and getArrayOfWritePointers()
    JUCE_DEPRECATED_WITH_BODY (const float** getArrayOfChannels() const,                  { return getArrayOfReadPointers(); })
    JUCE_DEPRECATED_WITH_BODY (float** getArrayOfChannels(),                              { return getArrayOfWritePointers(); })
   #endif

private:
    //==============================================================================
    int numChannels, size;
    size_t allocatedBytes;
    float** channels;
    HeapBlock<char, true> allocatedData;
    float* preallocatedChannelSpace [32];
    bool isClear;

    void allocateData();
    void allocateChannels (float* const*, int offset);

    JUCE_LEAK_DETECTOR (AudioSampleBuffer)
};


#endif   // JUCE_AUDIOSAMPLEBUFFER_H_INCLUDED
