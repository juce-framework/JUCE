/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

Identifier::Identifier() noexcept {}
Identifier::~Identifier() noexcept {}

Identifier::Identifier (const Identifier& other) noexcept  : name (other.name) {}

Identifier& Identifier::operator= (const Identifier other) noexcept
{
    name = other.name;
    return *this;
}

Identifier::Identifier (const String& nm)
    : name (StringPool::getGlobalPool().getPooledString (nm))
{
    /* An Identifier string must be suitable for use as a script variable or XML
       attribute, so it can only contain this limited set of characters.. */
    jassert (isValidIdentifier (toString()));
}

Identifier::Identifier (const char* nm)
    : name (StringPool::getGlobalPool().getPooledString (nm))
{
    /* An Identifier string must be suitable for use as a script variable or XML
       attribute, so it can only contain this limited set of characters.. */
    jassert (isValidIdentifier (toString()));
}

Identifier::Identifier (String::CharPointerType start, String::CharPointerType end)
    : name (StringPool::getGlobalPool().getPooledString (start, end))
{
    /* An Identifier string must be suitable for use as a script variable or XML
       attribute, so it can only contain this limited set of characters.. */
    jassert (isValidIdentifier (toString()));
}

Identifier Identifier::null;

bool Identifier::isValidIdentifier (const String& possibleIdentifier) noexcept
{
    return possibleIdentifier.isNotEmpty()
            && possibleIdentifier.containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-:#@$%");
}
