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
class StringComparator
{
public:
    static int compareElements (var first, var second)
    {
        if (first.toString() > second.toString())
            return 1;
        else if (first.toString() < second.toString())
            return -1;

        return 0;
    }
};

static void updateButtonTickColour (ToggleButton* button, bool usingDefault)
{
    button->setColour (ToggleButton::tickColourId, button->getLookAndFeel().findColour (ToggleButton::tickColourId)
                                                                              .withAlpha (usingDefault ? 0.4f : 1.0f));
}

//==============================================================================
class MultiChoicePropertyComponent::MultiChoiceRemapperSource    : public Value::ValueSource,
                                                                   private Value::Listener
{
public:
    MultiChoiceRemapperSource (const Value& source, var v, int c)
        : sourceValue (source),
          varToControl (v),
          maxChoices (c)
    {
        sourceValue.addListener (this);
    }

    var getValue() const override
    {
        if (auto* arr = sourceValue.getValue().getArray())
            if (arr->contains (varToControl))
                return true;

        return false;
    }

    void setValue (const var& newValue) override
    {
        if (auto* arr = sourceValue.getValue().getArray())
        {
            auto temp = *arr;

            if (static_cast<bool> (newValue))
            {
                if (temp.addIfNotAlreadyThere (varToControl) && (maxChoices != -1) && (temp.size() > maxChoices))
                     temp.remove (temp.size() - 2);
            }
            else
            {
                temp.remove (arr->indexOf (varToControl));
            }

            StringComparator c;
            temp.sort (c);

            sourceValue = temp;
        }
    }

private:
    Value sourceValue;
    var varToControl;

    int maxChoices;

    //==============================================================================
    void valueChanged (Value&) override    { sendChangeMessage (true); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChoiceRemapperSource)
};

//==============================================================================
class MultiChoicePropertyComponent::MultiChoiceRemapperSourceWithDefault    : public Value::ValueSource,
                                                                              private Value::Listener
{
public:
    MultiChoiceRemapperSourceWithDefault (const ValueTreePropertyWithDefault& val,
                                          var v, int c, ToggleButton* b)
        : value (val),
          varToControl (v),
          sourceValue (value.getPropertyAsValue()),
          maxChoices (c),
          buttonToControl (b)
    {
        sourceValue.addListener (this);
    }

    var getValue() const override
    {
        auto v = value.get();

        if (auto* arr = v.getArray())
        {
            if (arr->contains (varToControl))
            {
                updateButtonTickColour (buttonToControl, value.isUsingDefault());
                return true;
            }
        }

        return false;
    }

    void setValue (const var& newValue) override
    {
        auto v = value.get();

        OptionalScopedPointer<Array<var>> arrayToControl;

        if (value.isUsingDefault())
            arrayToControl.set (new Array<var>(), true); // use an empty array so the default values are overwritten
        else
            arrayToControl.set (v.getArray(), false);

        if (arrayToControl != nullptr)
        {
            auto temp = *arrayToControl;

            bool newState = newValue;

            if (value.isUsingDefault())
            {
                if (auto* defaultArray = v.getArray())
                {
                    if (defaultArray->contains (varToControl))
                        newState = true; // force the state as the user is setting it explicitly
                }
            }

            if (newState)
            {
                if (temp.addIfNotAlreadyThere (varToControl) && (maxChoices != -1) && (temp.size() > maxChoices))
                    temp.remove (temp.size() - 2);
            }
            else
            {
                temp.remove (temp.indexOf (varToControl));
            }

            StringComparator c;
            temp.sort (c);

            value = temp;

            if (temp.size() == 0)
                value.resetToDefault();
        }
    }

private:
    //==============================================================================
    void valueChanged (Value&) override { sendChangeMessage (true); }

    //==============================================================================
    ValueTreePropertyWithDefault value;
    var varToControl;
    Value sourceValue;

    int maxChoices;

    ToggleButton* buttonToControl;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChoiceRemapperSourceWithDefault)
};

//==============================================================================
int MultiChoicePropertyComponent::getTotalButtonsHeight (int numButtons)
{
    return numButtons * buttonHeight + 1;
}

MultiChoicePropertyComponent::MultiChoicePropertyComponent (const String& propertyName,
                                                            const StringArray& choices,
                                                            const Array<var>& correspondingValues)
    : PropertyComponent (propertyName, jmin (getTotalButtonsHeight (choices.size()), collapsedHeight))
{
    // The array of corresponding values must contain one value for each of the items in
    // the choices array!
    jassertquiet (choices.size() == correspondingValues.size());

    for (auto choice : choices)
        addAndMakeVisible (choiceButtons.add (new ToggleButton (choice)));

    if (preferredHeight >= collapsedHeight)
    {
        expandable = true;
        maxHeight = getTotalButtonsHeight (choiceButtons.size()) + expandAreaHeight;
    }

    if (isExpandable())
    {
        {
            Path expandShape;
            expandShape.addTriangle ({ 0, 0 }, { 5, 10 }, { 10, 0});
            expandButton.setShape (expandShape, true, true, false);
        }

        expandButton.onClick = [this] { setExpanded (! expanded); };
        addAndMakeVisible (expandButton);

        lookAndFeelChanged();
    }
}

MultiChoicePropertyComponent::MultiChoicePropertyComponent (const Value& valueToControl,
                                                            const String& propertyName,
                                                            const StringArray& choices,
                                                            const Array<var>& correspondingValues,
                                                            int maxChoices)
    : MultiChoicePropertyComponent (propertyName, choices, correspondingValues)
{
    // The value to control must be an array!
    jassert (valueToControl.getValue().isArray());

    for (int i = 0; i < choiceButtons.size(); ++i)
        choiceButtons[i]->getToggleStateValue().referTo (Value (new MultiChoiceRemapperSource (valueToControl,
                                                                                               correspondingValues[i],
                                                                                               maxChoices)));
}

MultiChoicePropertyComponent::MultiChoicePropertyComponent (const ValueTreePropertyWithDefault& valueToControl,
                                                            const String& propertyName,
                                                            const StringArray& choices,
                                                            const Array<var>& correspondingValues,
                                                            int maxChoices)
    : MultiChoicePropertyComponent (propertyName, choices, correspondingValues)
{
    value = valueToControl;

    // The value to control must be an array!
    jassert (value.get().isArray());

    for (int i = 0; i < choiceButtons.size(); ++i)
        choiceButtons[i]->getToggleStateValue().referTo (Value (new MultiChoiceRemapperSourceWithDefault (value,
                                                                                                          correspondingValues[i],
                                                                                                          maxChoices,
                                                                                                          choiceButtons[i])));

    value.onDefaultChange = [this] { repaint(); };
}

void MultiChoicePropertyComponent::paint (Graphics& g)
{
    g.setColour (findColour (TextEditor::backgroundColourId));
    g.fillRect (getLookAndFeel().getPropertyComponentContentPosition (*this));

    if (isExpandable() && ! isExpanded())
    {
        g.setColour (findColour (TextEditor::backgroundColourId).contrasting().withAlpha (0.4f));
        g.drawFittedText ("+ " + String (numHidden) + " more", getLookAndFeel().getPropertyComponentContentPosition (*this)
                                                                               .removeFromBottom (expandAreaHeight).withTrimmedLeft (10),
                          Justification::centredLeft, 1);
    }

    PropertyComponent::paint (g);
}

void MultiChoicePropertyComponent::resized()
{
    auto bounds = getLookAndFeel().getPropertyComponentContentPosition (*this);

    if (isExpandable())
    {
        bounds.removeFromBottom (5);

        auto buttonSlice = bounds.removeFromBottom (10);
        expandButton.setSize (10, 10);
        expandButton.setCentrePosition (buttonSlice.getCentre());
    }

    numHidden = 0;

    for (auto* b : choiceButtons)
    {
        if (bounds.getHeight() >= buttonHeight)
        {
            b->setVisible (true);
            b->setBounds (bounds.removeFromTop (buttonHeight).reduced (5, 2));
        }
        else
        {
            b->setVisible (false);
            ++numHidden;
        }
    }
}

void MultiChoicePropertyComponent::setExpanded (bool shouldBeExpanded) noexcept
{
    if (! isExpandable() || (isExpanded() == shouldBeExpanded))
        return;

    expanded = shouldBeExpanded;
    preferredHeight = expanded ? maxHeight : collapsedHeight;

    if (auto* propertyPanel = findParentComponentOfClass<PropertyPanel>())
        propertyPanel->resized();

    if (onHeightChange != nullptr)
        onHeightChange();

    expandButton.setTransform (AffineTransform::rotation (expanded ? MathConstants<float>::pi : MathConstants<float>::twoPi,
                                                          (float) expandButton.getBounds().getCentreX(),
                                                          (float) expandButton.getBounds().getCentreY()));

    resized();
}

//==============================================================================
void MultiChoicePropertyComponent::lookAndFeelChanged()
{
    auto iconColour = findColour (TextEditor::backgroundColourId).contrasting();
    expandButton.setColours (iconColour, iconColour.darker(), iconColour.darker());

    const auto usingDefault = value.isUsingDefault();

    for (auto* button : choiceButtons)
        updateButtonTickColour (button, usingDefault);
}

} // namespace juce
