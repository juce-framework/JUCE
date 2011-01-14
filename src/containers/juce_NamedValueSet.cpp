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
#include "../text/juce_XmlElement.h"


//==============================================================================
NamedValueSet::NamedValue::NamedValue() throw()
{
}

inline NamedValueSet::NamedValue::NamedValue (const Identifier& name_, const var& value_)
    : name (name_), value (value_)
{
}

NamedValueSet::NamedValue::NamedValue (const NamedValue& other)
    : name (other.name), value (other.value)
{
}

NamedValueSet::NamedValue& NamedValueSet::NamedValue::operator= (const NamedValueSet::NamedValue& other)
{
    name = other.name;
    value = other.value;
    return *this;
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
{
    values.addCopyOfList (other.values);
}

NamedValueSet& NamedValueSet::operator= (const NamedValueSet& other)
{
    clear();
    values.addCopyOfList (other.values);
    return *this;
}

NamedValueSet::~NamedValueSet()
{
    clear();
}

void NamedValueSet::clear()
{
    values.deleteAll();
}

bool NamedValueSet::operator== (const NamedValueSet& other) const
{
    const NamedValue* i1 = values;
    const NamedValue* i2 = other.values;

    while (i1 != 0 && i2 != 0)
    {
        if (! (*i1 == *i2))
            return false;

        i1 = i1->nextListItem;
        i2 = i2->nextListItem;
    }

    return true;
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
    for (NamedValue* i = values; i != 0; i = i->nextListItem)
        if (i->name == name)
            return i->value;

    return var::null;
}

const var NamedValueSet::getWithDefault (const Identifier& name, const var& defaultReturnValue) const
{
    const var* v = getVarPointer (name);
    return v != 0 ? *v : defaultReturnValue;
}

var* NamedValueSet::getVarPointer (const Identifier& name) const
{
    for (NamedValue* i = values; i != 0; i = i->nextListItem)
        if (i->name == name)
            return &(i->value);

    return 0;
}

bool NamedValueSet::set (const Identifier& name, const var& newValue)
{
    LinkedListPointer<NamedValue>* i = &values;

    while (i->get() != 0)
    {
        NamedValue* const v = i->get();

        if (v->name == name)
        {
            if (v->value == newValue)
                return false;

            v->value = newValue;
            return true;
        }

        i = &(v->nextListItem);
    }

    i->insertNext (new NamedValue (name, newValue));
    return true;
}

bool NamedValueSet::contains (const Identifier& name) const
{
    return getVarPointer (name) != 0;
}

bool NamedValueSet::remove (const Identifier& name)
{
    LinkedListPointer<NamedValue>* i = &values;

    for (;;)
    {
        NamedValue* const v = i->get();

        if (v == 0)
            break;

        if (v->name == name)
        {
            delete i->removeNext();
            return true;
        }

        i = &(v->nextListItem);
    }

    return false;
}

const Identifier NamedValueSet::getName (const int index) const
{
    const NamedValue* const v = values[index];
    jassert (v != 0);
    return v->name;
}

const var NamedValueSet::getValueAt (const int index) const
{
    const NamedValue* const v = values[index];
    jassert (v != 0);
    return v->value;
}

void NamedValueSet::setFromXmlAttributes (const XmlElement& xml)
{
    clear();
    LinkedListPointer<NamedValue>::Appender appender (values);

    const int numAtts = xml.getNumAttributes(); // xxx inefficient - should write an att iterator..

    for (int i = 0; i < numAtts; ++i)
        appender.append (new NamedValue (xml.getAttributeName (i), var (xml.getAttributeValue (i))));
}

void NamedValueSet::copyToXmlAttributes (XmlElement& xml) const
{
    for (NamedValue* i = values; i != 0; i = i->nextListItem)
    {
        jassert (! i->value.isObject()); // DynamicObjects can't be stored as XML!

        xml.setAttribute (i->name.toString(),
                          i->value.toString());
    }
}


END_JUCE_NAMESPACE
