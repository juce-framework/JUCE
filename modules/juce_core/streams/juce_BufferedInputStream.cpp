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

static int calcBufferStreamBufferSize (int requestedSize, InputStream* source) noexcept
{
    // You need to supply a real stream when creating a BufferedInputStream
    jassert (source != nullptr);

    requestedSize = jmax (256, requestedSize);
    auto sourceSize = source->getTotalLength();

    if (sourceSize >= 0 && sourceSize < requestedSize)
        return jmax (32, (int) sourceSize);

    return requestedSize;
}

//==============================================================================
BufferedInputStream::BufferedInputStream (InputStream* sourceStream, int size, bool takeOwnership)
    : source (sourceStream, takeOwnership),
      bufferedRange (sourceStream->getPosition(), sourceStream->getPosition()),
      position (bufferedRange.getStart()),
      bufferLength (calcBufferStreamBufferSize (size, sourceStream))
{
    buffer.malloc (bufferLength);
}

BufferedInputStream::BufferedInputStream (InputStream& sourceStream, int size)
    : BufferedInputStream (&sourceStream, size, false)
{
}

BufferedInputStream::~BufferedInputStream() = default;

//==============================================================================
char BufferedInputStream::peekByte()
{
    if (! ensureBuffered())
        return 0;

    return position < lastReadPos ? buffer[(int) (position - bufferedRange.getStart())] : 0;
}

int64 BufferedInputStream::getTotalLength()
{
    return source->getTotalLength();
}

int64 BufferedInputStream::getPosition()
{
    return position;
}

bool BufferedInputStream::setPosition (int64 newPosition)
{
    position = jmax ((int64) 0, newPosition);
    return true;
}

bool BufferedInputStream::isExhausted()
{
    return position >= lastReadPos && source->isExhausted();
}

bool BufferedInputStream::ensureBuffered()
{
    auto bufferEndOverlap = lastReadPos - bufferOverlap;

    if (position < bufferedRange.getStart() || position >= bufferEndOverlap)
    {
        int bytesRead = 0;

        if (position < lastReadPos
             && position >= bufferEndOverlap
             && position >= bufferedRange.getStart())
        {
            auto bytesToKeep = (int) (lastReadPos - position);
            memmove (buffer, buffer + (int) (position - bufferedRange.getStart()), (size_t) bytesToKeep);

            bytesRead = source->read (buffer + bytesToKeep,
                                      (int) (bufferLength - bytesToKeep));

            if (bytesRead < 0)
                return false;

            lastReadPos += bytesRead;
            bytesRead += bytesToKeep;
        }
        else
        {
            if (! source->setPosition (position))
                return false;

            bytesRead = (int) source->read (buffer, (size_t) bufferLength);

            if (bytesRead < 0)
                return false;

            lastReadPos = position + bytesRead;
        }

        bufferedRange = Range<int64> (position, lastReadPos);

        while (bytesRead < bufferLength)
            buffer[bytesRead++] = 0;
    }

    return true;
}

int BufferedInputStream::read (void* destBuffer, const int maxBytesToRead)
{
    const auto initialPosition = position;

    const auto getBufferedRange = [this] { return bufferedRange; };

    const auto readFromReservoir = [this, &destBuffer, &initialPosition] (const Range<int64> rangeToRead)
    {
        memcpy (static_cast<char*> (destBuffer) + (rangeToRead.getStart() - initialPosition),
                buffer + (rangeToRead.getStart() - bufferedRange.getStart()),
                (size_t) rangeToRead.getLength());
    };

    const auto fillReservoir = [this] (int64 requestedStart)
    {
        position = requestedStart;
        ensureBuffered();
    };

    const auto remaining = Reservoir::doBufferedRead (Range<int64> (position, position + maxBytesToRead),
                                                      getBufferedRange,
                                                      readFromReservoir,
                                                      fillReservoir);

    const auto bytesRead = maxBytesToRead - remaining.getLength();
    position = remaining.getStart();
    return (int) bytesRead;
}

String BufferedInputStream::readString()
{
    if (position >= bufferedRange.getStart()
         && position < lastReadPos)
    {
        auto maxChars = (int) (lastReadPos - position);
        auto* src = buffer + (int) (position - bufferedRange.getStart());

        for (int i = 0; i < maxChars; ++i)
        {
            if (src[i] == 0)
            {
                position += i + 1;
                return String::fromUTF8 (src, i);
            }
        }
    }

    return InputStream::readString();
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct BufferedInputStreamTests final : public UnitTest
{
    template <typename Fn, size_t... Ix, typename Values>
    static void applyImpl (Fn&& fn, std::index_sequence<Ix...>, Values&& values)
    {
        fn (std::get<Ix> (values)...);
    }

    template <typename Fn, typename... Values>
    static void apply (Fn&& fn, std::tuple<Values...> values)
    {
        applyImpl (fn, std::make_index_sequence<sizeof... (Values)>(), values);
    }

    template <typename Fn, typename Values>
    static void allCombinationsImpl (Fn&& fn, Values&& values)
    {
        apply (fn, values);
    }

    template <typename Fn, typename Values, typename Range, typename... Ranges>
    static void allCombinationsImpl (Fn&& fn, Values&& values, Range&& range, Ranges&&... ranges)
    {
        for (auto& item : range)
            allCombinationsImpl (fn, std::tuple_cat (values, std::tie (item)), ranges...);
    }

    template <typename Fn, typename... Ranges>
    static void allCombinations (Fn&& fn, Ranges&&... ranges)
    {
        allCombinationsImpl (fn, std::tie(), ranges...);
    }

    BufferedInputStreamTests()
        : UnitTest ("BufferedInputStream", UnitTestCategories::streams)
    {}

    void runTest() override
    {
        const MemoryBlock testBufferA ("abcdefghijklmnopqrstuvwxyz", 26);

        const auto testBufferB = [&]
        {
            MemoryBlock mb { 8192 };
            auto r = getRandom();

            std::for_each (mb.begin(), mb.end(), [&] (char& item)
            {
                item = (char) r.nextInt (std::numeric_limits<char>::max());
            });

            return mb;
        }();

        const MemoryBlock buffers[] { testBufferA, testBufferB };
        const int readSizes[] { 3, 10, 50 };
        const bool shouldPeek[] { false, true };

        const auto runTest = [this] (const MemoryBlock& data, const int readSize, const bool peek)
        {
            MemoryInputStream mi (data, true);

            BufferedInputStream stream (mi, jmin (200, (int) data.getSize()));

            beginTest ("Read");

            expectEquals (stream.getPosition(), (int64) 0);
            expectEquals (stream.getTotalLength(), (int64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
            expect (! stream.isExhausted());

            size_t numBytesRead = 0;
            MemoryBlock readBuffer (data.getSize());

            while (numBytesRead < data.getSize())
            {
                if (peek)
                    expectEquals (stream.peekByte(), *(char*) (data.begin() + numBytesRead));

                const auto startingPos = numBytesRead;
                numBytesRead += (size_t) stream.read (readBuffer.begin() + numBytesRead, readSize);

                expect (std::equal (readBuffer.begin() + startingPos,
                                    readBuffer.begin() + numBytesRead,
                                    data.begin() + startingPos,
                                    data.begin() + numBytesRead));
                expectEquals (stream.getPosition(), (int64) numBytesRead);
                expectEquals (stream.getNumBytesRemaining(), (int64) (data.getSize() - numBytesRead));
                expect (stream.isExhausted() == (numBytesRead == data.getSize()));
            }

            expectEquals (stream.getPosition(), (int64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), (int64) 0);
            expect (stream.isExhausted());

            expect (readBuffer == data);

            beginTest ("Skip");

            stream.setPosition (0);
            expectEquals (stream.getPosition(), (int64) 0);
            expectEquals (stream.getTotalLength(), (int64) data.getSize());
            expectEquals (stream.getNumBytesRemaining(), stream.getTotalLength());
            expect (! stream.isExhausted());

            numBytesRead = 0;
            const int numBytesToSkip = 5;

            while (numBytesRead < data.getSize())
            {
                expectEquals (stream.peekByte(), *(char*) (data.begin() + numBytesRead));

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
        };

        allCombinations (runTest, buffers, readSizes, shouldPeek);
    }
};

static BufferedInputStreamTests bufferedInputStreamTests;

#endif

} // namespace juce
