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

namespace juce
{

//==============================================================================
class ChoicePropertyComponent::RemapperValueSource    : public Value::ValueSource,
                                                        private Value::Listener
{
public:
    RemapperValueSource (const Value& source, const Array<var>& map)
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemapperValueSource)
};

//==============================================================================
class ChoicePropertyComponent::RemapperValueSourceWithDefault    : public Value::ValueSource,
                                                                   private Value::Listener
{
public:
    RemapperValueSourceWithDefault (ValueWithDefault* vwd, const Array<var>& map)
        : valueWithDefault (vwd),
          sourceValue (valueWithDefault->getPropertyAsValue()),
          mappings (map)
    {
        sourceValue.addListener (this);
    }

    var getValue() const override
    {
        if (valueWithDefault == nullptr)
            return {};

        if (valueWithDefault->isUsingDefault())
            return -1;

        auto targetValue = sourceValue.getValue();

        for (auto map : mappings)
            if (map.equalsWithSameType (targetValue))
                return mappings.indexOf (map) + 1;

        return mappings.indexOf (targetValue) + 1;
    }

    void setValue (const var& newValue) override
    {
        if (valueWithDefault == nullptr)
            return;

        auto newValueInt = static_cast<int> (newValue);

        if (newValueInt == -1)
        {
            valueWithDefault->resetToDefault();
        }
        else
        {
            auto remappedVal = mappings [newValueInt - 1];

            if (! remappedVal.equalsWithSameType (sourceValue))
                *valueWithDefault = remappedVal;
        }
    }

private:
    void valueChanged (Value&) override { sendChangeMessage (true); }

    WeakReference<ValueWithDefault> valueWithDefault;
    Value sourceValue;
    Array<var> mappings;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemapperValueSourceWithDefault)
};

//==============================================================================
ChoicePropertyComponent::ChoicePropertyComponent (const String& name)
    : PropertyComponent (name),
      isCustomClass (true)
{
}

ChoicePropertyComponent::ChoicePropertyComponent (const String& name,
                                                  const StringArray& choiceList,
                                                  const Array<var>& correspondingValues)
    : PropertyComponent (name),
      choices (choiceList)
{
    // The array of corresponding values must contain one value for each of the items in
    // the choices array!
    jassert (correspondingValues.size() == choices.size());

    ignoreUnused (correspondingValues);
}

ChoicePropertyComponent::ChoicePropertyComponent (const Value& valueToControl,
                                                  const String& name,
                                                  const StringArray& choiceList,
                                                  const Array<var>& correspondingValues)
    : ChoicePropertyComponent (name, choiceList, correspondingValues)
{
    refreshChoices();
    initialiseComboBox (Value (new RemapperValueSource (valueToControl, correspondingValues)));
}

ChoicePropertyComponent::ChoicePropertyComponent (ValueWithDefault& valueToControl,
                                                  const String& name,
                                                  const StringArray& choiceList,
                                                  const Array<var>& correspondingValues)
    : ChoicePropertyComponent (name, choiceList, correspondingValues)
{
    valueWithDefault = &valueToControl;

    auto getDefaultString = [this, correspondingValues] { return choices [correspondingValues.indexOf (valueWithDefault->getDefault())]; };

    refreshChoices (getDefaultString());
    initialiseComboBox (Value (new RemapperValueSourceWithDefault (valueWithDefault, correspondingValues)));

    valueWithDefault->onDefaultChange = [this, getDefaultString]
    {
        auto selectedId = comboBox.getSelectedId();
        refreshChoices (getDefaultString());
        comboBox.setSelectedId (selectedId);
    };
}

ChoicePropertyComponent::ChoicePropertyComponent (ValueWithDefault& valueToControl,
                                                  const String& name)
    : PropertyComponent (name),
      choices ({ "Enabled", "Disabled" })
{
    valueWithDefault = &valueToControl;

    auto getDefaultString = [this] { return valueWithDefault->getDefault() ? "Enabled" : "Disabled"; };

    refreshChoices (getDefaultString());
    initialiseComboBox (Value (new RemapperValueSourceWithDefault (valueWithDefault, { true, false })));

    valueWithDefault->onDefaultChange = [this, getDefaultString]
    {
        auto selectedId = comboBox.getSelectedId();
        refreshChoices (getDefaultString());
        comboBox.setSelectedId (selectedId);
    };
}

ChoicePropertyComponent::~ChoicePropertyComponent()
{
    if (valueWithDefault != nullptr)
        valueWithDefault->onDefaultChange = nullptr;
}

//==============================================================================
void ChoicePropertyComponent::initialiseComboBox (const Value& v)
{
    if (v != Value())
    {
        comboBox.setSelectedId (v.getValue(), dontSendNotification);
        comboBox.getSelectedIdAsValue().referTo (v);
    }

    comboBox.setEditableText (false);
    addAndMakeVisible (comboBox);
}

void ChoicePropertyComponent::refreshChoices()
{
    comboBox.clear();

    for (auto choice : choices)
    {
        if (choice.isNotEmpty())
            comboBox.addItem (choice, choices.indexOf (choice) + 1);
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
