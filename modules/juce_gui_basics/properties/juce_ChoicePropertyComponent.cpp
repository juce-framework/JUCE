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

namespace juce
{

//==============================================================================
class ChoiceRemapperValueSource final : public Value::ValueSource,
                                        private Value::Listener
{
public:
    ChoiceRemapperValueSource (const Value& source, const Array<var>& map)
       : sourceValue (source),
         mappings (map)
    {
        sourceValue.addListener (this);
    }

    var getValue() const override
    {
        auto targetValue = sourceValue.getValue();

        for (auto& map : mappings)
            if (map.equalsWithSameType (targetValue))
                return mappings.indexOf (map) + 1;

        return mappings.indexOf (targetValue) + 1;
    }

    void setValue (const var& newValue) override
    {
        auto remappedVal = mappings [static_cast<int> (newValue) - 1];

        if (! remappedVal.equalsWithSameType (sourceValue))
            sourceValue = remappedVal;
    }

protected:
    Value sourceValue;
    Array<var> mappings;

    void valueChanged (Value&) override    { sendChangeMessage (true); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoiceRemapperValueSource)
};

//==============================================================================
class ChoiceRemapperValueSourceWithDefault final : public Value::ValueSource,
                                                   private Value::Listener
{
public:
    ChoiceRemapperValueSourceWithDefault (const ValueTreePropertyWithDefault& v, const Array<var>& map)
        : value (v),
          sourceValue (value.getPropertyAsValue()),
          mappings (map)
    {
        sourceValue.addListener (this);
    }

    var getValue() const override
    {
        if (! value.isUsingDefault())
        {
            const auto target = sourceValue.getValue();
            const auto equalsWithSameType = [&target] (const var& map) { return map.equalsWithSameType (target); };

            auto iter = std::find_if (mappings.begin(), mappings.end(), equalsWithSameType);

            if (iter == mappings.end())
                iter = std::find (mappings.begin(), mappings.end(), target);

            if (iter != mappings.end())
                return 1 + (int) std::distance (mappings.begin(), iter);
        }

        return -1;
    }

    void setValue (const var& newValue) override
    {
        auto newValueInt = static_cast<int> (newValue);

        if (newValueInt == -1)
        {
            value.resetToDefault();
        }
        else
        {
            auto remappedVal = mappings [newValueInt - 1];

            if (! remappedVal.equalsWithSameType (sourceValue))
                value = remappedVal;
        }
    }

private:
    void valueChanged (Value&) override { sendChangeMessage (true); }

    ValueTreePropertyWithDefault value;
    Value sourceValue;
    Array<var> mappings;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoiceRemapperValueSourceWithDefault)
};

//==============================================================================
ChoicePropertyComponent::ChoicePropertyComponent (const String& name)
    : PropertyComponent (name),
      isCustomClass (true)
{
}

ChoicePropertyComponent::ChoicePropertyComponent (const String& name,
                                                  const StringArray& choiceList,
                                                  [[maybe_unused]] const Array<var>& correspondingValues)
    : PropertyComponent (name),
      choices (choiceList)
{
    // The array of corresponding values must contain one value for each of the items in
    // the choices array!
    jassert (correspondingValues.size() == choices.size());
}

ChoicePropertyComponent::ChoicePropertyComponent (const Value& valueToControl,
                                                  const String& name,
                                                  const StringArray& choiceList,
                                                  const Array<var>& correspondingValues)
    : ChoicePropertyComponent (name, choiceList, correspondingValues)
{
    refreshChoices();
    initialiseComboBox (Value (new ChoiceRemapperValueSource (valueToControl, correspondingValues)));
}

ChoicePropertyComponent::ChoicePropertyComponent (const ValueTreePropertyWithDefault& valueToControl,
                                                  const String& name,
                                                  const StringArray& choiceList,
                                                  const Array<var>& correspondingValues)
    : ChoicePropertyComponent (name, choiceList, correspondingValues)
{
    value = valueToControl;

    auto getDefaultString = [this, correspondingValues] { return choices [correspondingValues.indexOf (value.getDefault())]; };

    refreshChoices (getDefaultString());
    initialiseComboBox (Value (new ChoiceRemapperValueSourceWithDefault (value, correspondingValues)));

    value.onDefaultChange = [this, getDefaultString]
    {
        auto selectedId = comboBox.getSelectedId();
        refreshChoices (getDefaultString());
        comboBox.setSelectedId (selectedId);
    };
}

ChoicePropertyComponent::ChoicePropertyComponent (const ValueTreePropertyWithDefault& valueToControl,
                                                  const String& name)
    : PropertyComponent (name),
      choices ({ "Enabled", "Disabled" })
{
    value = valueToControl;

    auto getDefaultString = [this] { return value.getDefault() ? "Enabled" : "Disabled"; };

    refreshChoices (getDefaultString());
    initialiseComboBox (Value (new ChoiceRemapperValueSourceWithDefault (value, { true, false })));

    value.onDefaultChange = [this, getDefaultString]
    {
        auto selectedId = comboBox.getSelectedId();
        refreshChoices (getDefaultString());
        comboBox.setSelectedId (selectedId);
    };
}

//==============================================================================
void ChoicePropertyComponent::initialiseComboBox (const Value& v)
{
    if (v != Value())
        comboBox.setSelectedId (v.getValue(), dontSendNotification);

    comboBox.getSelectedIdAsValue().referTo (v);
    comboBox.setEditableText (false);
    addAndMakeVisible (comboBox);
}

void ChoicePropertyComponent::refreshChoices()
{
    comboBox.clear();

    for (int i = 0; i < choices.size(); ++i)
    {
        const auto& choice = choices[i];

        if (choice.isNotEmpty())
            comboBox.addItem (choice, i + 1);
        else
            comboBox.addSeparator();
    }
}

void ChoicePropertyComponent::refreshChoices (const String& defaultString)
{
    refreshChoices();
    comboBox.addItem ("Default" + (defaultString.isNotEmpty() ? " (" + defaultString + ")" : ""), -1);
}

//==============================================================================
void ChoicePropertyComponent::setIndex (const int /*newIndex*/)
{
    jassertfalse; // you need to override this method in your subclass!
}

int ChoicePropertyComponent::getIndex() const
{
    jassertfalse; // you need to override this method in your subclass!
    return -1;
}

const StringArray& ChoicePropertyComponent::getChoices() const
{
    return choices;
}

//==============================================================================
void ChoicePropertyComponent::refresh()
{
    if (isCustomClass)
    {
        if (! comboBox.isVisible())
        {
            refreshChoices();
            initialiseComboBox ({});
            comboBox.onChange = [this] { changeIndex(); };
        }

        comboBox.setSelectedId (getIndex() + 1, dontSendNotification);
    }
}

void ChoicePropertyComponent::changeIndex()
{
    if (isCustomClass)
    {
        auto newIndex = comboBox.getSelectedId() - 1;

        if (newIndex != getIndex())
            setIndex (newIndex);
    }
}

} // namespace juce
