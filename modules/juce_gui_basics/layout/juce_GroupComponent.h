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
    A component that draws an outline around itself and has an optional title at
    the top, for drawing an outline around a group of controls.


    @tags{GUI}
*/
class JUCE_API  GroupComponent    : public Component
{
public:
    //==============================================================================
    /** Creates a GroupComponent.

        @param componentName    the name to give the component
        @param labelText        the text to show at the top of the outline
    */
    GroupComponent (const String& componentName = {},
                    const String& labelText = {});

    /** Destructor. */
    ~GroupComponent() override;

    //==============================================================================
    /** Changes the text that's shown at the top of the component. */
    void setText (const String& newText);

    /** Returns the currently displayed text label. */
    String getText() const;

    /** Sets the positioning of the text label.
        (The default is Justification::left)
        @see getTextLabelPosition
    */
    void setTextLabelPosition (Justification justification);

    /** Returns the current text label position.
        @see setTextLabelPosition
    */
    Justification getTextLabelPosition() const noexcept           { return justification; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        outlineColourId     = 0x1005400,    /**< The colour to use for drawing the line around the edge. */
        textColourId        = 0x1005410     /**< The colour to use to draw the text label. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void drawGroupComponentOutline (Graphics&, int w, int h, const String& text,
                                                const Justification&, GroupComponent&) = 0;
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void enablementChanged() override;
    /** @internal */
    void colourChanged() override;

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    String text;
    Justification justification;

    JUCE_DECLARE_NON_COPYABLE (GroupComponent)
};

} // namespace juce
