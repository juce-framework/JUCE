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

ToggleButton::ToggleButton()
    : Button (String::empty)
{
    setClickingTogglesState (true);
}

ToggleButton::ToggleButton (const String& buttonText)
    : Button (buttonText)
{
    setClickingTogglesState (true);
}

ToggleButton::~ToggleButton()
{
}

void ToggleButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    getLookAndFeel().drawToggleButton (g, *this, isMouseOverButton, isButtonDown);
}

void ToggleButton::changeWidthToFitText()
{
    getLookAndFeel().changeToggleButtonWidthToFitText (*this);
}

void ToggleButton::colourChanged()
{
    repaint();
}
