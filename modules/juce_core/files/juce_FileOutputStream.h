/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An output stream that writes into a local file.

    @see OutputStream, FileInputStream, File::createOutputStream

    @tags{Core}
*/
class JUCE_API  FileOutputStream  : public OutputStream
{
public:
    //==============================================================================
    /** Creates a FileOutputStream.

        If the file doesn't exist, it will first be created. If the file can't be
        created or opened (for example, because the parent directory of the file
        does not exist), the failedToOpen() method will return true.

        If the file already exists when opened, the stream's write-position will
        be set to the end of the file. To overwrite an existing file, you can truncate
        it like this:

        @code
        FileOutputStream stream (file);

        if (stream.openedOk())
        {
            stream.setPosition (0);
            stream.truncate();
            ...
        }
        @endcode


        Destroying a FileOutputStream object does not force the operating system
        to write the buffered data to disk immediately. If this is required you
        should call flush() before triggering the destructor.

        @see TemporaryFile
    */
    FileOutputStream (const File& fileToWriteTo,
                      size_t bufferSizeToUse = 16384);

    /** Destructor. */
    ~FileOutputStream() override;

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
    void* fileHandle = nullptr;
    Result status { Result::ok() };
    int64 currentPosition = 0;
    size_t bufferSize, bytesInBuffer = 0;
    HeapBlock<char> buffer;

    void openHandle();
    void closeHandle();
    void flushInternal();
    bool flushBuffer();
    int64 setPositionInternal (int64);
    ssize_t writeInternal (const void*, size_t);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileOutputStream)
};

} // namespace juce
