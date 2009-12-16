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

#ifndef __JUCE_FILEINPUTSTREAM_JUCEHEADER__
#define __JUCE_FILEINPUTSTREAM_JUCEHEADER__

#include "juce_File.h"
#include "../streams/juce_InputStream.h"


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

    FileInputStream (const FileInputStream&);
    const FileInputStream& operator= (const FileInputStream&);
};

#endif   // __JUCE_FILEINPUTSTREAM_JUCEHEADER__
