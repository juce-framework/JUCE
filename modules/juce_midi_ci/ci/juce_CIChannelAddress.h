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
    Identifies a channel or set of channels in a multi-group MIDI endpoint.

    @tags{Audio}
*/
class ChannelAddress
{
private:
    uint8_t group{};              ///< A group within a MIDI endpoint, where 0 <= group && group < 16
    ChannelInGroup channel{};     ///< A set of channels related to specified group

    auto tie() const { return std::tie (group, channel); }

public:
    /** Returns a copy of this object with the specified group. */
    [[nodiscard]] ChannelAddress withGroup (int g) const
    {
        jassert (isPositiveAndBelow (g, 16));
        return withMember (*this, &ChannelAddress::group, (uint8_t) g);
    }

    /** Returns a copy of this object with the specified channel. */
    [[nodiscard]] ChannelAddress withChannel (ChannelInGroup c) const
    {
        return withMember (*this, &ChannelAddress::channel, c);
    }

    /** Returns the group. */
    [[nodiscard]] uint8_t getGroup()         const  { return group; }

    /** Returns the channel in the group. */
    [[nodiscard]] ChannelInGroup getChannel() const { return channel; }

    /** Returns true if this address refers to all channels in the function
        block containing the specified group.
    */
    bool isBlock()   const { return channel == ChannelInGroup::wholeBlock; }

    /** Returns true if this address refers to all channels in the specified
        group.
    */
    bool isGroup()   const { return channel == ChannelInGroup::wholeGroup; }

    /** Returns true if this address refers to a single channel. */
    bool isSingleChannel() const { return ! isBlock() && ! isGroup(); }

    bool operator<  (const ChannelAddress& other) const { return tie() <  other.tie(); }
    bool operator<= (const ChannelAddress& other) const { return tie() <= other.tie(); }
    bool operator>  (const ChannelAddress& other) const { return tie() >  other.tie(); }
    bool operator>= (const ChannelAddress& other) const { return tie() >= other.tie(); }
    bool operator== (const ChannelAddress& other) const { return tie() == other.tie(); }
    bool operator!= (const ChannelAddress& other) const { return ! operator== (other); }
};

} // namespace juce::midi_ci
