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

#include "juce_GZIPDecompressorInputStream.h"


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
    int getTotalOut() const throw()         { return (stream != 0) ? stream->total_out : 0; }

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
const int bufferSize = 32768;

GZIPDecompressorInputStream::GZIPDecompressorInputStream (InputStream* const sourceStream_,
                                                          const bool deleteSourceWhenDestroyed_,
                                                          const bool noWrap_)
  : sourceStream (sourceStream_),
    deleteSourceWhenDestroyed (deleteSourceWhenDestroyed_),
    noWrap (noWrap_),
    isEof (false),
    activeBufferSize (0),
    originalSourcePos (sourceStream_->getPosition())
{
    buffer = (uint8*) juce_malloc (bufferSize);
    helper = new GZIPDecompressHelper (noWrap_);
}

GZIPDecompressorInputStream::~GZIPDecompressorInputStream()
{
    juce_free (buffer);

    if (deleteSourceWhenDestroyed)
        delete sourceStream;

    GZIPDecompressHelper* const h = (GZIPDecompressHelper*)helper;
    delete h;
}

int64 GZIPDecompressorInputStream::getTotalLength()
{
    return -1; // unknown..
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

                if (n == 0)
                {
                    if (h->finished || h->needsDictionary)
                    {
                        isEof = true;
                        return numRead;
                    }

                    if (h->needsInput())
                    {
                        activeBufferSize = sourceStream->read (buffer, bufferSize);

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
    const GZIPDecompressHelper* const h = (GZIPDecompressHelper*) helper;

    return h->getTotalOut() + activeBufferSize;
}

bool GZIPDecompressorInputStream::setPosition (int64 newPos)
{
    const int64 currentPos = getPosition();

    if (newPos != currentPos)
    {
        // reset the stream and start again..
        GZIPDecompressHelper* const h = (GZIPDecompressHelper*) helper;
        delete h;

        isEof = false;
        activeBufferSize = 0;
        helper = new GZIPDecompressHelper (noWrap);

        sourceStream->setPosition (originalSourcePos);
        skipNextBytes (newPos);
    }

    return true;
}

END_JUCE_NAMESPACE
