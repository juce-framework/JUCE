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

#ifndef __JUCE_CARETCOMPONENT_JUCEHEADER__
#define __JUCE_CARETCOMPONENT_JUCEHEADER__

#include "../components/juce_Component.h"


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
    void paint (Graphics& g);

private:
    Component* owner;

    bool shouldBeShown() const;
    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE (CaretComponent)
};


#endif   // __JUCE_CARETCOMPONENT_JUCEHEADER__
