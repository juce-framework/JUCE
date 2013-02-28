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

int64 juce_fileSetPosition (void* handle, int64 pos);

//==============================================================================
FileOutputStream::FileOutputStream (const File& f, const int bufferSize_)
    : file (f),
      fileHandle (nullptr),
      status (Result::ok()),
      currentPosition (0),
      bufferSize (bufferSize_),
      bytesInBuffer (0),
      buffer ((size_t) jmax (bufferSize_, 16))
{
    openHandle();
}

FileOutputStream::~FileOutputStream()
{
    flushBuffer();
    flushInternal();
    closeHandle();
}

int64 FileOutputStream::getPosition()
{
    return currentPosition;
}

bool FileOutputStream::setPosition (int64 newPosition)
{
    if (newPosition != currentPosition)
    {
        flushBuffer();
        currentPosition = juce_fileSetPosition (fileHandle, newPosition);
    }

    return newPosition == currentPosition;
}

bool FileOutputStream::flushBuffer()
{
    bool ok = true;

    if (bytesInBuffer > 0)
    {
        ok = (writeInternal (buffer, bytesInBuffer) == (ssize_t) bytesInBuffer);
        bytesInBuffer = 0;
    }

    return ok;
}

void FileOutputStream::flush()
{
    flushBuffer();
    flushInternal();
}

bool FileOutputStream::write (const void* const src, const size_t numBytes)
{
    jassert (src != nullptr && ((ssize_t) numBytes) >= 0);

    if (bytesInBuffer + numBytes < bufferSize)
    {
        memcpy (buffer + bytesInBuffer, src, numBytes);
        bytesInBuffer += numBytes;
        currentPosition += numBytes;
    }
    else
    {
        if (! flushBuffer())
            return false;

        if (numBytes < bufferSize)
        {
            memcpy (buffer + bytesInBuffer, src, numBytes);
            bytesInBuffer += numBytes;
            currentPosition += numBytes;
        }
        else
        {
            const ssize_t bytesWritten = writeInternal (src, numBytes);

            if (bytesWritten < 0)
                return false;

            currentPosition += bytesWritten;
            return bytesWritten == (ssize_t) numBytes;
        }
    }

    return true;
}

void FileOutputStream::writeRepeatedByte (uint8 byte, size_t numBytes)
{
    jassert (((ssize_t) numBytes) >= 0);

    if (bytesInBuffer + numBytes < bufferSize)
    {
        memset (buffer + bytesInBuffer, byte, numBytes);
        bytesInBuffer += numBytes;
        currentPosition += numBytes;
    }
    else
    {
        OutputStream::writeRepeatedByte (byte, numBytes);
    }
}
