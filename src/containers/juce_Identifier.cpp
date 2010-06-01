/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "juce_Identifier.h"


//==============================================================================
StringPool& Identifier::getPool()
{
    static StringPool pool;
    return pool;
}

Identifier::Identifier() throw()
    : name (0)
{
}

Identifier::Identifier (const Identifier& other) throw()
    : name (other.name)
{
}

Identifier& Identifier::operator= (const Identifier& other) throw()
{
    name = other.name;
    return *this;
}

Identifier::Identifier (const String& name_)
    : name (Identifier::getPool().getPooledString (name_))
{
    /* An Identifier string must be suitable for use as a script variable or XML
       attribute, so it can only contain this limited set of characters.. */
    jassert (name_.containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") && name_.isNotEmpty());
}

Identifier::Identifier (const char* const name_)
    : name (Identifier::getPool().getPooledString (name_))
{
    /* An Identifier string must be suitable for use as a script variable or XML
       attribute, so it can only contain this limited set of characters.. */
    jassert (toString().containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") && toString().isNotEmpty());
}

Identifier::~Identifier()
{
}

END_JUCE_NAMESPACE
