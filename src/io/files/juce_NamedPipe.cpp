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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_NamedPipe.h"


//==============================================================================
NamedPipe::NamedPipe()
    : internal (0)
{
}

NamedPipe::~NamedPipe()
{
    close();
}

bool NamedPipe::openExisting (const String& pipeName)
{
    currentPipeName = pipeName;
    return openInternal (pipeName, false);
}

bool NamedPipe::createNewPipe (const String& pipeName)
{
    currentPipeName = pipeName;
    return openInternal (pipeName, true);
}

bool NamedPipe::isOpen() const
{
    return internal != 0;
}

const String NamedPipe::getName() const
{
    return currentPipeName;
}

// other methods for this class are implemented in the platform-specific files



END_JUCE_NAMESPACE
