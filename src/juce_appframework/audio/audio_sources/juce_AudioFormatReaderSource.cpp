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

#include "juce_AudioFormatReaderSource.h"
#include "../../../juce_core/threads/juce_ScopedLock.h"


//==============================================================================
AudioFormatReaderSource::AudioFormatReaderSource (AudioFormatReader* const reader_,
                                                  const bool deleteReaderWhenThisIsDeleted)
    : reader (reader_),
      deleteReader (deleteReaderWhenThisIsDeleted),
      nextPlayPos (0),
      looping (false)
{
    jassert (reader != 0);
}

AudioFormatReaderSource::~AudioFormatReaderSource()
{
    releaseResources();

    if (deleteReader)
        delete reader;
}

void AudioFormatReaderSource::setNextReadPosition (int newPosition)
{
    nextPlayPos = newPosition;
}

void AudioFormatReaderSource::setLooping (const bool shouldLoop) throw()
{
    looping = shouldLoop;
}

int AudioFormatReaderSource::getNextReadPosition() const
{
    return (looping) ? (nextPlayPos % (int) reader->lengthInSamples)
                     : nextPlayPos;
}

int AudioFormatReaderSource::getTotalLength() const
{
    return (int) reader->lengthInSamples;
}

void AudioFormatReaderSource::prepareToPlay (int /*samplesPerBlockExpected*/,
                                             double /*sampleRate*/)
{
}

void AudioFormatReaderSource::releaseResources()
{
}

void AudioFormatReaderSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (info.numSamples > 0)
    {
        const int start = nextPlayPos;

        if (looping)
        {
            const int newStart = start % (int) reader->lengthInSamples;
            const int newEnd = (start + info.numSamples) % (int) reader->lengthInSamples;

            if (newEnd > newStart)
            {
                info.buffer->readFromAudioReader (reader,
                                                  info.startSample,
                                                  newEnd - newStart,
                                                  newStart,
                                                  true, true);
            }
            else
            {
                const int endSamps = (int) reader->lengthInSamples - newStart;

                info.buffer->readFromAudioReader (reader,
                                                  info.startSample,
                                                  endSamps,
                                                  newStart,
                                                  true, true);

                info.buffer->readFromAudioReader (reader,
                                                  info.startSample + endSamps,
                                                  newEnd,
                                                  0,
                                                  true, true);
            }

            nextPlayPos = newEnd;
        }
        else
        {
            info.buffer->readFromAudioReader (reader,
                                              info.startSample,
                                              info.numSamples,
                                              start,
                                              true, true);

            nextPlayPos += info.numSamples;
        }
    }
}

END_JUCE_NAMESPACE
