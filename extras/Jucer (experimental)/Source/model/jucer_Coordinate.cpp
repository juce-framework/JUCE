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

#include "jucer_Coordinate.h"


//==============================================================================
const char* Coordinate::parentLeftMarkerName   = "parent.left";
const char* Coordinate::parentRightMarkerName  = "parent.right";
const char* Coordinate::parentTopMarkerName    = "parent.top";
const char* Coordinate::parentBottomMarkerName = "parent.bottom";

Coordinate::Coordinate (bool horizontal_)
    : value (0), isProportion (false), horizontal (horizontal_)
{
}

Coordinate::Coordinate (double absoluteDistanceFromOrigin, bool horizontal_)
    : value (absoluteDistanceFromOrigin), isProportion (false), horizontal (horizontal_)
{
}

Coordinate::Coordinate (double absoluteDistance, const String& source, bool horizontal_)
    : anchor1 (source), value (absoluteDistance), isProportion (false), horizontal (horizontal_)
{
}

Coordinate::Coordinate (double relativeProportion, const String& pos1, const String& pos2, bool horizontal_)
    : anchor1 (pos1), anchor2 (pos2), value (relativeProportion), isProportion (true), horizontal (horizontal_)
{
}

Coordinate::~Coordinate()
{
}

const Coordinate Coordinate::getAnchorPoint1() const
{
    return Coordinate (0.0, anchor1, horizontal);
}

const Coordinate Coordinate::getAnchorPoint2() const
{
    return Coordinate (0.0, anchor2, horizontal);
}

bool Coordinate::isOrigin (const String& name)
{
    return name.isEmpty() || name == parentLeftMarkerName || name == parentTopMarkerName;
}

const String Coordinate::getOriginMarkerName() const
{
    return horizontal ? parentLeftMarkerName : parentTopMarkerName;
}

const String Coordinate::getExtentMarkerName() const
{
    return horizontal ? parentRightMarkerName : parentBottomMarkerName;
}

const String Coordinate::checkName (const String& name) const
{
    return name.isEmpty() ? getOriginMarkerName() : name;
}

double Coordinate::getPosition (const String& name, const MarkerResolver& markerResolver, int recursionCounter) const
{
    if (isOrigin (name))
        return 0.0;

    return markerResolver.findMarker (name, horizontal)
                         .resolve (markerResolver, recursionCounter + 1);
}

struct RecursivePositionException
{
};

double Coordinate::resolve (const MarkerResolver& markerResolver, int recursionCounter) const
{
    if (recursionCounter > 100)
    {
        jassertfalse
        throw RecursivePositionException();
    }

    const double pos1 = getPosition (anchor1, markerResolver, recursionCounter);

    return isProportion ? pos1 + (getPosition (anchor2, markerResolver, recursionCounter) - pos1) * value
                        : pos1 + value;
}

double Coordinate::resolve (const MarkerResolver& markerResolver) const
{
    try
    {
        return resolve (markerResolver, 0);
    }
    catch (RecursivePositionException&)
    {}

    return 0.0;
}

void Coordinate::moveToAbsolute (double newPos, const MarkerResolver& markerResolver)
{
    try
    {
        const double pos1 = getPosition (anchor1, markerResolver, 0);

        if (isProportion)
        {
            const double size = getPosition (anchor2, markerResolver, 0) - pos1;

            if (size != 0)
                value = (newPos - pos1) / size;
        }
        else
        {
            value = newPos - pos1;
        }
    }
    catch (RecursivePositionException&)
    {}
}

bool Coordinate::referencesDirectly (const String& markerName) const
{
    jassert (markerName.isNotEmpty());
    return anchor1 == markerName || anchor2 == markerName;
}

bool Coordinate::referencesIndirectly (const String& markerName, const MarkerResolver& markerResolver) const
{
    if (isOrigin (anchor1) && ! isProportion)
        return isOrigin (markerName);

    return referencesDirectly (markerName)
            || markerResolver.findMarker (anchor1, horizontal).referencesIndirectly (markerName, markerResolver)
            || (isProportion && markerResolver.findMarker (anchor2, horizontal).referencesIndirectly (markerName, markerResolver));
}

void Coordinate::skipWhitespace (const String& s, int& i)
{
    while (CharacterFunctions::isWhitespace (s[i]))
        ++i;
}

const String Coordinate::readMarkerName (const String& s, int& i)
{
    skipWhitespace (s, i);

    if (CharacterFunctions::isLetter (s[i]) || s[i] == '_')
    {
        int start = i;

        while (CharacterFunctions::isLetterOrDigit (s[i]) || s[i] == '_' || s[i] == '.')
            ++i;

        return s.substring (start, i);
    }

    return String::empty;
}

double Coordinate::readNumber (const String& s, int& i)
{
    skipWhitespace (s, i);

    int start = i;

    if (CharacterFunctions::isDigit (s[i]) || s[i] == '.' || s[i] == '-')
        ++i;

    while (CharacterFunctions::isDigit (s[i]) || s[i] == '.')
        ++i;

    if ((s[i] == 'e' || s[i] == 'E')
         && (CharacterFunctions::isDigit (s[i + 1])
              || s[i + 1] == '-'
              || s[i + 1] == '+'))
    {
        i += 2;

        while (CharacterFunctions::isDigit (s[i]))
            ++i;
    }

    const double value = s.substring (start, i).getDoubleValue();

    while (CharacterFunctions::isWhitespace (s[i]) || s[i] == ',')
        ++i;

    return value;
}

Coordinate::Coordinate (const String& s, bool horizontal_)
    : value (0), isProportion (false), horizontal (horizontal_)
{
    int i = 0;

    anchor1 = readMarkerName (s, i);

    if (anchor1.isNotEmpty())
    {
        skipWhitespace (s, i);

        if (s[i] == '+')
            value = readNumber (s, ++i);
        else if (s[i] == '-')
            value = -readNumber (s, ++i);
    }
    else
    {
        value = readNumber (s, i);
        skipWhitespace (s, i);

        if (s[i] == '%')
        {
            isProportion = true;
            value /= 100.0;
            skipWhitespace (s, ++i);

            if (s[i] == '*')
            {
                anchor1 = readMarkerName (s, ++i);
                skipWhitespace (s, i);

                if (s[i] == '-' && s[i + 1] == '>')
                {
                    i += 2;
                    anchor2 = readMarkerName (s, i);
                }
                else
                {
                    anchor2 = anchor1;
                    anchor1 = getOriginMarkerName();
                }
            }
            else
            {
                anchor1 = getOriginMarkerName();
                anchor2 = getExtentMarkerName();
            }
        }
    }
}

static const String limitedAccuracyString (const double n)
{
    return String (n, 3).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
}

const String Coordinate::toString() const
{
    if (isProportion)
    {
        const String percent (limitedAccuracyString (value * 100.0));

        if (isOrigin (anchor1))
        {
            if (anchor2 == parentRightMarkerName || anchor2 == parentBottomMarkerName)
                return percent + "%";
            else
                return percent + "% * " + checkName (anchor2);
        }
        else
            return percent + "% * " + checkName (anchor1) + " -> " + checkName (anchor2);
    }
    else
    {
        if (isOrigin (anchor1))
            return limitedAccuracyString (value);
        else if (value > 0)
            return checkName (anchor1) + " + " + limitedAccuracyString (value);
        else if (value < 0)
            return checkName (anchor1) + " - " + limitedAccuracyString (-value);
        else
            return checkName (anchor1);
    }
}

const double Coordinate::getEditableValue() const
{
    return isProportion ? value * 100.0 : value;
}

void Coordinate::setEditableValue (const double newValue)
{
    value = isProportion ? newValue / 100.0 : newValue;
}

void Coordinate::toggleProportionality (const MarkerResolver& markerResolver)
{
    const double oldValue = resolve (markerResolver);

    isProportion = ! isProportion;
    anchor1 = getOriginMarkerName();
    anchor2 = getExtentMarkerName();

    moveToAbsolute (oldValue, markerResolver);
}

void Coordinate::changeAnchor1 (const String& newMarkerName, const MarkerResolver& markerResolver)
{
    const double oldValue = resolve (markerResolver);
    anchor1 = newMarkerName;
    moveToAbsolute (oldValue, markerResolver);
}

void Coordinate::changeAnchor2 (const String& newMarkerName, const MarkerResolver& markerResolver)
{
    const double oldValue = resolve (markerResolver);
    anchor2 = newMarkerName;
    moveToAbsolute (oldValue, markerResolver);
}

//==============================================================================
RectangleCoordinates::RectangleCoordinates()
    : left (true), right (true), top (false), bottom (false)
{
}

RectangleCoordinates::RectangleCoordinates (const Rectangle<int>& rect, const String& componentName)
    : left (rect.getX(), true),
      right (rect.getWidth(), componentName + ".left", true),
      top (rect.getY(), false),
      bottom (rect.getHeight(), componentName + ".top", false)
{
}

RectangleCoordinates::RectangleCoordinates (const String& stringVersion)
    : left (true), right (true), top (false), bottom (false)
{
    StringArray tokens;
    tokens.addTokens (stringVersion, ",", String::empty);

    left   = Coordinate (tokens [0], true);
    top    = Coordinate (tokens [1], false);
    right  = Coordinate (tokens [2], true);
    bottom = Coordinate (tokens [3], false);
}

const Rectangle<int> RectangleCoordinates::resolve (const Coordinate::MarkerResolver& markerResolver) const
{
    const int l = roundToInt (left.resolve (markerResolver));
    const int r = roundToInt (right.resolve (markerResolver));
    const int t = roundToInt (top.resolve (markerResolver));
    const int b = roundToInt (bottom.resolve (markerResolver));

    return Rectangle<int> (l, t, r - l, b - t);
}

void RectangleCoordinates::moveToAbsolute (const Rectangle<int>& newPos, const Coordinate::MarkerResolver& markerResolver)
{
    left.moveToAbsolute (newPos.getX(), markerResolver);
    right.moveToAbsolute (newPos.getRight(), markerResolver);
    top.moveToAbsolute (newPos.getY(), markerResolver);
    bottom.moveToAbsolute (newPos.getBottom(), markerResolver);
}

const String RectangleCoordinates::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}
