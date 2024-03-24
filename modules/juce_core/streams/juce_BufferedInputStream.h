/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/** Wraps another input stream, and reads from it using an intermediate buffer

    If you're using an input stream such as a file input stream, and making lots of
    small read accesses to it, it's probably sensible to wrap it in one of these,
    so that the source stream gets accessed in larger chunk sizes, meaning less
    work for the underlying stream.

    @tags{Core}
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
    BufferedInputStream (InputStream* sourceStream,
                         int bufferSize,
                         bool deleteSourceWhenDestroyed);

    /** Creates a BufferedInputStream from an input source.

        @param sourceStream     the source stream to read from - the source stream  must not
                                be deleted until this object has been destroyed.
        @param bufferSize       the size of reservoir to use to buffer the source
    */
    BufferedInputStream (InputStream& sourceStream, int bufferSize);

    /** Destructor.

        This may also delete the source stream, if that option was chosen when the
        buffered stream was created.
    */
    ~BufferedInputStream() override;


    //==============================================================================
    /** Returns the next byte that would be read by a call to readByte() */
    char peekByte();

    int64 getTotalLength() override;
    int64 getPosition() override;
    bool setPosition (int64 newPosition) override;
    int read (void* destBuffer, int maxBytesToRead) override;
    String readString() override;
    bool isExhausted() override;


private:
    //==============================================================================
    OptionalScopedPointer<InputStream> source;
    Range<int64> bufferedRange;
    int64 position, bufferLength, lastReadPos = 0, bufferOverlap = 128;
    HeapBlock<char> buffer;
    bool ensureBuffered();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferedInputStream)
};

} // namespace juce
