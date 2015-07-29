/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

namespace RelativePointHelpers
{
    inline void skipComma (String::CharPointerType& s)
    {
        s = s.findEndOfWhitespace();

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
    String::CharPointerType text (s.getCharPointer());
    x = RelativeCoordinate (Expression::parse (text));
    RelativePointHelpers::skipComma (text);
    y = RelativeCoordinate (Expression::parse (text));
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
