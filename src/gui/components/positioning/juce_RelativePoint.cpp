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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_RelativePoint.h"

namespace RelativePointHelpers
{
    void skipComma (const juce_wchar* const s, int& i)
    {
        while (CharacterFunctions::isWhitespace (s[i]))
            ++i;

        if (s[i] == ',')
            ++i;
    }
}

//==============================================================================
RelativePoint::RelativePoint()
{
}

RelativePoint::RelativePoint (const Point<float>& absolutePoint)
    : x (absolutePoint.getX()), y (absolutePoint.getY())
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
    int i = 0;
    x = RelativeCoordinate (Expression::parse (s, i));
    RelativePointHelpers::skipComma (s, i);
    y = RelativeCoordinate (Expression::parse (s, i));
}

bool RelativePoint::operator== (const RelativePoint& other) const throw()
{
    return x == other.x && y == other.y;
}

bool RelativePoint::operator!= (const RelativePoint& other) const throw()
{
    return ! operator== (other);
}

const Point<float> RelativePoint::resolve (const Expression::EvaluationContext* context) const
{
    return Point<float> ((float) x.resolve (context),
                         (float) y.resolve (context));
}

void RelativePoint::moveToAbsolute (const Point<float>& newPos, const Expression::EvaluationContext* context)
{
    x.moveToAbsolute (newPos.getX(), context);
    y.moveToAbsolute (newPos.getY(), context);
}

const String RelativePoint::toString() const
{
    return x.toString() + ", " + y.toString();
}

void RelativePoint::renameSymbolIfUsed (const String& oldName, const String& newName)
{
    x.renameSymbolIfUsed (oldName, newName);
    y.renameSymbolIfUsed (oldName, newName);
}

bool RelativePoint::isDynamic() const
{
    return x.isDynamic() || y.isDynamic();
}


END_JUCE_NAMESPACE
