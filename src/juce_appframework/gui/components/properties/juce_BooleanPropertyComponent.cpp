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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_BooleanPropertyComponent.h"
#include "../controls/juce_ComboBox.h"


//==============================================================================
BooleanPropertyComponent::BooleanPropertyComponent (const String& name,
                                                    const String& buttonTextWhenTrue,
                                                    const String& buttonTextWhenFalse)
    : PropertyComponent (name),
      onText (buttonTextWhenTrue),
      offText (buttonTextWhenFalse)
{
    addAndMakeVisible (button = new ToggleButton (String::empty));
    button->setClickingTogglesState (false);
    button->addButtonListener (this);
}

BooleanPropertyComponent::~BooleanPropertyComponent()
{
    deleteAllChildren();
}

void BooleanPropertyComponent::paint (Graphics& g)
{
    PropertyComponent::paint (g);

    const Rectangle r (button->getBounds());
    g.setColour (Colours::white);
    g.fillRect (r);

    g.setColour (findColour (ComboBox::outlineColourId));
    g.drawRect (r.getX(), r.getY(), r.getWidth(), r.getHeight());
}

void BooleanPropertyComponent::refresh()
{
    button->setToggleState (getState(), false);
    button->setButtonText (button->getToggleState() ? onText : offText);
}

void BooleanPropertyComponent::buttonClicked (Button*)
{
    setState (! getState());
}

END_JUCE_NAMESPACE
