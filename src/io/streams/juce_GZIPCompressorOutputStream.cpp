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

namespace zlibNamespace
{
#if JUCE_INCLUDE_ZLIB_CODE
  #undef OS_CODE
  #undef fdopen
  #include "zlib/zlib.h"
  #undef OS_CODE
#else
  #include <zlib.h>
#endif
}

BEGIN_JUCE_NAMESPACE

#include "juce_GZIPCompressorOutputStream.h"

using namespace zlibNamespace;

//==============================================================================
// internal helper object that holds the zlib structures so they don't have to be
// included publicly.
class GZIPCompressorHelper
{
private:
    HeapBlock <z_stream> stream;
    uint8* data;
    int dataSize, compLevel, strategy;
    bool setParams;

public:
    bool finished, shouldFinish;

    GZIPCompressorHelper (const int compressionLevel, const bool nowrap)
        : data (0),
          dataSize (0),
          compLevel (compressionLevel),
          strategy (0),
          setParams (true),
          finished (false),
          shouldFinish (false)
    {
        stream.calloc (1);

        if (deflateInit2 (stream,
                          compLevel,
                          Z_DEFLATED,
                          nowrap ? -MAX_WBITS : MAX_WBITS,
                          8,
                          strategy) != Z_OK)
        {
            stream.free();
        }
    }

    ~GZIPCompressorHelper()
    {
        if (stream != 0)
            deflateEnd (stream);
    }

    bool needsInput() const throw()
    {
        return dataSize <= 0;
    }

    void setInput (uint8* const newData, const int size) throw()
    {
        data = newData;
        dataSize = size;
    }

    int doNextBlock (uint8* const dest, const int destSize) throw()
    {
        if (stream != 0)
        {
            stream->next_in = data;
            stream->next_out = dest;
            stream->avail_in = dataSize;
            stream->avail_out = destSize;

            const int result = setParams ? deflateParams (stream, compLevel, strategy)
                                         : deflate (stream, shouldFinish ? Z_FINISH : Z_NO_FLUSH);

            setParams = false;

            switch (result)
            {
            case Z_STREAM_END:
                finished = true;

            case Z_OK:
                data += dataSize - stream->avail_in;
                dataSize = stream->avail_in;

                return destSize - stream->avail_out;

            default:
                break;
            }
        }

        return 0;
    }
};


//==============================================================================
const int gzipCompBufferSize = 32768;

GZIPCompressorOutputStream::GZIPCompressorOutputStream (OutputStream* const destStream_,
                                                        int compressionLevel,
                                                        const bool deleteDestStream_,
                                                        const bool noWrap)
  : destStream (destStream_),
    deleteDestStream (deleteDestStream_),
    buffer (gzipCompBufferSize)
{
    if (compressionLevel < 1 || compressionLevel > 9)
        compressionLevel = -1;

    helper = new GZIPCompressorHelper (compressionLevel, noWrap);

}

GZIPCompressorOutputStream::~GZIPCompressorOutputStream()
{
    flush();

    delete (GZIPCompressorHelper*) helper;

    if (deleteDestStream)
        delete destStream;
}

//==============================================================================
void GZIPCompressorOutputStream::flush()
{
    GZIPCompressorHelper* const h = (GZIPCompressorHelper*) helper;

    if (! h->finished)
    {
        h->shouldFinish = true;

        while (! h->finished)
            doNextBlock();
    }

    destStream->flush();
}

bool GZIPCompressorOutputStream::write (const void* destBuffer, int howMany)
{
    GZIPCompressorHelper* const h = (GZIPCompressorHelper*) helper;

    if (! h->finished)
    {
        h->setInput ((uint8*) destBuffer, howMany);

        while (! h->needsInput())
        {
            if (! doNextBlock())
                return false;
        }
    }

    return true;
}

bool GZIPCompressorOutputStream::doNextBlock()
{
    GZIPCompressorHelper* const h = (GZIPCompressorHelper*) helper;
    const int len = h->doNextBlock (buffer, gzipCompBufferSize);

    if (len > 0)
        return destStream->write (buffer, len);
    else
        return true;
}

int64 GZIPCompressorOutputStream::getPosition()
{
    return destStream->getPosition();
}

bool GZIPCompressorOutputStream::setPosition (int64 /*newPosition*/)
{
    jassertfalse // can't do it!
    return false;
}

END_JUCE_NAMESPACE
