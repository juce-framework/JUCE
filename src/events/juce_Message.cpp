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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MessageListener.h"


//==============================================================================
Message::Message() throw()
{
}

Message::~Message() throw()
{
}

Message::Message (const int intParameter1_,
                  const int intParameter2_,
                  const int intParameter3_,
                  void* const pointerParameter_) throw()
    : intParameter1 (intParameter1_),
      intParameter2 (intParameter2_),
      intParameter3 (intParameter3_),
      pointerParameter (pointerParameter_)
{
}

END_JUCE_NAMESPACE
