/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Writes samples to an audio file stream.

    A subclass that writes a specific type of audio format will be created by
    an AudioFormat object.

    After creating one of these with the AudioFormat::createWriterFor() method
    you can call its write() method to store the samples, and then delete it.

    @see AudioFormat, AudioFormatReader

    @tags{Audio}
*/
class JUCE_API  AudioFormatWriter
{
protected:
    //==============================================================================
    /** Creates an AudioFormatWriter object.

        @param destStream       the stream to write to - this will be deleted
                                by this object when it is no longer needed
        @param formatName       the description that will be returned by the getFormatName()
                                method
        @param sampleRate       the sample rate to use - the base class just stores
                                this value, it doesn't do anything with it
        @param numberOfChannels the number of channels to write - the base class just stores
                                this value, it doesn't do anything with it
        @param bitsPerSample    the bit depth of the stream - the base class just stores
                                this value, it doesn't do anything with it
    */
    AudioFormatWriter (OutputStream* destStream,
                       const String& formatName,
                       double sampleRate,
                       unsigned int numberOfChannels,
                       unsigned int bitsPerSample);

    //==============================================================================
    /** Creates an AudioFormatWriter object.

        @param destStream            the stream to write to - this will be deleted
                                     by this object when it is no longer needed
        @param formatName            the description that will be returned by the getFormatName()
                                     method
        @param sampleRate            the sample rate to use - the base class just stores
                                     this value, it doesn't do anything with it
        @param audioChannelLayout    the channel layout to use for the writer - the base class
                                     just stores this value, it doesn't do anything with it
        @param bitsPerSample         the bit depth of the stream - the base class just stores
                                     this value, it doesn't do anything with it
    */
    AudioFormatWriter (OutputStream* destStream,
                       const String& formatName,
                       double sampleRate,
                       const AudioChannelSet& audioChannelLayout,
                       unsigned int bitsPerSample);

public:
    using Options = AudioFormatWriterOptions;

    /** Destructor. */
    virtual ~AudioFormatWriter();

    //==============================================================================
    /** Returns a description of what type of format this is.

        E.g. "AIFF file"
    */
    const String& getFormatName() const noexcept        { return formatName; }

    //==============================================================================
    /** Writes a set of samples to the audio stream.

        Note that if you're trying to write the contents of an AudioBuffer, you
        can use writeFromAudioSampleBuffer().

        @param samplesToWrite   an array of arrays containing the sample data for
                                each channel to write. This is a zero-terminated
                                array of arrays, and can contain a different number
                                of channels than the actual stream uses, and the
                                writer should do its best to cope with this.
                                If the format is fixed-point, each channel will be formatted
                                as an array of signed integers using the full 32-bit
                                range -0x80000000 to 0x7fffffff, regardless of the source's
                                bit-depth. If it is a floating-point format, you should treat
                                the arrays as arrays of floats, and just cast it to an (int**)
                                to pass it into the method.
        @param numSamples       the number of samples to write
    */
    virtual bool write (const int** samplesToWrite, int numSamples) = 0;

    /** Some formats may support a flush operation that makes sure the file is in a
        valid state before carrying on.
        If supported, this means that by calling flush periodically when writing data
        to a large file, then it should still be left in a readable state if your program
        crashes.
        It goes without saying that this method must be called from the same thread that's
        calling write()!
        If the format supports flushing and the operation succeeds, this returns true.
    */
    virtual bool flush();

    //==============================================================================
    /** Reads a section of samples from an AudioFormatReader, and writes these to
        the output.

        This will take care of any floating-point conversion that's required to convert
        between the two formats. It won't deal with sample-rate conversion, though.

        If numSamplesToRead < 0, it will write the entire length of the reader.

        @returns false if it can't read or write properly during the operation
    */
    bool writeFromAudioReader (AudioFormatReader& reader,
                               int64 startSample,
                               int64 numSamplesToRead);

    /** Reads some samples from an AudioSource, and writes these to the output.

        The source must already have been initialised with the AudioSource::prepareToPlay() method

        @param source               the source to read from
        @param numSamplesToRead     total number of samples to read and write
        @param samplesPerBlock      the maximum number of samples to fetch from the source
        @returns false if it can't read or write properly during the operation
    */
    bool writeFromAudioSource (AudioSource& source,
                               int numSamplesToRead,
                               int samplesPerBlock = 2048);


    /** Writes some samples from an AudioBuffer. */
    bool writeFromAudioSampleBuffer (const AudioBuffer<float>& source,
                                     int startSample, int numSamples);

    /** Writes some samples from a set of float data channels. */
    bool writeFromFloatArrays (const float* const* channels, int numChannels, int numSamples);

    //==============================================================================
    /** Returns the sample rate being used. */
    double getSampleRate() const noexcept       { return sampleRate; }

    /** Returns the number of channels being written. */
    int getNumChannels() const noexcept         { return (int) numChannels; }

    /** Returns the bit-depth of the data being written. */
    int getBitsPerSample() const noexcept       { return (int) bitsPerSample; }

    /** Returns true if it's a floating-point format, false if it's fixed-point. */
    bool isFloatingPoint() const noexcept       { return usesFloatingPointData; }

    //==============================================================================
    /**
        Provides a FIFO for an AudioFormatWriter, allowing you to push incoming
        data into a buffer which will be flushed to disk by a background thread.
    */
    class ThreadedWriter
    {
    public:
        /** Creates a ThreadedWriter for a given writer and a thread.

            The writer object which is passed in here will be owned and deleted by
            the ThreadedWriter when it is no longer needed.

            To stop the writer and flush the buffer to disk, simply delete this object.
        */
        ThreadedWriter (AudioFormatWriter* writer,
                        TimeSliceThread& backgroundThread,
                        int numSamplesToBuffer);

        /** Destructor. */
        ~ThreadedWriter();

        /** Pushes some incoming audio data into the FIFO.

            If there's enough free space in the buffer, this will add the data to it,

            If the FIFO is too full to accept this many samples, the method will return
            false - then you could either wait until the background thread has had time to
            consume some of the buffered data and try again, or you can give up
            and lost this block.

            The data must be an array containing the same number of channels as the
            AudioFormatWriter object is using. None of these channels can be null.
        */
        bool write (const float* const* data, int numSamples);

        /** Receiver for incoming data. */
        class JUCE_API  IncomingDataReceiver
        {
        public:
            IncomingDataReceiver() = default;
            virtual ~IncomingDataReceiver() = default;

            virtual void reset (int numChannels, double sampleRate, int64 totalSamplesInSource) = 0;
            virtual void addBlock (int64 sampleNumberInSource, const AudioBuffer<float>& newData,
                                   int startOffsetInBuffer, int numSamples) = 0;
        };

        /** Allows you to specify a callback that this writer should update with the
            incoming data.
            The receiver will be cleared and the writer will begin adding data to it
            as the data arrives. Pass a null pointer to remove the current receiver.

            The object passed-in must not be deleted while this writer is still using it.
        */
        void setDataReceiver (IncomingDataReceiver*);

        /** Sets how many samples should be written before calling the AudioFormatWriter::flush method.
            Set this to 0 to disable flushing (this is the default).
        */
        void setFlushInterval (int numSamplesPerFlush) noexcept;

    private:
        class Buffer;
        std::unique_ptr<Buffer> buffer;
    };

protected:
    //==============================================================================
    /** The sample rate of the stream. */
    double sampleRate;

    /** The number of channels being written to the stream. */
    unsigned int numChannels;

    /** The bit depth of the file. */
    unsigned int bitsPerSample;

    /** True if it's a floating-point format, false if it's fixed-point. */
    bool usesFloatingPointData;

    /** The audio channel layout that the writer should use */
    AudioChannelSet channelLayout;

    /** The output stream for use by subclasses. */
    OutputStream* output;

    /** Used by AudioFormatWriter subclasses to copy data to different formats. */
    template <class DestSampleType, class SourceSampleType, class DestEndianness>
    struct WriteHelper
    {
        using DestType   = AudioData::Pointer <DestSampleType,   DestEndianness,          AudioData::Interleaved,    AudioData::NonConst>;
        using SourceType = AudioData::Pointer <SourceSampleType, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const>;

        static void write (void* destData, int numDestChannels, const int* const* source,
                           int numSamples, const int sourceOffset = 0) noexcept
        {
            for (int i = 0; i < numDestChannels; ++i)
            {
                const DestType dest (addBytesToPointer (destData, i * DestType::getBytesPerSample()), numDestChannels);

                if (*source != nullptr)
                {
                    dest.convertSamples (SourceType (*source + sourceOffset), numSamples);
                    ++source;
                }
                else
                {
                    dest.clearSamples (numSamples);
                }
            }
        }
    };

private:
    String formatName;
    friend class ThreadedWriter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFormatWriter)
};

} // namespace juce
