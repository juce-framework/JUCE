/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_CARETCOMPONENT_H_INCLUDED
#define JUCE_CARETCOMPONENT_H_INCLUDED


//==============================================================================
/**
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
        at the left of this area. You can override this method to customise its size.
        This method will also force the caret to reset its timer and become visible (if
        appropriate), so that as it moves, you can see where it is.
    */
    virtual void setCaretPosition (const Rectangle<int>& characterArea);

    /** A set of colour IDs to use to change the colour of various aspects of the caret.
        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.
        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        caretColourId    = 0x1000204, /**< The colour with which to draw the caret. */
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


#endif   // JUCE_CARETCOMPONENT_H_INCLUDED
