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
      lastComponentUnderMouse (nullptr),
      changedCompsSinceShown (true)
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
    hide();
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
    hide();
}

void TooltipWindow::showFor (const String& tip)
{
    jassert (tip.isNotEmpty());
    if (tipShowing != tip)
        repaint();

    tipShowing = tip;

    Point<int> mousePos (Desktop::getMousePosition());
    Rectangle<int> parentArea;
    Component* const parent = getParentComponent();

    if (parent != nullptr)
    {
        mousePos   = parent->getLocalPoint (nullptr, mousePos);
        parentArea = parent->getLocalBounds();
    }
    else
    {
        parentArea = Desktop::getInstance().getDisplays().getDisplayContaining (mousePos).userArea;
    }

    int w, h;
    getLookAndFeel().getTooltipSize (tip, w, h);

    int x = mousePos.x;
    if (x > parentArea.getCentreX())
        x -= (w + 12);
    else
        x += 24;

    int y = mousePos.y;
    if (y > parentArea.getCentreY())
        y -= (h + 6);
    else
        y += 6;

    x = jlimit (parentArea.getX(), parentArea.getRight()  - w, x);
    y = jlimit (parentArea.getY(), parentArea.getBottom() - h, y);

    setBounds (x, y, w, h);
    setVisible (true);

    if (parent == nullptr)
        addToDesktop (ComponentPeer::windowHasDropShadow
                        | ComponentPeer::windowIsTemporary
                        | ComponentPeer::windowIgnoresKeyPresses);

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

void TooltipWindow::hide()
{
    tipShowing = String::empty;
    removeFromDesktop();
    setVisible (false);
}

void TooltipWindow::timerCallback()
{
    const unsigned int now = Time::getApproximateMillisecondCounter();
    Component* const newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
    const String newTip (getTipFor (newComp));

    const bool tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
    lastComponentUnderMouse = newComp;
    lastTipUnderMouse = newTip;

    Desktop& desktop = Desktop::getInstance();
    const int clickCount = desktop.getMouseButtonClickCounter();
    const int wheelCount = desktop.getMouseWheelMoveCounter();
    const bool mouseWasClicked = (clickCount > mouseClicks || wheelCount > mouseWheelMoves);
    mouseClicks = clickCount;
    mouseWheelMoves = wheelCount;

    const Point<int> mousePos (Desktop::getMousePosition());
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
                hide();
            }
        }
        else if (tipChanged)
        {
            showFor (newTip);
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
            showFor (newTip);
        }
    }
}
