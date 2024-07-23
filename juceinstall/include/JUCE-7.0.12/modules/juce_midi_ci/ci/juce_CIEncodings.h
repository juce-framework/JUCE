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
    Utility functions for working with data formats used by property exchange
    messages.

    @tags{Audio}
*/
struct Encodings
{
    /** Text in ACK and NAK messages can't be utf-8 or ASCII because each byte only has 7 usable bits.
        The encoding rules are in section 5.10.4 of the CI spec.
    */
    static String stringFrom7BitText (Span<const std::byte> bytes);

    /** Text in ACK and NAK messages can't be utf-8 or ASCII because each byte only has 7 usable bits.
        The encoding rules are in section 5.10.4 of the CI spec.
    */
    static std::vector<std::byte> stringTo7BitText (const String& text);

    /** Converts a list of bytes representing a 7-bit ASCII string to JSON. */
    static var jsonFrom7BitText (Span<const std::byte> bytes)
    {
        return JSON::parse (stringFrom7BitText (bytes));
    }

    /** Converts a JSON object to a list of bytes in 7-bit ASCII format. */
    static std::vector<std::byte> jsonTo7BitText (const var& v)
    {
        return stringTo7BitText (JSON::toString (v, JSON::FormatOptions{}.withSpacing (JSON::Spacing::none)));
    }

    /** Each group of seven stored bytes is transmitted as eight bytes.
        First, the sign bits of the seven bytes are sent, followed by the low-order 7 bits of each byte.
    */
    static std::vector<std::byte> toMcoded7 (Span<const std::byte> bytes);

    /** Each group of seven stored bytes is transmitted as eight bytes.
        First, the sign bits of the seven bytes are sent, followed by the low-order 7 bits of each byte.
    */
    static std::vector<std::byte> fromMcoded7 (Span<const std::byte> bytes);

    /** Attempts to encode the provided byte span using the specified encoding.

        The ASCII encoding does not make any changes to the input stream, but
        encoding will fail if any byte has its most significant bit set.
    */
    static std::optional<std::vector<std::byte>> tryEncode (Span<const std::byte> bytes,
                                                            Encoding mutualEncoding);

    /** Decodes the provided byte span using the specified encoding.

        All bytes of the input must be 7-bit values, i.e. all most-significant bits
        are unset.
    */
    static std::vector<std::byte> decode (Span<const std::byte> bytes, Encoding mutualEncoding);

    Encodings() = delete;
};

} // namespace juce::midi_ci
