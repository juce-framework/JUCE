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

#ifndef __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
#define __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__

#include "../streams/juce_InputStream.h"
#include "../memory/juce_OptionalScopedPointer.h"
#include "../memory/juce_HeapBlock.h"


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
    GZIPDecompressorInputStream (InputStream* sourceStream,
                                 bool deleteSourceWhenDestroyed,
                                 bool noWrap = false,
                                 int64 uncompressedStreamLength = -1);

    /** Creates a decompressor stream.

        @param sourceStream     the stream to read from - the source stream must not be
                                deleted until this object has been destroyed
    */
    GZIPDecompressorInputStream (InputStream& sourceStream);

    /** Destructor. */
    ~GZIPDecompressorInputStream();

    //==============================================================================
    int64 getPosition();
    bool setPosition (int64 pos);
    int64 getTotalLength();
    bool isExhausted();
    int read (void* destBuffer, int maxBytesToRead);


    //==============================================================================
private:
    OptionalScopedPointer<InputStream> sourceStream;
    const int64 uncompressedStreamLength;
    const bool noWrap;
    bool isEof;
    int activeBufferSize;
    int64 originalSourcePos, currentPos;
    HeapBlock <uint8> buffer;

    class GZIPDecompressHelper;
    friend class ScopedPointer <GZIPDecompressHelper>;
    ScopedPointer <GZIPDecompressHelper> helper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GZIPDecompressorInputStream)
};

#endif   // __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
