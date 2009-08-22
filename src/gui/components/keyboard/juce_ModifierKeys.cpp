/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ModifierKeys.h"


//==============================================================================
ModifierKeys::ModifierKeys (const int flags_) throw()
    : flags (flags_)
{
}

ModifierKeys::ModifierKeys (const ModifierKeys& other) throw()
    : flags (other.flags)
{
}

const ModifierKeys& ModifierKeys::operator= (const ModifierKeys& other) throw()
{
    flags = other.flags;
    return *this;
}

int ModifierKeys::currentModifierFlags = 0;

const ModifierKeys ModifierKeys::getCurrentModifiers() throw()
{
    return ModifierKeys (currentModifierFlags);
}

END_JUCE_NAMESPACE
