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

int64 juce_fileSetPosition (void* handle, int64 pos);


//==============================================================================
FileInputStream::FileInputStream (const File& f)  : file (f)
{
    openHandle();
}

int64 FileInputStream::getTotalLength()
{
    // You should always check that a stream opened successfully before using it!
    jassert (openedOk());

    return file.getSize();
}

int FileInputStream::read (void* buffer, int bytesToRead)
{
    // You should always check that a stream opened successfully before using it!
    jassert (openedOk());

    // The buffer should never be null, and a negative size is probably a
    // sign that something is broken!
    jassert (buffer != nullptr && bytesToRead >= 0);

    auto num = readInternal (buffer, (size_t) bytesToRead);
    currentPosition += (int64) num;

    return (int) num;
}

bool FileInputStream::isExhausted()
{
    return currentPosition >= getTotalLength();
}

int64 FileInputStream::getPosition()
{
    return currentPosition;
}

bool FileInputStream::setPosition (int64 pos)
{
    // You should always check that a stream opened successfully before using it!
    jassert (openedOk());

    if (pos != currentPosition)
        currentPosition = juce_fileSetPosition (fileHandle, pos);

    return currentPosition == pos;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct FileInputStreamTests final : public UnitTest
{
    FileInputStreamTests()
        : UnitTest ("FileInputStream", UnitTestCategories::streams)
    {}

    void runTest() override
    {
        beginTest ("Open stream non-existent file");
        {
            auto tempFile = File::createTempFile (".txt");
            expect (! tempFile.exists());

            FileInputStream stream (tempFile);
            expect (stream.failedToOpen());
        }

        beginTest ("Open stream existing file");
        {
            auto tempFile = File::createTempFile (".txt");
            tempFile.create();
            expect (tempFile.exists());

            FileInputStream stream (tempFile);
            expect (stream.openedOk());
        }

        const MemoryBlock data ("abcdefghijklmnopqrstuvwxyz", 26);
        File f (File::createTempFile (".txt"));
        f.appendData (data.getData(), data.getSize());
        FileInputStream stream (f);

        beginTest ("Read");
        {
            expectEquals (stream.getPosition(), (int64) 0);
            expectEquals (stream.getTotalLength(), (int64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
            expect (! stream.isExhausted());

            size_t numBytesRead = 0;
            MemoryBlock readBuffer (data.getSize());

            while (numBytesRead < data.getSize())
            {
                numBytesRead += (size_t) stream.read (&readBuffer[numBytesRead], 3);

                expectEquals (stream.getPosition(), (int64) numBytesRead);
                expectEquals (stream.getNumBytesRemaining(), (int64) (data.getSize() - numBytesRead));
                expect (stream.isExhausted() == (numBytesRead == data.getSize()));
            }

            expectEquals (stream.getPosition(), (int64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), (int64) 0);
            expect (stream.isExhausted());

            expect (readBuffer == data);
        }

        beginTest ("Skip");
        {
            stream.setPosition (0);
            expectEquals (stream.getPosition(), (int64) 0);
            expectEquals (stream.getTotalLength(), (int64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
            expect (! stream.isExhausted());

            size_t numBytesRead = 0;
            const int numBytesToSkip = 5;

            while (numBytesRead < data.getSize())
            {
                stream.skipNextBytes (numBytesToSkip);
                numBytesRead += numBytesToSkip;
                numBytesRead = std::min (numBytesRead, data.getSize());

                expectEquals (stream.getPosition(), (int64) numBytesRead);
                expectEquals (stream.getNumBytesRemaining(), (int64) (data.getSize() - numBytesRead));
                expect (stream.isExhausted() == (numBytesRead == data.getSize()));
            }

            expectEquals (stream.getPosition(), (int64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), (int64) 0);
            expect (stream.isExhausted());

            f.deleteFile();
        }
    }
};

static FileInputStreamTests fileInputStreamTests;

#endif

} // namespace juce
