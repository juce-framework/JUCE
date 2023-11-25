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
