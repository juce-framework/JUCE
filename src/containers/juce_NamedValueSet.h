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

#ifndef __JUCE_NAMEDVALUESET_JUCEHEADER__
#define __JUCE_NAMEDVALUESET_JUCEHEADER__

#include "juce_Variant.h"
#include "../containers/juce_Array.h"


//==============================================================================
/** Holds a set of named var objects.

    This can be used as a basic structure to hold a set of var object, which can
    be retrieved by using their identifier.
*/
class JUCE_API  NamedValueSet
{
public:
    /** Creates an empty set. */
    NamedValueSet() throw();

    /** Creates a copy of another set. */
    NamedValueSet (const NamedValueSet& other);

    /** Replaces this set with a copy of another set. */
    NamedValueSet& operator= (const NamedValueSet& other);

    /** Destructor. */
    ~NamedValueSet();

    //==============================================================================
    /** Returns the total number of values that the set contains. */
    int size() const throw();

    /** Returns the value of a named item.
        If the name isn't found, this will return a void variant.
        @see getProperty
    */
    const var& operator[] (const var::identifier& name) const;

    /** Tries to return the named value, but if no such value is found, this will
        instead return the supplied default value.
    */
    const var getWithDefault (const var::identifier& name, const var& defaultReturnValue) const;

    /** Returns a pointer to the object holding a named value, or
        null if there is no value with this name. */
    var* getItem (const var::identifier& name) const;

    /** Changes or adds a named value.
        @returns true if a value was changed or added; false if the
                 value was already set the the value passed-in.
    */
    bool set (const var::identifier& name, const var& newValue);

    /** Returns true if the set contains an item with the specified name. */
    bool contains (const var::identifier& name) const;

    /** Removes a value from the set.
        @returns    true if a value was removed; false if there was no value
                    with the name that was given.
    */
    bool remove (const var::identifier& name);

    /** Returns the name of the value at a given index.
        The index must be between 0 and size() - 1. Out-of-range indexes will
        return an empty identifier.
    */
    const var::identifier getName (int index) const;

    /** Returns the value of the item at a given index.
        The index must be between 0 and size() - 1. Out-of-range indexes will
        return an empty identifier.
    */
    const var getValueAt (int index) const;

    /** Removes all values. */
    void clear();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    struct NamedValue
    {
        NamedValue() throw();
        NamedValue (const var::identifier& name, const var& value);

        var::identifier name;
        var value;
    };

    Array <NamedValue> values;
};


#endif   // __JUCE_NAMEDVALUESET_JUCEHEADER__
