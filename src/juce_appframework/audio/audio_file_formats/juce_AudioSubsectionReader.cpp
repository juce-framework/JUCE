/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioSubsectionReader.h"


//==============================================================================
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
bool AudioSubsectionReader::read (int** destSamples,
                                  int64 startSampleInFile,
                                  int numSamples)
{
    if (startSampleInFile < 0 || startSampleInFile + numSamples > length)
    {
        int** d = destSamples;
        while (*d != 0)
        {
            zeromem (*d, sizeof (int) * numSamples);
            ++d;
        }

        startSampleInFile = jmax ((int64) 0, startSampleInFile);
        numSamples = jmax (0, jmin (numSamples, (int) (length - startSampleInFile)));
    }

    return source->read (destSamples,
                         startSampleInFile + startSample,
                         numSamples);
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

    source->readMaxLevels (startSampleInFile + startSample,
                           numSamples,
                           lowestLeft,
                           highestLeft,
                           lowestRight,
                           highestRight);
}

END_JUCE_NAMESPACE
