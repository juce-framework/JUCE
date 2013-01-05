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

SubregionStream::SubregionStream (InputStream* const sourceStream,
                                  const int64 startPositionInSourceStream_,
                                  const int64 lengthOfSourceStream_,
                                  const bool deleteSourceWhenDestroyed)
  : source (sourceStream, deleteSourceWhenDestroyed),
    startPositionInSourceStream (startPositionInSourceStream_),
    lengthOfSourceStream (lengthOfSourceStream_)
{
    SubregionStream::setPosition (0);
}

SubregionStream::~SubregionStream()
{
}

int64 SubregionStream::getTotalLength()
{
    const int64 srcLen = source->getTotalLength() - startPositionInSourceStream;

    return (lengthOfSourceStream >= 0) ? jmin (lengthOfSourceStream, srcLen)
                                       : srcLen;
}

int64 SubregionStream::getPosition()
{
    return source->getPosition() - startPositionInSourceStream;
}

bool SubregionStream::setPosition (int64 newPosition)
{
    return source->setPosition (jmax ((int64) 0, newPosition + startPositionInSourceStream));
}

int SubregionStream::read (void* destBuffer, int maxBytesToRead)
{
    jassert (destBuffer != nullptr && maxBytesToRead >= 0);

    if (lengthOfSourceStream < 0)
    {
        return source->read (destBuffer, maxBytesToRead);
    }
    else
    {
        maxBytesToRead = (int) jmin ((int64) maxBytesToRead, lengthOfSourceStream - getPosition());

        if (maxBytesToRead <= 0)
            return 0;

        return source->read (destBuffer, maxBytesToRead);
    }
}

bool SubregionStream::isExhausted()
{
    if (lengthOfSourceStream >= 0 && getPosition() >= lengthOfSourceStream)
        return true;

    return source->isExhausted();
}
