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

namespace
{
    int calcBufferStreamBufferSize (int requestedSize, InputStream* const source) noexcept
    {
        // You need to supply a real stream when creating a BufferedInputStream
        jassert (source != nullptr);

        requestedSize = jmax (256, requestedSize);

        const int64 sourceSize = source->getTotalLength();
        if (sourceSize >= 0 && sourceSize < requestedSize)
            requestedSize = jmax (32, (int) sourceSize);

        return requestedSize;
    }
}

//==============================================================================
BufferedInputStream::BufferedInputStream (InputStream* const sourceStream, const int bufferSize_,
                                          const bool deleteSourceWhenDestroyed)
   : source (sourceStream, deleteSourceWhenDestroyed),
     bufferSize (calcBufferStreamBufferSize (bufferSize_, sourceStream)),
     position (sourceStream->getPosition()),
     lastReadPos (0),
     bufferStart (position),
     bufferOverlap (128)
{
    buffer.malloc ((size_t) bufferSize);
}

BufferedInputStream::BufferedInputStream (InputStream& sourceStream, const int bufferSize_)
   : source (&sourceStream, false),
     bufferSize (calcBufferStreamBufferSize (bufferSize_, &sourceStream)),
     position (sourceStream.getPosition()),
     lastReadPos (0),
     bufferStart (position),
     bufferOverlap (128)
{
    buffer.malloc ((size_t) bufferSize);
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
            memmove (buffer, buffer + (int) (position - bufferStart), (size_t) bytesToKeep);

            bufferStart = position;

            bytesRead = source->read (buffer + bytesToKeep,
                                      (int) (bufferSize - bytesToKeep));

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
    jassert (destBuffer != nullptr && maxBytesToRead >= 0);

    if (position >= bufferStart
         && position + maxBytesToRead <= lastReadPos)
    {
        memcpy (destBuffer, buffer + (int) (position - bufferStart), (size_t) maxBytesToRead);
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
                memcpy (destBuffer, buffer + (int) (position - bufferStart), (size_t) bytesAvailable);
                maxBytesToRead -= bytesAvailable;
                bytesRead += bytesAvailable;
                position += bytesAvailable;
                destBuffer = static_cast <char*> (destBuffer) + bytesAvailable;
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

String BufferedInputStream::readString()
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
                return String::fromUTF8 (src, i);
            }
        }
    }

    return InputStream::readString();
}
