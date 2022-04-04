/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioFormatReaderSource::AudioFormatReaderSource (AudioFormatReader* const r,
                                                  const bool deleteReaderWhenThisIsDeleted)
    : reader (r, deleteReaderWhenThisIsDeleted),
      nextPlayPos (0),
      looping (false)
{
    jassert (reader != nullptr);
}

AudioFormatReaderSource::~AudioFormatReaderSource() {}

int64 AudioFormatReaderSource::getTotalLength() const                   { return reader->lengthInSamples; }
void AudioFormatReaderSource::setNextReadPosition (int64 newPosition)   { nextPlayPos = newPosition; }
void AudioFormatReaderSource::setLooping (bool shouldLoop)              { looping = shouldLoop; }

int64 AudioFormatReaderSource::getNextReadPosition() const
{
    return looping ? nextPlayPos % reader->lengthInSamples
                   : nextPlayPos;
}

void AudioFormatReaderSource::prepareToPlay (int /*samplesPerBlockExpected*/, double /*sampleRate*/) {}
void AudioFormatReaderSource::releaseResources() {}

void AudioFormatReaderSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (info.numSamples > 0)
    {
        const int64 start = nextPlayPos;

        if (looping)
        {
            const int64 newStart = start % reader->lengthInSamples;
            const int64 newEnd = (start + info.numSamples) % reader->lengthInSamples;

            if (newEnd > newStart)
            {
                reader->read (info.buffer, info.startSample,
                              (int) (newEnd - newStart), newStart, true, true);
            }
            else
            {
                const int endSamps = (int) (reader->lengthInSamples - newStart);

                reader->read (info.buffer, info.startSample,
                              endSamps, newStart, true, true);

                reader->read (info.buffer, info.startSample + endSamps,
                              (int) newEnd, 0, true, true);
            }

            nextPlayPos = newEnd;
        }
        else
        {
            reader->read (info.buffer, info.startSample,
                          info.numSamples, start, true, true);
            nextPlayPos += info.numSamples;
        }
    }
}

} // namespace juce
