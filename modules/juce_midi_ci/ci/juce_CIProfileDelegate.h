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
