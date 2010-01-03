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

#ifndef __JUCE_SUBREGIONSTREAM_JUCEHEADER__
#define __JUCE_SUBREGIONSTREAM_JUCEHEADER__

#include "juce_InputStream.h"
#include "../../containers/juce_ScopedPointer.h"


//==============================================================================
/** Wraps another input stream, and reads from a specific part of it.

    This lets you take a subsection of a stream and present it as an entire
    stream in its own right.
*/
class JUCE_API  SubregionStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a SubregionStream from an input source.

        @param sourceStream                 the source stream to read from
        @param startPositionInSourceStream  this is the position in the source stream that
                                            corresponds to position 0 in this stream
        @param lengthOfSourceStream         this specifies the maximum number of bytes
                                            from the source stream that will be passed through
                                            by this stream. When the position of this stream
                                            exceeds lengthOfSourceStream, it will cause an end-of-stream.
                                            If the length passed in here is greater than the length
                                            of the source stream (as returned by getTotalLength()),
                                            then the smaller value will be used.
                                            Passing a negative value for this parameter means it
                                            will keep reading until the source's end-of-stream.
        @param deleteSourceWhenDestroyed    whether the sourceStream that is passed in should be
                                            deleted by this object when it is itself deleted.
    */
    SubregionStream (InputStream* const sourceStream,
                     const int64 startPositionInSourceStream,
                     const int64 lengthOfSourceStream,
                     const bool deleteSourceWhenDestroyed) throw();

    /** Destructor.

        This may also delete the source stream, if that option was chosen when the
        buffered stream was created.
    */
    ~SubregionStream() throw();


    //==============================================================================
    int64 getTotalLength();
    int64 getPosition();
    bool setPosition (int64 newPosition);
    int read (void* destBuffer, int maxBytesToRead);
    bool isExhausted();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    InputStream* const source;
    ScopedPointer <InputStream> sourceToDelete;
    const int64 startPositionInSourceStream, lengthOfSourceStream;

    SubregionStream (const SubregionStream&);
    const SubregionStream& operator= (const SubregionStream&);
};

#endif   // __JUCE_SUBREGIONSTREAM_JUCEHEADER__
