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

namespace RelativeRectangleHelpers
{
    inline void skipComma (String::CharPointerType& s)
    {
        s = s.findEndOfWhitespace();

        if (*s == ',')
            ++s;
    }

    static bool dependsOnSymbolsOtherThanThis (const Expression& e)
    {
        if (e.getType() == Expression::operatorType && e.getSymbolOrFunction() == ".")
            return true;

        if (e.getType() == Expression::symbolType)
        {
            switch (RelativeCoordinate::StandardStrings::getTypeOf (e.getSymbolOrFunction()))
            {
                case RelativeCoordinate::StandardStrings::x:
                case RelativeCoordinate::StandardStrings::y:
                case RelativeCoordinate::StandardStrings::left:
                case RelativeCoordinate::StandardStrings::right:
                case RelativeCoordinate::StandardStrings::top:
                case RelativeCoordinate::StandardStrings::bottom:   return false;
                default: break;
            }

            return true;
        }
        else
        {
            for (int i = e.getNumInputs(); --i >= 0;)
                if (dependsOnSymbolsOtherThanThis (e.getInput(i)))
                    return true;
        }

        return false;
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

RelativeRectangle::RelativeRectangle (const Rectangle<float>& rect)
    : left (rect.getX()),
      right (Expression::symbol (RelativeCoordinate::Strings::left) + Expression ((double) rect.getWidth())),
      top (rect.getY()),
      bottom (Expression::symbol (RelativeCoordinate::Strings::top) + Expression ((double) rect.getHeight()))
{
}

RelativeRectangle::RelativeRectangle (const String& s)
{
    String error;
    String::CharPointerType text (s.getCharPointer());
    left = RelativeCoordinate (Expression::parse (text, error));
    RelativeRectangleHelpers::skipComma (text);
    top = RelativeCoordinate (Expression::parse (text, error));
    RelativeRectangleHelpers::skipComma (text);
    right = RelativeCoordinate (Expression::parse (text, error));
    RelativeRectangleHelpers::skipComma (text);
    bottom = RelativeCoordinate (Expression::parse (text, error));
}

bool RelativeRectangle::operator== (const RelativeRectangle& other) const noexcept
{
    return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
}

bool RelativeRectangle::operator!= (const RelativeRectangle& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
// An expression context that can evaluate expressions using "this"
class RelativeRectangleLocalScope  : public Expression::Scope
{
public:
    RelativeRectangleLocalScope (const RelativeRectangle& rect_)  : rect (rect_) {}

    Expression getSymbolValue (const String& symbol) const
    {
        switch (RelativeCoordinate::StandardStrings::getTypeOf (symbol))
        {
            case RelativeCoordinate::StandardStrings::x:
            case RelativeCoordinate::StandardStrings::left:     return rect.left.getExpression();
            case RelativeCoordinate::StandardStrings::y:
            case RelativeCoordinate::StandardStrings::top:      return rect.top.getExpression();
            case RelativeCoordinate::StandardStrings::right:    return rect.right.getExpression();
            case RelativeCoordinate::StandardStrings::bottom:   return rect.bottom.getExpression();
            default: break;
        }

        return Expression::Scope::getSymbolValue (symbol);
    }

private:
    const RelativeRectangle& rect;

    JUCE_DECLARE_NON_COPYABLE (RelativeRectangleLocalScope)
};

const Rectangle<float> RelativeRectangle::resolve (const Expression::Scope* scope) const
{
    if (scope == nullptr)
    {
        RelativeRectangleLocalScope defaultScope (*this);
        return resolve (&defaultScope);
    }
    else
    {
        const double l = left.resolve (scope);
        const double r = right.resolve (scope);
        const double t = top.resolve (scope);
        const double b = bottom.resolve (scope);

        return Rectangle<float> ((float) l, (float) t, (float) jmax (0.0, r - l), (float) jmax (0.0, b - t));
    }
}

void RelativeRectangle::moveToAbsolute (const Rectangle<float>& newPos, const Expression::Scope* scope)
{
    left.moveToAbsolute (newPos.getX(), scope);
    right.moveToAbsolute (newPos.getRight(), scope);
    top.moveToAbsolute (newPos.getY(), scope);
    bottom.moveToAbsolute (newPos.getBottom(), scope);
}

bool RelativeRectangle::isDynamic() const
{
    using namespace RelativeRectangleHelpers;

    return dependsOnSymbolsOtherThanThis (left.getExpression())
            || dependsOnSymbolsOtherThanThis (right.getExpression())
            || dependsOnSymbolsOtherThanThis (top.getExpression())
            || dependsOnSymbolsOtherThanThis (bottom.getExpression());
}

String RelativeRectangle::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}

void RelativeRectangle::renameSymbol (const Expression::Symbol& oldSymbol, const String& newName, const Expression::Scope& scope)
{
    left = left.getExpression().withRenamedSymbol (oldSymbol, newName, scope);
    right = right.getExpression().withRenamedSymbol (oldSymbol, newName, scope);
    top = top.getExpression().withRenamedSymbol (oldSymbol, newName, scope);
    bottom = bottom.getExpression().withRenamedSymbol (oldSymbol, newName, scope);
}

//==============================================================================
class RelativeRectangleComponentPositioner  : public RelativeCoordinatePositionerBase
{
public:
    RelativeRectangleComponentPositioner (Component& comp, const RelativeRectangle& r)
        : RelativeCoordinatePositionerBase (comp),
          rectangle (r)
    {
    }

    bool registerCoordinates() override
    {
        bool ok = addCoordinate (rectangle.left);
        ok = addCoordinate (rectangle.right) && ok;
        ok = addCoordinate (rectangle.top) && ok;
        ok = addCoordinate (rectangle.bottom) && ok;
        return ok;
    }

    bool isUsingRectangle (const RelativeRectangle& other) const noexcept
    {
        return rectangle == other;
    }

    void applyToComponentBounds() override
    {
        for (int i = 32; --i >= 0;)
        {
            ComponentScope scope (getComponent());
            const Rectangle<int> newBounds (rectangle.resolve (&scope).getSmallestIntegerContainer());

            if (newBounds == getComponent().getBounds())
                return;

            getComponent().setBounds (newBounds);
        }

        jassertfalse; // Seems to be a recursive reference!
    }

    void applyNewBounds (const Rectangle<int>& newBounds) override
    {
        if (newBounds != getComponent().getBounds())
        {
            ComponentScope scope (getComponent());
            rectangle.moveToAbsolute (newBounds.toFloat(), &scope);

            applyToComponentBounds();
        }
    }

private:
    RelativeRectangle rectangle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativeRectangleComponentPositioner)
};

void RelativeRectangle::applyToComponent (Component& component) const
{
    if (isDynamic())
    {
        RelativeRectangleComponentPositioner* current = dynamic_cast<RelativeRectangleComponentPositioner*> (component.getPositioner());

        if (current == nullptr || ! current->isUsingRectangle (*this))
        {
            RelativeRectangleComponentPositioner* p = new RelativeRectangleComponentPositioner (component, *this);

            component.setPositioner (p);
            p->apply();
        }
    }
    else
    {
        component.setPositioner (nullptr);
        component.setBounds (resolve (nullptr).getSmallestIntegerContainer());
    }
}
