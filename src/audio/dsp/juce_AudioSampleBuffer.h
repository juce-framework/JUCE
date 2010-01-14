/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__
#define __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__

#include "../../containers/juce_HeapBlock.h"
class AudioFormatReader;
class AudioFormatWriter;


//==============================================================================
/**
    A multi-channel buffer of 32-bit floating point audio samples.

*/
class JUCE_API  AudioSampleBuffer
{
public:
    //==============================================================================
    /** Creates a buffer with a specified number of channels and samples.

        The contents of the buffer will initially be undefined, so use clear() to
        set all the samples to zero.

        The buffer will allocate its memory internally, and this will be released
        when the buffer is deleted.
    */
    AudioSampleBuffer (const int numChannels,
                       const int numSamples) throw();

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
    AudioSampleBuffer (float** dataToReferTo,
                       const int numChannels,
                       const int numSamples) throw();

    /** Copies another buffer.

        This buffer will make its own copy of the other's data, unless the buffer was created
        using an external data buffer, in which case boths buffers will just point to the same
        shared block of data.
    */
    AudioSampleBuffer (const AudioSampleBuffer& other) throw();

    /** Copies another buffer onto this one.

        This buffer's size will be changed to that of the other buffer.
    */
    const AudioSampleBuffer& operator= (const AudioSampleBuffer& other) throw();

    /** Destructor.

        This will free any memory allocated by the buffer.
    */
    virtual ~AudioSampleBuffer() throw();

    //==============================================================================
    /** Returns the number of channels of audio data that this buffer contains.

        @see getSampleData
    */
    int getNumChannels() const throw()      { return numChannels; }

    /** Returns the number of samples allocated in each of the buffer's channels.

        @see getSampleData
    */
    int getNumSamples() const throw()       { return size; }

    /** Returns a pointer one of the buffer's channels.

        For speed, this doesn't check whether the channel number is out of range,
        so be careful when using it!
    */
    float* getSampleData (const int channelNumber) const throw()
    {
        jassert (((unsigned int) channelNumber) < (unsigned int) numChannels);
        return channels [channelNumber];
    }

    /** Returns a pointer to a sample in one of the buffer's channels.

        For speed, this doesn't check whether the channel and sample number
        are out-of-range, so be careful when using it!
    */
    float* getSampleData (const int channelNumber,
                          const int sampleOffset) const throw()
    {
        jassert (((unsigned int) channelNumber) < (unsigned int) numChannels);
        jassert (((unsigned int) sampleOffset) < (unsigned int) size);
        return channels [channelNumber] + sampleOffset;
    }

    /** Returns an array of pointers to the channels in the buffer.

        Don't modify any of the pointers that are returned, and bear in mind that
        these will become invalid if the buffer is resized.
    */
    float** getArrayOfChannels() const throw()          { return channels; }

    //==============================================================================
    /** Chages the buffer's size or number of channels.

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
    */
    void setSize (const int newNumChannels,
                  const int newNumSamples,
                  const bool keepExistingContent = false,
                  const bool clearExtraSpace = false,
                  const bool avoidReallocating = false) throw();


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
                           const int numChannels,
                           const int numSamples) throw();

    //==============================================================================
    /** Clears all the samples in all channels. */
    void clear() throw();

    /** Clears a specified region of all the channels.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!
    */
    void clear (const int startSample,
                const int numSamples) throw();

    /** Clears a specified region of just one channel.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!
    */
    void clear (const int channel,
                const int startSample,
                const int numSamples) throw();

    /** Applies a gain multiple to a region of one channel.

        For speed, this doesn't check whether the channel and sample number
        are in-range, so be careful!
    */
    void applyGain (const int channel,
                    const int startSample,
                    int numSamples,
                    const float gain) throw();

    /** Applies a gain multiple to a region of all the channels.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGain (const int startSample,
                    const int numSamples,
                    const float gain) throw();

    /** Applies a range of gains to a region of a channel.

        The gain that is applied to each sample will vary from
        startGain on the first sample to endGain on the last Sample,
        so it can be used to do basic fades.

        For speed, this doesn't check whether the sample numbers
        are in-range, so be careful!
    */
    void applyGainRamp (const int channel,
                        const int startSample,
                        int numSamples,
                        float startGain,
                        float endGain) throw();

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
    void addFrom (const int destChannel,
                  const int destStartSample,
                  const AudioSampleBuffer& source,
                  const int sourceChannel,
                  const int sourceStartSample,
                  int numSamples,
                  const float gainToApplyToSource = 1.0f) throw();

    /** Adds samples from an array of floats to one of the channels.

        @param destChannel          the channel within this buffer to add the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source data to use
        @param numSamples           the number of samples to process
        @param gainToApplyToSource  an optional gain to apply to the source samples before they are
                                    added to this buffer's samples

        @see copyFrom
    */
    void addFrom (const int destChannel,
                  const int destStartSample,
                  const float* source,
                  int numSamples,
                  const float gainToApplyToSource = 1.0f) throw();

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
    void addFromWithRamp (const int destChannel,
                          const int destStartSample,
                          const float* source,
                          int numSamples,
                          float startGain,
                          float endGain) throw();

    /** Copies samples from another buffer to this one.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param sourceChannel        the channel within the source buffer to read from
        @param sourceStartSample    the offset within the source buffer's channel to start reading samples from
        @param numSamples           the number of samples to process

        @see addFrom
    */
    void copyFrom (const int destChannel,
                   const int destStartSample,
                   const AudioSampleBuffer& source,
                   const int sourceChannel,
                   const int sourceStartSample,
                   int numSamples) throw();

    /** Copies samples from an array of floats into one of the channels.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process

        @see addFrom
    */
    void copyFrom (const int destChannel,
                   const int destStartSample,
                   const float* source,
                   int numSamples) throw();

    /** Copies samples from an array of floats into one of the channels, applying a gain to it.

        @param destChannel          the channel within this buffer to copy the samples to
        @param destStartSample      the start sample within this buffer's channel
        @param source               the source buffer to read from
        @param numSamples           the number of samples to process
        @param gain                 the gain to apply

        @see addFrom
    */
    void copyFrom (const int destChannel,
                   const int destStartSample,
                   const float* source,
                   int numSamples,
                   const float gain) throw();

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
    void copyFromWithRamp (const int destChannel,
                           const int destStartSample,
                           const float* source,
                           int numSamples,
                           float startGain,
                           float endGain) throw();


    /** Finds the highest and lowest sample values in a given range.

        @param channel      the channel to read from
        @param startSample  the start sample within the channel
        @param numSamples   the number of samples to check
        @param minVal       on return, the lowest value that was found
        @param maxVal       on return, the highest value that was found
    */
    void findMinMax (const int channel,
                     const int startSample,
                     int numSamples,
                     float& minVal,
                     float& maxVal) const throw();

    /** Finds the highest absolute sample value within a region of a channel.
    */
    float getMagnitude (const int channel,
                        const int startSample,
                        const int numSamples) const throw();

    /** Finds the highest absolute sample value within a region on all channels.
    */
    float getMagnitude (const int startSample,
                        const int numSamples) const throw();

    /** Returns the root mean squared level for a region of a channel.
    */
    float getRMSLevel (const int channel,
                       const int startSample,
                       const int numSamples) const throw();

    //==============================================================================
    /** Fills a section of the buffer using an AudioReader as its source.

        This will convert the reader's fixed- or floating-point data to
        the buffer's floating-point format, and will try to intelligently
        cope with mismatches between the number of channels in the reader
        and the buffer.

        @see writeToAudioWriter
    */
    void readFromAudioReader (AudioFormatReader* reader,
                              const int startSample,
                              const int numSamples,
                              const int readerStartSample,
                              const bool useReaderLeftChan,
                              const bool useReaderRightChan) throw();

    /** Writes a section of this buffer to an audio writer.

        This saves you having to mess about with channels or floating/fixed
        point conversion.

        @see readFromAudioReader
    */
    void writeToAudioWriter (AudioFormatWriter* writer,
                             const int startSample,
                             const int numSamples) const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int numChannels, size;
    size_t allocatedBytes;
    float** channels;
    HeapBlock <char> allocatedData;
    float* preallocatedChannelSpace [32];

    void allocateData();
    void allocateChannels (float** const dataToReferTo);
};


#endif   // __JUCE_AUDIOSAMPLEBUFFER_JUCEHEADER__
