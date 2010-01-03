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

#ifndef __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
#define __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__

#include "juce_OutputStream.h"
#include "../../containers/juce_ScopedPointer.h"
class GZIPCompressorHelper;


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
        @param noWrap                           this is used internally by the ZipFile class
                                                and should be ignored by user applications
    */
    GZIPCompressorOutputStream (OutputStream* const destStream,
                                int compressionLevel = 0,
                                const bool deleteDestStreamWhenDestroyed = false,
                                const bool noWrap = false);

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
    ScopedPointer <OutputStream> streamToDelete;
    HeapBlock <uint8> buffer;
    ScopedPointer <GZIPCompressorHelper> helper;
    bool doNextBlock();

    GZIPCompressorOutputStream (const GZIPCompressorOutputStream&);
    const GZIPCompressorOutputStream& operator= (const GZIPCompressorOutputStream&);
};

#endif   // __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
