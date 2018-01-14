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

namespace juce
{

SidePanel::SidePanel (StringRef title, int width, bool positionOnLeft,
                      Component* contentToDisplay, bool deleteComponentWhenNoLongerNeeded)
    : titleLabel ("titleLabel", title),
      isOnLeft (positionOnLeft),
      panelWidth (width)
{
    lookAndFeelChanged();

    addAndMakeVisible (titleLabel);

    dismissButton.onClick = [this] { showOrHide (false); };
    addAndMakeVisible (dismissButton);

    Desktop::getInstance().addGlobalMouseListener (this);

    if (contentToDisplay != nullptr)
        setContent (contentToDisplay, deleteComponentWhenNoLongerNeeded);

    setOpaque (false);
}

SidePanel::~SidePanel()
{
    if (parent != nullptr)
        parent->removeComponentListener (this);
}

void SidePanel::setContent (Component* newContent, bool deleteComponentWhenNoLongerNeeded)
{
    if (contentComponent.get() != newContent)
    {
        if (deleteComponentWhenNoLongerNeeded)
            contentComponent.setOwned (newContent);
        else
            contentComponent.setNonOwned (newContent);

        addAndMakeVisible (contentComponent);

        resized();
    }
}

void SidePanel::showOrHide (bool show)
{
    if (parent != nullptr)
    {
        isShowing = show;

        Desktop::getInstance().getAnimator().animateComponent (this, calculateBoundsInParent (*parent),
                                                               1.0f, 250, true, 1.0, 0.0);
    }
}

void SidePanel::resized()
{
    auto bounds = getLocalBounds();

    calculateAndRemoveShadowBounds (bounds);

    auto titleBounds = bounds.removeFromTop (titleBarHeight);

    dismissButton.setBounds (isOnLeft ? titleBounds.removeFromRight (30).withTrimmedRight (10)
                                      : titleBounds.removeFromLeft  (30).withTrimmedLeft  (10));

    titleLabel.setBounds (isOnLeft ? titleBounds.withTrimmedRight (40)
                                   : titleBounds.withTrimmedLeft (40));

    if (contentComponent != nullptr)
        contentComponent->setBounds (bounds);
}

void SidePanel::paint (Graphics& g)
{
    auto& lf = getLookAndFeel();

    auto bgColour     = lf.findColour (SidePanel::backgroundColour);
    auto shadowColour = lf.findColour (SidePanel::shadowBaseColour);

    g.setGradientFill (ColourGradient (shadowColour.withAlpha (0.7f), (isOnLeft ? shadowArea.getTopLeft()
                                                                                : shadowArea.getTopRight()).toFloat(),
                                       shadowColour.withAlpha (0.0f), (isOnLeft ? shadowArea.getTopRight()
                                                                                : shadowArea.getTopLeft()).toFloat(), false));
    g.fillRect (shadowArea);

    g.excludeClipRegion (shadowArea);
    g.fillAll (bgColour);
}

void SidePanel::parentHierarchyChanged()
{
    auto* newParent = getParentComponent();

    if ((newParent != nullptr) && (parent != newParent))
    {
        if (parent != nullptr)
            parent->removeComponentListener (this);

        parent = newParent;
        parent->addComponentListener (this);
    }
}

void SidePanel::mouseDrag (const MouseEvent& e)
{
    if (shouldResize)
    {
        Point<int> convertedPoint;

        if (getParentComponent() == nullptr)
            convertedPoint = e.eventComponent->localPointToGlobal (e.getPosition());
        else
            convertedPoint = getParentComponent()->getLocalPoint (e.eventComponent, e.getPosition());

        auto currentMouseDragX = convertedPoint.x;

        if (isOnLeft)
        {
            amountMoved = startingBounds.getRight() - currentMouseDragX;
            setBounds (getBounds().withX (startingBounds.getX() - jmax (amountMoved, 0)));
        }
        else
        {
            amountMoved = currentMouseDragX - startingBounds.getX();
            setBounds (getBounds().withX (startingBounds.getX() + jmax (amountMoved, 0)));
        }
    }
    else if (isShowing)
    {
        auto relativeMouseDownPosition = getLocalPoint (e.eventComponent, e.getMouseDownPosition());
        auto relativeMouseDragPosition = getLocalPoint (e.eventComponent, e.getPosition());

        if (! getLocalBounds().contains (relativeMouseDownPosition)
              && getLocalBounds().contains (relativeMouseDragPosition))
        {
            shouldResize = true;
            startingBounds = getBounds();
        }
    }
}

void SidePanel::mouseUp (const MouseEvent&)
{
    if (shouldResize)
    {
        showOrHide (amountMoved < (panelWidth / 2));

        amountMoved = 0;
        shouldResize = false;
    }
}

//==========================================================================
void SidePanel::lookAndFeelChanged()
{
    auto& lf = getLookAndFeel();

    dismissButton.setShape (lf.getSidePanelDismissButtonShape (*this), false, true, false);

    dismissButton.setColours (lf.findColour (SidePanel::dismissButtonNormalColour),
                              lf.findColour (SidePanel::dismissButtonOverColour),
                              lf.findColour (SidePanel::dismissButtonDownColour));

    titleLabel.setFont (lf.getSidePanelTitleFont (*this));
    titleLabel.setColour (Label::textColourId, findColour (SidePanel::titleTextColour));
    titleLabel.setJustificationType (lf.getSidePanelTitleJustification (*this));
}

void SidePanel::componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
{
    ignoreUnused (wasMoved);

    if (wasResized && (&component == parent))
        setBounds (calculateBoundsInParent (component));
}

Rectangle<int> SidePanel::calculateBoundsInParent (Component& parentComp) const
{
    auto parentBounds = parentComp.getBounds();

    if (isOnLeft)
    {
        return isShowing ? parentBounds.removeFromLeft (panelWidth)
                         : parentBounds.withX (parentBounds.getX() - panelWidth).withWidth (panelWidth);
    }

    return isShowing ? parentBounds.removeFromRight (panelWidth)
                     : parentBounds.withX (parentBounds.getRight()).withWidth (panelWidth);
}

void SidePanel::calculateAndRemoveShadowBounds (Rectangle<int>& bounds)
{
    shadowArea = isOnLeft ? bounds.removeFromRight (shadowWidth)
                          : bounds.removeFromLeft  (shadowWidth);
}

bool SidePanel::isMouseEventInThisOrChildren (Component* eventComponent)
{
    if (eventComponent == this)
        return true;

    for (auto& child : getChildren())
        if (eventComponent == child)
            return true;

    return false;
}

} // namespace juce
