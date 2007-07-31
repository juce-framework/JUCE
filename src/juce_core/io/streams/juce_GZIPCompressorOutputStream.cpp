/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../basics/juce_StandardHeader.h"
#include "zlib/zlib.h"

BEGIN_JUCE_NAMESPACE

#include "juce_GZIPCompressorOutputStream.h"


//==============================================================================
// internal helper object that holds the zlib structures so they don't have to be
// included publicly.
class GZIPCompressorHelper
{
private:
    z_stream* stream;
    uint8* data;
    int dataSize, compLevel, strategy;
    bool setParams;

public:
    bool finished, shouldFinish;

    GZIPCompressorHelper (const int compressionLevel)
        : data (0),
          dataSize (0),
          compLevel (compressionLevel),
          strategy (0),
          setParams (true),
          finished (false),
          shouldFinish (false)
    {
        stream = (z_stream*) juce_calloc (sizeof (z_stream));

        bool nowrap = false;

        if (deflateInit2 (stream,
                          compLevel,
                          Z_DEFLATED,
                          nowrap ? -MAX_WBITS : MAX_WBITS,
                          8,
                          strategy) != Z_OK)
        {
            juce_free (stream);
            stream = 0;
        }
    }

    ~GZIPCompressorHelper()
    {
        if (stream != 0)
        {
            deflateEnd (stream);
            juce_free (stream);
        }
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
const int bufferSize = 32768;

GZIPCompressorOutputStream::GZIPCompressorOutputStream (OutputStream* const destStream_,
                                                        int compressionLevel,
                                                        const bool deleteDestStream_)
  : destStream (destStream_),
    deleteDestStream (deleteDestStream_)
{
    if (compressionLevel < 1 || compressionLevel > 9)
        compressionLevel = -1;

    helper = new GZIPCompressorHelper (compressionLevel);

    buffer = (uint8*) juce_malloc (bufferSize);
}

GZIPCompressorOutputStream::~GZIPCompressorOutputStream()
{
    flush();

    GZIPCompressorHelper* const h = (GZIPCompressorHelper*) helper;
    delete h;

    juce_free (buffer);

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
    const int len = h->doNextBlock (buffer, bufferSize);

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
