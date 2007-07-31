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


#include "juce_BufferedInputStream.h"


//==============================================================================
BufferedInputStream::BufferedInputStream (InputStream* const source_,
                                          const int bufferSize_,
                                          const bool deleteSourceWhenDestroyed_) throw()
   : source (source_),
     deleteSourceWhenDestroyed (deleteSourceWhenDestroyed_),
     bufferSize (jmax (256, bufferSize_)),
     position (source_->getPosition()),
     lastReadPos (0),
     bufferOverlap (128)
{
    const int sourceSize = (int) source_->getTotalLength();
    if (sourceSize >= 0)
        bufferSize = jmin (jmax (32, sourceSize), bufferSize);

    bufferStart = position + 1;
    buffer = (char*) juce_malloc (bufferSize);
}

BufferedInputStream::~BufferedInputStream() throw()
{
    if (deleteSourceWhenDestroyed)
        delete source;

    juce_free (buffer);
}

//==============================================================================
int64 BufferedInputStream::getTotalLength()
{
    return source->getTotalLength();
}

int64 BufferedInputStream::getPosition()
{
    return position;
}

bool BufferedInputStream::setPosition (int64 newPosition)
{
    position = jmax ((int64) 0, newPosition);
    return true;
}

bool BufferedInputStream::isExhausted()
{
    return (position >= lastReadPos)
             && source->isExhausted();
}

void BufferedInputStream::ensureBuffered()
{
    const int64 bufferEndOverlap = lastReadPos - bufferOverlap;

    if (position < bufferStart || position >= bufferEndOverlap)
    {
        int bytesRead;

        if (position < lastReadPos
             && position >= bufferEndOverlap
             && position >= bufferStart)
        {
            const int bytesToKeep = (int) (lastReadPos - position);
            memmove (buffer, buffer + position - bufferStart, bytesToKeep);

            bufferStart = position;

            bytesRead = source->read (buffer + bytesToKeep,
                                      bufferSize - bytesToKeep);

            lastReadPos += bytesRead;
            bytesRead += bytesToKeep;
        }
        else
        {
            bufferStart = position;
            source->setPosition (bufferStart);
            bytesRead = source->read (buffer, bufferSize);
            lastReadPos = bufferStart + bytesRead;
        }

        while (bytesRead < bufferSize)
            buffer [bytesRead++] = 0;
    }
}

int BufferedInputStream::read (void* destBuffer, int maxBytesToRead)
{
    if (position >= bufferStart
         && position + maxBytesToRead < lastReadPos)
    {
        memcpy (destBuffer, buffer + (position - bufferStart), maxBytesToRead);
        position += maxBytesToRead;

        return maxBytesToRead;
    }
    else
    {
        int bytesRead = 0;
        char* d = (char*) destBuffer;

        while (bytesRead < maxBytesToRead)
        {
            ensureBuffered();

            if (isExhausted())
                break;

            *d++ = buffer [position - bufferStart];
            ++position;
            ++bytesRead;
        }

        return bytesRead;
    }
}

const String BufferedInputStream::readString()
{
    if (position >= bufferStart
         && position < lastReadPos)
    {
        const int maxChars = (int) (lastReadPos - position);

        const char* const src = buffer + (position - bufferStart);

        for (int i = 0; i < maxChars; ++i)
        {
            if (src[i] == 0)
            {
                position += i + 1;

                return String::fromUTF8 ((const uint8*) src, i);
            }
        }
    }

    return InputStream::readString();
}

END_JUCE_NAMESPACE
