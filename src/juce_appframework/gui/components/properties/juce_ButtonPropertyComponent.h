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

#ifndef __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__

#include "juce_PropertyComponent.h"
#include "../buttons/juce_TextButton.h"


//==============================================================================
/**
    A PropertyComponent that contains a button.

    This type of property component can be used if you need a button to trigger some
    kind of action.

    @see PropertyComponent
*/
class JUCE_API  ButtonPropertyComponent  : public PropertyComponent,
                                           private ButtonListener
{
public:
    //==============================================================================
    /** Creates a button component.

        @param propertyName         the property name to be passed to the PropertyComponent
        @param triggerOnMouseDown   this is passed to the Button::setTriggeredOnMouseDown() method
    */
    ButtonPropertyComponent (const String& propertyName,
                             const bool triggerOnMouseDown);

    /** Destructor. */
    ~ButtonPropertyComponent();

    //==============================================================================
    /** Called when the user clicks the button.
    */
    virtual void buttonClicked() = 0;

    /** Returns the string that should be displayed in the button.

        If you need to change this string, call refresh() to update the component.
    */
    virtual const String getButtonText() const = 0;

    //==============================================================================
    /** @internal */
    void refresh();
    /** @internal */
    void buttonClicked (Button*);

    juce_UseDebuggingNewOperator

private:
    TextButton* button;
};


#endif   // __JUCE_BUTTONPROPERTYCOMPONENT_JUCEHEADER__
