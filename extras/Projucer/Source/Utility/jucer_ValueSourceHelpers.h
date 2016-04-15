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

#ifndef JUCER_VALUESOURCEHELPERS_H_INCLUDED
#define JUCER_VALUESOURCEHELPERS_H_INCLUDED


//==============================================================================
/**
*/
template <typename Type>
class NumericValueSource   : public ValueSourceFilter
{
public:
    NumericValueSource (const Value& source)  : ValueSourceFilter (source) {}

    var getValue() const override
    {
        return (Type) sourceValue.getValue();
    }

    void setValue (const var& newValue) override
    {
        const Type newVal = static_cast<Type> (newValue);

        if (newVal != static_cast<Type> (getValue()))  // this test is important, because if a property is missing, it won't
            sourceValue = newVal;                      // create it (causing an unwanted undo action) when a control sets it to 0
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumericValueSource)
};


#endif   // JUCER_VALUESOURCEHELPERS_H_INCLUDED
