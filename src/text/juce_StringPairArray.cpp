/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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


#include "juce_StringPairArray.h"


//==============================================================================
StringPairArray::StringPairArray (const bool ignoreCase_) throw()
    : ignoreCase (ignoreCase_)
{
}

StringPairArray::StringPairArray (const StringPairArray& other) throw()
    : keys (other.keys),
      values (other.values),
      ignoreCase (other.ignoreCase)
{
}

StringPairArray::~StringPairArray() throw()
{
}

const StringPairArray& StringPairArray::operator= (const StringPairArray& other) throw()
{
    keys = other.keys;
    values = other.values;

    return *this;
}

bool StringPairArray::operator== (const StringPairArray& other) const throw()
{
    for (int i = keys.size(); --i >= 0;)
        if (other [keys[i]] != values[i])
            return false;

    return true;
}

bool StringPairArray::operator!= (const StringPairArray& other) const throw()
{
    return ! operator== (other);
}

const String& StringPairArray::operator[] (const String& key) const throw()
{
    return values [keys.indexOf (key, ignoreCase)];
}

const String StringPairArray::getValue (const String& key, const String& defaultReturnValue) const
{
    const int i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
        return values[i];

    return defaultReturnValue;
}

void StringPairArray::set (const String& key,
                           const String& value) throw()
{
    const int i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
    {
        values.set (i, value);
    }
    else
    {
        keys.add (key);
        values.add (value);
    }
}

void StringPairArray::addArray (const StringPairArray& other)
{
    for (int i = 0; i < other.size(); ++i)
        set (other.keys[i], other.values[i]);
}

void StringPairArray::clear() throw()
{
    keys.clear();
    values.clear();
}

void StringPairArray::remove (const String& key) throw()
{
    remove (keys.indexOf (key, ignoreCase));
}

void StringPairArray::remove (const int index) throw()
{
    keys.remove (index);
    values.remove (index);
}

void StringPairArray::setIgnoresCase (const bool shouldIgnoreCase) throw()
{
    ignoreCase = shouldIgnoreCase;
}

const String StringPairArray::getDescription() const
{
    String s;

    for (int i = 0; i < keys.size(); ++i)
    {
        s << keys[i] << T(" = ") << values[i];
        if (i < keys.size())
            s << T(", ");
    }

    return s;
}

void StringPairArray::minimiseStorageOverheads() throw()
{
    keys.minimiseStorageOverheads();
    values.minimiseStorageOverheads();
}

END_JUCE_NAMESPACE
