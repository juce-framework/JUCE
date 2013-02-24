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

void AudioSubsectionReader::readMaxLevels (int64 startSampleInFile,
                                           int64 numSamples,
                                           float& lowestLeft,
                                           float& highestLeft,
                                           float& lowestRight,
                                           float& highestRight)
{
    startSampleInFile = jmax ((int64) 0, startSampleInFile);
    numSamples = jmax ((int64) 0, jmin (numSamples, length - startSampleInFile));

    source->readMaxLevels (startSampleInFile + startSample, numSamples,
                           lowestLeft, highestLeft,
                           lowestRight, highestRight);
}
