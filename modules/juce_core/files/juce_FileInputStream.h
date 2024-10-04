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
    An input stream that reads from a local file.

    @see InputStream, FileOutputStream, File::createInputStream

    @tags{Core}
*/
class JUCE_API  FileInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a FileInputStream to read from the given file.

        After creating a FileInputStream, you should use openedOk() or failedToOpen()
        to make sure that it's OK before trying to read from it! If it failed, you
        can call getStatus() to get more error information.
    */
    explicit FileInputStream (const File& fileToRead);

    /** Destructor. */
    ~FileInputStream() override;

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
    int64 getTotalLength() override;
    int read (void*, int) override;
    bool isExhausted() override;
    int64 getPosition() override;
    bool setPosition (int64) override;

private:
    //==============================================================================
    const File file;
    void* fileHandle = nullptr;
    int64 currentPosition = 0;
    Result status { Result::ok() };

    void openHandle();
    size_t readInternal (void*, size_t);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInputStream)
};

} // namespace juce
