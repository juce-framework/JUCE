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
                                          w - bubblePaddingX, h - bubblePaddingY));
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
