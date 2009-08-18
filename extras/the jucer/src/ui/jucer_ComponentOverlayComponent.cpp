/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_ComponentLayoutEditor.h"


//==============================================================================
ComponentOverlayComponent::ComponentOverlayComponent (Component* const target_,
                                                      ComponentLayout& layout_)
    : target (target_),
      borderThickness (4),
      deletionWatcher (target_),
      layout (layout_),
      selected (false),
      dragging (false),
      originalAspectRatio (1.0)
{
    setMinimumOnscreenAmounts (0, 0, 0, 0);
    setSizeLimits (borderThickness * 2 + 2, borderThickness * 2 + 2, 8192, 8192);

    addChildComponent (border = new ResizableBorderComponent (this, this));

    border->setBorderThickness (BorderSize (borderThickness));

    target->addComponentListener (this);

    changeListenerCallback (0);
    layout.getSelectedSet().addChangeListener (this);

    setRepaintsOnMouseActivity (true);
    border->setRepaintsOnMouseActivity (true);
}

ComponentOverlayComponent::~ComponentOverlayComponent()
{
    layout.getSelectedSet().removeChangeListener (this);

    if (! deletionWatcher.hasBeenDeleted())
        target->removeComponentListener (this);

    delete border;
}

void ComponentOverlayComponent::changeListenerCallback (void*)
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
    jassert (! deletionWatcher.hasBeenDeleted());

    if (selected)
    {
        const BorderSize borderSize (border->getBorderThickness());

        drawResizableBorder (g, getWidth(), getHeight(), borderSize, (isMouseOverOrDragging() || border->isMouseOverOrDragging()));
    }
    else if (isMouseOverOrDragging())
    {
        drawMouseOverCorners (g, getWidth(), getHeight());
    }
}

void ComponentOverlayComponent::resized()
{
    jassert (! deletionWatcher.hasBeenDeleted());

    border->setBounds (0, 0, getWidth(), getHeight());
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
            dragging = ! e.mouseWasClicked();

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

void ComponentOverlayComponent::componentMovedOrResized (Component& component, bool wasMoved, bool wasResized)
{
    updateBoundsToMatchTarget();
}

void ComponentOverlayComponent::updateBoundsToMatchTarget()
{
    Component* const parent = target->getParentComponent();
    jassert (parent != 0);

    if (parent != 0)
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

    layout.getDocument()->getUndoManager().beginNewTransaction (T("Resize components"));
}

void ComponentOverlayComponent::resizeEnd()
{
    layout.getDocument()->getUndoManager().beginNewTransaction();
}

void ComponentOverlayComponent::checkBounds (int& x, int& y, int& w, int& h,
                                             const Rectangle& previousBounds,
                                             const Rectangle& limits,
                                             const bool isStretchingTop,
                                             const bool isStretchingLeft,
                                             const bool isStretchingBottom,
                                             const bool isStretchingRight)
{
    if (ModifierKeys::getCurrentModifiers().isShiftDown())
        setFixedAspectRatio (originalAspectRatio);
    else
        setFixedAspectRatio (0.0);

    ComponentBoundsConstrainer::checkBounds (x, y, w, h, previousBounds, limits, isStretchingTop, isStretchingLeft, isStretchingBottom, isStretchingRight);

    if (layout.getDocument()->isSnapActive (true))
    {
        Component* const parent = target->getParentComponent();
        jassert (parent != 0);

        if (parent != 0)
        {
            const int dx = parent->getX();
            const int dy = parent->getY();

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
        }
    }
}

void ComponentOverlayComponent::applyBoundsToComponent (Component* component, int x, int y, int w, int h)
{
    if (component->getBounds() != Rectangle (x, y, w, h))
    {
        layout.getDocument()->getUndoManager().undoCurrentTransactionOnly();

        component->setBounds (x, y, w, h);

        Component* const parent = target->getParentComponent();
        jassert (parent != 0);

        if (parent != 0)
        {
            target->setBounds (x + borderThickness - parent->getX(),
                               y + borderThickness - parent->getY(),
                               w - borderThickness * 2,
                               h - borderThickness * 2);
        }

        layout.updateStoredComponentPosition (target, true);
    }
}

void ComponentOverlayComponent::showPopupMenu()
{
    ComponentTypeHandler::getHandlerFor (*target)->showPopupMenu (target, layout);
}
