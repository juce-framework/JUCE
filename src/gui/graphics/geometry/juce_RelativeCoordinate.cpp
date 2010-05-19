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

#include "juce_RelativeCoordinate.h"

//==============================================================================
namespace RelativeCoordinateHelpers
{
    static bool isOrigin (const String& name)
    {
        return name.isEmpty()
                || name == RelativeCoordinate::Strings::parentLeft
                || name == RelativeCoordinate::Strings::parentTop;
    }

    static const String getOriginAnchorName (const bool isHorizontal) throw()
    {
        return isHorizontal ? RelativeCoordinate::Strings::parentLeft
                            : RelativeCoordinate::Strings::parentTop;
    }

    static const String getExtentAnchorName (const bool isHorizontal) throw()
    {
        return isHorizontal ? RelativeCoordinate::Strings::parentRight
                            : RelativeCoordinate::Strings::parentBottom;
    }

    static const String getObjectName (const String& fullName)
    {
        return fullName.upToFirstOccurrenceOf (".", false, false);
    }

    static const String getEdgeName (const String& fullName)
    {
        return fullName.fromFirstOccurrenceOf (".", false, false);
    }

    static const RelativeCoordinate findCoordinate (const String& name, const RelativeCoordinate::NamedCoordinateFinder& nameFinder)
    {
        return nameFinder.findNamedCoordinate (getObjectName (name), getEdgeName (name));
    }

    //==============================================================================
    struct RecursionException  : public std::runtime_error
    {
        RecursionException()   : std::runtime_error ("Recursive RelativeCoordinate expression")
        {
        }
    };

    //==============================================================================
    static void skipWhitespace (const String& s, int& i)
    {
        while (CharacterFunctions::isWhitespace (s[i]))
            ++i;
    }

    static const String readAnchorName (const String& s, int& i)
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

    static double readNumber (const String& s, int& i)
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

    static const String limitedAccuracyString (const double n)
    {
        return String (n, 3).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
    }
}

//==============================================================================
const String RelativeCoordinate::Strings::parent ("parent");
const String RelativeCoordinate::Strings::left ("left");
const String RelativeCoordinate::Strings::right ("right");
const String RelativeCoordinate::Strings::top ("top");
const String RelativeCoordinate::Strings::bottom ("bottom");
const String RelativeCoordinate::Strings::parentLeft ("parent.left");
const String RelativeCoordinate::Strings::parentTop ("parent.top");
const String RelativeCoordinate::Strings::parentRight ("parent.right");
const String RelativeCoordinate::Strings::parentBottom ("parent.bottom");

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
    : value (0)
{
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin, const bool horizontal_)
    : anchor1 (RelativeCoordinateHelpers::getOriginAnchorName (horizontal_)),
      value (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistance, const String& source)
    : anchor1 (source.trim()),
      value (absoluteDistance)
{
    jassert (anchor1.isNotEmpty());
}

RelativeCoordinate::RelativeCoordinate (const double relativeProportion, const String& pos1, const String& pos2)
    : anchor1 (pos1.trim()),
      anchor2 (pos2.trim()),
      value (relativeProportion)
{
    jassert (anchor1.isNotEmpty());
    jassert (anchor2.isNotEmpty());
}

RelativeCoordinate::~RelativeCoordinate()
{
}

//==============================================================================
const RelativeCoordinate RelativeCoordinate::getAnchorCoordinate1() const
{
    return RelativeCoordinate (0.0, anchor1);
}

const RelativeCoordinate RelativeCoordinate::getAnchorCoordinate2() const
{
    return RelativeCoordinate (0.0, anchor2);
}

double RelativeCoordinate::resolveAnchor (const String& anchorName, const NamedCoordinateFinder& nameFinder, int recursionCounter)
{
    if (RelativeCoordinateHelpers::isOrigin (anchorName))
        return 0.0;

    return RelativeCoordinateHelpers::findCoordinate (anchorName, nameFinder).resolve (nameFinder, recursionCounter + 1);
}

double RelativeCoordinate::resolve (const NamedCoordinateFinder& nameFinder, int recursionCounter) const
{
    if (recursionCounter > 150)
    {
        jassertfalse
        throw RelativeCoordinateHelpers::RecursionException();
    }

    const double pos1 = resolveAnchor (anchor1, nameFinder, recursionCounter);

    return isProportional() ? pos1 + (resolveAnchor (anchor2, nameFinder, recursionCounter) - pos1) * value
                            : pos1 + value;
}

double RelativeCoordinate::resolve (const NamedCoordinateFinder& nameFinder) const
{
    try
    {
        return resolve (nameFinder, 0);
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {}

    return 0.0;
}

bool RelativeCoordinate::isRecursive (const NamedCoordinateFinder& nameFinder) const
{
    try
    {
        (void) resolve (nameFinder, 0);
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {
        return true;
    }

    return false;
}

void RelativeCoordinate::moveToAbsolute (double newPos, const NamedCoordinateFinder& nameFinder)
{
    try
    {
        const double pos1 = resolveAnchor (anchor1, nameFinder, 0);

        if (isProportional())
        {
            const double size = resolveAnchor (anchor2, nameFinder, 0) - pos1;

            if (size != 0)
                value = (newPos - pos1) / size;
        }
        else
        {
            value = newPos - pos1;
        }
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {}
}

void RelativeCoordinate::toggleProportionality (const NamedCoordinateFinder& nameFinder, bool isHorizontal)
{
    const double oldValue = resolve (nameFinder);

    anchor1 = RelativeCoordinateHelpers::getOriginAnchorName (isHorizontal);
    anchor2 = isProportional() ? String::empty
                               : RelativeCoordinateHelpers::getExtentAnchorName (isHorizontal);

    moveToAbsolute (oldValue, nameFinder);
}

//==============================================================================
bool RelativeCoordinate::references (const String& coordName, const NamedCoordinateFinder& nameFinder) const
{
    using namespace RelativeCoordinateHelpers;

    if (isOrigin (anchor1) && ! isProportional())
        return isOrigin (coordName);

    return anchor1 == coordName
            || anchor2 == coordName
            || findCoordinate (anchor1, nameFinder).references (coordName, nameFinder)
            || (isProportional() && findCoordinate (anchor2, nameFinder).references (coordName, nameFinder));
}

//==============================================================================
RelativeCoordinate::RelativeCoordinate (const String& s, bool isHorizontal)
    : value (0)
{
    using namespace RelativeCoordinateHelpers;
    int i = 0;

    anchor1 = readAnchorName (s, i);

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
        anchor1 = getOriginAnchorName (isHorizontal);

        value = readNumber (s, i);
        skipWhitespace (s, i);

        if (s[i] == '%')
        {
            value /= 100.0;
            skipWhitespace (s, ++i);

            if (s[i] == '*')
            {
                anchor1 = readAnchorName (s, ++i);

                if (anchor1.isEmpty())
                    anchor1 = getOriginAnchorName (isHorizontal);

                skipWhitespace (s, i);

                if (s[i] == '-' && s[i + 1] == '>')
                {
                    i += 2;
                    anchor2 = readAnchorName (s, i);
                }
                else
                {
                    anchor2 = anchor1;
                    anchor1 = getOriginAnchorName (isHorizontal);
                }
            }
            else
            {
                anchor1 = getOriginAnchorName (isHorizontal);
                anchor2 = getExtentAnchorName (isHorizontal);
            }
        }
    }
}

const String RelativeCoordinate::toString() const
{
    using namespace RelativeCoordinateHelpers;

    if (isProportional())
    {
        const String percent (limitedAccuracyString (value * 100.0));

        if (isOrigin (anchor1))
        {
            if (anchor2 == "parent.right" || anchor2 == "parent.bottom")
                return percent + "%";
            else
                return percent + "% * " + anchor2;
        }
        else
            return percent + "% * " + anchor1 + " -> " + anchor2;
    }
    else
    {
        if (isOrigin (anchor1))
            return limitedAccuracyString (value);
        else if (value > 0)
            return anchor1 + " + " + limitedAccuracyString (value);
        else if (value < 0)
            return anchor1 + " - " + limitedAccuracyString (-value);
        else
            return anchor1;
    }
}

//==============================================================================
const double RelativeCoordinate::getEditableNumber() const
{
    return isProportional() ? value * 100.0 : value;
}

void RelativeCoordinate::setEditableNumber (const double newValue)
{
    value = isProportional() ? newValue / 100.0 : newValue;
}

//==============================================================================
void RelativeCoordinate::changeAnchor1 (const String& newAnchorName, const NamedCoordinateFinder& nameFinder)
{
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameFinder);
    anchor1 = newAnchorName;
    moveToAbsolute (oldValue, nameFinder);
}

void RelativeCoordinate::changeAnchor2 (const String& newAnchorName, const NamedCoordinateFinder& nameFinder)
{
    jassert (isProportional());
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameFinder);
    anchor2 = newAnchorName;
    moveToAbsolute (oldValue, nameFinder);
}

void RelativeCoordinate::renameAnchorIfUsed (const String& oldName, const String& newName, const NamedCoordinateFinder& nameFinder)
{
    using namespace RelativeCoordinateHelpers;
    jassert (oldName.isNotEmpty());
    jassert (newName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    if (newName.isEmpty())
    {
        if (getObjectName (anchor1) == oldName
             || getObjectName (anchor2) == oldName)
        {
            value = resolve (nameFinder);
            anchor1 = String::empty;
            anchor2 = String::empty;
        }
    }
    else
    {
        if (getObjectName (anchor1) == oldName)
            anchor1 = newName + "." + getEdgeName (anchor1);

        if (getObjectName (anchor2) == oldName)
            anchor2 = newName + "." + getEdgeName (anchor2);
    }
}

//==============================================================================
RelativePoint::RelativePoint()
    : x (0, true), y (0, false)
{
}

RelativePoint::RelativePoint (const Point<float>& absolutePoint)
    : x (absolutePoint.getX(), true), y (absolutePoint.getY(), false)
{
}

RelativePoint::RelativePoint (const String& stringVersion)
{
    const int separator = stringVersion.indexOfChar (',');

    x = RelativeCoordinate (stringVersion.substring (0, separator), true);
    y = RelativeCoordinate (stringVersion.substring (separator + 1), false);
}

const Point<float> RelativePoint::resolve (const RelativeCoordinate::NamedCoordinateFinder& nameFinder) const
{
    return Point<float> ((float) x.resolve (nameFinder),
                         (float) y.resolve (nameFinder));
}

void RelativePoint::moveToAbsolute (const Point<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder& nameFinder)
{
    x.moveToAbsolute (newPos.getX(), nameFinder);
    y.moveToAbsolute (newPos.getY(), nameFinder);
}

const String RelativePoint::toString() const
{
    return x.toString() + ", " + y.toString();
}

void RelativePoint::renameAnchorIfUsed (const String& oldName, const String& newName, const RelativeCoordinate::NamedCoordinateFinder& nameFinder)
{
    x.renameAnchorIfUsed (oldName, newName, nameFinder);
    y.renameAnchorIfUsed (oldName, newName, nameFinder);
}


//==============================================================================
RelativeRectangle::RelativeRectangle()
{
}

RelativeRectangle::RelativeRectangle (const Rectangle<float>& rect, const String& componentName)
    : left (rect.getX(), true),
      right (rect.getWidth(), componentName + "." + RelativeCoordinate::Strings::left),
      top (rect.getY(), false),
      bottom (rect.getHeight(), componentName + "." + RelativeCoordinate::Strings::top)
{
}

RelativeRectangle::RelativeRectangle (const String& stringVersion)
{
    StringArray tokens;
    tokens.addTokens (stringVersion, ",", String::empty);

    left   = RelativeCoordinate (tokens [0], true);
    top    = RelativeCoordinate (tokens [1], false);
    right  = RelativeCoordinate (tokens [2], true);
    bottom = RelativeCoordinate (tokens [3], false);
}

const Rectangle<float> RelativeRectangle::resolve (const RelativeCoordinate::NamedCoordinateFinder& nameFinder) const
{
    const double l = left.resolve (nameFinder);
    const double r = right.resolve (nameFinder);
    const double t = top.resolve (nameFinder);
    const double b = bottom.resolve (nameFinder);

    return Rectangle<float> ((float) l, (float) t, (float) (r - l), (float) (b - t));
}

void RelativeRectangle::moveToAbsolute (const Rectangle<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder& nameFinder)
{
    left.moveToAbsolute (newPos.getX(), nameFinder);
    right.moveToAbsolute (newPos.getRight(), nameFinder);
    top.moveToAbsolute (newPos.getY(), nameFinder);
    bottom.moveToAbsolute (newPos.getBottom(), nameFinder);
}

const String RelativeRectangle::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}

void RelativeRectangle::renameAnchorIfUsed (const String& oldName, const String& newName,
                                               const RelativeCoordinate::NamedCoordinateFinder& nameFinder)
{
    left.renameAnchorIfUsed (oldName, newName, nameFinder);
    right.renameAnchorIfUsed (oldName, newName, nameFinder);
    top.renameAnchorIfUsed (oldName, newName, nameFinder);
    bottom.renameAnchorIfUsed (oldName, newName, nameFinder);
}


END_JUCE_NAMESPACE
