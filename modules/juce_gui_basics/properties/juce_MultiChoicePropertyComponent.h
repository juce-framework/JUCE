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


namespace juce
{

//==============================================================================
/**
    A PropertyComponent that shows its value as an expandable list of ToggleButtons.

    This type of property component contains a list of options where multiple options
    can be selected at once.

    @see PropertyComponent, PropertyPanel

    @tags{GUI}
*/
class MultiChoicePropertyComponent    : public PropertyComponent
{
private:
    /** Delegating constructor. */
    MultiChoicePropertyComponent (const String&, const StringArray&, const Array<var>&);

public:
    /** Creates the component. Note that the underlying var object that the Value refers to must be an array.

        @param valueToControl       the value that the ToggleButtons will read and control
        @param propertyName         the name of the property
        @param choices              the list of possible values that will be represented
        @param correspondingValues  a list of values corresponding to each item in the 'choices' StringArray.
                                    These are the values that will be read and written to the
                                    valueToControl value. This array must contain the same number of items
                                    as the choices array
        @param maxChoices           the maxmimum number of values which can be selected at once. The default of
                                    -1 will not limit the number that can be selected
    */
    MultiChoicePropertyComponent (const Value& valueToControl,
                                  const String& propertyName,
                                  const StringArray& choices,
                                  const Array<var>& correspondingValues,
                                  int maxChoices = -1);

    /** Creates the component using a ValueWithDefault object. This will select the default options.

        @param valueToControl       the ValueWithDefault object that contains the Value object that the ToggleButtons will read and control.
        @param propertyName         the name of the property
        @param choices              the list of possible values that will be represented
        @param correspondingValues  a list of values corresponding to each item in the 'choices' StringArray.
                                    These are the values that will be read and written to the
                                    valueToControl value. This array must contain the same number of items
                                    as the choices array
        @param maxChoices           the maxmimum number of values which can be selected at once. The default of
                                    -1 will not limit the number that can be selected
    */
    MultiChoicePropertyComponent (ValueWithDefault& valueToControl,
                                  const String& propertyName,
                                  const StringArray& choices,
                                  const Array<var>& correspondingValues,
                                  int maxChoices = -1);

    ~MultiChoicePropertyComponent() override;

    //==============================================================================
    /** Returns true if the list of options is expanded. */
    bool isExpanded() const noexcept    { return expanded; }

    /** Expands or shrinks the list of options.

        N.B. This will just set the preferredHeight value of the PropertyComponent and attempt to
        call PropertyPanel::resized(), so if you are not displaying this object in a PropertyPanel
        then you should use the onHeightChange callback to resize it when the height changes.

        @see onHeightChange
    */
    void setExpanded (bool expanded) noexcept;

    /** You can assign a lambda to this callback object to have it called when the MultiChoicePropertyComponent height changes. */
    std::function<void()> onHeightChange;

    //==============================================================================
    /** @internal */
    void paint (Graphics& g) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void refresh() override {}

private:
    //==============================================================================
    class MultiChoiceRemapperSource;
    class MultiChoiceRemapperSourceWithDefault;

    //==============================================================================
    void lookAndFeelChanged() override;

    //==============================================================================
    WeakReference<ValueWithDefault> valueWithDefault;

    int maxHeight = 0;
    int numHidden = 0;
    bool expanded = false;

    OwnedArray<ToggleButton> choiceButtons;
    ShapeButton expandButton { "Expand", Colours::transparentBlack, Colours::transparentBlack, Colours::transparentBlack };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChoicePropertyComponent)
};

} // namespace juce
