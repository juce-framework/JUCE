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

namespace juce
{

//==============================================================================
/** Writes OSC data to an internal memory buffer, which grows as required.

    The data that was written into the stream can then be accessed later as
    a contiguous block of memory.

    This class implements the Open Sound Control 1.0 Specification for
    the format in which the OSC data will be written into the buffer.
*/
struct JUCE_API OSCOutputStream
{
    OSCOutputStream() noexcept {}

    /** Returns a pointer to the data that has been written to the stream. */
    const void* getData() const noexcept    { return output.getData(); }

    /** Returns the number of bytes of data that have been written to the stream. */
    size_t getDataSize() const noexcept     { return output.getDataSize(); }

    //==============================================================================
    bool writeInt32 (int32 value);

    bool writeUint64 (uint64 value);

    bool writeFloat32 (float value);

    bool writeString (const String& value);

    bool writeBlob (const MemoryBlock& blob);

    bool writeColour (OSCColour colour);

    bool writeTimeTag (OSCTimeTag timeTag);

    bool writeAddress (const OSCAddress& address);

    bool writeAddressPattern (const OSCAddressPattern& ap);

    bool writeTypeTagString (const OSCTypeList& typeList);

    bool writeArgument (const OSCArgument& arg);

    //==============================================================================
    bool writeMessage (const OSCMessage& msg);

    bool writeBundle (const OSCBundle& bundle);

    //==============================================================================
    bool writeBundleElement (const OSCBundle::Element& element);

private:
    MemoryOutputStream output;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCOutputStream)
};

} // namespace
