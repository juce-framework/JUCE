/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

AudioFormatReader::AudioFormatReader (InputStream* in, const String& name)
    : input (in), formatName (name)
{
}

AudioFormatReader::~AudioFormatReader()
{
    delete input;
}

static void convertFixedToFloat (int* const* channels, int numChannels, int numSamples)
{
    constexpr auto scaleFactor = 1.0f / static_cast<float> (0x7fffffff);

    for (int i = 0; i < numChannels; ++i)
        if (auto d = channels[i])
            FloatVectorOperations::convertFixedToFloat (reinterpret_cast<float*> (d), d, scaleFactor, numSamples);
}

bool AudioFormatReader::read (float* const* destChannels, int numDestChannels,
                              int64 startSampleInSource, int numSamplesToRead)
{
    auto channelsAsInt = reinterpret_cast<int* const*> (destChannels);

    if (! read (channelsAsInt, numDestChannels, startSampleInSource, numSamplesToRead, false))
        return false;

    if (! usesFloatingPointData)
        convertFixedToFloat (channelsAsInt, numDestChannels, numSamplesToRead);

    return true;
}

bool AudioFormatReader::read (int* const* destChannels,
                              int numDestChannels,
                              int64 startSampleInSource,
                              int numSamplesToRead,
                              bool fillLeftoverChannelsWithCopies)
{
    jassert (numDestChannels > 0); // you have to actually give this some channels to work with!

    auto originalNumSamplesToRead = (size_t) numSamplesToRead;
    int startOffsetInDestBuffer = 0;

    if (startSampleInSource < 0)
    {
        auto silence = (int) jmin (-startSampleInSource, (int64) numSamplesToRead);

        for (int i = numDestChannels; --i >= 0;)
            if (auto d = destChannels[i])
                zeromem (d, (size_t) silence * sizeof (int));

        startOffsetInDestBuffer += silence;
        numSamplesToRead -= silence;
        startSampleInSource = 0;
    }

    if (numSamplesToRead <= 0)
        return true;

    if (! readSamples (const_cast<int**> (destChannels),
                       jmin ((int) numChannels, numDestChannels), startOffsetInDestBuffer,
                       startSampleInSource, numSamplesToRead))
        return false;

    if (numDestChannels > (int) numChannels)
    {
        if (fillLeftoverChannelsWithCopies)
        {
            auto lastFullChannel = destChannels[0];

            for (int i = (int) numChannels; --i > 0;)
            {
                if (destChannels[i] != nullptr)
                {
                    lastFullChannel = destChannels[i];
                    break;
                }
            }

            if (lastFullChannel != nullptr)
                for (int i = (int) numChannels; i < numDestChannels; ++i)
                    if (auto d = destChannels[i])
                        memcpy (d, lastFullChannel, sizeof (int) * originalNumSamplesToRead);
        }
        else
        {
            for (int i = (int) numChannels; i < numDestChannels; ++i)
                if (auto d = destChannels[i])
                    zeromem (d, sizeof (int) * originalNumSamplesToRead);
        }
    }

    return true;
}

static void readChannels (AudioFormatReader& reader, int** chans, AudioBuffer<float>* buffer,
                          int startSample, int numSamples, int64 readerStartSample, int numTargetChannels,
                          bool convertToFloat)
{
    for (int j = 0; j < numTargetChannels; ++j)
        chans[j] = reinterpret_cast<int*> (buffer->getWritePointer (j, startSample));

    chans[numTargetChannels] = nullptr;
    reader.read (chans, numTargetChannels, readerStartSample, numSamples, true);

    if (convertToFloat)
        convertFixedToFloat (chans, numTargetChannels, numSamples);
}

void AudioFormatReader::read (AudioBuffer<float>* buffer,
                              int startSample,
                              int numSamples,
                              int64 readerStartSample,
                              bool useReaderLeftChan,
                              bool useReaderRightChan)
{
    jassert (buffer != nullptr);
    jassert (startSample >= 0 && startSample + numSamples <= buffer->getNumSamples());

    if (numSamples > 0)
    {
        auto numTargetChannels = buffer->getNumChannels();

        if (numTargetChannels <= 2)
        {
            int* dests[2] = { reinterpret_cast<int*> (buffer->getWritePointer (0, startSample)),
                              reinterpret_cast<int*> (numTargetChannels > 1 ? buffer->getWritePointer (1, startSample) : nullptr) };
            int* chans[3] = {};

            if (useReaderLeftChan == useReaderRightChan)
            {
                chans[0] = dests[0];

                if (numChannels > 1)
                    chans[1] = dests[1];
            }
            else if (useReaderLeftChan || (numChannels == 1))
            {
                chans[0] = dests[0];
            }
            else if (useReaderRightChan)
            {
                chans[1] = dests[0];
            }

            read (chans, 2, readerStartSample, numSamples, true);

            // if the target's stereo and the source is mono, dupe the first channel..
            if (numTargetChannels > 1
                && (chans[0] == nullptr || chans[1] == nullptr)
                && (dests[0] != nullptr && dests[1] != nullptr))
            {
                memcpy (dests[1], dests[0], (size_t) numSamples * sizeof (float));
            }

            if (! usesFloatingPointData)
                convertFixedToFloat (dests, 2, numSamples);
        }
        else if (numTargetChannels <= 64)
        {
            int* chans[65];
            readChannels (*this, chans, buffer, startSample, numSamples,
                          readerStartSample, numTargetChannels, ! usesFloatingPointData);
        }
        else
        {
            HeapBlock<int*> chans (numTargetChannels + 1);
            readChannels (*this, chans, buffer, startSample, numSamples,
                          readerStartSample, numTargetChannels, ! usesFloatingPointData);
        }
    }
}

void AudioFormatReader::readMaxLevels (int64 startSampleInFile, int64 numSamples,
                                       Range<float>* const results, const int channelsToRead)
{
    jassert (channelsToRead > 0 && channelsToRead <= (int) numChannels);

    if (numSamples <= 0)
    {
        for (int i = 0; i < channelsToRead; ++i)
            results[i] = Range<float>();

        return;
    }

    auto bufferSize = (int) jmin (numSamples, (int64) 4096);
    AudioBuffer<float> tempSampleBuffer ((int) channelsToRead, bufferSize);

    auto floatBuffer = tempSampleBuffer.getArrayOfWritePointers();
    auto intBuffer = reinterpret_cast<int* const*> (floatBuffer);
    bool isFirstBlock = true;

    while (numSamples > 0)
    {
        auto numToDo = (int) jmin (numSamples, (int64) bufferSize);

        if (! read (intBuffer, channelsToRead, startSampleInFile, numToDo, false))
            break;

        for (int i = 0; i < channelsToRead; ++i)
        {
            Range<float> r;

            if (usesFloatingPointData)
            {
                r = FloatVectorOperations::findMinAndMax (floatBuffer[i], numToDo);
            }
            else
            {
                auto intRange = Range<int>::findMinAndMax (intBuffer[i], numToDo);

                r = Range<float> ((float) intRange.getStart() / (float) std::numeric_limits<int>::max(),
                                  (float) intRange.getEnd()   / (float) std::numeric_limits<int>::max());
            }

            results[i] = isFirstBlock ? r : results[i].getUnionWith (r);
        }

        isFirstBlock = false;
        numSamples -= numToDo;
        startSampleInFile += numToDo;
    }
}

void AudioFormatReader::readMaxLevels (int64 startSampleInFile, int64 numSamples,
                                       float& lowestLeft, float& highestLeft,
                                       float& lowestRight, float& highestRight)
{
    Range<float> levels[2];

    if (numChannels < 2)
    {
        readMaxLevels (startSampleInFile, numSamples, levels, (int) numChannels);
        levels[1] = levels[0];
    }
    else
    {
        readMaxLevels (startSampleInFile, numSamples, levels, 2);
    }

    lowestLeft   = levels[0].getStart();
    highestLeft  = levels[0].getEnd();
    lowestRight  = levels[1].getStart();
    highestRight = levels[1].getEnd();
}

int64 AudioFormatReader::searchForLevel (int64 startSample,
                                         int64 numSamplesToSearch,
                                         double magnitudeRangeMinimum,
                                         double magnitudeRangeMaximum,
                                         int minimumConsecutiveSamples)
{
    if (numSamplesToSearch == 0)
        return -1;

    const int bufferSize = 4096;
    HeapBlock<int> tempSpace (bufferSize * 2 + 64);

    int* tempBuffer[3] = { tempSpace.get(),
                           tempSpace.get() + bufferSize,
                           nullptr };

    int consecutive = 0;
    int64 firstMatchPos = -1;

    jassert (magnitudeRangeMaximum > magnitudeRangeMinimum);

    auto doubleMin = jlimit (0.0, (double) std::numeric_limits<int>::max(), magnitudeRangeMinimum * std::numeric_limits<int>::max());
    auto doubleMax = jlimit (doubleMin, (double) std::numeric_limits<int>::max(), magnitudeRangeMaximum * std::numeric_limits<int>::max());
    auto intMagnitudeRangeMinimum = roundToInt (doubleMin);
    auto intMagnitudeRangeMaximum = roundToInt (doubleMax);

    while (numSamplesToSearch != 0)
    {
        auto numThisTime = (int) jmin (std::abs (numSamplesToSearch), (int64) bufferSize);
        int64 bufferStart = startSample;

        if (numSamplesToSearch < 0)
            bufferStart -= numThisTime;

        if (bufferStart >= lengthInSamples)
            break;

        read (tempBuffer, 2, bufferStart, numThisTime, false);
        auto num = numThisTime;

        while (--num >= 0)
        {
            if (numSamplesToSearch < 0)
                --startSample;

            bool matches = false;
            auto index = (int) (startSample - bufferStart);

            if (usesFloatingPointData)
            {
                const float sample1 = std::abs (((float*) tempBuffer[0]) [index]);

                if (sample1 >= magnitudeRangeMinimum
                     && sample1 <= magnitudeRangeMaximum)
                {
                    matches = true;
                }
                else if (numChannels > 1)
                {
                    const float sample2 = std::abs (((float*) tempBuffer[1]) [index]);

                    matches = (sample2 >= magnitudeRangeMinimum
                                 && sample2 <= magnitudeRangeMaximum);
                }
            }
            else
            {
                const int sample1 = std::abs (tempBuffer[0] [index]);

                if (sample1 >= intMagnitudeRangeMinimum
                     && sample1 <= intMagnitudeRangeMaximum)
                {
                    matches = true;
                }
                else if (numChannels > 1)
                {
                    const int sample2 = std::abs (tempBuffer[1][index]);

                    matches = (sample2 >= intMagnitudeRangeMinimum
                                 && sample2 <= intMagnitudeRangeMaximum);
                }
            }

            if (matches)
            {
                if (firstMatchPos < 0)
                    firstMatchPos = startSample;

                if (++consecutive >= minimumConsecutiveSamples)
                {
                    if (firstMatchPos < 0 || firstMatchPos >= lengthInSamples)
                        return -1;

                    return firstMatchPos;
                }
            }
            else
            {
                consecutive = 0;
                firstMatchPos = -1;
            }

            if (numSamplesToSearch > 0)
                ++startSample;
        }

        if (numSamplesToSearch > 0)
            numSamplesToSearch -= numThisTime;
        else
            numSamplesToSearch += numThisTime;
    }

    return -1;
}

AudioChannelSet AudioFormatReader::getChannelLayout()
{
    return AudioChannelSet::canonicalChannelSet (static_cast<int> (numChannels));
}

//==============================================================================
MemoryMappedAudioFormatReader::MemoryMappedAudioFormatReader (const File& f, const AudioFormatReader& reader,
                                                              int64 start, int64 length, int frameSize)
    : AudioFormatReader (nullptr, reader.getFormatName()), file (f),
      dataChunkStart (start), dataLength (length), bytesPerFrame (frameSize)
{
    sampleRate      = reader.sampleRate;
    bitsPerSample   = reader.bitsPerSample;
    lengthInSamples = reader.lengthInSamples;
    numChannels     = reader.numChannels;
    metadataValues  = reader.metadataValues;
    usesFloatingPointData = reader.usesFloatingPointData;
}

bool MemoryMappedAudioFormatReader::mapEntireFile()
{
    return mapSectionOfFile (Range<int64> (0, lengthInSamples));
}

bool MemoryMappedAudioFormatReader::mapSectionOfFile (Range<int64> samplesToMap)
{
    if (map == nullptr || samplesToMap != mappedSection)
    {
        map.reset();

        const Range<int64> fileRange (sampleToFilePos (samplesToMap.getStart()),
                                      sampleToFilePos (samplesToMap.getEnd()));

        map.reset (new MemoryMappedFile (file, fileRange, MemoryMappedFile::readOnly));

        if (map->getData() == nullptr)
            map.reset();
        else
            mappedSection = Range<int64> (jmax ((int64) 0, filePosToSample (map->getRange().getStart() + (bytesPerFrame - 1))),
                                          jmin (lengthInSamples, filePosToSample (map->getRange().getEnd())));
    }

    return map != nullptr;
}

static int memoryReadDummyVariable; // used to force the compiler not to optimise-away the read operation

void MemoryMappedAudioFormatReader::touchSample (int64 sample) const noexcept
{
    if (map != nullptr && mappedSection.contains (sample))
        memoryReadDummyVariable += *(char*) sampleToPointer (sample);
    else
        jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.
}

} // namespace juce
