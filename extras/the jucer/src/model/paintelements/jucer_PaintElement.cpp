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

#include "../../jucer_Headers.h"
#include "../jucer_PaintRoutine.h"
#include "../../ui/jucer_PaintRoutineEditor.h"
#include "../../properties/jucer_PositionPropertyBase.h"
#include "jucer_ElementSiblingComponent.h"
#include "jucer_PaintElementUndoableAction.h"


//==============================================================================
PaintElement::PaintElement (PaintRoutine* owner_,
                            const String& typeName_)
    : owner (owner_),
      typeName (typeName_),
      selected (false),
      dragging (false),
      borderThickness (4),
      originalAspectRatio (1.0)
{
    setRepaintsOnMouseActivity (true);

    position.rect.setWidth (100);
    position.rect.setHeight (100);

    setMinimumOnscreenAmounts (0, 0, 0, 0);
    setSizeLimits (borderThickness * 2 + 1, borderThickness * 2 + 1, 8192, 8192);

    addChildComponent (border = new ResizableBorderComponent (this, this));

    border->setBorderThickness (BorderSize (borderThickness));

    if (owner != 0)
        owner->getSelectedElements().addChangeListener (this);

    selfChangeListenerList.addChangeListener (this);
    siblingComponentsChanged();
}

PaintElement::~PaintElement()
{
    siblingComponents.clear();

    if (owner != 0)
        owner->getSelectedElements().removeChangeListener (this);

    delete border;
}


//==============================================================================
void PaintElement::setInitialBounds (int parentWidth, int parentHeight)
{
    RelativePositionedRectangle pr (getPosition());
    pr.rect.setX (parentWidth / 4 + Random::getSystemRandom().nextInt (parentWidth / 4) - parentWidth / 8);
    pr.rect.setY (parentHeight / 3 + Random::getSystemRandom().nextInt (parentHeight / 4) - parentHeight / 8);
    setPosition (pr, false);
}

//==============================================================================
const RelativePositionedRectangle& PaintElement::getPosition() const
{
    return position;
}

class PaintElementMoveAction  : public PaintElementUndoableAction <PaintElement>
{
public:
    PaintElementMoveAction (PaintElement* const element, const RelativePositionedRectangle& newState_)
        : PaintElementUndoableAction <PaintElement> (element),
          newState (newState_),
          oldState (element->getPosition())
    {
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setPosition (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setPosition (oldState, false);
        return true;
    }

private:
    RelativePositionedRectangle newState, oldState;
};

void PaintElement::setPosition (const RelativePositionedRectangle& newPosition, const bool undoable)
{
    if (position != newPosition)
    {
        if (undoable)
        {
            perform (new PaintElementMoveAction (this, newPosition),
                     T("Move ") + getTypeName());
        }
        else
        {
            position = newPosition;

            if (owner != 0)
                owner->changed();
        }
    }
}

//==============================================================================
const Rectangle PaintElement::getCurrentBounds (const Rectangle& parentArea) const
{
    return position.getRectangle (parentArea, getDocument()->getComponentLayout());
}

void PaintElement::setCurrentBounds (const Rectangle& newBounds,
                                     const Rectangle& parentArea,
                                     const bool undoable)
{
    RelativePositionedRectangle pr (position);
    pr.updateFrom (newBounds.getX() - parentArea.getX(),
                   newBounds.getY() - parentArea.getY(),
                   jmax (1, newBounds.getWidth()),
                   jmax (1, newBounds.getHeight()),
                   Rectangle (0, 0, parentArea.getWidth(), parentArea.getHeight()),
                   getDocument()->getComponentLayout());

    setPosition (pr, undoable);

    updateBounds (parentArea);
}

void PaintElement::updateBounds (const Rectangle& parentArea)
{
    if (! parentArea.isEmpty())
    {
        setBounds (getCurrentBounds (parentArea)
                        .expanded (borderThickness,
                                   borderThickness));

        for (int i = siblingComponents.size(); --i >= 0;)
            siblingComponents.getUnchecked(i)->updatePosition();
    }
}

//==============================================================================
class ElementPositionProperty   : public PositionPropertyBase
{
public:
    //==============================================================================
    ElementPositionProperty (PaintElement* element_,
                             const String& name,
                             ComponentPositionDimension dimension_)
       : PositionPropertyBase (element_, name, dimension_, true, false,
                               element_->getDocument()->getComponentLayout()),
         element (element_)
    {
        element_->getDocument()->addChangeListener (this);
    }

    ~ElementPositionProperty()
    {
        element->getDocument()->removeChangeListener (this);
    }

    //==============================================================================
    void setPosition (const RelativePositionedRectangle& newPos)
    {
        element->setPosition (newPos, true);
    }

    const RelativePositionedRectangle getPosition() const
    {
        return element->getPosition();
    }

private:
    PaintElement* const element;
};

//==============================================================================
void PaintElement::getEditableProperties (Array <PropertyComponent*>& properties)
{
    properties.add (new ElementPositionProperty (this, T("x"), PositionPropertyBase::componentX));
    properties.add (new ElementPositionProperty (this, T("y"), PositionPropertyBase::componentY));
    properties.add (new ElementPositionProperty (this, T("width"), PositionPropertyBase::componentWidth));
    properties.add (new ElementPositionProperty (this, T("height"), PositionPropertyBase::componentHeight));
}

//==============================================================================
JucerDocument* PaintElement::getDocument() const
{
    return owner->getDocument();
}

void PaintElement::changed()
{
    repaint();
    owner->changed();
}

bool PaintElement::perform (UndoableAction* action, const String& actionName)
{
    return owner->perform (action, actionName);
}

void PaintElement::parentHierarchyChanged()
{
    updateSiblingComps();
}

//==============================================================================
void PaintElement::drawExtraEditorGraphics (Graphics& g, const Rectangle& relativeTo)
{
}

void PaintElement::paint (Graphics& g)
{
    Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

    g.saveState();
    g.setOrigin (area.getX() - getX(), area.getY() - getY());
    area.setPosition (0, 0);

    g.saveState();
    g.reduceClipRegion (0, 0, area.getWidth(), area.getHeight());

    draw (g, getDocument()->getComponentLayout(), area);

    g.restoreState();

    drawExtraEditorGraphics (g, area);
    g.restoreState();

    if (selected)
    {
        const BorderSize borderSize (border->getBorderThickness());

        drawResizableBorder (g, getWidth(), getHeight(), borderSize,
                             (isMouseOverOrDragging() || border->isMouseOverOrDragging()));
    }
    else if (isMouseOverOrDragging())
    {
        drawMouseOverCorners (g, getWidth(), getHeight());
    }
}

void PaintElement::resized()
{
    border->setBounds (0, 0, getWidth(), getHeight());
}

void PaintElement::mouseDown (const MouseEvent& e)
{
    dragging = false;

    if (owner != 0)
    {
        owner->getSelectedPoints().deselectAll();
        mouseDownSelectStatus = owner->getSelectedElements().addToSelectionOnMouseDown (this, e.mods);
    }

    if (e.mods.isPopupMenu())
    {
        showPopupMenu();
        return; // this may be deleted now..
    }
}

void PaintElement::mouseDrag (const MouseEvent& e)
{
    if (! e.mods.isPopupMenu())
    {
        jassert (dynamic_cast <PaintRoutineEditor*> (getParentComponent()) != 0);
        const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

        if (selected && ! dragging)
        {
            dragging = ! e.mouseWasClicked();

            if (dragging)
                owner->startDragging (area);
        }

        if (dragging)
            owner->dragSelectedComps (e.getDistanceFromDragStartX(),
                                      e.getDistanceFromDragStartY(),
                                      area);
    }
}

void PaintElement::mouseUp (const MouseEvent& e)
{
    if (dragging)
        owner->endDragging();

    if (owner != 0)
        owner->getSelectedElements().addToSelectionOnMouseUp (this, e.mods, dragging, mouseDownSelectStatus);
}

void PaintElement::resizeStart()
{
    if (getHeight() > 0)
        originalAspectRatio = getWidth() / (double) getHeight();
    else
        originalAspectRatio = 1.0;
}

void PaintElement::resizeEnd()
{
}

void PaintElement::checkBounds (int& x, int& y, int& w, int& h,
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

    JucerDocument* document = getDocument();

    if (document != 0 && document->isSnapActive (true))
    {
        jassert (getParentComponent() != 0);
        const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

        x += borderThickness - area.getX();
        y += borderThickness - area.getY();
        w -= borderThickness * 2;
        h -= borderThickness * 2;

        int right = x + w;
        int bottom = y + h;

        if (isStretchingRight)
            right = document->snapPosition (right);

        if (isStretchingBottom)
            bottom = document->snapPosition (bottom);

        if (isStretchingLeft)
            x = document->snapPosition (x);

        if (isStretchingTop)
            y = document->snapPosition (y);

        w = (right - x) + borderThickness * 2;
        h = (bottom - y) + borderThickness * 2;
        x -= borderThickness - area.getX();
        y -= borderThickness - area.getY();
    }
}

void PaintElement::applyBoundsToComponent (Component* component, int x, int y, int w, int h)
{
    if (getBounds() != Rectangle (x, y, w, h))
    {
        getDocument()->getUndoManager().undoCurrentTransactionOnly();

        jassert (dynamic_cast <PaintRoutineEditor*> (getParentComponent()) != 0);

        setCurrentBounds (Rectangle (x, y, w, h)
                            .expanded (-borderThickness, -borderThickness),
                          ((PaintRoutineEditor*) getParentComponent())->getComponentArea(),
                          true);
    }
}

const Rectangle PaintElement::getCurrentAbsoluteBounds() const
{
    jassert (dynamic_cast <PaintRoutineEditor*> (getParentComponent()) != 0);
    const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

    return position.getRectangle (area, getDocument()->getComponentLayout());
}

void PaintElement::getCurrentAbsoluteBoundsDouble (double& x, double& y, double& w, double& h) const
{
    jassert (dynamic_cast <PaintRoutineEditor*> (getParentComponent()) != 0);
    const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

    position.getRectangleDouble (x, y, w, h, area, getDocument()->getComponentLayout());
}

void PaintElement::changeListenerCallback (void*)
{
    const bool nowSelected = owner != 0 && owner->getSelectedElements().isSelected (this);

    if (selected != nowSelected)
    {
        selected = nowSelected;
        border->setVisible (nowSelected);
        repaint();

        selectionChanged (nowSelected);
    }

    updateSiblingComps();
}

void PaintElement::selectionChanged (const bool isSelected)
{
}

void PaintElement::createSiblingComponents()
{
}

void PaintElement::siblingComponentsChanged()
{
    siblingComponents.clear();
    selfChangeListenerList.sendChangeMessage (0);
}

void PaintElement::updateSiblingComps()
{
    if (selected && getParentComponent() != 0 && owner->getSelectedElements().getNumSelected() == 1)
    {
        if (siblingComponents.size() == 0)
            createSiblingComponents();

        for (int i = siblingComponents.size(); --i >= 0;)
            siblingComponents.getUnchecked(i)->updatePosition();
    }
    else
    {
        siblingComponents.clear();
    }
}


void PaintElement::showPopupMenu()
{
    PopupMenu m;

    m.addCommandItem (commandManager, CommandIDs::toFront);
    m.addCommandItem (commandManager, CommandIDs::toBack);
    m.addSeparator();
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::cut);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::copy);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::paste);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::del);

    m.show();
}
