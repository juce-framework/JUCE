/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

RelativeCoordinate::RelativeCoordinate (RelativeCoordinate&& other) noexcept
    : term (std::move (other.term))
{
}

RelativeCoordinate& RelativeCoordinate::operator= (RelativeCoordinate&& other) noexcept
{
    term = std::move (other.term);
    return *this;
}

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

} // namespace juce
