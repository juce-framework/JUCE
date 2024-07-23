/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

BubbleMessageComponent::BubbleMessageComponent (int fadeOutLengthMs)
    : fadeOutLength (fadeOutLengthMs), mouseClickCounter (0),
      expiryTime (0), deleteAfterUse (false)
{
}

BubbleMessageComponent::~BubbleMessageComponent()
{
}

void BubbleMessageComponent::showAt (const Rectangle<int>& pos,
                                     const AttributedString& text,
                                     const int numMillisecondsBeforeRemoving,
                                     const bool removeWhenMouseClicked,
                                     const bool deleteSelfAfterUse)
{
    createLayout (text);
    setPosition (pos);
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
    setAlpha (1.0f);
    setVisible (true);
    deleteAfterUse = deleteSelfAfterUse;

    expiryTime = numMillisecondsBeforeRemoving > 0
                    ? (Time::getMillisecondCounter() + (uint32) numMillisecondsBeforeRemoving) : 0;

    mouseClickCounter = Desktop::getInstance().getMouseButtonClickCounter();

    if (! (removeWhenMouseClicked && isShowing()))
        mouseClickCounter += 0xfffff;

    startTimer (77);
    repaint();
}

const float bubblePaddingX = 20.0f;
const float bubblePaddingY = 14.0f;

void BubbleMessageComponent::getContentSize (int& w, int& h)
{
    w = (int) (bubblePaddingX + textLayout.getWidth());
    h = (int) (bubblePaddingY + textLayout.getHeight());
}

void BubbleMessageComponent::paintContent (Graphics& g, int w, int h)
{
    g.setColour (findColour (TooltipWindow::textColourId));

    textLayout.draw (g, Rectangle<float> (bubblePaddingX / 2.0f, bubblePaddingY / 2.0f,
                                          (float) w - bubblePaddingX, (float) h - bubblePaddingY));
}

void BubbleMessageComponent::timerCallback()
{
    if (Desktop::getInstance().getMouseButtonClickCounter() > mouseClickCounter)
        hide (false);
    else if (expiryTime != 0 && Time::getMillisecondCounter() > expiryTime)
        hide (true);
}

void BubbleMessageComponent::hide (const bool fadeOut)
{
    stopTimer();

    if (fadeOut)
        Desktop::getInstance().getAnimator().fadeOut (this, fadeOutLength);
    else
        setVisible (false);

    if (deleteAfterUse)
        delete this;
}

} // namespace juce
