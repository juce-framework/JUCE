/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__

#include "juce_PropertyComponent.h"
#include "../controls/juce_ComboBox.h"


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
                                             private ComboBoxListener
{
protected:
    /** Creates the component.

        Your subclass's constructor must add a list of options to the choices
        member variable.
    */
    ChoicePropertyComponent (const String& propertyName);

public:
    /** Creates the component.

        Your subclass's constructor must add a list of options to the choices
        member variable.
    */
    ChoicePropertyComponent (const Value& valueToControl,
                             const String& propertyName,
                             const StringArray& choices);

    /** Destructor. */
    ~ChoicePropertyComponent();

    //==============================================================================
    /** Called when the user selects an item from the combo box.

        Your subclass must use this callback to update the value that this component
        represents. The index is the index of the chosen item in the choices
        StringArray.
    */
    virtual void setIndex (const int newIndex);

    /** Returns the index of the item that should currently be shown.

        This is the index of the item in the choices StringArray that will be
        shown.
    */
    virtual int getIndex() const;

    /** Returns the list of options. */
    const StringArray& getChoices() const;


    //==============================================================================
    /** @internal */
    void refresh();
    /** @internal */
    void comboBoxChanged (ComboBox*);

    juce_UseDebuggingNewOperator

protected:
    /** The list of options that will be shown in the combo box.

        Your subclass must populate this array in its constructor. If any empty
        strings are added, these will be replaced with horizontal separators (see
        ComboBox::addSeparator() for more info).
    */
    StringArray choices;

private:
    ComboBox* comboBox;

    void createComboBox();

    ChoicePropertyComponent (const ChoicePropertyComponent&);
    const ChoicePropertyComponent& operator= (const ChoicePropertyComponent&);
};




#endif   // __JUCE_CHOICEPROPERTYCOMPONENT_JUCEHEADER__
