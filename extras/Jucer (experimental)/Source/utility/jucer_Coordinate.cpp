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

void Coordinate::renameAnchorIfUsed (const String& oldName, const String& newName, const MarkerResolver& markerResolver)
{
    jassert (oldName.isNotEmpty());

    if (newName.isEmpty())
    {
        if (anchor1.upToFirstOccurrenceOf (".", false, false) == oldName
            || anchor2.upToFirstOccurrenceOf (".", false, false) == oldName)
        {
            value = resolve (markerResolver);
            isProportion = false;
            anchor1 = String::empty;
            anchor2 = String::empty;
        }
    }
    else
    {
        if (anchor1.upToFirstOccurrenceOf (".", false, false) == oldName)
            anchor1 = newName + anchor1.fromFirstOccurrenceOf (".", true, false);

        if (anchor2.upToFirstOccurrenceOf (".", false, false) == oldName)
            anchor2 = newName + anchor2.fromFirstOccurrenceOf (".", true, false);
    }
}

//==============================================================================
RectangleCoordinates::RectangleCoordinates()
    : left (true), right (true), top (false), bottom (false)
{
}

RectangleCoordinates::RectangleCoordinates (const Rectangle<float>& rect, const String& componentName)
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

void RectangleCoordinates::moveToAbsolute (const Rectangle<float>& newPos, const Coordinate::MarkerResolver& markerResolver)
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

void RectangleCoordinates::renameAnchorIfUsed (const String& oldName, const String& newName,
                                               const Coordinate::MarkerResolver& markerResolver)
{
    left.renameAnchorIfUsed (oldName, newName, markerResolver);
    right.renameAnchorIfUsed (oldName, newName, markerResolver);
    top.renameAnchorIfUsed (oldName, newName, markerResolver);
    bottom.renameAnchorIfUsed (oldName, newName, markerResolver);
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

void ComponentAutoLayoutManager::setComponentLayout (Component* comp, const String& name, const RectangleCoordinates& coords)
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

const Coordinate ComponentAutoLayoutManager::findMarker (const String& name, bool isHorizontal) const
{
    if (name == Coordinate::parentRightMarkerName)     return Coordinate ((double) parent->getWidth(), isHorizontal);
    if (name == Coordinate::parentBottomMarkerName)    return Coordinate ((double) parent->getHeight(), isHorizontal);

    if (name.containsChar ('.'))
    {
        const String compName (name.upToFirstOccurrenceOf (".", false, false).trim());
        const String edge (name.fromFirstOccurrenceOf (".", false, false).trim());

        if (compName.isNotEmpty() && edge.isNotEmpty())
        {
            for (int i = components.size(); --i >= 0;)
            {
                ComponentPosition* c = components.getUnchecked(i);

                if (c->name == compName)
                {
                    if (edge == "left")   return c->coords.left;
                    if (edge == "right")  return c->coords.right;
                    if (edge == "top")    return c->coords.top;
                    if (edge == "bottom") return c->coords.bottom;
                }
            }
        }
    }

    for (int i = markers.size(); --i >= 0;)
    {
        MarkerPosition* m = markers.getUnchecked(i);

        if (m->markerName == name)
            return m->position;
    }

    return Coordinate (isHorizontal);
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
