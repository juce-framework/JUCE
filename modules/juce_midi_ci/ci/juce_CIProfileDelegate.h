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
    An interface with methods that can be overridden to customise how a Device
    implementing profiles responds to profile inquiries.

    @tags{Audio}
*/
struct ProfileDelegate
{
    ProfileDelegate() = default;
    virtual ~ProfileDelegate() = default;
    ProfileDelegate (const ProfileDelegate&) = default;
    ProfileDelegate (ProfileDelegate&&) = default;
    ProfileDelegate& operator= (const ProfileDelegate&) = default;
    ProfileDelegate& operator= (ProfileDelegate&&) = default;

    /** Called when a remote device requests that a profile is enabled or disabled.

        Old MIDI-CI implementations on remote devices may request that a profile
        is enabled with zero channels active - in this situation, it is
        recommended that you use ProfileHost::enableProfile to enable the
        default number of channels for that profile.

        Additionally, profiles for entire groups or function blocks may be enabled with zero
        active channels. In this case, the profile should be enabled on the entire group or
        function block.
    */
    virtual void profileEnablementRequested ([[maybe_unused]] MUID x,
                                             [[maybe_unused]] ProfileAtAddress profileAtAddress,
                                             [[maybe_unused]] int numChannels,
                                             [[maybe_unused]] bool enabled) = 0;
};

} // namespace juce::midi_ci
