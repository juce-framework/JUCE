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
    explicit FileInputStream (const File& fileToRead);

    /** Destructor. */
    ~FileInputStream();

    //==============================================================================
    /** Returns the file that this stream is reading from. */
    const File& getFile() const noexcept                { return file; }

    /** Returns the status of the file stream.
        The result will be ok if the file opened successfully. If an error occurs while
        opening or reading from the file, this will contain an error message.
    */
    const Result& getStatus() const noexcept            { return status; }

    /** Returns true if the stream couldn't be opened for some reason.
        @see getResult()
    */
    bool failedToOpen() const noexcept                  { return status.failed(); }

    /** Returns true if the stream opened without problems.
        @see getResult()
    */
    bool openedOk() const noexcept                      { return status.wasOk(); }


    //==============================================================================
    int64 getTotalLength();
    int read (void* destBuffer, int maxBytesToRead);
    bool isExhausted();
    int64 getPosition();
    bool setPosition (int64 pos);

private:
    //==============================================================================
    File file;
    void* fileHandle;
    int64 currentPosition;
    Result status;
    bool needToSeek;

    void openHandle();
    void closeHandle();
    size_t readInternal (void* buffer, size_t numBytes);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInputStream)
};

#endif   // __JUCE_FILEINPUTSTREAM_JUCEHEADER__
