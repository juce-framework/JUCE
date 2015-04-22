/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

AudioSubsectionReader::AudioSubsectionReader (AudioFormatReader* const source_,
                                              const int64 startSample_,
                                              const int64 length_,
                                              const bool deleteSourceWhenDeleted_)
   : AudioFormatReader (0, source_->getFormatName()),
     source (source_),
     startSample (startSample_),
     deleteSourceWhenDeleted (deleteSourceWhenDeleted_)
{
    length = jmin (jmax ((int64) 0, source->lengthInSamples - startSample), length_);

    sampleRate = source->sampleRate;
    bitsPerSample = source->bitsPerSample;
    lengthInSamples = length;
    numChannels = source->numChannels;
    usesFloatingPointData = source->usesFloatingPointData;
}

AudioSubsectionReader::~AudioSubsectionReader()
{
    if (deleteSourceWhenDeleted)
        delete source;
}

//==============================================================================
bool AudioSubsectionReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                         int64 startSampleInFile, int numSamples)
{
    clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                       startSampleInFile, numSamples, length);

    return source->readSamples (destSamples, numDestChannels, startOffsetInDestBuffer,
                                startSampleInFile + startSample, numSamples);
}

void AudioSubsectionReader::readMaxLevels (int64 startSampleInFile, int64 numSamples, Range<float>* results, int numChannelsToRead)
{
    startSampleInFile = jmax ((int64) 0, startSampleInFile);
    numSamples = jmax ((int64) 0, jmin (numSamples, length - startSampleInFile));

    source->readMaxLevels (startSampleInFile + startSample, numSamples, results, numChannelsToRead);
}
