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

namespace juce
{

Identifier::Identifier() noexcept {}
Identifier::~Identifier() noexcept {}

Identifier::Identifier (const Identifier& other) noexcept  : name (other.name) {}

Identifier::Identifier (Identifier&& other) noexcept : name (std::move (other.name)) {}

Identifier& Identifier::operator= (Identifier&& other) noexcept
{
    name = std::move (other.name);
    return *this;
}

Identifier& Identifier::operator= (const Identifier& other) noexcept
{
    name = other.name;
    return *this;
}

Identifier::Identifier (const String& nm)
    : name (StringPool::getGlobalPool().getPooledString (nm))
{
    // An Identifier cannot be created from an empty string!
    jassert (nm.isNotEmpty());
}

Identifier::Identifier (const char* nm)
    : name (StringPool::getGlobalPool().getPooledString (nm))
{
    // An Identifier cannot be created from an empty string!
    jassert (nm != nullptr && nm[0] != 0);
}

Identifier::Identifier (String::CharPointerType start, String::CharPointerType end)
    : name (StringPool::getGlobalPool().getPooledString (start, end))
{
    // An Identifier cannot be created from an empty string!
    jassert (start < end);
}

Identifier Identifier::null;

bool Identifier::isValidIdentifier (const String& possibleIdentifier) noexcept
{
    return possibleIdentifier.isNotEmpty()
            && possibleIdentifier.containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-:#@$%");
}

} // namespace juce
