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
Coordinate::Coordinate()
    : value (0)
{
}

Coordinate::Coordinate (const double absoluteDistanceFromOrigin, const bool horizontal_)
    : anchor1 (getOriginAnchorName (horizontal_)),
      value (absoluteDistanceFromOrigin)
{
}

Coordinate::Coordinate (const double absoluteDistance, const String& source)
    : anchor1 (source.trim()),
      value (absoluteDistance)
{
    jassert (anchor1.isNotEmpty());
}

Coordinate::Coordinate (const double relativeProportion, const String& pos1, const String& pos2)
    : anchor1 (pos1.trim()),
      anchor2 (pos2.trim()),
      value (relativeProportion)
{
    jassert (anchor1.isNotEmpty());
    jassert (anchor2.isNotEmpty());
}

Coordinate::~Coordinate()
{
}

//==============================================================================
const String Coordinate::Strings::parent ("parent");
const String Coordinate::Strings::left ("left");
const String Coordinate::Strings::right ("right");
const String Coordinate::Strings::top ("top");
const String Coordinate::Strings::bottom ("bottom");

const String Coordinate::Strings::originX ("parent.left");
const String Coordinate::Strings::originY ("parent.top");
const String Coordinate::Strings::extentX ("parent.right");
const String Coordinate::Strings::extentY ("parent.bottom");

const String Coordinate::getObjectName (const String& fullName)
{
    return fullName.upToFirstOccurrenceOf (".", false, false);
}

const String Coordinate::getEdgeName (const String& fullName)
{
    return fullName.fromFirstOccurrenceOf (".", false, false);
}

const Coordinate Coordinate::getAnchorCoordinate1() const
{
    return Coordinate (0.0, anchor1);
}

const Coordinate Coordinate::getAnchorCoordinate2() const
{
    return Coordinate (0.0, anchor2);
}

bool Coordinate::isOrigin (const String& name)
{
    return name.isEmpty()
            || name == Strings::originX
            || name == Strings::originY;
}

const String Coordinate::getOriginAnchorName (const bool isHorizontal) const throw()
{
    return isHorizontal ? Strings::originX : Strings::originY;
}

const String Coordinate::getExtentAnchorName (const bool isHorizontal) const throw()
{
    return isHorizontal ? Strings::extentX : Strings::extentY;
}

//==============================================================================
Coordinate::RecursiveCoordinateException::RecursiveCoordinateException()
    : std::runtime_error ("Coordinate::RecursiveCoordinateException")
{
}

const Coordinate Coordinate::lookUpName (const String& name, const NamedCoordinateFinder& nameSource) const
{
    return nameSource.findNamedCoordinate (getObjectName (name), getEdgeName (name));
}

double Coordinate::resolveAnchor (const String& anchorName, const NamedCoordinateFinder& nameSource, int recursionCounter) const
{
    if (isOrigin (anchorName))
        return 0.0;

    return lookUpName (anchorName, nameSource).resolve (nameSource, recursionCounter + 1);
}

double Coordinate::resolve (const NamedCoordinateFinder& nameSource, int recursionCounter) const
{
    if (recursionCounter > 100)
    {
        jassertfalse
        throw RecursiveCoordinateException();
    }

    const double pos1 = resolveAnchor (anchor1, nameSource, recursionCounter);

    return isProportional() ? pos1 + (resolveAnchor (anchor2, nameSource, recursionCounter) - pos1) * value
                            : pos1 + value;
}

double Coordinate::resolve (const NamedCoordinateFinder& nameSource) const
{
    try
    {
        return resolve (nameSource, 0);
    }
    catch (RecursiveCoordinateException&)
    {}

    return 0.0;
}

void Coordinate::moveToAbsolute (double newPos, const NamedCoordinateFinder& nameSource)
{
    try
    {
        const double pos1 = resolveAnchor (anchor1, nameSource, 0);

        if (isProportional())
        {
            const double size = resolveAnchor (anchor2, nameSource, 0) - pos1;

            if (size != 0)
                value = (newPos - pos1) / size;
        }
        else
        {
            value = newPos - pos1;
        }
    }
    catch (RecursiveCoordinateException&)
    {}
}

void Coordinate::toggleProportionality (const NamedCoordinateFinder& nameSource, bool isHorizontal)
{
    const double oldValue = resolve (nameSource);

    anchor1 = getOriginAnchorName (isHorizontal);
    anchor2 = isProportional() ? String::empty
                               : getExtentAnchorName (isHorizontal);

    moveToAbsolute (oldValue, nameSource);
}

//==============================================================================
bool Coordinate::references (const String& coordName, const NamedCoordinateFinder& nameSource) const
{
    if (isOrigin (anchor1) && ! isProportional())
        return isOrigin (coordName);

    return anchor1 == coordName
            || anchor2 == coordName
            || lookUpName (anchor1, nameSource).references (coordName, nameSource)
            || (isProportional() && lookUpName (anchor2, nameSource).references (coordName, nameSource));
}

//==============================================================================
namespace CoordParserHelpers
{
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

Coordinate::Coordinate (const String& s, bool isHorizontal)
    : value (0)
{
    int i = 0;

    anchor1 = CoordParserHelpers::readAnchorName (s, i);

    if (anchor1.isNotEmpty())
    {
        CoordParserHelpers::skipWhitespace (s, i);

        if (s[i] == '+')
            value = CoordParserHelpers::readNumber (s, ++i);
        else if (s[i] == '-')
            value = -CoordParserHelpers::readNumber (s, ++i);
    }
    else
    {
        anchor1 = getOriginAnchorName (isHorizontal);

        value = CoordParserHelpers::readNumber (s, i);
        CoordParserHelpers::skipWhitespace (s, i);

        if (s[i] == '%')
        {
            value /= 100.0;
            CoordParserHelpers::skipWhitespace (s, ++i);

            if (s[i] == '*')
            {
                anchor1 = CoordParserHelpers::readAnchorName (s, ++i);

                if (anchor1.isEmpty())
                    anchor1 = getOriginAnchorName (isHorizontal);

                CoordParserHelpers::skipWhitespace (s, i);

                if (s[i] == '-' && s[i + 1] == '>')
                {
                    i += 2;
                    anchor2 = CoordParserHelpers::readAnchorName (s, i);
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

const String Coordinate::toString() const
{
    if (isProportional())
    {
        const String percent (CoordParserHelpers::limitedAccuracyString (value * 100.0));

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
            return CoordParserHelpers::limitedAccuracyString (value);
        else if (value > 0)
            return anchor1 + " + " + CoordParserHelpers::limitedAccuracyString (value);
        else if (value < 0)
            return anchor1 + " - " + CoordParserHelpers::limitedAccuracyString (-value);
        else
            return anchor1;
    }
}

//==============================================================================
const double Coordinate::getEditableNumber() const
{
    return isProportional() ? value * 100.0 : value;
}

void Coordinate::setEditableNumber (const double newValue)
{
    value = isProportional() ? newValue / 100.0 : newValue;
}

//==============================================================================
void Coordinate::changeAnchor1 (const String& newAnchorName, const NamedCoordinateFinder& nameSource)
{
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameSource);
    anchor1 = newAnchorName;
    moveToAbsolute (oldValue, nameSource);
}

void Coordinate::changeAnchor2 (const String& newAnchorName, const NamedCoordinateFinder& nameSource)
{
    jassert (isProportional());
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameSource);
    anchor2 = newAnchorName;
    moveToAbsolute (oldValue, nameSource);
}

void Coordinate::renameAnchorIfUsed (const String& oldName, const String& newName, const NamedCoordinateFinder& nameSource)
{
    jassert (oldName.isNotEmpty());
    jassert (newName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    if (newName.isEmpty())
    {
        if (getObjectName (anchor1) == oldName
             || getObjectName (anchor2) == oldName)
        {
            value = resolve (nameSource);
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
CoordinatePair::CoordinatePair()
{
}

CoordinatePair::CoordinatePair (const Point<float>& absolutePoint)
    : x (absolutePoint.getX(), true), y (absolutePoint.getY(), false)
{
}

CoordinatePair::CoordinatePair (const String& stringVersion)
{
    StringArray tokens;
    tokens.addTokens (stringVersion, ",", String::empty);

    x = Coordinate (tokens [0], true);
    y = Coordinate (tokens [1], false);
}

const Point<float> CoordinatePair::resolve (const Coordinate::NamedCoordinateFinder& nameSource) const
{
    return Point<float> ((float) x.resolve (nameSource),
                         (float) y.resolve (nameSource));
}

void CoordinatePair::moveToAbsolute (const Point<float>& newPos, const Coordinate::NamedCoordinateFinder& nameSource)
{
    x.moveToAbsolute (newPos.getX(), nameSource);
    y.moveToAbsolute (newPos.getY(), nameSource);
}

const String CoordinatePair::toString() const
{
    return x.toString() + ", " + y.toString();
}

void CoordinatePair::renameAnchorIfUsed (const String& oldName, const String& newName, const Coordinate::NamedCoordinateFinder& nameSource)
{
    x.renameAnchorIfUsed (oldName, newName, nameSource);
    y.renameAnchorIfUsed (oldName, newName, nameSource);
}


//==============================================================================
RectangleCoordinates::RectangleCoordinates()
{
}

RectangleCoordinates::RectangleCoordinates (const Rectangle<float>& rect, const String& componentName)
    : left (rect.getX(), true),
      right (rect.getWidth(), componentName + "." + Coordinate::Strings::left),
      top (rect.getY(), false),
      bottom (rect.getHeight(), componentName + "." + Coordinate::Strings::top)
{
}

RectangleCoordinates::RectangleCoordinates (const String& stringVersion)
{
    StringArray tokens;
    tokens.addTokens (stringVersion, ",", String::empty);

    left   = Coordinate (tokens [0], true);
    top    = Coordinate (tokens [1], false);
    right  = Coordinate (tokens [2], true);
    bottom = Coordinate (tokens [3], false);
}

const Rectangle<int> RectangleCoordinates::resolve (const Coordinate::NamedCoordinateFinder& nameSource) const
{
    const int l = roundToInt (left.resolve (nameSource));
    const int r = roundToInt (right.resolve (nameSource));
    const int t = roundToInt (top.resolve (nameSource));
    const int b = roundToInt (bottom.resolve (nameSource));

    return Rectangle<int> (l, t, r - l, b - t);
}

void RectangleCoordinates::moveToAbsolute (const Rectangle<float>& newPos, const Coordinate::NamedCoordinateFinder& nameSource)
{
    left.moveToAbsolute (newPos.getX(), nameSource);
    right.moveToAbsolute (newPos.getRight(), nameSource);
    top.moveToAbsolute (newPos.getY(), nameSource);
    bottom.moveToAbsolute (newPos.getBottom(), nameSource);
}

const String RectangleCoordinates::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}

void RectangleCoordinates::renameAnchorIfUsed (const String& oldName, const String& newName,
                                               const Coordinate::NamedCoordinateFinder& nameSource)
{
    left.renameAnchorIfUsed (oldName, newName, nameSource);
    right.renameAnchorIfUsed (oldName, newName, nameSource);
    top.renameAnchorIfUsed (oldName, newName, nameSource);
    bottom.renameAnchorIfUsed (oldName, newName, nameSource);
}



//==============================================================================
ComponentAutoLayoutManager::ComponentAutoLayoutManager (Component* parentComponent)
    : parent (parentComponent)
{
    parent->addComponentListener (this);
}

ComponentAutoLayoutManager::~ComponentAutoLayoutManager()
{
    parent->removeComponentListener (this);

    for (int i = components.size(); --i >= 0;)
        components.getUnchecked(i)->component->removeComponentListener (this);
}

void ComponentAutoLayoutManager::setMarker (const String& name, const Coordinate& coord)
{
    for (int i = markers.size(); --i >= 0;)
    {
        MarkerPosition* m = markers.getUnchecked(i);
        if (m->markerName == name)
        {
            m->position = coord;
            applyLayout();
            return;
        }
    }

    markers.add (new MarkerPosition (name, coord));
    applyLayout();
}

void ComponentAutoLayoutManager::setComponentBounds (Component* comp, const String& name, const RectangleCoordinates& coords)
{
    jassert (comp != 0);

    // All the components that this layout manages must be inside the parent component..
    jassert (parent->isParentOf (comp));

    for (int i = components.size(); --i >= 0;)
    {
        ComponentPosition* c = components.getUnchecked(i);
        if (c->component == comp)
        {
            c->name = name;
            c->coords = coords;
            triggerAsyncUpdate();
            return;
        }
    }

    components.add (new ComponentPosition (comp, name, coords));
    comp->addComponentListener (this);
    triggerAsyncUpdate();
}

void ComponentAutoLayoutManager::applyLayout()
{
    for (int i = components.size(); --i >= 0;)
    {
        ComponentPosition* c = components.getUnchecked(i);

        // All the components that this layout manages must be inside the parent component..
        jassert (parent->isParentOf (c->component));

        c->component->setBounds (c->coords.resolve (*this));
    }
}

const Coordinate ComponentAutoLayoutManager::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == Coordinate::Strings::parent)
    {
        if (edge == Coordinate::Strings::right)     return Coordinate ((double) parent->getWidth(), true);
        if (edge == Coordinate::Strings::bottom)    return Coordinate ((double) parent->getHeight(), false);
    }

    if (objectName.isNotEmpty() && edge.isNotEmpty())
    {
        for (int i = components.size(); --i >= 0;)
        {
            ComponentPosition* c = components.getUnchecked(i);

            if (c->name == objectName)
            {
                if (edge == Coordinate::Strings::left)   return c->coords.left;
                if (edge == Coordinate::Strings::right)  return c->coords.right;
                if (edge == Coordinate::Strings::top)    return c->coords.top;
                if (edge == Coordinate::Strings::bottom) return c->coords.bottom;
            }
        }
    }

    for (int i = markers.size(); --i >= 0;)
    {
        MarkerPosition* m = markers.getUnchecked(i);

        if (m->markerName == objectName)
            return m->position;
    }

    return Coordinate();
}

void ComponentAutoLayoutManager::componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
{
    triggerAsyncUpdate();

    if (parent == &component)
        handleUpdateNowIfNeeded();
}

void ComponentAutoLayoutManager::componentBeingDeleted (Component& component)
{
    for (int i = components.size(); --i >= 0;)
    {
        ComponentPosition* c = components.getUnchecked(i);
        if (c->component == &component)
        {
            components.remove (i);
            break;
        }
    }
}

void ComponentAutoLayoutManager::handleAsyncUpdate()
{
    applyLayout();
}

ComponentAutoLayoutManager::MarkerPosition::MarkerPosition (const String& name, const Coordinate& coord)
    : markerName (name), position (coord)
{
}

ComponentAutoLayoutManager::ComponentPosition::ComponentPosition (Component* component_, const String& name_, const RectangleCoordinates& coords_)
    : component (component_), name (name_), coords (coords_)
{
}
