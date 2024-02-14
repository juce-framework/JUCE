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
    A 28-bit ID that uniquely identifies a device taking part in a series of
    MIDI-CI transactions.

    @tags{Audio}
*/
class MUID
{
    constexpr explicit MUID (uint32_t v) : value (v) {}

    // 0x0fffff00 to 0x0ffffffe are reserved, 0x0fffffff is 'broadcast'
    static constexpr uint32_t userMuidEnd = 0x0fffff00;
    static constexpr uint32_t mask = 0x0fffffff;
    uint32_t value{};

public:
    /** Returns the ID as a plain integer. */
    constexpr uint32_t get() const { return value; }

    /** Converts the provided integer to a MUID without validation that it
        is within the allowed range.
    */
    static MUID makeUnchecked (uint32_t v)
    {
        // If this is hit, the MUID has too many bits set!
        jassert ((v & mask) == v);
        return MUID (v);
    }

    /** Returns a MUID if the provided value is within the valid range for
        MUID values; otherwise returns nullopt.
    */
    static std::optional<MUID> make (uint32_t v)
    {
        if ((v & mask) == v)
            return makeUnchecked (v);

        return {};
    }

    /** Makes a random MUID using the provided random engine. */
    static MUID makeRandom (Random& r)
    {
        return makeUnchecked ((uint32_t) r.nextInt (userMuidEnd));
    }

    bool operator== (const MUID other) const { return value == other.value; }
    bool operator!= (const MUID other) const { return value != other.value; }
    bool operator<  (const MUID other) const { return value <  other.value; }

    /** Returns the special MUID representing the broadcast address. */
    static constexpr MUID getBroadcast() { return MUID { mask }; }
};

} // namespace juce::midi_ci
