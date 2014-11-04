/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

class ChoicePropertyComponent::RemapperValueSource    : public Value::ValueSource,
                                                        private ValueListener
{
public:
    RemapperValueSource (const Value& source, const Array<var>& map)
       : sourceValue (source), mappings (map)
    {
        sourceValue.addListener (this);
    }

    var getValue() const
    {
        const var targetValue (sourceValue.getValue());

        for (int i = 0; i < mappings.size(); ++i)
            if (mappings.getReference(i).equalsWithSameType (targetValue))
                return i + 1;

        return mappings.indexOf (targetValue) + 1;
    }

    void setValue (const var& newValue)
    {
        const var remappedVal (mappings [static_cast <int> (newValue) - 1]);

        if (! remappedVal.equalsWithSameType (sourceValue))
            sourceValue = remappedVal;
    }

protected:
    Value sourceValue;
    Array<var> mappings;

    void valueChanged (Value&)
    {
        sendChangeMessage (true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemapperValueSource)
};


//==============================================================================
ChoicePropertyComponent::ChoicePropertyComponent (const String& name)
    : PropertyComponent (name),
      isCustomClass (true)
{
}

ChoicePropertyComponent::ChoicePropertyComponent (const Value& valueToControl,
                                                  const String& name,
                                                  const StringArray& choiceList,
                                                  const Array<var>& correspondingValues)
    : PropertyComponent (name),
      choices (choiceList),
      isCustomClass (false)
{
    // The array of corresponding values must contain one value for each of the items in
    // the choices array!
    jassert (correspondingValues.size() == choices.size());

    createComboBox();

    comboBox.getSelectedIdAsValue().referTo (Value (new RemapperValueSource (valueToControl,
                                                                             correspondingValues)));
}

ChoicePropertyComponent::~ChoicePropertyComponent()
{
}

//==============================================================================
void ChoicePropertyComponent::createComboBox()
{
    addAndMakeVisible (comboBox);

    for (int i = 0; i < choices.size(); ++i)
    {
        if (choices[i].isNotEmpty())
            comboBox.addItem (choices[i], i + 1);
        else
            comboBox.addSeparator();
    }

    comboBox.setEditableText (false);
}

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
            createComboBox();
            comboBox.addListener (this);
        }

        comboBox.setSelectedId (getIndex() + 1, dontSendNotification);
    }
}

void ChoicePropertyComponent::comboBoxChanged (ComboBox*)
{
    if (isCustomClass)
    {
        const int newIndex = comboBox.getSelectedId() - 1;

        if (newIndex != getIndex())
            setIndex (newIndex);
    }
}
