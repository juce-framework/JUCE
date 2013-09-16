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

TooltipWindow::TooltipWindow (Component* const parentComp, const int delayMs)
    : Component ("tooltip"),
      millisecondsBeforeTipAppears (delayMs),
      mouseClicks (0),
      mouseWheelMoves (0),
      lastHideTime (0),
      lastComponentUnderMouse (nullptr)
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

void TooltipWindow::updatePosition (const String& tip, Point<int> pos, const Rectangle<int>& parentArea)
{
    int w, h;
    getLookAndFeel().getTooltipSize (tip, w, h);

    Rectangle<int> bounds (pos.x > parentArea.getCentreX() ? pos.x - (w + 12) : pos.x + 24,
                           pos.y > parentArea.getCentreY() ? pos.y - (h + 6)  : pos.y + 6,
                           w, h);

    setBounds (bounds.constrainedWithin (parentArea));
    setVisible (true);
}

void TooltipWindow::displayTip (Point<int> screenPos, const String& tip)
{
    jassert (tip.isNotEmpty());
    if (tipShowing != tip)
        repaint();

    tipShowing = tip;

    if (Component* const parent = getParentComponent())
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
                        | ComponentPeer::windowIgnoresKeyPresses);
    }

    toFront (false);
}

String TooltipWindow::getTipFor (Component* const c)
{
    if (c != nullptr
         && Process::isForegroundProcess()
         && ! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
    {
        if (TooltipClient* const ttc = dynamic_cast <TooltipClient*> (c))
            if (! c->isCurrentlyBlockedByAnotherModalComponent())
                return ttc->getTooltip();
    }

    return String::empty;
}

void TooltipWindow::hideTip()
{
    tipShowing = String::empty;
    removeFromDesktop();
    setVisible (false);
}

void TooltipWindow::timerCallback()
{
    Desktop& desktop = Desktop::getInstance();
    const MouseInputSource mouseSource (desktop.getMainMouseSource());
    const unsigned int now = Time::getApproximateMillisecondCounter();

    Component* const newComp = mouseSource.isMouse() ? mouseSource.getComponentUnderMouse() : nullptr;
    const String newTip (getTipFor (newComp));
    const bool tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
    lastComponentUnderMouse = newComp;
    lastTipUnderMouse = newTip;

    const int clickCount = desktop.getMouseButtonClickCounter();
    const int wheelCount = desktop.getMouseWheelMoveCounter();
    const bool mouseWasClicked = (clickCount > mouseClicks || wheelCount > mouseWheelMoves);
    mouseClicks = clickCount;
    mouseWheelMoves = wheelCount;

    const Point<int> mousePos (mouseSource.getScreenPosition());
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
            displayTip (mousePos, newTip);
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
            displayTip (mousePos, newTip);
        }
    }
}
