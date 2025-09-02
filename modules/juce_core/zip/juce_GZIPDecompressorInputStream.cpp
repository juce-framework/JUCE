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

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4127 4244 4309 4305 4365 6385 6326 6340)

namespace zlibNamespace
{
 #if JUCE_INCLUDE_ZLIB_CODE
  JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wconversion",
                                       "-Wsign-conversion",
                                       "-Wshadow",
                                       "-Wdeprecated-register",
                                       "-Wswitch-enum",
                                       "-Wswitch-default",
                                       "-Wredundant-decls",
                                       "-Wimplicit-fallthrough",
                                       "-Wzero-as-null-pointer-constant",
                                       "-Wcomma",
                                       "-Wcast-align",
                                       "-Wkeyword-macro",
                                       "-Wmissing-prototypes")

  #pragma push_macro ("register")
  #define register

  #pragma push_macro ("MIN")
  #undef MIN

  #pragma push_macro ("read")
  #pragma push_macro ("write")
  #pragma push_macro ("open")
  #pragma push_macro ("close")

  #undef OS_CODE
  #undef fdopen
  #define ZLIB_INTERNAL
  #define NO_DUMMY_DECL
  #include "zlib/adler32.c"
  #include "zlib/compress.c"
  #undef DO1
  #undef DO8
  #include "zlib/crc32.c"
  #undef N
  #include "zlib/deflate.c"
  #include "zlib/inffast.c"
  #undef PULLBYTE
  #undef LOAD
  #undef RESTORE
  #undef INITBITS
  #undef NEEDBITS
  #undef DROPBITS
  #undef BYTEBITS
  #undef GZIP
  #include "zlib/inflate.c"
  #include "zlib/inftrees.c"
  #include "zlib/trees.c"
  #include "zlib/zutil.c"
  #undef Byte
  #undef fdopen
  #undef local
  #undef Freq
  #undef Code
  #undef Dad
  #undef Len

  #pragma pop_macro ("close")
  #pragma pop_macro ("open")
  #pragma pop_macro ("write")
  #pragma pop_macro ("read")
  #pragma pop_macro ("MIN")
  #pragma pop_macro ("register")

  JUCE_END_IGNORE_WARNINGS_GCC_LIKE
 #else
  #include JUCE_ZLIB_INCLUDE_PATH
 #endif

#ifndef z_uInt
 #ifdef uInt
  #define z_uInt uInt
 #else
  #define z_uInt unsigned int
 #endif
#endif

}

JUCE_END_IGNORE_WARNINGS_MSVC

//==============================================================================
// internal helper object that holds the zlib structures so they don't have to be
// included publicly.
class GZIPDecompressorInputStream::GZIPDecompressHelper
{
public:
    GZIPDecompressHelper (Format f)
    {
        using namespace zlibNamespace;
        zerostruct (stream);
        streamIsValid = (inflateInit2 (&stream, getBitsForFormat (f)) == Z_OK);
        finished = error = ! streamIsValid;
    }

    ~GZIPDecompressHelper()
    {
        if (streamIsValid)
            zlibNamespace::inflateEnd (&stream);
    }

    bool needsInput() const noexcept        { return dataSize <= 0; }

    void setInput (uint8* const data_, const size_t size) noexcept
    {
        data = data_;
        dataSize = size;
    }

    int doNextBlock (uint8* const dest, const unsigned int destSize)
    {
        using namespace zlibNamespace;

        if (streamIsValid && data != nullptr && ! finished)
        {
            stream.next_in  = data;
            stream.next_out = dest;
            stream.avail_in  = (z_uInt) dataSize;
            stream.avail_out = (z_uInt) destSize;

            switch (inflate (&stream, Z_PARTIAL_FLUSH))
            {
            case Z_STREAM_END:
                finished = true;
                JUCE_FALLTHROUGH
            case Z_OK:
                data += dataSize - stream.avail_in;
                dataSize = (z_uInt) stream.avail_in;
                return (int) (destSize - stream.avail_out);

            case Z_NEED_DICT:
                needsDictionary = true;
                data += dataSize - stream.avail_in;
                dataSize = (size_t) stream.avail_in;
                break;

            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                error = true;
                JUCE_FALLTHROUGH
            default:
                break;
            }
        }

        return 0;
    }

    static int getBitsForFormat (Format f) noexcept
    {
        switch (f)
        {
            case zlibFormat:     return  MAX_WBITS;
            case deflateFormat:  return -MAX_WBITS;
            case gzipFormat:     return  MAX_WBITS | 16;
            default:             jassertfalse; break;
        }

        return MAX_WBITS;
    }

    bool finished = true, needsDictionary = false, error = true, streamIsValid = false;

    enum { gzipDecompBufferSize = 32768 };

private:
    zlibNamespace::z_stream stream;
    uint8* data = nullptr;
    size_t dataSize = 0;

    JUCE_DECLARE_NON_COPYABLE (GZIPDecompressHelper)
};

//==============================================================================
GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream* source, bool deleteSourceWhenDestroyed,
                                                          Format f, int64 uncompressedLength)
  : sourceStream (source, deleteSourceWhenDestroyed),
    uncompressedStreamLength (uncompressedLength),
    format (f),
    originalSourcePos (source->getPosition()),
    buffer ((size_t) GZIPDecompressHelper::gzipDecompBufferSize),
    helper (new GZIPDecompressHelper (f))
{
}

GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream& source)
  : sourceStream (&source, false),
    uncompressedStreamLength (-1),
    format (zlibFormat),
    originalSourcePos (source.getPosition()),
    buffer ((size_t) GZIPDecompressHelper::gzipDecompBufferSize),
    helper (new GZIPDecompressHelper (zlibFormat))
{
}

GZIPDecompressorInputStream::~GZIPDecompressorInputStream()
{
}

int64 GZIPDecompressorInputStream::getTotalLength()
{
    return uncompressedStreamLength;
}

int GZIPDecompressorInputStream::read (void* destBuffer, int howMany)
{
    jassert (destBuffer != nullptr && howMany >= 0);

    if (howMany > 0 && ! isEof)
    {
        int numRead = 0;
        auto d = static_cast<uint8*> (destBuffer);

        while (! helper->error)
        {
            auto n = helper->doNextBlock (d, (unsigned int) howMany);
            currentPos += n;

            if (n == 0)
            {
                if (helper->finished || helper->needsDictionary)
                {
                    isEof = true;
                    return numRead;
                }

                if (helper->needsInput())
                {
                    activeBufferSize = sourceStream->read (buffer, (int) GZIPDecompressHelper::gzipDecompBufferSize);

                    if (activeBufferSize > 0)
                    {
                        helper->setInput (buffer, (size_t) activeBufferSize);
                    }
                    else
                    {
                        isEof = true;
                        return numRead;
                    }
                }
            }
            else
            {
                numRead += n;
                howMany -= n;
                d += n;

                if (howMany <= 0)
                    return numRead;
            }
        }
    }

    return 0;
}

bool GZIPDecompressorInputStream::isExhausted()
{
    return helper->error || helper->finished || isEof;
}

int64 GZIPDecompressorInputStream::getPosition()
{
    return currentPos;
}

bool GZIPDecompressorInputStream::setPosition (int64 newPos)
{
    if (newPos < currentPos)
    {
        // to go backwards, reset the stream and start again..
        isEof = false;
        activeBufferSize = 0;
        currentPos = 0;
        helper.reset (new GZIPDecompressHelper (format));

        sourceStream->setPosition (originalSourcePos);
    }

    skipNextBytes (newPos - currentPos);
    return true;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct GZIPDecompressorInputStreamTests final : public UnitTest
{
    GZIPDecompressorInputStreamTests()
        : UnitTest ("GZIPDecompressorInputStreamTests", UnitTestCategories::streams)
    {}

    void runTest() override
    {
        const MemoryBlock data ("abcdefghijklmnopqrstuvwxyz", 26);

        MemoryOutputStream mo;
        GZIPCompressorOutputStream gzipOutputStream (mo);
        gzipOutputStream.write (data.getData(), data.getSize());
        gzipOutputStream.flush();

        MemoryInputStream mi (mo.getData(), mo.getDataSize(), false);
        GZIPDecompressorInputStream stream (&mi, false, GZIPDecompressorInputStream::zlibFormat, (int64) data.getSize());

        beginTest ("Read");

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
    }
};

static GZIPDecompressorInputStreamTests gzipDecompressorInputStreamTests;

#endif

} // namespace juce
