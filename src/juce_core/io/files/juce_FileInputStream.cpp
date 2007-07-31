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

#include "juce_FileInputStream.h"


//==============================================================================
void* juce_fileOpen (const String& path, bool forWriting) throw();
void juce_fileClose (void* handle) throw();
int juce_fileRead (void* handle, void* buffer, int size) throw();
int64 juce_fileSetPosition (void* handle, int64 pos) throw();


//==============================================================================
FileInputStream::FileInputStream (const File& f)
    : file (f),
      currentPosition (0),
      needToSeek (true)
{
    totalSize = f.getSize();

    fileHandle = juce_fileOpen (f.getFullPathName(), false);
}

FileInputStream::~FileInputStream()
{
    juce_fileClose (fileHandle);
}

//==============================================================================
int64 FileInputStream::getTotalLength()
{
    return totalSize;
}

int FileInputStream::read (void* buffer, int bytesToRead)
{
    int num = 0;

    if (needToSeek)
    {
        if (juce_fileSetPosition (fileHandle, currentPosition) < 0)
            return 0;

        needToSeek = false;
    }

    num = juce_fileRead (fileHandle, buffer, bytesToRead);
    currentPosition += num;

    return num;
}

bool FileInputStream::isExhausted()
{
    return currentPosition >= totalSize;
}

int64 FileInputStream::getPosition()
{
    return currentPosition;
}

bool FileInputStream::setPosition (int64 pos)
{
    pos = jlimit ((int64) 0, totalSize, pos);

    needToSeek |= currentPosition != pos;
    currentPosition = pos;

    return true;
}

END_JUCE_NAMESPACE
