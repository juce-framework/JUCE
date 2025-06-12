/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

TooltipWindow::TooltipWindow (Component* parentComp, int delayMs)
    : Component ("tooltip"),
      millisecondsBeforeTipAppears (delayMs)
{
    setAlwaysOnTop (true);
    setOpaque (true);
    setAccessible (false);

    if (parentComp != nullptr)
        parentComp->addChildComponent (this);

    auto& desktop = Desktop::getInstance();

    if (desktop.getMainMouseSource().canHover())
    {
        desktop.addGlobalMouseListener (this);
        startTimer (123);
    }
}

TooltipWindow::~TooltipWindow()
{
    hideTip();
    Desktop::getInstance().removeGlobalMouseListener (this);
}

void TooltipWindow::setMillisecondsBeforeTipAppears (const int newTimeMs) noexcept
{
    millisecondsBeforeTipAppears = newTimeMs;
}

void TooltipWindow::paint (Graphics& g)
{
    getLookAndFeel().drawTooltip (g, tipShowing, getWidth(), getHeight());
}

void TooltipWindow::mouseEnter (const MouseEvent& e)
{
    if (e.eventComponent == this)
        hideTip();
}

void TooltipWindow::mouseDown (const MouseEvent&)
{
    if (isVisible())
        dismissalMouseEventOccurred = true;
}

void TooltipWindow::mouseWheelMove (const MouseEvent&, const MouseWheelDetails&)
{
    if (isVisible())
        dismissalMouseEventOccurred = true;
}

void TooltipWindow::updatePosition (const String& tip, Point<int> pos, Rectangle<int> parentArea)
{
    setBounds (getLookAndFeel().getTooltipBounds (tip, pos, parentArea));
    setVisible (true);
}

#if JUCE_DEBUG
static Array<TooltipWindow*> activeTooltipWindows;
#endif

void TooltipWindow::displayTip (Point<int> screenPos, const String& tip)
{
    jassert (tip.isNotEmpty());

    displayTipInternal (screenPos, tip, ShownManually::yes);
}

void TooltipWindow::displayTipInternal (Point<int> screenPos, const String& tip, ShownManually shownManually)
{
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
            const auto physicalPos = detail::ScalingHelpers::scaledScreenPosToUnscaled (screenPos);
            const auto scaledPos = detail::ScalingHelpers::unscaledScreenPosToScaled (*this, physicalPos);
            updatePosition (tip, scaledPos, Desktop::getInstance().getDisplays().getDisplayForPoint (screenPos)->userArea);

            addToDesktop (ComponentPeer::windowHasDropShadow
                          | ComponentPeer::windowIsTemporary
                          | ComponentPeer::windowIgnoresKeyPresses
                          | ComponentPeer::windowIgnoresMouseClicks);
        }

       #if JUCE_DEBUG
        activeTooltipWindows.addIfNotAlreadyThere (this);

        auto* parent = getParentComponent();

        for (auto* w : activeTooltipWindows)
        {
            if (w != nullptr && w != this && w->tipShowing == tipShowing && w->getParentComponent() == parent)
            {
                // Looks like you have more than one TooltipWindow showing the same tip..
                // Be careful not to create more than one instance of this class with the
                // same parent component!
                jassertfalse;
            }
        }
       #endif

        toFront (false);
        manuallyShownTip = shownManually == ShownManually::yes ? tip : String();
        dismissalMouseEventOccurred = false;
    }
}

String TooltipWindow::getTipFor (Component& c)
{
    if (detail::WindowingHelpers::isForegroundOrEmbeddedProcess (&c)
         && ! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
    {
        if (auto* ttc = dynamic_cast<TooltipClient*> (&c))
            if (! c.isCurrentlyBlockedByAnotherModalComponent())
                return ttc->getTooltip();
    }

    return {};
}

void TooltipWindow::hideTip()
{
    if (isVisible() && ! reentrant)
    {
        tipShowing = {};
        manuallyShownTip = {};
        dismissalMouseEventOccurred = false;

        removeFromDesktop();
        setVisible (false);

        lastHideTime = Time::getApproximateMillisecondCounter();

       #if JUCE_DEBUG
        activeTooltipWindows.removeAllInstancesOf (this);
       #endif
    }
}

float TooltipWindow::getDesktopScaleFactor() const
{
    if (lastComponentUnderMouse != nullptr)
        return Component::getApproximateScaleFactorForComponent (lastComponentUnderMouse);

    return Component::getDesktopScaleFactor();
}

std::unique_ptr<AccessibilityHandler> TooltipWindow::createAccessibilityHandler()
{
    return createIgnoredAccessibilityHandler (*this);
}

void TooltipWindow::timerCallback()
{
    const auto mouseSource = Desktop::getInstance().getMainMouseSource();
    auto* newComp = mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();

    if (manuallyShownTip.isNotEmpty())
    {
        if (dismissalMouseEventOccurred || newComp == nullptr)
            hideTip();

        return;
    }

    if (newComp == nullptr || getParentComponent() == nullptr || newComp->getPeer() == getPeer())
    {
        const auto newTip = newComp != nullptr ? getTipFor (*newComp) : String();

        const auto mousePos = mouseSource.getScreenPosition();
        const auto mouseMovedQuickly = (mousePos.getDistanceFrom (lastMousePos) > 12);
        lastMousePos = mousePos;

        const auto tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
        const auto now = Time::getApproximateMillisecondCounter();

        lastComponentUnderMouse = newComp;
        lastTipUnderMouse = newTip;

        if (tipChanged || dismissalMouseEventOccurred || mouseMovedQuickly)
            lastCompChangeTime = now;

        const auto showTip = [this, &mouseSource, &mousePos, &newTip]
        {
            if (mouseSource.getLastMouseDownPosition() != lastMousePos)
                displayTipInternal (mousePos.roundToInt(), newTip, ShownManually::no);
        };

        if (isVisible() || now < lastHideTime + 500)
        {
            // if a tip is currently visible (or has just disappeared), update to a new one
            // immediately if needed..
            if (newComp == nullptr || dismissalMouseEventOccurred || newTip.isEmpty())
                hideTip();
            else if (tipChanged)
                showTip();
        }
        else
        {
            // if there isn't currently a tip, but one is needed, only let it appear after a timeout
            if (newTip.isNotEmpty()
                && newTip != tipShowing
                && now > lastCompChangeTime + (uint32) millisecondsBeforeTipAppears)
            {
                showTip();
            }
        }
    }
}

} // namespace juce
