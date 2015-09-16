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

const String RelativeCoordinate::Strings::parent ("parent");
const String RelativeCoordinate::Strings::left ("left");
const String RelativeCoordinate::Strings::right ("right");
const String RelativeCoordinate::Strings::top ("top");
const String RelativeCoordinate::Strings::bottom ("bottom");
const String RelativeCoordinate::Strings::x ("x");
const String RelativeCoordinate::Strings::y ("y");
const String RelativeCoordinate::Strings::width ("width");
const String RelativeCoordinate::Strings::height ("height");

RelativeCoordinate::StandardStrings::Type RelativeCoordinate::StandardStrings::getTypeOf (const String& s) noexcept
{
    if (s == Strings::left)    return left;
    if (s == Strings::right)   return right;
    if (s == Strings::top)     return top;
    if (s == Strings::bottom)  return bottom;
    if (s == Strings::x)       return x;
    if (s == Strings::y)       return y;
    if (s == Strings::width)   return width;
    if (s == Strings::height)  return height;
    if (s == Strings::parent)  return parent;
    return unknown;
}

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
{
}

RelativeCoordinate::RelativeCoordinate (const Expression& term_)
    : term (term_)
{
}

RelativeCoordinate::RelativeCoordinate (const RelativeCoordinate& other)
    : term (other.term)
{
}

RelativeCoordinate& RelativeCoordinate::operator= (const RelativeCoordinate& other)
{
    term = other.term;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
RelativeCoordinate::RelativeCoordinate (RelativeCoordinate&& other) noexcept
    : term (static_cast <Expression&&> (other.term))
{
}

RelativeCoordinate& RelativeCoordinate::operator= (RelativeCoordinate&& other) noexcept
{
    term = static_cast <Expression&&> (other.term);
    return *this;
}
#endif

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin)
    : term (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const String& s)
{
    String error;
    term = Expression (s, error);
}

RelativeCoordinate::~RelativeCoordinate()
{
}

bool RelativeCoordinate::operator== (const RelativeCoordinate& other) const noexcept
{
    return term.toString() == other.term.toString();
}

bool RelativeCoordinate::operator!= (const RelativeCoordinate& other) const noexcept
{
    return ! operator== (other);
}

double RelativeCoordinate::resolve (const Expression::Scope* scope) const
{
    if (scope != nullptr)
        return term.evaluate (*scope);

    return term.evaluate();
}

bool RelativeCoordinate::isRecursive (const Expression::Scope* scope) const
{
    String error;

    if (scope != nullptr)
        term.evaluate (*scope, error);
    else
        term.evaluate (Expression::Scope(), error);

    return error.isNotEmpty();
}

void RelativeCoordinate::moveToAbsolute (double newPos, const Expression::Scope* scope)
{
    if (scope != nullptr)
    {
        term = term.adjustedToGiveNewResult (newPos, *scope);
    }
    else
    {
        Expression::Scope defaultScope;
        term = term.adjustedToGiveNewResult (newPos, defaultScope);
    }
}

bool RelativeCoordinate::isDynamic() const
{
    return term.usesAnySymbols();
}

String RelativeCoordinate::toString() const
{
    return term.toString();
}
