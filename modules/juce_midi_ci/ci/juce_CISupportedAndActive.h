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
