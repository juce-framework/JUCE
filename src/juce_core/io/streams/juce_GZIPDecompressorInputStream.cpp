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

#include "../../basics/juce_StandardHeader.h"

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
private:
    z_stream* stream;
    uint8* data;
    int dataSize;

public:
    bool finished, needsDictionary, error;

    GZIPDecompressHelper (const bool noWrap) throw()
        : data (0),
          dataSize (0),
          finished (false),
          needsDictionary (false),
          error (false)
    {
        stream = (z_stream*) juce_calloc (sizeof (z_stream));

        if (inflateInit2 (stream, (noWrap) ? -MAX_WBITS
                                           : MAX_WBITS) != Z_OK)
        {
            juce_free (stream);
            stream = 0;
            error = true;
            finished = true;
        }
    }

    ~GZIPDecompressHelper() throw()
    {
        if (stream != 0)
        {
            inflateEnd (stream);
            juce_free (stream);
        }
    }

    bool needsInput() const throw()         { return dataSize <= 0; }

    void setInput (uint8* const data_, const int size) throw()
    {
        data = data_;
        dataSize = size;
    }

    int doNextBlock (uint8* const dest, const int destSize) throw()
    {
        if (stream != 0 && data != 0 && ! finished)
        {
            stream->next_in  = data;
            stream->next_out = dest;
            stream->avail_in  = dataSize;
            stream->avail_out = destSize;

            switch (inflate (stream, Z_PARTIAL_FLUSH))
            {
            case Z_STREAM_END:
                finished = true;
                // deliberate fall-through

            case Z_OK:
                data += dataSize - stream->avail_in;
                dataSize = stream->avail_in;
                return destSize - stream->avail_out;

            case Z_NEED_DICT:
                needsDictionary = true;
                data += dataSize - stream->avail_in;
                dataSize = stream->avail_in;
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
};

//==============================================================================
const int gzipDecompBufferSize = 32768;

GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream* const sourceStream_,
                                                          const bool deleteSourceWhenDestroyed_,
                                                          const bool noWrap_,
                                                          const int64 uncompressedStreamLength_)
  : sourceStream (sourceStream_),
    uncompressedStreamLength (uncompressedStreamLength_),
    deleteSourceWhenDestroyed (deleteSourceWhenDestroyed_),
    noWrap (noWrap_),
    isEof (false),
    activeBufferSize (0),
    originalSourcePos (sourceStream_->getPosition()),
    currentPos (0)
{
    buffer = (uint8*) juce_malloc (gzipDecompBufferSize);
    helper = new GZIPDecompressHelper (noWrap_);
}

GZIPDecompressorInputStream::~GZIPDecompressorInputStream()
{
    juce_free (buffer);

    if (deleteSourceWhenDestroyed)
        delete sourceStream;

    GZIPDecompressHelper* const h = (GZIPDecompressHelper*) helper;
    delete h;
}

int64 GZIPDecompressorInputStream::getTotalLength()
{
    return uncompressedStreamLength;
}

int GZIPDecompressorInputStream::read (void* destBuffer, int howMany)
{
    GZIPDecompressHelper* const h = (GZIPDecompressHelper*) helper;

    if ((howMany > 0) && ! isEof)
    {
        jassert (destBuffer != 0);

        if (destBuffer != 0)
        {
            int numRead = 0;
            uint8* d = (uint8*) destBuffer;

            while (! h->error)
            {
                const int n = h->doNextBlock (d, howMany);
                currentPos += n;

                if (n == 0)
                {
                    if (h->finished || h->needsDictionary)
                    {
                        isEof = true;
                        return numRead;
                    }

                    if (h->needsInput())
                    {
                        activeBufferSize = sourceStream->read (buffer, gzipDecompBufferSize);

                        if (activeBufferSize > 0)
                        {
                            h->setInput ((uint8*) buffer, activeBufferSize);
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
    const GZIPDecompressHelper* const h = (GZIPDecompressHelper*) helper;

    return h->error || isEof;
}

int64 GZIPDecompressorInputStream::getPosition()
{
    return currentPos;
}

bool GZIPDecompressorInputStream::setPosition (int64 newPos)
{
    if (newPos != currentPos)
    {
        if (newPos > currentPos)
        {
            skipNextBytes (newPos - currentPos);
        }
        else
        {
            // reset the stream and start again..
            GZIPDecompressHelper* const h = (GZIPDecompressHelper*) helper;
            delete h;

            isEof = false;
            activeBufferSize = 0;
            currentPos = 0;
            helper = new GZIPDecompressHelper (noWrap);

            sourceStream->setPosition (originalSourcePos);
            skipNextBytes (newPos);
        }
    }

    return true;
}

END_JUCE_NAMESPACE
