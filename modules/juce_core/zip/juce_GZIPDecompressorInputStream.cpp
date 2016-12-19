/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4309 4305 4365)
#endif

namespace zlibNamespace
{
 #if JUCE_INCLUDE_ZLIB_CODE
  #if JUCE_CLANG
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wconversion"
   #pragma clang diagnostic ignored "-Wshadow"
   #pragma clang diagnostic ignored "-Wdeprecated-register"
  #endif

  #undef OS_CODE
  #undef fdopen
  #define ZLIB_INTERNAL
  #define NO_DUMMY_DECL
  #include "zlib/zlib.h"
  #include "zlib/adler32.c"
  #include "zlib/compress.c"
  #undef DO1
  #undef DO8
  #include "zlib/crc32.c"
  #include "zlib/deflate.c"
  #include "zlib/inffast.c"
  #undef PULLBYTE
  #undef LOAD
  #undef RESTORE
  #undef INITBITS
  #undef NEEDBITS
  #undef DROPBITS
  #undef BYTEBITS
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

  #if JUCE_CLANG
   #pragma clang diagnostic pop
  #endif
 #else
  #include JUCE_ZLIB_INCLUDE_PATH

  #ifndef z_uInt
   #ifdef uInt
    #define z_uInt uInt
   #else
    #define z_uInt unsigned int
   #endif
  #endif

 #endif
}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
// internal helper object that holds the zlib structures so they don't have to be
// included publicly.
class GZIPDecompressorInputStream::GZIPDecompressHelper
{
public:
    GZIPDecompressHelper (Format f)
        : finished (true),
          needsDictionary (false),
          error (true),
          streamIsValid (false),
          data (nullptr),
          dataSize (0)
    {
        using namespace zlibNamespace;
        zerostruct (stream);
        streamIsValid = (inflateInit2 (&stream, getBitsForFormat (f)) == Z_OK);
        finished = error = ! streamIsValid;
    }

    ~GZIPDecompressHelper()
    {
        using namespace zlibNamespace;
        if (streamIsValid)
            inflateEnd (&stream);
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
                // deliberate fall-through
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

    bool finished, needsDictionary, error, streamIsValid;

    enum { gzipDecompBufferSize = 32768 };

private:
    zlibNamespace::z_stream stream;
    uint8* data;
    size_t dataSize;

    JUCE_DECLARE_NON_COPYABLE (GZIPDecompressHelper)
};

//==============================================================================
GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream* source, bool deleteSourceWhenDestroyed,
                                                          Format f, int64 uncompressedLength)
  : sourceStream (source, deleteSourceWhenDestroyed),
    uncompressedStreamLength (uncompressedLength),
    format (f),
    isEof (false),
    activeBufferSize (0),
    originalSourcePos (source->getPosition()),
    currentPos (0),
    buffer ((size_t) GZIPDecompressHelper::gzipDecompBufferSize),
    helper (new GZIPDecompressHelper (f))
{
}

GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream& source)
  : sourceStream (&source, false),
    uncompressedStreamLength (-1),
    format (zlibFormat),
    isEof (false),
    activeBufferSize (0),
    originalSourcePos (source.getPosition()),
    currentPos (0),
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
        uint8* d = static_cast<uint8*> (destBuffer);

        while (! helper->error)
        {
            const int n = helper->doNextBlock (d, (unsigned int) howMany);
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
    return helper->error || isEof;
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
        helper = new GZIPDecompressHelper (format);

        sourceStream->setPosition (originalSourcePos);
    }

    skipNextBytes (newPos - currentPos);
    return true;
}
