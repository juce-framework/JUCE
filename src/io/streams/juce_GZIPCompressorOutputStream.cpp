/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

//==============================================================================
class GZIPCompressorOutputStream::GZIPCompressorHelper
{
public:
    GZIPCompressorHelper (const int compressionLevel, const int windowBits)
        : data (nullptr),
          dataSize (0),
          compLevel (compressionLevel),
          strategy (0),
          setParams (true),
          streamIsValid (false),
          finished (false),
          shouldFinish (false)
    {
        using namespace zlibNamespace;
        zerostruct (stream);

        streamIsValid = (deflateInit2 (&stream, compLevel, Z_DEFLATED,
                                       windowBits != 0 ? windowBits : MAX_WBITS,
                                       8, strategy) == Z_OK);
    }

    ~GZIPCompressorHelper()
    {
        using namespace zlibNamespace;
        if (streamIsValid)
            deflateEnd (&stream);
    }

    bool needsInput() const noexcept
    {
        return dataSize <= 0;
    }

    void setInput (const uint8* const newData, const int size) noexcept
    {
        data = newData;
        dataSize = size;
    }

    int doNextBlock (uint8* const dest, const int destSize) noexcept
    {
        using namespace zlibNamespace;
        if (streamIsValid)
        {
            stream.next_in = const_cast <uint8*> (data);
            stream.next_out = dest;
            stream.avail_in = dataSize;
            stream.avail_out = destSize;

            const int result = setParams ? deflateParams (&stream, compLevel, strategy)
                                         : deflate (&stream, shouldFinish ? Z_FINISH : Z_NO_FLUSH);

            setParams = false;

            switch (result)
            {
            case Z_STREAM_END:
                finished = true;
                // Deliberate fall-through..
            case Z_OK:
                data += dataSize - stream.avail_in;
                dataSize = stream.avail_in;

                return destSize - stream.avail_out;

            default:
                break;
            }
        }

        return 0;
    }

    enum { gzipCompBufferSize = 32768 };

private:
    zlibNamespace::z_stream stream;
    const uint8* data;
    int dataSize, compLevel, strategy;
    bool setParams, streamIsValid;

public:
    bool finished, shouldFinish;
};

//==============================================================================
GZIPCompressorOutputStream::GZIPCompressorOutputStream (OutputStream* const destStream_,
                                                        int compressionLevel,
                                                        const bool deleteDestStream,
                                                        const int windowBits)
  : destStream (destStream_, deleteDestStream),
    buffer ((size_t) GZIPCompressorHelper::gzipCompBufferSize)
{
    if (compressionLevel < 1 || compressionLevel > 9)
        compressionLevel = -1;

    helper = new GZIPCompressorHelper (compressionLevel, windowBits);
}

GZIPCompressorOutputStream::~GZIPCompressorOutputStream()
{
    flushInternal();
}

//==============================================================================
void GZIPCompressorOutputStream::flushInternal()
{
    if (! helper->finished)
    {
        helper->shouldFinish = true;

        while (! helper->finished)
            doNextBlock();
    }

    destStream->flush();
}

void GZIPCompressorOutputStream::flush()
{
    flushInternal();
}

bool GZIPCompressorOutputStream::write (const void* destBuffer, int howMany)
{
    if (! helper->finished)
    {
        helper->setInput (static_cast <const uint8*> (destBuffer), howMany);

        while (! helper->needsInput())
        {
            if (! doNextBlock())
                return false;
        }
    }

    return true;
}

bool GZIPCompressorOutputStream::doNextBlock()
{
    const int len = helper->doNextBlock (buffer, (int) GZIPCompressorHelper::gzipCompBufferSize);
    return len <= 0 || destStream->write (buffer, len);
}

int64 GZIPCompressorOutputStream::getPosition()
{
    return destStream->getPosition();
}

bool GZIPCompressorOutputStream::setPosition (int64 /*newPosition*/)
{
    jassertfalse; // can't do it!
    return false;
}

END_JUCE_NAMESPACE
