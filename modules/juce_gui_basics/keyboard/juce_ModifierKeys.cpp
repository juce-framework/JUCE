/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ModifierKeys ModifierKeys::currentModifiers;

ModifierKeys::ModifierKeys (int rawFlags) noexcept               : flags (rawFlags)    {}

int ModifierKeys::getNumMouseButtonsDown() const noexcept
{
    int num = 0;

    if (isLeftButtonDown())     ++num;
    if (isRightButtonDown())    ++num;
    if (isMiddleButtonDown())   ++num;

    return num;
}

ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    return ComponentPeer::getCurrentModifiersRealtime();
}

} // namespace juce
