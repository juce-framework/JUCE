/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

const OSCType OSCTypes::int32   = 'i';
const OSCType OSCTypes::float32 = 'f';
const OSCType OSCTypes::string  = 's';
const OSCType OSCTypes::blob    = 'b';
const OSCType OSCTypes::colour  = 'r';

uint32 OSCColour::toInt32() const
{
    return ByteOrder::makeInt (alpha, blue, green, red);
}

OSCColour OSCColour::fromInt32 (uint32 c)
{
    return { (uint8) (c >> 24),
             (uint8) (c >> 16),
             (uint8) (c >> 8),
             (uint8) c };
}

} // namespace juce
