/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_BufferedInputStream.h"


//==============================================================================
BufferedInputStream::BufferedInputStream (InputStream* const source_,
                                          const int bufferSize_,
                                          const bool deleteSourceWhenDestroyed)
   : source (source_),
     sourceToDelete (deleteSourceWhenDestroyed ? source_ : 0),
     bufferSize (jmax (256, bufferSize_)),
     position (source_->getPosition()),
     lastReadPos (0),
     bufferOverlap (128)
{
    const int sourceSize = (int) source_->getTotalLength();
    if (sourceSize >= 0)
        bufferSize = jmin (jmax (32, sourceSize), bufferSize);

    bufferStart = position;
    buffer.malloc (bufferSize);
}

BufferedInputStream::~BufferedInputStream()
{
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
            memmove (buffer, buffer + (int) (position - bufferStart), bytesToKeep);

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
         && position + maxBytesToRead <= lastReadPos)
    {
        memcpy (destBuffer, buffer + (int) (position - bufferStart), maxBytesToRead);
        position += maxBytesToRead;

        return maxBytesToRead;
    }
    else
    {
        if (position < bufferStart || position >= lastReadPos)
            ensureBuffered();

        int bytesRead = 0;

        while (maxBytesToRead > 0)
        {
            const int bytesAvailable = jmin (maxBytesToRead, (int) (lastReadPos - position));

            if (bytesAvailable > 0)
            {
                memcpy (destBuffer, buffer + (int) (position - bufferStart), bytesAvailable);
                maxBytesToRead -= bytesAvailable;
                bytesRead += bytesAvailable;
                position += bytesAvailable;
                destBuffer = (void*) (((char*) destBuffer) + bytesAvailable);
            }

            const int64 oldLastReadPos = lastReadPos;
            ensureBuffered();

            if (oldLastReadPos == lastReadPos)
                break; // if ensureBuffered() failed to read any more data, bail out

            if (isExhausted())
                break;
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

        const char* const src = buffer + (int) (position - bufferStart);

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
