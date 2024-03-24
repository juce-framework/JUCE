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
