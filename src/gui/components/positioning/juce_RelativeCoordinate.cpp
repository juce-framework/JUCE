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
const String RelativeCoordinate::Strings::parent ("parent");
const String RelativeCoordinate::Strings::this_ ("this");
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

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin)
    : term (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const String& s)
{
    try
    {
        term = Expression (s);
    }
    catch (...)
    {}
}

RelativeCoordinate::~RelativeCoordinate()
{
}

bool RelativeCoordinate::operator== (const RelativeCoordinate& other) const throw()
{
    return term.toString() == other.term.toString();
}

bool RelativeCoordinate::operator!= (const RelativeCoordinate& other) const throw()
{
    return ! operator== (other);
}

double RelativeCoordinate::resolve (const Expression::EvaluationContext* context) const
{
    try
    {
        if (context != 0)
            return term.evaluate (*context);
        else
            return term.evaluate();
    }
    catch (...)
    {}

    return 0.0;
}

bool RelativeCoordinate::isRecursive (const Expression::EvaluationContext* context) const
{
    try
    {
        if (context != 0)
            term.evaluate (*context);
        else
            term.evaluate();
    }
    catch (...)
    {
        return true;
    }

    return false;
}

void RelativeCoordinate::moveToAbsolute (double newPos, const Expression::EvaluationContext* context)
{
    try
    {
        if (context != 0)
        {
            term = term.adjustedToGiveNewResult (newPos, *context);
        }
        else
        {
            Expression::EvaluationContext defaultContext;
            term = term.adjustedToGiveNewResult (newPos, defaultContext);
        }
    }
    catch (...)
    {}
}

bool RelativeCoordinate::references (const String& coordName, const Expression::EvaluationContext* context) const
{
    try
    {
        return term.referencesSymbol (coordName, context);
    }
    catch (...)
    {}

    return false;
}

bool RelativeCoordinate::isDynamic() const
{
    return term.usesAnySymbols();
}

const String RelativeCoordinate::toString() const
{
    return term.toString();
}

void RelativeCoordinate::renameSymbolIfUsed (const String& oldName, const String& newName)
{
    jassert (newName.isNotEmpty() && newName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    if (term.referencesSymbol (oldName, 0))
        term = term.withRenamedSymbol (oldName, newName);
}


END_JUCE_NAMESPACE
