/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_FILEOUTPUTSTREAM_H_INCLUDED
#define JUCE_FILEOUTPUTSTREAM_H_INCLUDED


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
                      size_t bufferSizeToUse = 16384);

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
    void flush() override;
    int64 getPosition() override;
    bool setPosition (int64) override;
    bool write (const void*, size_t) override;
    bool writeRepeatedByte (uint8 byte, size_t numTimesToRepeat) override;


private:
    //==============================================================================
    File file;
    void* fileHandle;
    Result status;
    int64 currentPosition;
    size_t bufferSize, bytesInBuffer;
    HeapBlock <char> buffer;

    void openHandle();
    void closeHandle();
    void flushInternal();
    bool flushBuffer();
    int64 setPositionInternal (int64);
    ssize_t writeInternal (const void*, size_t);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileOutputStream)
};

#endif   // JUCE_FILEOUTPUTSTREAM_H_INCLUDED
