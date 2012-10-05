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

StringPool& Identifier::getPool()
{
    static StringPool pool;
    return pool;
}

Identifier::Identifier() noexcept
    : name (nullptr)
{
}

Identifier::Identifier (const Identifier& other) noexcept
    : name (other.name)
{
}

Identifier& Identifier::operator= (const Identifier& other) noexcept
{
    name = other.name;
    return *this;
}

Identifier::Identifier (const String& nm)
    : name (Identifier::getPool().getPooledString (nm))
{
    /* An Identifier string must be suitable for use as a script variable or XML
       attribute, so it can only contain this limited set of characters.. */
    jassert (isValidIdentifier (nm));
}

Identifier::Identifier (const char* const nm)
    : name (Identifier::getPool().getPooledString (nm))
{
    /* An Identifier string must be suitable for use as a script variable or XML
       attribute, so it can only contain this limited set of characters.. */
    jassert (isValidIdentifier (toString()));
}

Identifier::~Identifier()
{
}

Identifier Identifier::null;

bool Identifier::isValidIdentifier (const String& possibleIdentifier) noexcept
{
    return possibleIdentifier.isNotEmpty()
            && possibleIdentifier.containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-:#@$%");
}
