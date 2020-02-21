/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce{

//==============================================================================
/** Allows a block of data to be accessed as a stream of OSC data.

    The memory is shared and will be neither copied nor owned by the OSCInputStream.

    This class is implementing the Open Sound Control 1.0 Specification for
    interpreting the data.

    Note: Some older implementations of OSC may omit the OSC Type Tag string
    in OSC messages. This class will treat such OSC messages as format errors.
*/
class JUCE_API OSCInputStream
{
public:
    /** Creates an OSCInputStream.

        @param sourceData               the block of data to use as the stream's source
        @param sourceDataSize           the number of bytes in the source data block
    */
    OSCInputStream (const void* sourceData, size_t sourceDataSize);

    //==============================================================================
    /** Returns a pointer to the source data block from which this stream is reading. */
    const void* getData() const noexcept        { return input.getData(); }

    /** Returns the number of bytes of source data in the block from which this stream is reading. */
    size_t getDataSize() const noexcept         { return input.getDataSize(); }

    /** Returns the current position of the stream. */
    uint64 getPosition()                        { return (uint64) input.getPosition(); }

    /** Attempts to set the current position of the stream. Returns true if this was successful. */
    bool setPosition (int64 pos)                { return input.setPosition (pos); }

    /** Returns the total amount of data in bytes accessible by this stream. */
    int64 getTotalLength()                      { return input.getTotalLength(); }

    /** Returns true if the stream has no more data to read. */
    bool isExhausted()                          { return input.isExhausted(); }

    //==============================================================================
    int32 readInt32();

    uint64 readUint64();

    float readFloat32();

    String readString();

    MemoryBlock readBlob();

    OSCColour readColour();

    OSCTimeTag readTimeTag();

    OSCAddress readAddress();
    
    OSCAddressPattern readAddressPattern();

    OSCTypeList readTypeTagString();

    OSCArgument readArgument (OSCType type);

    //==============================================================================
    OSCMessage readMessage();

    OSCBundle readBundle (size_t maxBytesToRead = std::numeric_limits<size_t>::max());

    //==============================================================================
    OSCBundle::Element readElement();

    OSCBundle::Element readElementWithKnownSize (size_t elementSize);

private:
    MemoryInputStream input;

    void readPaddingZeros (size_t bytesRead);
    
    OSCBundle readBundleWithCheckedSize (size_t size);
    
    OSCMessage readMessageWithCheckedSize (size_t size);
    
    void checkBytesAvailable (int64 requiredBytes, const char* message);
};

} // namespace

