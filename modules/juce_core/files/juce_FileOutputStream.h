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

#ifndef __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__
#define __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__

#include "juce_File.h"
#include "../streams/juce_OutputStream.h"


//==============================================================================
/**
    An output stream that writes into a local file.

    @see OutputStream, FileInputStream, File::createOutputStream
*/
class JUCE_API  FileOutputStream  : public OutputStream
{
public:
    //==============================================================================
    /** Creates a FileOutputStream.

        If the file doesn't exist, it will first be created. If the file can't be
        created or opened, the failedToOpen() method will return
        true.

        If the file already exists when opened, the stream's write-postion will
        be set to the end of the file. To overwrite an existing file,
        use File::deleteFile() before opening the stream, or use setPosition(0)
        after it's opened (although this won't truncate the file).

        @see TemporaryFile
    */
    FileOutputStream (const File& fileToWriteTo,
                      int bufferSizeToUse = 16384);

    /** Destructor. */
    ~FileOutputStream();

    //==============================================================================
    /** Returns the file that this stream is writing to.
    */
    const File& getFile() const                         { return file; }

    /** Returns the status of the file stream.
        The result will be ok if the file opened successfully. If an error occurs while
        opening or writing to the file, this will contain an error message.
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

    /** Attempts to truncate the file to the current write position.
        To truncate a file to a specific size, first use setPosition() to seek to the
        appropriate location, and then call this method.
    */
    Result truncate();

    //==============================================================================
    void flush();
    int64 getPosition();
    bool setPosition (int64 pos);
    bool write (const void* data, int numBytes);
    void writeRepeatedByte (uint8 byte, int numTimesToRepeat);


private:
    //==============================================================================
    File file;
    void* fileHandle;
    Result status;
    int64 currentPosition;
    int bufferSize, bytesInBuffer;
    HeapBlock <char> buffer;

    void openHandle();
    void closeHandle();
    void flushInternal();
    bool flushBuffer();
    int64 setPositionInternal (int64 newPosition);
    int writeInternal (const void* data, int numBytes);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileOutputStream)
};

#endif   // __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__
