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

#ifndef JUCE_CHOICEPROPERTYCOMPONENT_H_INCLUDED
#define JUCE_CHOICEPROPERTYCOMPONENT_H_INCLUDED


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
*/
class JUCE_API  ChoicePropertyComponent    : public PropertyComponent,
                                             private ComboBoxListener  // (can't use ComboBox::Listener due to idiotic VC2005 bug)
{
protected:
    /** Creates the component.

        Your subclass's constructor must add a list of options to the choices
        member variable.
    */
    ChoicePropertyComponent (const String& propertyName);

public:
    /** Creates the component.

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
    void refresh();

protected:
    /** The list of options that will be shown in the combo box.

        Your subclass must populate this array in its constructor. If any empty
        strings are added, these will be replaced with horizontal separators (see
        ComboBox::addSeparator() for more info).
    */
    StringArray choices;

private:
    ComboBox comboBox;
    bool isCustomClass;

    class RemapperValueSource;
    void createComboBox();
    void comboBoxChanged (ComboBox*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoicePropertyComponent)
};




#endif   // JUCE_CHOICEPROPERTYCOMPONENT_H_INCLUDED
