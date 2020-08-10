/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
    A PropertyComponent that contains a button.

    This type of property component can be used if you need a button to trigger some
    kind of action.

    @see PropertyComponent

    @tags{GUI}
*/
class JUCE_API  ButtonPropertyComponent  : public PropertyComponent
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
    ~ButtonPropertyComponent() override;

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
    void refresh() override;

private:
    TextButton button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonPropertyComponent)
};

} // namespace juce
