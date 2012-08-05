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

GroupComponent::GroupComponent (const String& name,
                                const String& labelText)
    : Component (name),
      text (labelText),
      justification (Justification::left)
{
    setInterceptsMouseClicks  (false, true);
}

GroupComponent::~GroupComponent()
{
}

//==============================================================================
void GroupComponent::setText (const String& newText)
{
    if (text != newText)
    {
        text = newText;
        repaint();
    }
}

String GroupComponent::getText() const
{
    return text;
}

//==============================================================================
void GroupComponent::setTextLabelPosition (const Justification& newJustification)
{
    if (justification != newJustification)
    {
        justification = newJustification;
        repaint();
    }
}

void GroupComponent::paint (Graphics& g)
{
    getLookAndFeel()
        .drawGroupComponentOutline (g, getWidth(), getHeight(),
                                    text, justification,
                                    *this);
}

void GroupComponent::enablementChanged()
{
    repaint();
}

void GroupComponent::colourChanged()
{
    repaint();
}
