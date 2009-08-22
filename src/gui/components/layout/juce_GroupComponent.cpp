/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_GroupComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
GroupComponent::GroupComponent (const String& componentName,
                                const String& labelText)
    : Component (componentName),
      text (labelText),
      justification (Justification::left)
{
    setInterceptsMouseClicks  (false, true);
}

GroupComponent::~GroupComponent()
{
}

//==============================================================================
void GroupComponent::setText (const String& newText) throw()
{
    if (text != newText)
    {
        text = newText;
        repaint();
    }
}

const String GroupComponent::getText() const throw()
{
    return text;
}

//==============================================================================
void GroupComponent::setTextLabelPosition (const Justification& newJustification)
{
    if (justification.getFlags() != newJustification.getFlags())
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


END_JUCE_NAMESPACE
