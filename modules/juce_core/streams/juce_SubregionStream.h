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
/** Wraps another input stream, and reads from a specific part of it.

    This lets you take a subsection of a stream and present it as an entire
    stream in its own right.

    @tags{Core}
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
    SubregionStream (InputStream* sourceStream,
                     int64 startPositionInSourceStream,
                     int64 lengthOfSourceStream,
                     bool deleteSourceWhenDestroyed);

    /** Destructor.

        This may also delete the source stream, if that option was chosen when the
        buffered stream was created.
    */
    ~SubregionStream() override;


    //==============================================================================
    int64 getTotalLength() override;
    int64 getPosition() override;
    bool setPosition (int64 newPosition) override;
    int read (void* destBuffer, int maxBytesToRead) override;
    bool isExhausted() override;

private:
    //==============================================================================
    OptionalScopedPointer<InputStream> source;
    const int64 startPositionInSourceStream, lengthOfSourceStream;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubregionStream)
};

} // namespace juce
