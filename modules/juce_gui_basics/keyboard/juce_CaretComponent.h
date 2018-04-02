/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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

    @tags{GUI}
*/
class JUCE_API  CaretComponent   : public Component,
                                   private Timer
{
public:
    //==============================================================================
    /** Creates the caret component.
        The keyFocusOwner is an optional component which the caret will check, making
        itself visible only when the keyFocusOwner has keyboard focus.
    */
    CaretComponent (Component* keyFocusOwner);

    /** Destructor. */
    ~CaretComponent();

    //==============================================================================
    /** Sets the caret's position to place it next to the given character.
        The area is the rectangle containing the entire character that the caret is
        positioned on, so by default a vertical-line caret may choose to just show itself
        at the left of this area. You can override this method to customize its size.
        This method will also force the caret to reset its timer and become visible (if
        appropriate), so that as it moves, you can see where it is.
    */
    virtual void setCaretPosition (const Rectangle<int>& characterArea);

    /** A set of color IDs to use to change the color of various aspects of the caret.
        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.
        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        caretColorId    = 0x1000204, /**< The color with which to draw the caret. */
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;

private:
    Component* owner;

    bool shouldBeShown() const;
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE (CaretComponent)
};

} // namespace juce
