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

#include "juce_FileOutputStream.h"

void* juce_fileOpen (const String& path, bool forWriting) throw();
void juce_fileClose (void* handle) throw();
int juce_fileWrite (void* handle, const void* buffer, int size) throw();
void juce_fileFlush (void* handle) throw();
int64 juce_fileGetPosition (void* handle) throw();
int64 juce_fileSetPosition (void* handle, int64 pos) throw();


//==============================================================================
FileOutputStream::FileOutputStream (const File& f,
                                    const int bufferSize_)
    : file (f),
      bufferSize (bufferSize_),
      bytesInBuffer (0)
{
    fileHandle = juce_fileOpen (f.getFullPathName(), true);

    if (fileHandle != 0)
    {
        currentPosition = juce_fileGetPosition (fileHandle);

        if (currentPosition < 0)
        {
            jassertfalse
            juce_fileClose (fileHandle);
            fileHandle = 0;
        }
    }

    buffer = (char*) juce_malloc (jmax (bufferSize_, 16));
}

FileOutputStream::~FileOutputStream()
{
    flush();

    juce_fileClose (fileHandle);
    juce_free (buffer);
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
        juce_fileWrite (fileHandle, buffer, bytesInBuffer);
        bytesInBuffer = 0;
    }

    juce_fileFlush (fileHandle);
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
            const bool wroteOk = (juce_fileWrite (fileHandle, buffer, bytesInBuffer) == bytesInBuffer);
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
            const int bytesWritten = juce_fileWrite (fileHandle, src, numBytes);
            currentPosition += bytesWritten;

            return bytesWritten == numBytes;
        }
    }

    return true;
}

END_JUCE_NAMESPACE
