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

Result::Result() noexcept {}

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

Result::Result (Result&& other) noexcept
    : errorMessage (std::move (other.errorMessage))
{
}

Result& Result::operator= (Result&& other) noexcept
{
    errorMessage = std::move (other.errorMessage);
    return *this;
}

bool Result::operator== (const Result& other) const noexcept
{
    return errorMessage == other.errorMessage;
}

bool Result::operator!= (const Result& other) const noexcept
{
    return errorMessage != other.errorMessage;
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

} // namespace juce
