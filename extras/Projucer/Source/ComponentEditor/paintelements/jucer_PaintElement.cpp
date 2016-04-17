/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#include "../../jucer_Headers.h"
#include "../../Application/jucer_Application.h"
#include "../jucer_PaintRoutine.h"
#include "../jucer_UtilityFunctions.h"
#include "../ui/jucer_JucerCommandIDs.h"
#include "../ui/jucer_PaintRoutineEditor.h"
#include "../properties/jucer_PositionPropertyBase.h"
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

    addChildComponent (border = new ResizableBorderComponent (this, this));

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
         listener (e)
    {
        listener.setPropertyToRefresh (*this);
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        listener.owner->setPosition (newPos, true);
    }

    RelativePositionedRectangle getPosition() const
    {
        return listener.owner->getPosition();
    }

    ElementListener<PaintElement> listener;
};

//==============================================================================
void PaintElement::getEditableProperties (Array <PropertyComponent*>& props)
{
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
    Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

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
        jassert (dynamic_cast<PaintRoutineEditor*> (getParentComponent()) != nullptr);
        const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

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

void PaintElement::mouseUp (const MouseEvent& e)
{
    if (dragging)
        owner->endDragging();

    if (owner != nullptr)
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

void PaintElement::checkBounds (Rectangle<int>& b,
                                const Rectangle<int>& previousBounds,
                                const Rectangle<int>& limits,
                                const bool isStretchingTop,
                                const bool isStretchingLeft,
                                const bool isStretchingBottom,
                                const bool isStretchingRight)
{
    if (ModifierKeys::getCurrentModifiers().isShiftDown())
        setFixedAspectRatio (originalAspectRatio);
    else
        setFixedAspectRatio (0.0);

    ComponentBoundsConstrainer::checkBounds (b, previousBounds, limits, isStretchingTop, isStretchingLeft, isStretchingBottom, isStretchingRight);

    JucerDocument* document = getDocument();

    if (document != nullptr && document->isSnapActive (true))
    {
        jassert (getParentComponent() != nullptr);
        const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

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

        b = Rectangle<int> (x, y, w, h);
    }
}

void PaintElement::applyBoundsToComponent (Component*, const Rectangle<int>& newBounds)
{
    if (getBounds() != newBounds)
    {
        getDocument()->getUndoManager().undoCurrentTransactionOnly();

        jassert (dynamic_cast<PaintRoutineEditor*> (getParentComponent()) != nullptr);

        setCurrentBounds (newBounds.expanded (-borderThickness, -borderThickness),
                          ((PaintRoutineEditor*) getParentComponent())->getComponentArea(),
                          true);
    }
}

Rectangle<int> PaintElement::getCurrentAbsoluteBounds() const
{
    jassert (dynamic_cast<PaintRoutineEditor*> (getParentComponent()) != nullptr);
    const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

    return position.getRectangle (area, getDocument()->getComponentLayout());
}

void PaintElement::getCurrentAbsoluteBoundsDouble (double& x, double& y, double& w, double& h) const
{
    jassert (dynamic_cast<PaintRoutineEditor*> (getParentComponent()) != nullptr);
    const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

    position.getRectangleDouble (x, y, w, h, area, getDocument()->getComponentLayout());
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
    ApplicationCommandManager* commandManager = &ProjucerApplication::getCommandManager();

    PopupMenu m;

    m.addCommandItem (commandManager, JucerCommandIDs::toFront);
    m.addCommandItem (commandManager, JucerCommandIDs::toBack);
    m.addSeparator();
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::cut);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::copy);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::paste);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::del);

    m.show();
}
