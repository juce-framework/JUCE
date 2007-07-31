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

#ifndef __JUCE_FILEINPUTSTREAM_JUCEHEADER__
#define __JUCE_FILEINPUTSTREAM_JUCEHEADER__

#include "juce_File.h"
#include "../juce_InputStream.h"


//==============================================================================
/**
    An input stream that reads from a local file.

    @see InputStream, FileOutputStream, File::createInputStream
*/
class JUCE_API  FileInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a FileInputStream.

        @param fileToRead   the file to read from - if the file can't be accessed for some
                            reason, then the stream will just contain no data
    */
    FileInputStream (const File& fileToRead);

    /** Destructor. */
    ~FileInputStream();

    //==============================================================================
    const File& getFile() const throw()                     { return file; }

    //==============================================================================
    int64 getTotalLength();
    int read (void* destBuffer, int maxBytesToRead);
    bool isExhausted();
    int64 getPosition();
    bool setPosition (int64 pos);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    File file;
    void* fileHandle;
    int64 currentPosition, totalSize;
    bool needToSeek;
};

#endif   // __JUCE_FILEINPUTSTREAM_JUCEHEADER__
