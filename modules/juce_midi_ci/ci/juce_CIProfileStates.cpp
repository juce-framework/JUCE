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

SupportedAndActive ChannelProfileStates::get (const Profile& profile) const
{
    const auto iter = std::lower_bound (entries.begin(), entries.end(), profile);

    if (iter != entries.end() && iter->profile == profile)
        return iter->state;

    return {};
}

std::vector<Profile> ChannelProfileStates::getActive() const
{
    std::vector<Profile> result;

    for (const auto& item : entries)
        if (item.state.isActive())
            result.push_back (item.profile);

    return result;
}

std::vector<Profile> ChannelProfileStates::getInactive() const
{
    std::vector<Profile> result;

    for (const auto& item : entries)
        if (item.state.isSupported())
            result.push_back (item.profile);

    return result;
}

void ChannelProfileStates::set (const Profile& profile, SupportedAndActive state)
{
    const auto iter = std::lower_bound (entries.begin(), entries.end(), profile);

    if (iter != entries.end() && iter->profile == profile)
    {
        if (state != SupportedAndActive{})
            iter->state = state;
        else
            entries.erase (iter);
    }
    else if (state != SupportedAndActive{})
    {
        entries.insert (iter, { profile, state });
    }
}

void ChannelProfileStates::erase (const Profile& profile)
{
    const auto iter = std::lower_bound (entries.begin(), entries.end(), profile);

    if (iter != entries.end() && iter->profile == profile)
        entries.erase (iter);
}

} // namespace juce::midi_ci
