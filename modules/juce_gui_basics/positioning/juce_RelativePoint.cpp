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

namespace RelativePointHelpers
{
    inline void skipComma (String::CharPointerType& s)
    {
        s.incrementToEndOfWhitespace();

        if (*s == ',')
            ++s;
    }
}

//==============================================================================
RelativePoint::RelativePoint()
{
}

RelativePoint::RelativePoint (Point<float> absolutePoint)
    : x (absolutePoint.x), y (absolutePoint.y)
{
}

RelativePoint::RelativePoint (const float x_, const float y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const RelativeCoordinate& x_, const RelativeCoordinate& y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const String& s)
{
    String error;
    String::CharPointerType text (s.getCharPointer());
    x = RelativeCoordinate (Expression::parse (text, error));
    RelativePointHelpers::skipComma (text);
    y = RelativeCoordinate (Expression::parse (text, error));
}

bool RelativePoint::operator== (const RelativePoint& other) const noexcept
{
    return x == other.x && y == other.y;
}

bool RelativePoint::operator!= (const RelativePoint& other) const noexcept
{
    return ! operator== (other);
}

Point<float> RelativePoint::resolve (const Expression::Scope* scope) const
{
    return Point<float> ((float) x.resolve (scope),
                         (float) y.resolve (scope));
}

void RelativePoint::moveToAbsolute (Point<float> newPos, const Expression::Scope* scope)
{
    x.moveToAbsolute (newPos.x, scope);
    y.moveToAbsolute (newPos.y, scope);
}

String RelativePoint::toString() const
{
    return x.toString() + ", " + y.toString();
}

bool RelativePoint::isDynamic() const
{
    return x.isDynamic() || y.isDynamic();
}

} // namespace juce
