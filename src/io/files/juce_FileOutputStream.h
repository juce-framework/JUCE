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

        It's better to use File::createOutputStream() to create one of these, rather
        than using the class directly.

        @see TemporaryFile
    */
    FileOutputStream (const File& fileToWriteTo,
                      const int bufferSizeToUse = 16384);

    /** Destructor. */
    ~FileOutputStream();

    //==============================================================================
    /** Returns the file that this stream is writing to.
    */
    const File& getFile() const                         { return file; }

    /** Returns true if the stream couldn't be opened for some reason.
    */
    bool failedToOpen() const                           { return fileHandle == 0; }

    //==============================================================================
    void flush();
    int64 getPosition();
    bool setPosition (int64 pos);
    bool write (const void* data, int numBytes);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    File file;
    void* fileHandle;
    int64 currentPosition;
    int bufferSize, bytesInBuffer;
    HeapBlock <char> buffer;
};

#endif   // __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__
