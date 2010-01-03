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

#ifndef __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
#define __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__

#include "juce_InputStream.h"
#include "../../containers/juce_ScopedPointer.h"
class GZIPDecompressHelper;


//==============================================================================
/**
    This stream will decompress a source-stream using zlib.

    Tip: if you're reading lots of small items from one of these streams, you
         can increase the performance enormously by passing it through a
         BufferedInputStream, so that it has to read larger blocks less often.

    @see GZIPCompressorOutputStream
*/
class JUCE_API  GZIPDecompressorInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a decompressor stream.

        @param sourceStream                 the stream to read from
        @param deleteSourceWhenDestroyed    whether or not to delete the source stream
                                            when this object is destroyed
        @param noWrap                       this is used internally by the ZipFile class
                                            and should be ignored by user applications
        @param uncompressedStreamLength     if the creator knows the length that the
                                            uncompressed stream will be, then it can supply this
                                            value, which will be returned by getTotalLength()
    */
    GZIPDecompressorInputStream (InputStream* const sourceStream,
                                 const bool deleteSourceWhenDestroyed,
                                 const bool noWrap = false,
                                 const int64 uncompressedStreamLength = -1);

    /** Destructor. */
    ~GZIPDecompressorInputStream();

    //==============================================================================
    int64 getPosition();
    bool setPosition (int64 pos);
    int64 getTotalLength();
    bool isExhausted();
    int read (void* destBuffer, int maxBytesToRead);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    InputStream* const sourceStream;
    ScopedPointer <InputStream> streamToDelete;
    const int64 uncompressedStreamLength;
    const bool noWrap;
    bool isEof;
    int activeBufferSize;
    int64 originalSourcePos, currentPos;
    HeapBlock <uint8> buffer;
    ScopedPointer <GZIPDecompressHelper> helper;

    GZIPDecompressorInputStream (const GZIPDecompressorInputStream&);
    const GZIPDecompressorInputStream& operator= (const GZIPDecompressorInputStream&);
};

#endif   // __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
