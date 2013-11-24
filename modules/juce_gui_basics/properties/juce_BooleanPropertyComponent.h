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

#ifndef JUCE_BOOLEANPROPERTYCOMPONENT_H_INCLUDED
#define JUCE_BOOLEANPROPERTYCOMPONENT_H_INCLUDED


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
    void paint (Graphics&) override;
    /** @internal */
    void refresh() override;
    /** @internal */
    void buttonClicked (Button*) override;

private:
    ToggleButton button;
    String onText, offText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BooleanPropertyComponent)
};


#endif   // JUCE_BOOLEANPROPERTYCOMPONENT_H_INCLUDED
