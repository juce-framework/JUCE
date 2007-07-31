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

#ifndef __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__
#define __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__

#include "../juce_InputStream.h"


//==============================================================================
/** Wraps another input stream, and reads from it using an intermediate buffer

    If you're using an input stream such as a file input stream, and making lots of
    small read accesses to it, it's probably sensible to wrap it in one of these,
    so that the source stream gets accessed in larger chunk sizes, meaning less
    work for the underlying stream.
*/
class JUCE_API  BufferedInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a BufferedInputStream from an input source.

        @param sourceStream                 the source stream to read from
        @param bufferSize                   the size of reservoir to use to buffer the source
        @param deleteSourceWhenDestroyed    whether the sourceStream that is passed in should be
                                            deleted by this object when it is itself deleted.
    */
    BufferedInputStream (InputStream* const sourceStream,
                         const int bufferSize,
                         const bool deleteSourceWhenDestroyed) throw();

    /** Destructor.

        This may also delete the source stream, if that option was chosen when the
        buffered stream was created.
    */
    ~BufferedInputStream() throw();


    //==============================================================================
    int64 getTotalLength();
    int64 getPosition();
    bool setPosition (int64 newPosition);
    int read (void* destBuffer, int maxBytesToRead);
    const String readString();
    bool isExhausted();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    InputStream* const source;
    const bool deleteSourceWhenDestroyed;
    int bufferSize;
    int64 position, lastReadPos, bufferStart, bufferOverlap;
    char* buffer;
    void ensureBuffered();

    BufferedInputStream (const BufferedInputStream&);
    const BufferedInputStream& operator= (const BufferedInputStream&);
};

#endif   // __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__
