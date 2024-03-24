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
