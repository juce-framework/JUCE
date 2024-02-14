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
    Holds the maximum number of channels that may be activated for a MIDI-CI
    profile, along with the number of channels that are currently active.

    @tags{Audio}
*/
struct SupportedAndActive
{
    uint16_t supported{};   ///< The maximum number of member channels for a profile.
                            ///< 0 indicates that the profile is unsupported.
                            ///< For group/block profiles, 1/0 indicates that the
                            ///< profile is supported/unsupported respectively.

    uint16_t active{};      ///< The number of member channels currently active for a profile.
                            ///< 0 indicates that the profile is inactive.
                            ///< For group/block profiles, 1/0 indicates that the
                            ///< profile is supported/unsupported respectively.

    /** Returns true if supported is non-zero. */
    bool isSupported()  const { return supported != 0; }

    /** Returns true if active is non-zero. */
    bool isActive()     const { return active != 0; }

    bool operator== (const SupportedAndActive& other) const
    {
        const auto tie = [] (auto& x) { return std::tie (x.supported, x.active); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const SupportedAndActive& other) const { return ! operator== (other); }
};

} // namespace juce::midi_ci
