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
