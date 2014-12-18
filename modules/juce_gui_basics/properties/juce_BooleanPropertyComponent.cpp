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

BooleanPropertyComponent::BooleanPropertyComponent (const String& name,
                                                    const String& buttonTextWhenTrue,
                                                    const String& buttonTextWhenFalse)
    : PropertyComponent (name),
      onText (buttonTextWhenTrue),
      offText (buttonTextWhenFalse)
{
    addAndMakeVisible (button);
    button.setClickingTogglesState (false);
    button.addListener (this);
}

BooleanPropertyComponent::BooleanPropertyComponent (const Value& valueToControl,
                                                    const String& name,
                                                    const String& buttonText)
    : PropertyComponent (name),
      onText (buttonText),
      offText (buttonText)
{
    addAndMakeVisible (button);
    button.setClickingTogglesState (false);
    button.setButtonText (buttonText);
    button.getToggleStateValue().referTo (valueToControl);
    button.setClickingTogglesState (true);
}

BooleanPropertyComponent::~BooleanPropertyComponent()
{
}

void BooleanPropertyComponent::setState (const bool newState)
{
    button.setToggleState (newState, sendNotification);
}

bool BooleanPropertyComponent::getState() const
{
    return button.getToggleState();
}

void BooleanPropertyComponent::paint (Graphics& g)
{
    PropertyComponent::paint (g);

    g.setColour (findColour (backgroundColourId));
    g.fillRect (button.getBounds());

    g.setColour (findColour (ComboBox::outlineColourId));
    g.drawRect (button.getBounds());
}

void BooleanPropertyComponent::refresh()
{
    button.setToggleState (getState(), dontSendNotification);
    button.setButtonText (button.getToggleState() ? onText : offText);
}

void BooleanPropertyComponent::buttonClicked (Button*)
{
    setState (! getState());
}
