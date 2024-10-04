/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class TextPropertyComponentWithEnablement final : public TextPropertyComponent,
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
class ChoicePropertyComponentWithEnablement final : public ChoicePropertyComponent,
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
        handleValueChanged();
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

        handleValueChanged();
    }

    ChoicePropertyComponentWithEnablement (const ValueTreePropertyWithDefault& valueToControl,
                                           ValueTreePropertyWithDefault valueToListenTo,
                                           const String& propertyName)
        : ChoicePropertyComponent (valueToControl, propertyName),
          propertyWithDefault (valueToListenTo),
          value (valueToListenTo.getPropertyAsValue())
    {
        value.addListener (this);
        handleValueChanged();
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

    void handleValueChanged()
    {
        if (isMultiChoice)
            setEnabled (checkMultiChoiceVar());
        else
            setEnabled (propertyWithDefault.get());
    }

    void valueChanged (Value&) override
    {
        handleValueChanged();
    }
};

//==============================================================================
class MultiChoicePropertyComponentWithEnablement final : public MultiChoicePropertyComponent,
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
