/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

AudioFormatWriter::AudioFormatWriter (OutputStream* const out,
                                      const String& formatName_,
                                      const double rate,
                                      const unsigned int numChannels_,
                                      const unsigned int bitsPerSample_)
  : sampleRate (rate),
    numChannels (numChannels_),
    bitsPerSample (bitsPerSample_),
    usesFloatingPointData (false),
    output (out),
    formatName (formatName_)
{
}

AudioFormatWriter::~AudioFormatWriter()
{
    delete output;
}

bool AudioFormatWriter::writeFromAudioReader (AudioFormatReader& reader,
                                              int64 startSample,
                                              int64 numSamplesToRead)
{
    const int bufferSize = 16384;
    AudioSampleBuffer tempBuffer ((int) numChannels, bufferSize);

    int* buffers [128] = { 0 };

    for (int i = tempBuffer.getNumChannels(); --i >= 0;)
        buffers[i] = reinterpret_cast<int*> (tempBuffer.getSampleData (i, 0));

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
                int* b = *bufferChan++;

                if (isFloatingPoint())
                {
                    // int -> float
                    const double factor = 1.0 / std::numeric_limits<int>::max();

                    for (int i = 0; i < numToDo; ++i)
                        reinterpret_cast<float*> (b)[i] = (float) (factor * b[i]);
                }
                else
                {
                    // float -> int
                    for (int i = 0; i < numToDo; ++i)
                    {
                        const double samp = *(const float*) b;

                        if (samp <= -1.0)
                            *b++ = std::numeric_limits<int>::min();
                        else if (samp >= 1.0)
                            *b++ = std::numeric_limits<int>::max();
                        else
                            *b++ = roundToInt (std::numeric_limits<int>::max() * samp);
                    }
                }
            }
        }

        if (! write (const_cast <const int**> (buffers), numToDo))
            return false;

        numSamplesToRead -= numToDo;
        startSample += numToDo;
    }

    return true;
}

bool AudioFormatWriter::writeFromAudioSource (AudioSource& source, int numSamplesToRead, const int samplesPerBlock)
{
    AudioSampleBuffer tempBuffer (getNumChannels(), samplesPerBlock);

    while (numSamplesToRead > 0)
    {
        const int numToDo = jmin (numSamplesToRead, samplesPerBlock);

        AudioSourceChannelInfo info (&tempBuffer, 0, numToDo);
        info.clearActiveBufferRegion();

        source.getNextAudioBlock (info);

        if (! writeFromAudioSampleBuffer (tempBuffer, 0, numToDo))
            return false;

        numSamplesToRead -= numToDo;
    }

    return true;
}

bool AudioFormatWriter::writeFromAudioSampleBuffer (const AudioSampleBuffer& source, int startSample, int numSamples)
{
    jassert (startSample >= 0 && startSample + numSamples <= source.getNumSamples() && source.getNumChannels() > 0);

    if (numSamples <= 0)
        return true;

    HeapBlock<int> tempBuffer;
    HeapBlock<int*> chans (numChannels + 1);
    chans [numChannels] = 0;

    if (isFloatingPoint())
    {
        for (int i = (int) numChannels; --i >= 0;)
            chans[i] = reinterpret_cast<int*> (source.getSampleData (i, startSample));
    }
    else
    {
        tempBuffer.malloc (((size_t) numSamples) * (size_t) numChannels);

        for (unsigned int i = 0; i < numChannels; ++i)
        {
            typedef AudioData::Pointer <AudioData::Int32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst> DestSampleType;
            typedef AudioData::Pointer <AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const> SourceSampleType;

            chans[i] = tempBuffer + (int) i * numSamples;
            DestSampleType destData (chans[i]);
            SourceSampleType sourceData (source.getSampleData ((int) i, startSample));
            destData.convertSamples (sourceData, numSamples);
        }
    }

    return write ((const int**) chans.getData(), numSamples);
}

//==============================================================================
class AudioFormatWriter::ThreadedWriter::Buffer   : public AbstractFifo,
                                                    private TimeSliceClient
{
public:
    Buffer (TimeSliceThread& tst, AudioFormatWriter* w, int channels, int bufferSize)
        : AbstractFifo (bufferSize),
          buffer (channels, bufferSize),
          timeSliceThread (tst),
          writer (w),
          receiver (nullptr),
          samplesWritten (0),
          isRunning (true)
    {
        timeSliceThread.addTimeSliceClient (this);
    }

    ~Buffer()
    {
        isRunning = false;
        timeSliceThread.removeTimeSliceClient (this);

        while (writePendingData() == 0)
        {}
    }

    bool write (const float** data, int numSamples)
    {
        if (numSamples <= 0 || ! isRunning)
            return true;

        jassert (timeSliceThread.isThreadRunning());  // you need to get your thread running before pumping data into this!

        int start1, size1, start2, size2;
        prepareToWrite (numSamples, start1, size1, start2, size2);

        if (size1 + size2 < numSamples)
            return false;

        for (int i = buffer.getNumChannels(); --i >= 0;)
        {
            buffer.copyFrom (i, start1, data[i], size1);
            buffer.copyFrom (i, start2, data[i] + size1, size2);
        }

        finishedWrite (size1 + size2);
        timeSliceThread.notify();
        return true;
    }

    int useTimeSlice()
    {
        return writePendingData();
    }

    int writePendingData()
    {
        const int numToDo = getTotalSize() / 4;

        int start1, size1, start2, size2;
        prepareToRead (numToDo, start1, size1, start2, size2);

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

        finishedRead (size1 + size2);
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

private:
    AudioSampleBuffer buffer;
    TimeSliceThread& timeSliceThread;
    ScopedPointer<AudioFormatWriter> writer;
    CriticalSection thumbnailLock;
    IncomingDataReceiver* receiver;
    int64 samplesWritten;
    volatile bool isRunning;

    JUCE_DECLARE_NON_COPYABLE (Buffer)
};

AudioFormatWriter::ThreadedWriter::ThreadedWriter (AudioFormatWriter* writer, TimeSliceThread& backgroundThread, int numSamplesToBuffer)
    : buffer (new AudioFormatWriter::ThreadedWriter::Buffer (backgroundThread, writer, (int) writer->numChannels, numSamplesToBuffer))
{
}

AudioFormatWriter::ThreadedWriter::~ThreadedWriter()
{
}

bool AudioFormatWriter::ThreadedWriter::write (const float** data, int numSamples)
{
    return buffer->write (data, numSamples);
}

void AudioFormatWriter::ThreadedWriter::setDataReceiver (AudioFormatWriter::ThreadedWriter::IncomingDataReceiver* receiver)
{
    buffer->setDataReceiver (receiver);
}
