/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
                                            private ButtonListener
{
public:
    //==============================================================================
    /** Creates a button component.

        @param propertyName         the property name to be passed to the PropertyComponent
        @param buttonTextWhenTrue   the text shown in the button when the value is true
        @param buttonTextWhenFalse  the text shown in the button when the value is false
    */
    BooleanPropertyComponent (const String& propertyName,
                              const String& buttonTextWhenTrue,
                              const String& buttonTextWhenFalse);

    /** Destructor. */
    ~BooleanPropertyComponent();

    //==============================================================================
    /** Called to change the state of the boolean value. */
    virtual void setState (const bool newState) = 0;

    /** Must return the current value of the property. */
    virtual bool getState() const = 0;

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void refresh();
    /** @internal */
    void buttonClicked (Button*);

    juce_UseDebuggingNewOperator

private:
    ToggleButton* button;
    String onText, offText;
};


#endif   // __JUCE_BOOLEANPROPERTYCOMPONENT_JUCEHEADER__
