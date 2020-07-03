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
/**
    Wraps a ValueWithDefault object that has a default which depends on a global value.
*/
class ValueWithDefaultWrapper  : private Value::Listener
{
public:
    ValueWithDefaultWrapper() = default;

    void init (const ValueWithDefault& vwd, ValueWithDefault global, TargetOS::OS targetOS)
    {
        wrappedValue = vwd;
        globalValue = global.getPropertyAsValue();
        globalIdentifier = global.getPropertyID();
        os = targetOS;

        if (wrappedValue.get() == var())
            wrappedValue.resetToDefault();

        globalValue.addListener (this);
        valueChanged (globalValue);
    }

    ValueWithDefault& getWrappedValueWithDefault()
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

    ValueWithDefault wrappedValue;
    Value globalValue;

    Identifier globalIdentifier;
    TargetOS::OS os;
};
