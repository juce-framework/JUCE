/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "../../Application/jucer_Application.h"
#include "../jucer_PaintRoutine.h"
#include "../jucer_UtilityFunctions.h"
#include "../UI/jucer_JucerCommandIDs.h"
#include "../UI/jucer_PaintRoutineEditor.h"
#include "../Properties/jucer_PositionPropertyBase.h"
#include "jucer_ElementSiblingComponent.h"
#include "jucer_PaintElementUndoableAction.h"


//==============================================================================
PaintElement::PaintElement (PaintRoutine* owner_,
                            const String& typeName_)
    : borderThickness (4),
      owner (owner_),
      typeName (typeName_),
      selected (false),
      dragging (false),
      originalAspectRatio (1.0)
{
    setRepaintsOnMouseActivity (true);

    position.rect.setWidth (100);
    position.rect.setHeight (100);

    setMinimumOnscreenAmounts (0, 0, 0, 0);
    setSizeLimits (borderThickness * 2 + 1, borderThickness * 2 + 1, 8192, 8192);

    border.reset (new ResizableBorderComponent (this, this));
    addChildComponent (border.get());

    border->setBorderThickness (BorderSize<int> (borderThickness));

    if (owner != nullptr)
        owner->getSelectedElements().addChangeListener (this);

    selfChangeListenerList.addChangeListener (this);
    siblingComponentsChanged();
}

PaintElement::~PaintElement()
{
    siblingComponents.clear();

    if (owner != nullptr)
    {
        owner->getSelectedElements().deselect (this);
        owner->getSelectedElements().removeChangeListener (this);
    }
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

    RelativePositionedRectangle newState, oldState;
};

class ChangePaintElementBoundsAction    : public PaintElementUndoableAction <PaintElement>
{
public:
    ChangePaintElementBoundsAction (PaintElement* const element, const Rectangle<int>& bounds)
        : PaintElementUndoableAction <PaintElement> (element),
          newBounds (bounds),
          oldBounds (element->getBounds())
    {
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setBounds (newBounds);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setBounds (oldBounds);
        return true;
    }

private:
    Rectangle<int> newBounds, oldBounds;
};

class ChangePaintElementBoundsAndPropertiesAction    : public PaintElementUndoableAction <PaintElement>
{
public:
    ChangePaintElementBoundsAndPropertiesAction (PaintElement* const element, const Rectangle<int>& bounds,
                                                 const NamedValueSet& props)
        : PaintElementUndoableAction <PaintElement> (element),
          newBounds (bounds),
          oldBounds (element->getBounds()),
          newProps (props),
          oldProps (element->getProperties())
    {
    }

    bool perform()
    {
        showCorrectTab();

        if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getElement()->getParentComponent()))
            getElement()->setCurrentBounds (newBounds, pe->getComponentArea(), false);

        getElement()->getProperties() = newProps;
        return true;
    }

    bool undo()
    {
        showCorrectTab();

        if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getElement()->getParentComponent()))
            getElement()->setCurrentBounds (oldBounds, pe->getComponentArea(), false);

        getElement()->getProperties() = oldProps;
        return true;
    }

private:
    Rectangle<int> newBounds, oldBounds;
    NamedValueSet newProps, oldProps;
};

void PaintElement::setPosition (const RelativePositionedRectangle& newPosition, const bool undoable)
{
    if (position != newPosition)
    {
        if (undoable)
        {
            perform (new PaintElementMoveAction (this, newPosition),
                     "Move " + getTypeName());
        }
        else
        {
            position = newPosition;

            if (owner != nullptr)
                owner->changed();
        }
    }
}

void PaintElement::setPaintElementBounds (const Rectangle<int>& newBounds, const bool undoable)
{
    if (getBounds() != newBounds)
    {
        if (undoable)
        {
            perform (new ChangePaintElementBoundsAction (this, newBounds), "Change paint element bounds");
        }
        else
        {
            setBounds (newBounds);
            changed();
        }
    }
}

void PaintElement::setPaintElementBoundsAndProperties (PaintElement* elementToPosition, const Rectangle<int>& newBounds,
                                                       PaintElement* referenceElement, const bool undoable)
{
    auto props = NamedValueSet (elementToPosition->getProperties());

    auto rect = elementToPosition->getPosition().rect;
    auto referenceElementPosition = referenceElement->getPosition();
    auto referenceElementRect = referenceElementPosition.rect;

    rect.setModes (referenceElementRect.getAnchorPointX(), referenceElementRect.getPositionModeX(),
                   referenceElementRect.getAnchorPointY(), referenceElementRect.getPositionModeY(),
                   referenceElementRect.getWidthMode(),    referenceElementRect.getHeightMode(),
                   elementToPosition->getBounds());

    props.set ("pos",         rect.toString());
    props.set ("relativeToX", String::toHexString (referenceElementPosition.relativeToX));
    props.set ("relativeToY", String::toHexString (referenceElementPosition.relativeToY));
    props.set ("relativeToW", String::toHexString (referenceElementPosition.relativeToW));
    props.set ("relativeToH", String::toHexString (referenceElementPosition.relativeToH));

    if (elementToPosition->getBounds() != newBounds || elementToPosition->getProperties() != props)
    {
        if (undoable)
        {
            perform (new ChangePaintElementBoundsAndPropertiesAction (elementToPosition, newBounds, props),
                     "Change paint element bounds");
        }
        else
        {
            if (auto* pe = dynamic_cast<PaintRoutineEditor*> (elementToPosition->getParentComponent()))
                elementToPosition->setCurrentBounds (newBounds, pe->getComponentArea(), false);

            elementToPosition->getProperties() = props;
            owner->changed();
        }
    }
}

//==============================================================================
Rectangle<int> PaintElement::getCurrentBounds (const Rectangle<int>& parentArea) const
{
    return position.getRectangle (parentArea, getDocument()->getComponentLayout());
}

void PaintElement::setCurrentBounds (const Rectangle<int>& newBounds,
                                     const Rectangle<int>& parentArea,
                                     const bool undoable)
{
    RelativePositionedRectangle pr (position);
    pr.updateFrom (newBounds.getX() - parentArea.getX(),
                   newBounds.getY() - parentArea.getY(),
                   jmax (1, newBounds.getWidth()),
                   jmax (1, newBounds.getHeight()),
                   Rectangle<int> (0, 0, parentArea.getWidth(), parentArea.getHeight()),
                   getDocument()->getComponentLayout());

    setPosition (pr, undoable);

    updateBounds (parentArea);
}

void PaintElement::updateBounds (const Rectangle<int>& parentArea)
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
    ElementPositionProperty (PaintElement* e, const String& name,
                             ComponentPositionDimension dimension_)
       : PositionPropertyBase (e, name, dimension_, true, false,
                               e->getDocument()->getComponentLayout()),
         listener (e),
         element (e)
    {
        listener.setPropertyToRefresh (*this);
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        if (element->getOwner()->getSelectedElements().getNumSelected() > 1)
            positionOtherSelectedElements (getPosition(), newPos);

        listener.owner->setPosition (newPos, true);
    }

    RelativePositionedRectangle getPosition() const
    {
        return listener.owner->getPosition();
    }

private:
    ElementListener<PaintElement> listener;
    PaintElement* element;

    void positionOtherSelectedElements (const RelativePositionedRectangle& oldPos, const RelativePositionedRectangle& newPos)
    {
        for (auto* s : element->getOwner()->getSelectedElements())
        {
            if (s != element)
            {
                auto currentPos = s->getPosition();
                auto diff = 0.0;

                if (dimension == ComponentPositionDimension::componentX)
                {
                    diff = newPos.rect.getX() - oldPos.rect.getX();
                    currentPos.rect.setX (currentPos.rect.getX() + diff);
                }
                else if (dimension == ComponentPositionDimension::componentY)
                {
                    diff = newPos.rect.getY() - oldPos.rect.getY();
                    currentPos.rect.setY (currentPos.rect.getY() + diff);
                }
                else if (dimension == ComponentPositionDimension::componentWidth)
                {
                    diff = newPos.rect.getWidth() - oldPos.rect.getWidth();
                    currentPos.rect.setWidth (currentPos.rect.getWidth() + diff);
                }
                else if (dimension == ComponentPositionDimension::componentHeight)
                {
                    diff = newPos.rect.getHeight() - oldPos.rect.getHeight();
                    currentPos.rect.setHeight (currentPos.rect.getHeight() + diff);
                }

                s->setPosition (currentPos, true);
            }
        }
    }
};

//==============================================================================
void PaintElement::getEditableProperties (Array <PropertyComponent*>& props, bool multipleSelected)
{
    ignoreUnused (multipleSelected);

    props.add (new ElementPositionProperty (this, "x", PositionPropertyBase::componentX));
    props.add (new ElementPositionProperty (this, "y", PositionPropertyBase::componentY));
    props.add (new ElementPositionProperty (this, "width", PositionPropertyBase::componentWidth));
    props.add (new ElementPositionProperty (this, "height", PositionPropertyBase::componentHeight));
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
void PaintElement::drawExtraEditorGraphics (Graphics&, const Rectangle<int>& /*relativeTo*/)
{
}

void PaintElement::paint (Graphics& g)
{
    if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
    {
        auto area = pe->getComponentArea();

        g.saveState();
        g.setOrigin (area.getPosition() - Component::getPosition());
        area.setPosition (0, 0);

        g.saveState();
        g.reduceClipRegion (0, 0, area.getWidth(), area.getHeight());

        draw (g, getDocument()->getComponentLayout(), area);

        g.restoreState();

        drawExtraEditorGraphics (g, area);
        g.restoreState();

        if (selected)
        {
            const BorderSize<int> borderSize (border->getBorderThickness());
            auto baseColour = findColour (defaultHighlightColourId);

            drawResizableBorder (g, getWidth(), getHeight(), borderSize,
                                 (isMouseOverOrDragging() || border->isMouseOverOrDragging()),
                                 baseColour.withAlpha (owner->getSelectedElements().getSelectedItem (0) == this ? 1.0f : 0.3f));
        }
        else if (isMouseOverOrDragging())
        {
            drawMouseOverCorners (g, getWidth(), getHeight());
        }
    }
}

void PaintElement::resized()
{
    border->setBounds (getLocalBounds());
}

void PaintElement::mouseDown (const MouseEvent& e)
{
    dragging = false;

    if (owner != nullptr)
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
        if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
        {
            auto area = pe->getComponentArea();

            if (selected && ! dragging)
            {
                dragging = e.mouseWasDraggedSinceMouseDown();

                if (dragging)
                    owner->startDragging (area);
            }

            if (dragging)
                owner->dragSelectedComps (e.getDistanceFromDragStartX(),
                                          e.getDistanceFromDragStartY(),
                                          area);
        }
    }
}

void PaintElement::mouseUp (const MouseEvent& e)
{
    if (owner != nullptr)
    {
        if (dragging)
            owner->endDragging();

        if (owner != nullptr)
            owner->getSelectedElements().addToSelectionOnMouseUp (this, e.mods, dragging, mouseDownSelectStatus);
    }
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

void PaintElement::checkBounds (Rectangle<int>& b,
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

    if (auto* document = getDocument())
    {
        if (document->isSnapActive (true))
        {
            if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
            {
                auto area = pe->getComponentArea();

                int x = b.getX();
                int y = b.getY();
                int w = b.getWidth();
                int h = b.getHeight();

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

                b = { x, y, w, h };
            }
        }
    }
}

void PaintElement::applyBoundsToComponent (Component&, Rectangle<int> newBounds)
{
    if (getBounds() != newBounds)
    {
        getDocument()->getUndoManager().undoCurrentTransactionOnly();

        auto dX = newBounds.getX() - getX();
        auto dY = newBounds.getY() - getY();
        auto dW = newBounds.getWidth() - getWidth();
        auto dH = newBounds.getHeight() - getHeight();

        if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
            setCurrentBounds (newBounds.expanded (-borderThickness, -borderThickness),
                              pe->getComponentArea(), true);

        if (owner->getSelectedElements().getNumSelected() > 1)
        {
            for (auto selectedElement : owner->getSelectedElements())
            {
                if (selectedElement != nullptr && selectedElement != this)
                {
                    if (auto* pe = dynamic_cast<PaintRoutineEditor*> (selectedElement->getParentComponent()))
                    {
                        Rectangle<int> r { selectedElement->getX() + dX, selectedElement->getY() + dY,
                                           selectedElement->getWidth() + dW, selectedElement->getHeight() + dH };

                        selectedElement->setCurrentBounds (r.expanded (-borderThickness, -borderThickness),
                                                           pe->getComponentArea(), true);
                    }
                }
            }
        }
    }
}

Rectangle<int> PaintElement::getCurrentAbsoluteBounds() const
{
    if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
        return position.getRectangle (pe->getComponentArea(), getDocument()->getComponentLayout());

    return {};
}

void PaintElement::getCurrentAbsoluteBoundsDouble (double& x, double& y, double& w, double& h) const
{
    if (auto* pe = dynamic_cast<PaintRoutineEditor*> (getParentComponent()))
        position.getRectangleDouble (x, y, w, h, pe->getComponentArea(), getDocument()->getComponentLayout());
}

void PaintElement::changeListenerCallback (ChangeBroadcaster*)
{
    const bool nowSelected = owner != nullptr && owner->getSelectedElements().isSelected (this);

    if (selected != nowSelected)
    {
        selected = nowSelected;
        border->setVisible (nowSelected);
        repaint();

        selectionChanged (nowSelected);
    }

    updateSiblingComps();
}

void PaintElement::selectionChanged (const bool /*isSelected*/)
{
}

void PaintElement::createSiblingComponents()
{
}

void PaintElement::siblingComponentsChanged()
{
    siblingComponents.clear();
    selfChangeListenerList.sendChangeMessage();
}

void PaintElement::updateSiblingComps()
{
    if (selected && getParentComponent() != nullptr && owner->getSelectedElements().getNumSelected() == 1)
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
    auto* commandManager = &ProjucerApplication::getCommandManager();

    PopupMenu m;

    m.addCommandItem (commandManager, JucerCommandIDs::toFront);
    m.addCommandItem (commandManager, JucerCommandIDs::toBack);
    m.addSeparator();

    if (owner != nullptr && owner->getSelectedElements().getNumSelected() > 1)
    {
        m.addCommandItem (commandManager, JucerCommandIDs::alignTop);
        m.addCommandItem (commandManager, JucerCommandIDs::alignRight);
        m.addCommandItem (commandManager, JucerCommandIDs::alignBottom);
        m.addCommandItem (commandManager, JucerCommandIDs::alignLeft);
        m.addSeparator();
    }

    m.addCommandItem (commandManager, StandardApplicationCommandIDs::cut);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::copy);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::paste);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::del);

    m.showMenuAsync ({});
}
