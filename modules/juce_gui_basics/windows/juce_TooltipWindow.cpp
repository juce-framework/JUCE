/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

TooltipWindow::TooltipWindow (Component* const parentComp, const int delayMs)
    : Component ("tooltip"),
      lastComponentUnderMouse (nullptr),
      millisecondsBeforeTipAppears (delayMs),
      mouseClicks (0), mouseWheelMoves (0),
      lastCompChangeTime (0), lastHideTime (0),
      reentrant (false)
{
    if (Desktop::getInstance().getMainMouseSource().canHover())
        startTimer (123);

    setAlwaysOnTop (true);
    setOpaque (true);

    if (parentComp != nullptr)
        parentComp->addChildComponent (this);
}

TooltipWindow::~TooltipWindow()
{
    hideTip();
}

void TooltipWindow::setMillisecondsBeforeTipAppears (const int newTimeMs) noexcept
{
    millisecondsBeforeTipAppears = newTimeMs;
}

void TooltipWindow::paint (Graphics& g)
{
    getLookAndFeel().drawTooltip (g, tipShowing, getWidth(), getHeight());
}

void TooltipWindow::mouseEnter (const MouseEvent&)
{
    hideTip();
}

void TooltipWindow::updatePosition (const String& tip, Point<int> pos, Rectangle<int> parentArea)
{
    setBounds (getLookAndFeel().getTooltipBounds (tip, pos, parentArea));
    setVisible (true);
}

void TooltipWindow::displayTip (Point<int> screenPos, const String& tip)
{
    jassert (tip.isNotEmpty());

    if (! reentrant)
    {
        ScopedValueSetter<bool> setter (reentrant, true, false);

        if (tipShowing != tip)
        {
            tipShowing = tip;
            repaint();
        }

        if (auto* parent = getParentComponent())
        {
            updatePosition (tip, parent->getLocalPoint (nullptr, screenPos),
                            parent->getLocalBounds());
        }
        else
        {
            updatePosition (tip, screenPos, Desktop::getInstance().getDisplays()
                                                .getDisplayContaining (screenPos).userArea);

            addToDesktop (ComponentPeer::windowHasDropShadow
                            | ComponentPeer::windowIsTemporary
                            | ComponentPeer::windowIgnoresKeyPresses
                            | ComponentPeer::windowIgnoresMouseClicks);
        }

        toFront (false);
    }
}

String TooltipWindow::getTipFor (Component* const c)
{
    if (c != nullptr
         && Process::isForegroundProcess()
         && ! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
    {
        if (TooltipClient* const ttc = dynamic_cast<TooltipClient*> (c))
            if (! c->isCurrentlyBlockedByAnotherModalComponent())
                return ttc->getTooltip();
    }

    return {};
}

void TooltipWindow::hideTip()
{
    if (! reentrant)
    {
        tipShowing.clear();
        removeFromDesktop();
        setVisible (false);
    }
}

void TooltipWindow::timerCallback()
{
    Desktop& desktop = Desktop::getInstance();
    const MouseInputSource mouseSource (desktop.getMainMouseSource());
    const unsigned int now = Time::getApproximateMillisecondCounter();

    Component* const newComp = ! mouseSource.isTouch() ? mouseSource.getComponentUnderMouse() : nullptr;
    const String newTip (getTipFor (newComp));
    const bool tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
    lastComponentUnderMouse = newComp;
    lastTipUnderMouse = newTip;

    const int clickCount = desktop.getMouseButtonClickCounter();
    const int wheelCount = desktop.getMouseWheelMoveCounter();
    const bool mouseWasClicked = (clickCount > mouseClicks || wheelCount > mouseWheelMoves);
    mouseClicks = clickCount;
    mouseWheelMoves = wheelCount;

    const Point<float> mousePos (mouseSource.getScreenPosition());
    const bool mouseMovedQuickly = mousePos.getDistanceFrom (lastMousePos) > 12;
    lastMousePos = mousePos;

    if (tipChanged || mouseWasClicked || mouseMovedQuickly)
        lastCompChangeTime = now;

    if (isVisible() || now < lastHideTime + 500)
    {
        // if a tip is currently visible (or has just disappeared), update to a new one
        // immediately if needed..
        if (newComp == nullptr || mouseWasClicked || newTip.isEmpty())
        {
            if (isVisible())
            {
                lastHideTime = now;
                hideTip();
            }
        }
        else if (tipChanged)
        {
            displayTip (mousePos.roundToInt(), newTip);
        }
    }
    else
    {
        // if there isn't currently a tip, but one is needed, only let it
        // appear after a timeout..
        if (newTip.isNotEmpty()
             && newTip != tipShowing
             && now > lastCompChangeTime + (unsigned int) millisecondsBeforeTipAppears)
        {
            displayTip (mousePos.roundToInt(), newTip);
        }
    }
}
