/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
class TextPropertyComponentWithEnablement    : public TextPropertyComponent,
                                               private Value::Listener
{
public:
    TextPropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                         ValueTreePropertyWithDefault valueToListenTo,
                                         const String& propertyName,
                                         int maxNumChars,
                                         bool multiLine)
        : TextPropertyComponent (valueToControl, propertyName, maxNumChars, multiLine),
          propertyWithDefault (valueToListenTo),
          value (propertyWithDefault.getPropertyAsValue())
    {
        value.addListener (this);
        setEnabled (propertyWithDefault.get());
    }

    ~TextPropertyComponentWithEnablement() override  { value.removeListener (this); }

private:
    ValueTreePropertyWithDefault propertyWithDefault;
    Value value;

    void valueChanged (Value&) override  { setEnabled (propertyWithDefault.get()); }
};

//==============================================================================
class ChoicePropertyComponentWithEnablement    : public ChoicePropertyComponent,
                                                 private Value::Listener
{
public:
    ChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                           ValueTreePropertyWithDefault valueToListenTo,
                                           const String& propertyName,
                                           const StringArray& choiceToUse,
                                           const Array<var>& correspondingValues)
        : ChoicePropertyComponent (valueToControl, propertyName, choiceToUse, correspondingValues),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        valueChanged (value);
    }

    ChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                           ValueTreePropertyWithDefault valueToListenTo,
                                           const Identifier& multiChoiceID,
                                           const String& propertyName,
                                           const StringArray& choicesToUse,
                                           const Array<var>& correspondingValues)
        : ChoicePropertyComponentWithEnablement (valueToControl, valueToListenTo, propertyName, choicesToUse, correspondingValues)
    {
        jassert (valueToListenTo.get().getArray() != nullptr);

        isMultiChoice = true;
        idToCheck = multiChoiceID;

        valueChanged (value);
    }

    ChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                           ValueTreePropertyWithDefault valueToListenTo,
                                           const String& propertyName)
        : ChoicePropertyComponent (valueToControl, propertyName),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        valueChanged (value);
    }

    ~ChoicePropertyComponentWithEnablement() override    { value.removeListener (this); }

private:
    ValueTreePropertyWithDefault propertyWithDefault;
    Value value;

    bool isMultiChoice = false;
    Identifier idToCheck;

    bool checkMultiChoiceVar() const
    {
        jassert (isMultiChoice);

        auto v = propertyWithDefault.get();

        if (auto* varArray = v.getArray())
            return varArray->contains (idToCheck.toString());

        jassertfalse;
        return false;
    }

    void valueChanged (Value&) override
    {
        if (isMultiChoice)
            setEnabled (checkMultiChoiceVar());
        else
            setEnabled (propertyWithDefault.get());
    }
};

//==============================================================================
class MultiChoicePropertyComponentWithEnablement    : public MultiChoicePropertyComponent,
                                                      private Value::Listener
{
public:
    MultiChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                                ValueTreePropertyWithDefault valueToListenTo,
                                                const String& propertyName,
                                                const StringArray& choices,
                                                const Array<var>& correspondingValues)
        : MultiChoicePropertyComponent (valueToControl,
                                        propertyName,
                                        choices,
                                        correspondingValues),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        valueChanged (value);
    }

    ~MultiChoicePropertyComponentWithEnablement() override    { value.removeListener (this); }

private:
    void valueChanged (Value&) override       { setEnabled (propertyWithDefault.get()); }

    ValueTreePropertyWithDefault propertyWithDefault;
    Value value;
};
