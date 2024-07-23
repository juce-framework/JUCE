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
    Contains information about a MIDI 2.0 function block.

    @tags{Audio}
*/
struct FunctionBlock
{
    std::byte identifier { 0x7f }; ///< 0x7f == no function block
    uint8_t firstGroup = 0;        ///< The first group that is part of the block, 0-based
    uint8_t numGroups = 1;         ///< The number of groups contained in the block

    bool operator== (const FunctionBlock& other) const
    {
        const auto tie = [] (auto& x) { return std::tie (x.identifier, x.firstGroup, x.numGroups); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const FunctionBlock& other) const { return ! operator== (other); }
};

} // namespace juce::midi_ci
