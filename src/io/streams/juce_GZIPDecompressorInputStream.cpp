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

#include "../../core/juce_StandardHeader.h"

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4309 4305)
#endif

namespace zlibNamespace
{
#if JUCE_INCLUDE_ZLIB_CODE
  extern "C"
  {
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
  }
#else
  #include <zlib.h>
#endif
}

#if JUCE_MSVC
  #pragma warning (pop)
#endif

BEGIN_JUCE_NAMESPACE

#include "juce_GZIPDecompressorInputStream.h"

using namespace zlibNamespace;

//==============================================================================
// internal helper object that holds the zlib structures so they don't have to be
// included publicly.
class GZIPDecompressHelper
{
public:
    GZIPDecompressHelper (const bool noWrap) throw()
        : data (0),
          dataSize (0),
          finished (true),
          needsDictionary (false),
          error (true),
          streamIsValid (false)
    {
        zerostruct (stream);
        streamIsValid = (inflateInit2 (&stream, noWrap ? -MAX_WBITS : MAX_WBITS) == Z_OK);
        finished = error = ! streamIsValid;
    }

    ~GZIPDecompressHelper() throw()
    {
        if (streamIsValid)
            inflateEnd (&stream);
    }

    bool needsInput() const throw()         { return dataSize <= 0; }

    void setInput (uint8* const data_, const int size) throw()
    {
        data = data_;
        dataSize = size;
    }

    int doNextBlock (uint8* const dest, const int destSize) throw()
    {
        if (streamIsValid && data != 0 && ! finished)
        {
            stream.next_in  = data;
            stream.next_out = dest;
            stream.avail_in  = dataSize;
            stream.avail_out = destSize;

            switch (inflate (&stream, Z_PARTIAL_FLUSH))
            {
            case Z_STREAM_END:
                finished = true;
                // deliberate fall-through
            case Z_OK:
                data += dataSize - stream.avail_in;
                dataSize = stream.avail_in;
                return destSize - stream.avail_out;

            case Z_NEED_DICT:
                needsDictionary = true;
                data += dataSize - stream.avail_in;
                dataSize = stream.avail_in;
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

private:
    z_stream stream;
    uint8* data;
    int dataSize;

public:
    bool finished, needsDictionary, error, streamIsValid;
};

//==============================================================================
const int gzipDecompBufferSize = 32768;

GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream* const sourceStream_,
                                                          const bool deleteSourceWhenDestroyed,
                                                          const bool noWrap_,
                                                          const int64 uncompressedStreamLength_)
  : sourceStream (sourceStream_),
    streamToDelete (deleteSourceWhenDestroyed ? sourceStream_ : 0),
    uncompressedStreamLength (uncompressedStreamLength_),
    noWrap (noWrap_),
    isEof (false),
    activeBufferSize (0),
    originalSourcePos (sourceStream_->getPosition()),
    currentPos (0),
    buffer (gzipDecompBufferSize),
    helper (new GZIPDecompressHelper (noWrap_))
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
    if ((howMany > 0) && ! isEof)
    {
        jassert (destBuffer != 0);

        if (destBuffer != 0)
        {
            int numRead = 0;
            uint8* d = (uint8*) destBuffer;

            while (! helper->error)
            {
                const int n = helper->doNextBlock (d, howMany);
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
                        activeBufferSize = sourceStream->read (buffer, gzipDecompBufferSize);

                        if (activeBufferSize > 0)
                        {
                            helper->setInput ((uint8*) buffer, activeBufferSize);
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
        helper = new GZIPDecompressHelper (noWrap);

        sourceStream->setPosition (originalSourcePos);
    }

    skipNextBytes (newPos - currentPos);
    return true;
}

END_JUCE_NAMESPACE
