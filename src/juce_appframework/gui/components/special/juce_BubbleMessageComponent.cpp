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

#include "juce_BubbleMessageComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../juce_Desktop.h"


//==============================================================================
BubbleMessageComponent::BubbleMessageComponent (int fadeOutLengthMs)
    : fadeOutLength (fadeOutLengthMs),
      deleteAfterUse (false)
{
}

BubbleMessageComponent::~BubbleMessageComponent()
{
    fadeOutComponent (fadeOutLength);
}

void BubbleMessageComponent::showAt (int x, int y,
                                     const String& text,
                                     const int numMillisecondsBeforeRemoving,
                                     const bool removeWhenMouseClicked,
                                     const bool deleteSelfAfterUse)
{
    textLayout.clear();
    textLayout.setText (text, Font (14.0f));
    textLayout.layout (256, Justification::centredLeft, true);

    setPosition (x, y);

    init (numMillisecondsBeforeRemoving, removeWhenMouseClicked, deleteSelfAfterUse);
}

void BubbleMessageComponent::showAt (Component* const component,
                                     const String& text,
                                     const int numMillisecondsBeforeRemoving,
                                     const bool removeWhenMouseClicked,
                                     const bool deleteSelfAfterUse)
{
    textLayout.clear();
    textLayout.setText (text, Font (14.0f));
    textLayout.layout (256, Justification::centredLeft, true);

    setPosition (component);

    init (numMillisecondsBeforeRemoving, removeWhenMouseClicked, deleteSelfAfterUse);
}

void BubbleMessageComponent::init (const int numMillisecondsBeforeRemoving,
                                   const bool removeWhenMouseClicked,
                                   const bool deleteSelfAfterUse)
{
    setVisible (true);

    deleteAfterUse = deleteSelfAfterUse;

    if (numMillisecondsBeforeRemoving > 0)
        expiryTime = Time::getMillisecondCounter() + numMillisecondsBeforeRemoving;
    else
        expiryTime = 0;

    startTimer (77);

    mouseClickCounter = Desktop::getInstance().getMouseButtonClickCounter();

    if (! (removeWhenMouseClicked && isShowing()))
        mouseClickCounter += 0xfffff;

    repaint();
}

void BubbleMessageComponent::getContentSize (int& w, int& h)
{
    w = textLayout.getWidth() + 16;
    h = textLayout.getHeight() + 16;
}

void BubbleMessageComponent::paintContent (Graphics& g, int w, int h)
{
    g.setColour (findColour (TooltipWindow::textColourId));

    textLayout.drawWithin (g, 0, 0, w, h, Justification::centred);
}

void BubbleMessageComponent::timerCallback()
{
    if (Desktop::getInstance().getMouseButtonClickCounter() > mouseClickCounter)
    {
        stopTimer();
        setVisible (false);

        if (deleteAfterUse)
            delete this;
    }
    else if (expiryTime != 0 && Time::getMillisecondCounter() > expiryTime)
    {
        stopTimer();
        fadeOutComponent (fadeOutLength);

        if (deleteAfterUse)
            delete this;
    }
}

END_JUCE_NAMESPACE
