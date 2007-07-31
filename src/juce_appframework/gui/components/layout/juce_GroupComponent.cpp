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
