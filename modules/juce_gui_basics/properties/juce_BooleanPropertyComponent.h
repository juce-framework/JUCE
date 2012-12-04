/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__

#include "juce_PropertyComponent.h"
#include "../buttons/juce_ToggleButton.h"


//==============================================================================
/**
    A PropertyComponent that contains an on/off toggle button.

    This type of property component can be used if you have a boolean value to
    toggle on/off.

    @see PropertyComponent
*/
class JUCE_API  BooleanPropertyComponent  : public PropertyComponent,
                                            private ButtonListener // (can't use Button::Listener due to idiotic VC2005 bug)
{
protected:
    //==============================================================================
    /** Creates a button component.

        If you use this constructor, you must override the getState() and setState()
        methods.

        @param propertyName         the property name to be passed to the PropertyComponent
        @param buttonTextWhenTrue   the text shown in the button when the value is true
        @param buttonTextWhenFalse  the text shown in the button when the value is false
    */
    BooleanPropertyComponent (const String& propertyName,
                              const String& buttonTextWhenTrue,
                              const String& buttonTextWhenFalse);

public:
    /** Creates a button component.

        @param valueToControl       a Value object that this property should refer to.
        @param propertyName         the property name to be passed to the PropertyComponent
        @param buttonText           the text shown in the ToggleButton component
    */
    BooleanPropertyComponent (const Value& valueToControl,
                              const String& propertyName,
                              const String& buttonText);

    /** Destructor. */
    ~BooleanPropertyComponent();

    //==============================================================================
    /** Called to change the state of the boolean value. */
    virtual void setState (bool newState);

    /** Must return the current value of the property. */
    virtual bool getState() const;

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void refresh();
    /** @internal */
    void buttonClicked (Button*);

private:
    ToggleButton button;
    String onText, offText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BooleanPropertyComponent)
};


#endif   // __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__
