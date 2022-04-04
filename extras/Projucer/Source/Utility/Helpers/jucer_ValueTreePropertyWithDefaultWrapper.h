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

#pragma once


//==============================================================================
/**
    Wraps a ValueTreePropertyWithDefault object that has a default which depends on a global value.
*/
class ValueTreePropertyWithDefaultWrapper  : private Value::Listener
{
public:
    ValueTreePropertyWithDefaultWrapper() = default;

    void init (const ValueTreePropertyWithDefault& v,
               ValueTreePropertyWithDefault global,
               TargetOS::OS targetOS)
    {
        wrappedValue = v;
        globalValue = global.getPropertyAsValue();
        globalIdentifier = global.getPropertyID();
        os = targetOS;

        if (wrappedValue.get() == var())
            wrappedValue.resetToDefault();

        globalValue.addListener (this);
        valueChanged (globalValue);
    }

    ValueTreePropertyWithDefault& getWrappedValueTreePropertyWithDefault()
    {
        return wrappedValue;
    }

    var getCurrentValue() const
    {
        return wrappedValue.get();
    }

private:
    void valueChanged (Value&) override
    {
        wrappedValue.setDefault (getAppSettings().getStoredPath (globalIdentifier, os).get());
    }

    ValueTreePropertyWithDefault wrappedValue;
    Value globalValue;

    Identifier globalIdentifier;
    TargetOS::OS os;
};
