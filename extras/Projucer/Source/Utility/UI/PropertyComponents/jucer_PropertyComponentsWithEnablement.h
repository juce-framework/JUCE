/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class TextPropertyComponentWithEnablement    : public TextPropertyComponent,
                                               private Value::Listener
{
public:
    TextPropertyComponentWithEnablement (ValueWithDefault& valueToControl,
                                         ValueWithDefault valueToListenTo,
                                         const String& propertyName,
                                         int maxNumChars,
                                         bool isMultiLine)
        : TextPropertyComponent (valueToControl, propertyName, maxNumChars, isMultiLine),
          valueWithDefault (valueToListenTo),
          value (valueWithDefault.getPropertyAsValue())
    {
        value.addListener (this);
        setEnabled (valueWithDefault.get());
    }

    ~TextPropertyComponentWithEnablement() override    { value.removeListener (this); }

private:
    ValueWithDefault valueWithDefault;
    Value value;

    void valueChanged (Value&) override       { setEnabled (valueWithDefault.get()); }
};

//==============================================================================
class ChoicePropertyComponentWithEnablement    : public ChoicePropertyComponent,
                                                 private Value::Listener
{
public:
    ChoicePropertyComponentWithEnablement (ValueWithDefault& valueToControl,
                                           ValueWithDefault valueToListenTo,
                                           const String& propertyName,
                                           const StringArray& choices,
                                           const Array<var>& correspondingValues)
        : ChoicePropertyComponent (valueToControl, propertyName, choices, correspondingValues),
          valueWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        setEnabled (valueWithDefault.get());
    }

    ~ChoicePropertyComponentWithEnablement() override    { value.removeListener (this); }

private:
    ValueWithDefault valueWithDefault;
    Value value;

    void valueChanged (Value&) override         { setEnabled (valueWithDefault.get()); }
};
