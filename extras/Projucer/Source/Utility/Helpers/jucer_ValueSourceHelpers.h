/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
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
