/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCER_VALUESOURCEHELPERS_JUCEHEADER__
#define __JUCER_VALUESOURCEHELPERS_JUCEHEADER__


//==============================================================================
/**
*/
template <typename Type>
class NumericValueSource   : public ValueSourceFilter
{
public:
    NumericValueSource (const Value& source)  : ValueSourceFilter (source) {}

    var getValue() const
    {
        return (Type) sourceValue.getValue();
    }

    void setValue (const var& newValue)
    {
        const Type newVal = static_cast <Type> (newValue);

        if (newVal != static_cast <Type> (getValue()))  // this test is important, because if a property is missing, it won't
            sourceValue = newVal;                       // create it (causing an unwanted undo action) when a control sets it to 0
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumericValueSource);
};


#endif   // __JUCER_VALUESOURCEHELPERS_JUCEHEADER__
