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

#include "juce_RelativeRectangle.h"
#include "juce_RelativeCoordinatePositioner.h"

namespace RelativeRectangleHelpers
{
    inline void skipComma (const juce_wchar* const s, int& i)
    {
        while (CharacterFunctions::isWhitespace (s[i]))
            ++i;

        if (s[i] == ',')
            ++i;
    }
}

//==============================================================================
RelativeRectangle::RelativeRectangle()
{
}

RelativeRectangle::RelativeRectangle (const RelativeCoordinate& left_, const RelativeCoordinate& right_,
                                      const RelativeCoordinate& top_, const RelativeCoordinate& bottom_)
    : left (left_), right (right_), top (top_), bottom (bottom_)
{
}

RelativeRectangle::RelativeRectangle (const Rectangle<float>& rect, const String& componentName)
    : left (rect.getX()),
      right (Expression::symbol (componentName + "." + RelativeCoordinate::Strings::left) + Expression ((double) rect.getWidth())),
      top (rect.getY()),
      bottom (Expression::symbol (componentName + "." + RelativeCoordinate::Strings::top) + Expression ((double) rect.getHeight()))
{
}

RelativeRectangle::RelativeRectangle (const String& s)
{
    int i = 0;
    left = RelativeCoordinate (Expression::parse (s, i));
    RelativeRectangleHelpers::skipComma (s, i);
    top = RelativeCoordinate (Expression::parse (s, i));
    RelativeRectangleHelpers::skipComma (s, i);
    right = RelativeCoordinate (Expression::parse (s, i));
    RelativeRectangleHelpers::skipComma (s, i);
    bottom = RelativeCoordinate (Expression::parse (s, i));
}

bool RelativeRectangle::operator== (const RelativeRectangle& other) const throw()
{
    return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
}

bool RelativeRectangle::operator!= (const RelativeRectangle& other) const throw()
{
    return ! operator== (other);
}

const Rectangle<float> RelativeRectangle::resolve (const Expression::EvaluationContext* context) const
{
    const double l = left.resolve (context);
    const double r = right.resolve (context);
    const double t = top.resolve (context);
    const double b = bottom.resolve (context);

    return Rectangle<float> ((float) l, (float) t, (float) jmax (0.0, r - l), (float) jmax (0.0, b - t));
}

void RelativeRectangle::moveToAbsolute (const Rectangle<float>& newPos, const Expression::EvaluationContext* context)
{
    left.moveToAbsolute (newPos.getX(), context);
    right.moveToAbsolute (newPos.getRight(), context);
    top.moveToAbsolute (newPos.getY(), context);
    bottom.moveToAbsolute (newPos.getBottom(), context);
}

bool RelativeRectangle::isDynamic() const
{
    return left.isDynamic() || right.isDynamic() || top.isDynamic() || bottom.isDynamic();
}

const String RelativeRectangle::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}

void RelativeRectangle::renameSymbolIfUsed (const String& oldName, const String& newName)
{
    left.renameSymbolIfUsed (oldName, newName);
    right.renameSymbolIfUsed (oldName, newName);
    top.renameSymbolIfUsed (oldName, newName);
    bottom.renameSymbolIfUsed (oldName, newName);
}

//==============================================================================
class RelativeRectangleComponentPositioner  : public RelativeCoordinatePositionerBase
{
public:
    RelativeRectangleComponentPositioner (Component& component_, const RelativeRectangle& rectangle_)
        : RelativeCoordinatePositionerBase (component_),
          rectangle (rectangle_)
    {
    }

    bool registerCoordinates()
    {
        bool ok = addCoordinate (rectangle.left);
        ok = addCoordinate (rectangle.right) && ok;
        ok = addCoordinate (rectangle.top) && ok;
        ok = addCoordinate (rectangle.bottom) && ok;
        return ok;
    }

    bool isUsingRectangle (const RelativeRectangle& other) const throw()
    {
        return rectangle == other;
    }

    void applyToComponentBounds()
    {
        for (int i = 4; --i >= 0;)
        {
            const Rectangle<int> newBounds (rectangle.resolve (this).getSmallestIntegerContainer());

            if (newBounds == getComponent().getBounds())
                return;

            getComponent().setBounds (newBounds);
        }

        jassertfalse; // must be a recursive reference!
    }

private:
    const RelativeRectangle rectangle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativeRectangleComponentPositioner);
};

void RelativeRectangle::applyToComponent (Component& component) const
{
    if (isDynamic())
    {
        RelativeRectangleComponentPositioner* current = dynamic_cast <RelativeRectangleComponentPositioner*> (component.getPositioner());

        if (current == 0 || ! current->isUsingRectangle (*this))
        {
            RelativeRectangleComponentPositioner* p = new RelativeRectangleComponentPositioner (component, *this);

            component.setPositioner (p);
            p->apply();
        }
    }
    else
    {
        component.setPositioner (0);
        component.setBounds (resolve (0).getSmallestIntegerContainer());
    }
}


END_JUCE_NAMESPACE
