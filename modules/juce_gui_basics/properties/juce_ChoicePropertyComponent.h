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
    A PropertyComponent that shows its value as a combo box.

    This type of property component contains a list of options and has a
    combo box to choose one.

    Your subclass's constructor must add some strings to the choices StringArray
    and these are shown in the list.

    The getIndex() method will be called to find out which option is the currently
    selected one. If you call refresh() it will call getIndex() to check whether
    the value has changed, and will update the combo box if needed.

    If the user selects a different item from the list, setIndex() will be
    called to let your class process this.

    @see PropertyComponent, PropertyPanel

    @tags{GUI}
*/
class JUCE_API  ChoicePropertyComponent    : public PropertyComponent
{
private:
    /** Delegating constructor. */
    ChoicePropertyComponent (const String&, const StringArray&, const Array<var>&);

protected:
    /** Creates the component.
        Your subclass's constructor must add a list of options to the choices member variable.
    */
    ChoicePropertyComponent (const String& propertyName);

public:
    /** Creates the component.

        Note that if you call this constructor then you must use the Value to interact with the
        index, and you can't override the class with your own setIndex or getIndex methods.
        If you want to use those methods, call the other constructor instead.

        @param valueToControl       the value that the combo box will read and control
        @param propertyName         the name of the property
        @param choices              the list of possible values that the drop-down list will contain
        @param correspondingValues  a list of values corresponding to each item in the 'choices' StringArray.
                                    These are the values that will be read and written to the
                                    valueToControl value. This array must contain the same number of items
                                    as the choices array
    */
    ChoicePropertyComponent (const Value& valueToControl,
                             const String& propertyName,
                             const StringArray& choices,
                             const Array<var>& correspondingValues);

    /** Creates the component using a ValueWithDefault object. This will add an item to the ComboBox for the
        default value with an ID of -1.

        @param valueToControl       the ValueWithDefault object that contains the Value object that the combo box will read and control.
                                    NB: this object must outlive the ChoicePropertyComponent.
        @param propertyName         the name of the property
        @param choices              the list of possible values that the drop-down list will contain
        @param correspondingValues  a list of values corresponding to each item in the 'choices' StringArray.
                                    These are the values that will be read and written to the
                                    valueToControl value. This array must contain the same number of items
                                    as the choices array

    */
    ChoicePropertyComponent (ValueWithDefault& valueToControl,
                             const String& propertyName,
                             const StringArray& choices,
                             const Array<var>& correspondingValues);

    /** Creates the component using a ValueWithDefault object, adding an item to the ComboBox for the
        default value with an ID of -1 as well as adding separate "Enabled" and "Disabled" options.

        This is useful for simple on/off choices that also need a default value.
    */
    ChoicePropertyComponent (ValueWithDefault& valueToControl,
                             const String& propertyName);

    /** Destructor. */
    ~ChoicePropertyComponent();

    //==============================================================================
    /** Called when the user selects an item from the combo box.

        Your subclass must use this callback to update the value that this component
        represents. The index is the index of the chosen item in the choices
        StringArray.
    */
    virtual void setIndex (int newIndex);

    /** Returns the index of the item that should currently be shown.
        This is the index of the item in the choices StringArray that will be shown.
    */
    virtual int getIndex() const;

    /** Returns the list of options. */
    const StringArray& getChoices() const;

    //==============================================================================
    /** @internal */
    void refresh() override;

protected:
    /** The list of options that will be shown in the combo box.

        Your subclass must populate this array in its constructor. If any empty
        strings are added, these will be replaced with horizontal separators (see
        ComboBox::addSeparator() for more info).
    */
    StringArray choices;

private:
    //==============================================================================
    class RemapperValueSource;
    class RemapperValueSourceWithDefault;

    //==============================================================================
    void createComboBox();
    void createComboBoxWithDefault (const String&);

    void changeIndex();

    //==============================================================================
    ComboBox comboBox;
    bool isCustomClass = false;

    ValueWithDefault* valueWithDefault = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoicePropertyComponent)
};

} // namespace juce
