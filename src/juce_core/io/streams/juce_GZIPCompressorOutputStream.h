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

#ifndef __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
#define __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__

#include "../juce_OutputStream.h"


//==============================================================================
/**
    A stream which uses zlib to compress the data written into it.

    @see GZIPDecompressorInputStream
*/
class JUCE_API  GZIPCompressorOutputStream  : public OutputStream
{
public:
    //==============================================================================
    /** Creates a compression stream.

        @param destStream                       the stream into which the compressed data should
                                                be written
        @param compressionLevel                 how much to compress the data, between 1 and 9, where
                                                1 is the fastest/lowest compression, and 9 is the
                                                slowest/highest compression. Any value outside this range
                                                indicates that a default compression level should be used.
        @param deleteDestStreamWhenDestroyed    whether or not to delete the destStream object when
                                                this stream is destroyed
    */
    GZIPCompressorOutputStream (OutputStream* const destStream,
                                int compressionLevel = 0,
                                const bool deleteDestStreamWhenDestroyed = false);

    /** Destructor. */
    ~GZIPCompressorOutputStream();

    //==============================================================================
    void flush();
    int64 getPosition();
    bool setPosition (int64 newPosition);
    bool write (const void* destBuffer, int howMany);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OutputStream* const destStream;
    const bool deleteDestStream;
    uint8* buffer;
    void* helper;
    bool doNextBlock();

    GZIPCompressorOutputStream (const GZIPCompressorOutputStream&);
    const GZIPCompressorOutputStream& operator= (const GZIPCompressorOutputStream&);
};

#endif   // __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
