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

#include "../../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_SubregionStream.h"


//==============================================================================
SubregionStream::SubregionStream (InputStream* const sourceStream,
                                  const int64 startPositionInSourceStream_,
                                  const int64 lengthOfSourceStream_,
                                  const bool deleteSourceWhenDestroyed_) throw()
  : source (sourceStream),
    deleteSourceWhenDestroyed (deleteSourceWhenDestroyed_),
    startPositionInSourceStream (startPositionInSourceStream_),
    lengthOfSourceStream (lengthOfSourceStream_)
{
    setPosition (0);
}

SubregionStream::~SubregionStream() throw()
{
    if (deleteSourceWhenDestroyed)
        delete source;
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
    if (lengthOfSourceStream >= 0)
        return (getPosition() >= lengthOfSourceStream) || source->isExhausted();
    else
        return source->isExhausted();
}

END_JUCE_NAMESPACE
