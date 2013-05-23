/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

ModifierKeys::ModifierKeys() noexcept
    : flags (0)
{
}

ModifierKeys::ModifierKeys (const int flags_) noexcept
    : flags (flags_)
{
}

ModifierKeys::ModifierKeys (const ModifierKeys& other) noexcept
    : flags (other.flags)
{
}

ModifierKeys& ModifierKeys::operator= (const ModifierKeys other) noexcept
{
    flags = other.flags;
    return *this;
}

ModifierKeys ModifierKeys::currentModifiers;

ModifierKeys ModifierKeys::getCurrentModifiers() noexcept
{
    return currentModifiers;
}

int ModifierKeys::getNumMouseButtonsDown() const noexcept
{
    int num = 0;

    if (isLeftButtonDown())     ++num;
    if (isRightButtonDown())    ++num;
    if (isMiddleButtonDown())   ++num;

    return num;
}
