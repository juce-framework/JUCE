/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
/**
    A PropertyComponent that contains an on/off toggle button.

    This type of property component can be used if you have a boolean value to
    toggle on/off.

    @see PropertyComponent

    @tags{GUI}
*/
class JUCE_API  BooleanPropertyComponent  : public PropertyComponent
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

        Note that if you call this constructor then you must use the Value to interact with the
        button state, and you can't override the class with your own setState or getState methods.
        If you want to use getState and setState, call the other constructor instead.

        @param valueToControl       a Value object that this property should refer to.
        @param propertyName         the property name to be passed to the PropertyComponent
        @param buttonText           the text shown in the ToggleButton component
    */
    BooleanPropertyComponent (const Value& valueToControl,
                              const String& propertyName,
                              const String& buttonText);

    /** Destructor. */
    ~BooleanPropertyComponent() override;

    //==============================================================================
    /** Called to change the state of the boolean value. */
    virtual void setState (bool newState);

    /** Must return the current value of the property. */
    virtual bool getState() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x100e801,    /**< The colour to fill the background of the button area. */
        outlineColourId             = 0x100e803,    /**< The colour to use to draw an outline around the text area. */
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void refresh() override;

private:
    ToggleButton button;
    String onText, offText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BooleanPropertyComponent)
};

} // namespace juce
