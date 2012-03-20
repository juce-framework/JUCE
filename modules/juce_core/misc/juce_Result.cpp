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

Result::Result (const String& message) noexcept
    : errorMessage (message)
{
}

Result::Result (const Result& other)
    : errorMessage (other.errorMessage)
{
}

Result& Result::operator= (const Result& other)
{
    errorMessage = other.errorMessage;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
Result::Result (Result&& other) noexcept
    : errorMessage (static_cast <String&&> (other.errorMessage))
{
}

Result& Result::operator= (Result&& other) noexcept
{
    errorMessage = static_cast <String&&> (other.errorMessage);
    return *this;
}
#endif

bool Result::operator== (const Result& other) const noexcept
{
    return errorMessage == other.errorMessage;
}

bool Result::operator!= (const Result& other) const noexcept
{
    return errorMessage != other.errorMessage;
}

Result Result::ok() noexcept
{
    return Result (String::empty);
}

Result Result::fail (const String& errorMessage) noexcept
{
    return Result (errorMessage.isEmpty() ? "Unknown Error" : errorMessage);
}

const String& Result::getErrorMessage() const noexcept
{
    return errorMessage;
}

bool Result::wasOk() const noexcept         { return errorMessage.isEmpty(); }
Result::operator bool() const noexcept      { return errorMessage.isEmpty(); }
bool Result::failed() const noexcept        { return errorMessage.isNotEmpty(); }
bool Result::operator!() const noexcept     { return errorMessage.isNotEmpty(); }
