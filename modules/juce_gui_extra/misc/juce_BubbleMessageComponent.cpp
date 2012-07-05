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

BubbleMessageComponent::BubbleMessageComponent (int fadeOutLengthMs)
    : fadeOutLength (fadeOutLengthMs),
      deleteAfterUse (false)
{
}

BubbleMessageComponent::~BubbleMessageComponent()
{
    Desktop::getInstance().getAnimator().fadeOut (this, fadeOutLength);
}

void BubbleMessageComponent::showAt (int x, int y,
                                     const AttributedString& text,
                                     const int numMillisecondsBeforeRemoving,
                                     const bool removeWhenMouseClicked,
                                     const bool deleteSelfAfterUse)
{
    createLayout (text);
    setPosition (x, y);
    init (numMillisecondsBeforeRemoving, removeWhenMouseClicked, deleteSelfAfterUse);
}

void BubbleMessageComponent::showAt (Component* const component,
                                     const AttributedString& text,
                                     const int numMillisecondsBeforeRemoving,
                                     const bool removeWhenMouseClicked,
                                     const bool deleteSelfAfterUse)
{
    createLayout (text);
    setPosition (component);
    init (numMillisecondsBeforeRemoving, removeWhenMouseClicked, deleteSelfAfterUse);
}

void BubbleMessageComponent::createLayout (const AttributedString& text)
{
    textLayout.createLayoutWithBalancedLineLengths (text, 256);
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
    w = 20 + (int) textLayout.getWidth();
    h = 20 + (int) textLayout.getHeight();
}

void BubbleMessageComponent::paintContent (Graphics& g, int w, int h)
{
    g.setColour (findColour (TooltipWindow::textColourId));

    textLayout.draw (g, Rectangle<float> (6.0f, 6.0f, w - 12.0f, h - 12.0f));
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

        if (deleteAfterUse)
            delete this;
        else
            Desktop::getInstance().getAnimator().fadeOut (this, fadeOutLength);
    }
}
