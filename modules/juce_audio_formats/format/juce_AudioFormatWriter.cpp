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

AudioFormatWriter::AudioFormatWriter (OutputStream* const out,
                                      const String& formatName_,
                                      const double rate,
                                      const unsigned int numChannels_,
                                      const unsigned int bitsPerSample_)
  : sampleRate (rate),
    numChannels (numChannels_),
    bitsPerSample (bitsPerSample_),
    usesFloatingPointData (false),
    channelLayout (AudioChannelSet::canonicalChannelSet (static_cast<int> (numChannels_))),
    output (out),
    formatName (formatName_)
{
}

AudioFormatWriter::AudioFormatWriter (OutputStream* const out,
                                      const String& formatName_,
                                      const double rate,
                                      const AudioChannelSet& channelLayout_,
                                      const unsigned int bitsPerSample_)
  : sampleRate (rate),
    numChannels (static_cast<unsigned int> (channelLayout_.size())),
    bitsPerSample (bitsPerSample_),
    usesFloatingPointData (false),
    channelLayout (channelLayout_),
    output (out),
    formatName (formatName_)
{
}

AudioFormatWriter::~AudioFormatWriter()
{
    delete output;
}

static void convertFloatsToInts (int* dest, const float* src, int numSamples) noexcept
{
    while (--numSamples >= 0)
    {
        const double samp = *src++;

        if (samp <= -1.0)
            *dest = std::numeric_limits<int>::min();
        else if (samp >= 1.0)
            *dest = std::numeric_limits<int>::max();
        else
            *dest = roundToInt (std::numeric_limits<int>::max() * samp);

        ++dest;
    }
}

bool AudioFormatWriter::writeFromAudioReader (AudioFormatReader& reader,
                                              int64 startSample,
                                              int64 numSamplesToRead)
{
    const int bufferSize = 16384;
    AudioBuffer<float> tempBuffer ((int) numChannels, bufferSize);

    int* buffers[128] = { nullptr };

    for (int i = tempBuffer.getNumChannels(); --i >= 0;)
        buffers[i] = reinterpret_cast<int*> (tempBuffer.getWritePointer (i, 0));

    if (numSamplesToRead < 0)
        numSamplesToRead = reader.lengthInSamples;

    while (numSamplesToRead > 0)
    {
        const int numToDo = (int) jmin (numSamplesToRead, (int64) bufferSize);

        if (! reader.read (buffers, (int) numChannels, startSample, numToDo, false))
            return false;

        if (reader.usesFloatingPointData != isFloatingPoint())
        {
            int** bufferChan = buffers;

            while (*bufferChan != nullptr)
            {
                void* const b = *bufferChan++;

                constexpr auto scaleFactor = 1.0f / static_cast<float> (0x7fffffff);

                if (isFloatingPoint())
                    FloatVectorOperations::convertFixedToFloat ((float*) b, (int*) b, scaleFactor, numToDo);
                else
                    convertFloatsToInts ((int*) b, (float*) b, numToDo);
            }
        }

        if (! write (const_cast<const int**> (buffers), numToDo))
            return false;

        numSamplesToRead -= numToDo;
        startSample += numToDo;
    }

    return true;
}

bool AudioFormatWriter::writeFromAudioSource (AudioSource& source, int numSamplesToRead, const int samplesPerBlock)
{
    AudioBuffer<float> tempBuffer (getNumChannels(), samplesPerBlock);

    while (numSamplesToRead > 0)
    {
        auto numToDo = jmin (numSamplesToRead, samplesPerBlock);

        AudioSourceChannelInfo info (&tempBuffer, 0, numToDo);
        info.clearActiveBufferRegion();

        source.getNextAudioBlock (info);

        if (! writeFromAudioSampleBuffer (tempBuffer, 0, numToDo))
            return false;

        numSamplesToRead -= numToDo;
    }

    return true;
}

bool AudioFormatWriter::writeFromFloatArrays (const float* const* channels, int numSourceChannels, int numSamples)
{
    if (numSamples <= 0)
        return true;

    if (isFloatingPoint())
        return write ((const int**) channels, numSamples);

    std::vector<int*> chans (numSourceChannels + 1);
    std::vector<int> scratch (4096);

    jassert (numSourceChannels < (int) chans.size());
    const int maxSamples = (int) scratch.size() / numSourceChannels;

    for (int i = 0; i < numSourceChannels; ++i)
        chans[(size_t) i] = scratch.data() + (i * maxSamples);

    chans[(size_t) numSourceChannels] = nullptr;
    int startSample = 0;

    while (numSamples > 0)
    {
        auto numToDo = jmin (numSamples, maxSamples);

        for (int i = 0; i < numSourceChannels; ++i)
            convertFloatsToInts (chans[(size_t) i], channels[(size_t) i] + startSample, numToDo);

        if (! write ((const int**) chans.data(), numToDo))
            return false;

        startSample += numToDo;
        numSamples  -= numToDo;
    }

    return true;
}

bool AudioFormatWriter::writeFromAudioSampleBuffer (const AudioBuffer<float>& source, int startSample, int numSamples)
{
    auto numSourceChannels = source.getNumChannels();
    jassert (startSample >= 0 && startSample + numSamples <= source.getNumSamples() && numSourceChannels > 0);

    if (startSample == 0)
        return writeFromFloatArrays (source.getArrayOfReadPointers(), numSourceChannels, numSamples);

    const float* chans[256];
    jassert ((int) numChannels < numElementsInArray (chans));

    for (int i = 0; i < numSourceChannels; ++i)
        chans[i] = source.getReadPointer (i, startSample);

    chans[numSourceChannels] = nullptr;

    return writeFromFloatArrays (chans, numSourceChannels, numSamples);
}

bool AudioFormatWriter::flush()
{
    return false;
}

//==============================================================================
class AudioFormatWriter::ThreadedWriter::Buffer final : private TimeSliceClient
{
public:
    Buffer (TimeSliceThread& tst, AudioFormatWriter* w, int channels, int numSamples)
        : fifo (numSamples),
          buffer (channels, numSamples),
          timeSliceThread (tst),
          writer (w)
    {
        timeSliceThread.addTimeSliceClient (this);
    }

    ~Buffer() override
    {
        isRunning = false;
        timeSliceThread.removeTimeSliceClient (this);

        while (writePendingData() == 0)
        {}
    }

    bool write (const float* const* data, int numSamples)
    {
        if (numSamples <= 0 || ! isRunning)
            return true;

        jassert (timeSliceThread.isThreadRunning());  // you need to get your thread running before pumping data into this!

        int start1, size1, start2, size2;
        fifo.prepareToWrite (numSamples, start1, size1, start2, size2);

        if (size1 + size2 < numSamples)
            return false;

        for (int i = buffer.getNumChannels(); --i >= 0;)
        {
            buffer.copyFrom (i, start1, data[i], size1);
            buffer.copyFrom (i, start2, data[i] + size1, size2);
        }

        fifo.finishedWrite (size1 + size2);
        timeSliceThread.notify();
        return true;
    }

    int useTimeSlice() override
    {
        return writePendingData();
    }

    int writePendingData()
    {
        auto numToDo = fifo.getTotalSize() / 4;

        int start1, size1, start2, size2;
        fifo.prepareToRead (numToDo, start1, size1, start2, size2);

        if (size1 <= 0)
            return 10;

        writer->writeFromAudioSampleBuffer (buffer, start1, size1);

        const ScopedLock sl (thumbnailLock);

        if (receiver != nullptr)
            receiver->addBlock (samplesWritten, buffer, start1, size1);

        samplesWritten += size1;

        if (size2 > 0)
        {
            writer->writeFromAudioSampleBuffer (buffer, start2, size2);

            if (receiver != nullptr)
                receiver->addBlock (samplesWritten, buffer, start2, size2);

            samplesWritten += size2;
        }

        fifo.finishedRead (size1 + size2);

        if (samplesPerFlush > 0)
        {
            flushSampleCounter -= size1 + size2;

            if (flushSampleCounter <= 0)
            {
                flushSampleCounter = samplesPerFlush;
                writer->flush();
            }
        }

        return 0;
    }

    void setDataReceiver (IncomingDataReceiver* newReceiver)
    {
        if (newReceiver != nullptr)
            newReceiver->reset (buffer.getNumChannels(), writer->getSampleRate(), 0);

        const ScopedLock sl (thumbnailLock);
        receiver = newReceiver;
        samplesWritten = 0;
    }

    void setFlushInterval (int numSamples) noexcept
    {
        samplesPerFlush = numSamples;
    }

private:
    AbstractFifo fifo;
    AudioBuffer<float> buffer;
    TimeSliceThread& timeSliceThread;
    std::unique_ptr<AudioFormatWriter> writer;
    CriticalSection thumbnailLock;
    IncomingDataReceiver* receiver = {};
    int64 samplesWritten = 0;
    int samplesPerFlush = 0, flushSampleCounter = 0;
    std::atomic<bool> isRunning { true };

    JUCE_DECLARE_NON_COPYABLE (Buffer)
};

AudioFormatWriter::ThreadedWriter::ThreadedWriter (AudioFormatWriter* writer, TimeSliceThread& backgroundThread, int numSamplesToBuffer)
    : buffer (new AudioFormatWriter::ThreadedWriter::Buffer (backgroundThread, writer, (int) writer->numChannels, numSamplesToBuffer))
{
}

AudioFormatWriter::ThreadedWriter::~ThreadedWriter()
{
}

bool AudioFormatWriter::ThreadedWriter::write (const float* const* data, int numSamples)
{
    return buffer->write (data, numSamples);
}

void AudioFormatWriter::ThreadedWriter::setDataReceiver (AudioFormatWriter::ThreadedWriter::IncomingDataReceiver* receiver)
{
    buffer->setDataReceiver (receiver);
}

void AudioFormatWriter::ThreadedWriter::setFlushInterval (int numSamplesPerFlush) noexcept
{
    buffer->setFlushInterval (numSamplesPerFlush);
}

} // namespace juce
