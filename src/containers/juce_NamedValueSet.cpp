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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_NamedValueSet.h"


//==============================================================================
NamedValueSet::NamedValue::NamedValue() throw()
{
}

inline NamedValueSet::NamedValue::NamedValue (const Identifier& name_, const var& value_)
    : name (name_), value (value_)
{
}

bool NamedValueSet::NamedValue::operator== (const NamedValueSet::NamedValue& other) const throw()
{
    return name == other.name && value == other.value;
}

//==============================================================================
NamedValueSet::NamedValueSet() throw()
{
}

NamedValueSet::NamedValueSet (const NamedValueSet& other)
    : values (other.values)
{
}

NamedValueSet& NamedValueSet::operator= (const NamedValueSet& other)
{
    values = other.values;
    return *this;
}

NamedValueSet::~NamedValueSet()
{
}

bool NamedValueSet::operator== (const NamedValueSet& other) const
{
    return values == other.values;
}

bool NamedValueSet::operator!= (const NamedValueSet& other) const
{
    return ! operator== (other);
}

int NamedValueSet::size() const throw()
{
    return values.size();
}

const var& NamedValueSet::operator[] (const Identifier& name) const
{
    for (int i = values.size(); --i >= 0;)
    {
        const NamedValue& v = values.getReference(i);

        if (v.name == name)
            return v.value;
    }

    return var::null;
}

const var NamedValueSet::getWithDefault (const Identifier& name, const var& defaultReturnValue) const
{
    const var* v = getVarPointer (name);
    return v != 0 ? *v : defaultReturnValue;
}

var* NamedValueSet::getVarPointer (const Identifier& name) const
{
    for (int i = values.size(); --i >= 0;)
    {
        NamedValue& v = values.getReference(i);

        if (v.name == name)
            return &(v.value);
    }

    return 0;
}

bool NamedValueSet::set (const Identifier& name, const var& newValue)
{
    for (int i = values.size(); --i >= 0;)
    {
        NamedValue& v = values.getReference(i);

        if (v.name == name)
        {
            if (v.value == newValue)
                return false;

            v.value = newValue;
            return true;
        }
    }

    values.add (NamedValue (name, newValue));
    return true;
}

bool NamedValueSet::contains (const Identifier& name) const
{
    return getVarPointer (name) != 0;
}

bool NamedValueSet::remove (const Identifier& name)
{
    for (int i = values.size(); --i >= 0;)
    {
        if (values.getReference(i).name == name)
        {
            values.remove (i);
            return true;
        }
    }

    return false;
}

const Identifier NamedValueSet::getName (const int index) const
{
    jassert (isPositiveAndBelow (index, values.size()));
    return values [index].name;
}

const var NamedValueSet::getValueAt (const int index) const
{
    jassert (isPositiveAndBelow (index, values.size()));
    return values [index].value;
}

void NamedValueSet::clear()
{
    values.clear();
}


END_JUCE_NAMESPACE
