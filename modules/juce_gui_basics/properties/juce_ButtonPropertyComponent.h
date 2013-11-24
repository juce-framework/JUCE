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

#ifndef JUCE_BUTTONPROPERTYCOMPONENT_H_INCLUDED
#define JUCE_BUTTONPROPERTYCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A PropertyComponent that contains a button.

    This type of property component can be used if you need a button to trigger some
    kind of action.

    @see PropertyComponent
*/
class JUCE_API  ButtonPropertyComponent  : public PropertyComponent,
                                           private ButtonListener // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    //==============================================================================
    /** Creates a button component.

        @param propertyName         the property name to be passed to the PropertyComponent
        @param triggerOnMouseDown   this is passed to the Button::setTriggeredOnMouseDown() method
    */
    ButtonPropertyComponent (const String& propertyName,
                             bool triggerOnMouseDown);

    /** Destructor. */
    ~ButtonPropertyComponent();

    //==============================================================================
    /** Called when the user clicks the button.
    */
    virtual void buttonClicked() = 0;

    /** Returns the string that should be displayed in the button.

        If you need to change this string, call refresh() to update the component.
    */
    virtual String getButtonText() const = 0;

    //==============================================================================
    /** @internal */
    void refresh();
    /** @internal */
    void buttonClicked (Button*);

private:
    TextButton button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonPropertyComponent)
};


#endif   // JUCE_BUTTONPROPERTYCOMPONENT_H_INCLUDED
