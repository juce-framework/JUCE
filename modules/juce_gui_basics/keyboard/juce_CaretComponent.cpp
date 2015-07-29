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

CaretComponent::CaretComponent (Component* const keyFocusOwner)
    : owner (keyFocusOwner)
{
    setPaintingIsUnclipped (true);
    setInterceptsMouseClicks (false, false);
}

CaretComponent::~CaretComponent()
{
}

void CaretComponent::paint (Graphics& g)
{
    g.setColour (findColour (caretColourId, true));
    g.fillRect (getLocalBounds());
}

void CaretComponent::timerCallback()
{
    setVisible (shouldBeShown() && ! isVisible());
}

void CaretComponent::setCaretPosition (const Rectangle<int>& characterArea)
{
    startTimer (380);
    setVisible (shouldBeShown());
    setBounds (characterArea.withWidth (2));
}

bool CaretComponent::shouldBeShown() const
{
    return owner == nullptr || (owner->hasKeyboardFocus (false)
                                 && ! owner->isCurrentlyBlockedByAnotherModalComponent());
}
