/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "juce_FileOutputStream.h"

int64 juce_fileSetPosition (void* handle, int64 pos);


//==============================================================================
FileOutputStream::FileOutputStream (const File& f, const int bufferSize_)
    : file (f),
      fileHandle (0),
      currentPosition (0),
      bufferSize (bufferSize_),
      bytesInBuffer (0),
      buffer (jmax (bufferSize_, 16))
{
    openHandle();
}

FileOutputStream::~FileOutputStream()
{
    flush();
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
        flush();
        currentPosition = juce_fileSetPosition (fileHandle, newPosition);
    }

    return newPosition == currentPosition;
}

void FileOutputStream::flush()
{
    if (bytesInBuffer > 0)
    {
        writeInternal (buffer, bytesInBuffer);
        bytesInBuffer = 0;
    }

    flushInternal();
}

bool FileOutputStream::write (const void* const src, const int numBytes)
{
    if (bytesInBuffer + numBytes < bufferSize)
    {
        memcpy (buffer + bytesInBuffer, src, numBytes);
        bytesInBuffer += numBytes;
        currentPosition += numBytes;
    }
    else
    {
        if (bytesInBuffer > 0)
        {
            // flush the reservoir
            const bool wroteOk = (writeInternal (buffer, bytesInBuffer) == bytesInBuffer);
            bytesInBuffer = 0;

            if (! wroteOk)
                return false;
        }

        if (numBytes < bufferSize)
        {
            memcpy (buffer + bytesInBuffer, src, numBytes);
            bytesInBuffer += numBytes;
            currentPosition += numBytes;
        }
        else
        {
            const int bytesWritten = writeInternal (src, numBytes);

            if (bytesWritten < 0)
                return false;

            currentPosition += bytesWritten;
            return bytesWritten == numBytes;
        }
    }

    return true;
}

END_JUCE_NAMESPACE
