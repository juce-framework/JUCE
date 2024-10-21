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

namespace juce::midi_ci::detail
{

/*
    Breaks up a large property exchange message into chunks of the requested size.

    Note that the header *must* fit inside the first block, so you must ensure
    that the header is small enough to fit inside the requested chunk size.
*/
class PropertyDataMessageChunker
{
    auto tie() const { return std::tie (storage, body, source, dest, chunkSize, messageKind, requestId); }

public:
    /*  Constructs a chunker instance.

        @param storageIn        backing storage where each chunk will be written
        @param chunkSizeIn      the maximum size of each chunk
        @param messageKindIn    the subID2 byte identifying the type of message in each chunk
        @param requestIdIn      the id that should be included in all messages that are part of the same property exchange transaction
        @param headerIn         the header bytes of the message. This is always JSON encoded as 7-bit ASCII text, see the MIDI-CI spec for full details
        @param sourceIn         the MUID of the device sending the chunked messages
        @param destIn           the MUID of the recipient of the chunked messages
        @param bodyIn           a stream that can supply the data payload for this chunk sequence. All payload bytes *must* be 7-bit (MSB not set).
    */
    PropertyDataMessageChunker (std::vector<std::byte>& storageIn,
                                int chunkSizeIn,
                                const std::byte messageKindIn,
                                const std::byte requestIdIn,
                                Span<const std::byte> headerIn,
                                MUID sourceIn,
                                MUID destIn,
                                InputStream& bodyIn);

    /*  Returns true if this chunker hasn't finished producing chunks. */
    explicit operator bool() const { return *this != PropertyDataMessageChunker(); }

    /*  Allowing foreach usage. */
    auto begin() const { return *this; }

    /*  Allow foreach usage. */
    auto end()   const { return PropertyDataMessageChunker{}; }

    /*  Writes the bytes of the next chunk, if any, into the storage buffer. */
    PropertyDataMessageChunker& operator++() noexcept;

    /*  Checks whether the state of this chunker matches the state of another chunker, enabling foreach usage. */
    bool operator== (const PropertyDataMessageChunker& other) const noexcept { return tie() == other.tie(); }
    bool operator!= (const PropertyDataMessageChunker& other) const noexcept { return tie() != other.tie(); }

    /*  Returns a span over the valid bytes in the output buffer. */
    Span<const std::byte> operator*() const noexcept;

private:
    PropertyDataMessageChunker() = default;

    Span<const std::byte> getHeaderForBlock() const;
    int getRoomForBody() const;
    bool hasRoomForBody() const;
    void populateStorage() const;

    Span<const std::byte> header;
    std::vector<std::byte>* storage{};
    InputStream* body{};
    MUID source = MUID::makeUnchecked (0), dest = MUID::makeUnchecked (0);
    int chunkSize{};
    uint16_t thisChunk { 0x01 };
    std::byte messageKind{};
    std::byte requestId{};
};

} // namespace juce::midi_ci::detail
