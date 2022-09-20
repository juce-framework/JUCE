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

#include "../../Application/jucer_Headers.h"
#include "jucer_ComponentLayoutEditor.h"
#include "../jucer_UtilityFunctions.h"

//==============================================================================
ComponentOverlayComponent::ComponentOverlayComponent (Component* const target_,
                                                      ComponentLayout& layout_)
    : target (target_),
      borderThickness (4),
      layout (layout_),
      selected (false),
      dragging (false),
      originalAspectRatio (1.0)
{
    setMinimumOnscreenAmounts (0, 0, 0, 0);
    setSizeLimits (borderThickness * 2 + 2, borderThickness * 2 + 2, 8192, 8192);

    border.reset (new ResizableBorderComponent (this, this));
    addChildComponent (border.get());

    border->setBorderThickness (BorderSize<int> (borderThickness));

    target->addComponentListener (this);

    changeListenerCallback (nullptr);
    layout.getSelectedSet().addChangeListener (this);

    setRepaintsOnMouseActivity (true);
    border->setRepaintsOnMouseActivity (true);
}

ComponentOverlayComponent::~ComponentOverlayComponent()
{
    layout.getSelectedSet().removeChangeListener (this);

    if (target != nullptr)
        target->removeComponentListener (this);
}

void ComponentOverlayComponent::changeListenerCallback (ChangeBroadcaster*)
{
    const bool nowSelected = layout.getSelectedSet().isSelected (target);

    if (selected != nowSelected)
    {
        selected = nowSelected;
        border->setVisible (nowSelected);
        repaint();
    }
}

void ComponentOverlayComponent::paint (Graphics& g)
{
    jassert (target != nullptr);

    border->setColour (backgroundColourId, Colours::transparentBlack);
    if (selected)
    {
        auto selectedItems = layout.getSelectedSet();
        auto baseColour = findColour (defaultHighlightColourId);

        const BorderSize<int> borderSize (border->getBorderThickness());

        drawResizableBorder (g, getWidth(), getHeight(), borderSize,
                             (isMouseOverOrDragging() || border->isMouseOverOrDragging()),
                             baseColour.withAlpha (selectedItems.getSelectedItem (0) == target ? 1.0f : 0.3f));
    }
    else if (isMouseOverOrDragging())
    {
        drawMouseOverCorners (g, getWidth(), getHeight());
    }
}

void ComponentOverlayComponent::resized()
{
    jassert (target != nullptr);

    border->setBounds (getLocalBounds());
}

void ComponentOverlayComponent::mouseDown (const MouseEvent& e)
{
    dragging = false;
    mouseDownSelectStatus = layout.getSelectedSet().addToSelectionOnMouseDown (target, e.mods);

    if (e.mods.isPopupMenu())
    {
        showPopupMenu();
        return; // this may be deleted now..
    }
}

void ComponentOverlayComponent::mouseDrag (const MouseEvent& e)
{
    if (! e.mods.isPopupMenu())
    {
        if (selected && ! dragging)
        {
            dragging = e.mouseWasDraggedSinceMouseDown();

            if (dragging)
                layout.startDragging();
        }

        if (dragging)
        {
            layout.dragSelectedComps (e.getDistanceFromDragStartX(),
                                      e.getDistanceFromDragStartY());
        }
    }
}

void ComponentOverlayComponent::mouseUp (const MouseEvent& e)
{
    if (dragging)
        layout.endDragging();

    layout.getSelectedSet().addToSelectionOnMouseUp (target, e.mods, dragging, mouseDownSelectStatus);
}

void ComponentOverlayComponent::componentMovedOrResized (Component&, bool /*wasMoved*/, bool /*wasResized*/)
{
    updateBoundsToMatchTarget();
}

void ComponentOverlayComponent::updateBoundsToMatchTarget()
{
    if (Component* const parent = target->getParentComponent())
    {
        const int dx = parent->getX();
        const int dy = parent->getY();

        setBounds (dx + target->getX() - borderThickness,
                   dy + target->getY() - borderThickness,
                   target->getWidth() + borderThickness * 2,
                   target->getHeight() + borderThickness * 2);
    }

    if (border->isMouseButtonDown())
        layout.changed();
}

void ComponentOverlayComponent::resizeStart()
{
    if (getHeight() > 0)
        originalAspectRatio = getWidth() / (double) getHeight();
    else
        originalAspectRatio = 1.0;

    layout.getDocument()->beginTransaction ("Resize components");
}

void ComponentOverlayComponent::resizeEnd()
{
    layout.getDocument()->beginTransaction();
}

void ComponentOverlayComponent::checkBounds (Rectangle<int>& b,
                                             const Rectangle<int>& previousBounds,
                                             const Rectangle<int>& limits,
                                             const bool isStretchingTop,
                                             const bool isStretchingLeft,
                                             const bool isStretchingBottom,
                                             const bool isStretchingRight)
{
    if (ModifierKeys::currentModifiers.isShiftDown())
        setFixedAspectRatio (originalAspectRatio);
    else
        setFixedAspectRatio (0.0);

    ComponentBoundsConstrainer::checkBounds (b, previousBounds, limits, isStretchingTop, isStretchingLeft, isStretchingBottom, isStretchingRight);

    if (layout.getDocument()->isSnapActive (true))
    {
        if (Component* const parent = target->getParentComponent())
        {
            const int dx = parent->getX();
            const int dy = parent->getY();

            int x = b.getX();
            int y = b.getY();
            int w = b.getWidth();
            int h = b.getHeight();

            x += borderThickness - dx;
            y += borderThickness - dy;
            w -= borderThickness * 2;
            h -= borderThickness * 2;

            int right = x + w;
            int bottom = y + h;

            if (isStretchingRight)
                right = layout.getDocument()->snapPosition (right);

            if (isStretchingBottom)
                bottom = layout.getDocument()->snapPosition (bottom);

            if (isStretchingLeft)
                x = layout.getDocument()->snapPosition (x);

            if (isStretchingTop)
                y = layout.getDocument()->snapPosition (y);

            w = (right - x) + borderThickness * 2;
            h = (bottom - y) + borderThickness * 2;
            x -= borderThickness - dx;
            y -= borderThickness - dy;

            b = Rectangle<int> (x, y, w, h);
        }
    }
}

void ComponentOverlayComponent::applyBoundsToComponent (Component& component, Rectangle<int> b)
{
    if (component.getBounds() != b)
    {
        layout.getDocument()->getUndoManager().undoCurrentTransactionOnly();

        auto dX = b.getX() - component.getX();
        auto dY = b.getY() - component.getY();
        auto dW = b.getWidth() - component.getWidth();
        auto dH = b.getHeight() - component.getHeight();

        component.setBounds (b);

        if (auto* parent = target->getParentComponent())
            target->setBounds (b.getX() + borderThickness - parent->getX(),
                               b.getY() + borderThickness - parent->getY(),
                               b.getWidth() - borderThickness * 2,
                               b.getHeight() - borderThickness * 2);

        layout.updateStoredComponentPosition (target, true);

        if (layout.getSelectedSet().getNumSelected() > 1)
        {
            for (auto s : layout.getSelectedSet())
            {
                if (s != target)
                {
                    s->setBounds (s->getX() + dX, s->getY() + dY, s->getWidth() + dW, s->getHeight() + dH);
                    layout.updateStoredComponentPosition (s, true);
                }
            }
        }
    }
}

void ComponentOverlayComponent::showPopupMenu()
{
    ComponentTypeHandler::getHandlerFor (*target)->showPopupMenu (target, layout);
}
