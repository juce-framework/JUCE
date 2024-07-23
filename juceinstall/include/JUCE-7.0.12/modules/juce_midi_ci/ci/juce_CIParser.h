/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci
{

/**
    Parses CI messages.

    @tags{Audio}
*/
class Parser
{
public:
    Parser() = delete;

    enum class Status
    {
        noError,                ///< Parsing was successful
        mismatchedMUID,         ///< The message destination MUID doesn't match the provided MUID
        collidingMUID,          ///< The message source MUID matches the provided MUID
        unrecognisedMessage,    ///< The message ID doesn't correspond to a known message
        reservedVersion,        ///< The MIDI CI version uses an unrecognised major version
        malformed,              ///< The message (whole message, or just body) could not be parsed
    };

    /** Parses the provided message;

        Call this with a full CI message. Don't include any "extra" bytes such as
        the leading/trailing 0xf0/0xf7 for messages that were originally in bytestream midi format,
        or the packet-header bytes from UMP-formatted sysex messages.

        Returns nullopt if the message doesn't need to be acknowledged by the entity with the provided MUID,
        or if the message is malformed.
        Otherwise, returns a parsed header, and optionally a body.
        If the body is std::monostate, then something went wrong while parsing. For example, the body
        may be malformed, or the CI version might be unrecognised.
    */
    static std::optional<Message::Parsed> parse (MUID ourMUID, Span<const std::byte> message, Status* = nullptr);

    /** Parses the provided message;

        Call this with a full CI message. Don't include any "extra" bytes such as
        the leading/trailing 0xf0/0xf7 for messages that were originally in bytestream midi format,
        or the packet-header bytes from UMP-formatted sysex messages.

        Returns nullopt if the message is malformed.
        Otherwise, returns a parsed header, and optionally a body.
        If the body is std::monostate, then something went wrong while parsing. For example, the body
        may be malformed, or the CI version might be unrecognised.
    */
    static std::optional<Message::Parsed> parse (Span<const std::byte> message, Status* = nullptr);

    /** Returns a human-readable string describing the message. */
    static String getMessageDescription (const Message::Parsed& message);
};

} // namespace juce::midi_ci
