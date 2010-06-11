/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
#include "../../utility/jucer_TickIterator.h"
#include "jucer_EditorCanvas.h"
#include "jucer_EditorPanel.h"


//==============================================================================
class EditorCanvasBase::ResizeFrame    : public EditorCanvasBase::OverlayItemComponent
{
public:
    ResizeFrame (EditorCanvasBase* canvas_, const String& objectId_, const ValueTree& objectState_)
        : OverlayItemComponent (canvas_),
          objectState (objectState_),
          objectId (objectId_),
          borderThickness (4),
          isDragging (false),
          isRotating (false),
          canRotate (canvas_->canRotate())
    {
        jassert (objectState.isValid());
    }

    ~ResizeFrame()
    {
    }

    void paint (Graphics& g)
    {
        if (! canvas->isRotating())
        {
            g.setColour (resizableBorderColour);
            g.drawRect (0, 0, getWidth(), getHeight(), borderThickness);

            g.fillRect (rotateArea);
        }
    }

    void mouseEnter (const MouseEvent& e)                       { updateDragZone (e.getPosition()); }
    void mouseExit (const MouseEvent& e)                        { updateDragZone (e.getPosition()); }
    void mouseMove (const MouseEvent& e)                        { updateDragZone (e.getPosition()); }

    void mouseDown (const MouseEvent& e)
    {
        updateDragZone (e.getPosition());
        isDragging = false;

        if (e.mods.isPopupMenu())
        {
            canvas->showPopupMenu (true);
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (! (isDragging || e.mods.isPopupMenu() || e.mouseWasClicked()))
        {
            isDragging = true;
            bool isRotating = rotateArea.contains (e.getMouseDownPosition());

            canvas->beginDrag (e.withNewPosition (e.getMouseDownPosition()),
                               dragZone, isRotating, canvas->getObjectPosition (objectState).getCentre().toFloat());

            if (! isRotating)
                canvas->showSizeGuides();

            repaint();
        }

        if (isDragging)
        {
            canvas->continueDrag (e);
            autoScrollForMouseEvent (e);
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        if (isDragging || isRotating)
        {
            isRotating = false;
            canvas->hideSizeGuides();
            canvas->endDrag (e);
            updateDragZone (e.getPosition());

            repaint();
        }
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        canvas->objectDoubleClicked (e, objectState);
    }

    bool hitTest (int x, int y)
    {
        if (ModifierKeys::getCurrentModifiers().isAnyModifierKeyDown())
            return rotateArea.contains (x, y) || ! getCentreArea().contains (x, y);

        return true;
    }

    bool updatePosition()
    {
        if (! objectState.getParent().isValid())
            return false;

        const Rectangle<int> bounds (canvas->getObjectPosition (objectState));
        setBoundsInTargetSpace (bounds.expanded (borderThickness, borderThickness));

        if (canRotate)
            rotateArea = Rectangle<int> (2, 2, 10, 10);

        int i;
        for (i = sizeGuides.size(); --i >= 0;)
        {
            sizeGuides.getUnchecked(i)->setVisible (isVisible());
            sizeGuides.getUnchecked(i)->updatePosition (bounds);
        }

        return true;
    }

    const String& getTargetObjectID() const     { return objectId; }

    //==============================================================================
    class SizeGuideComponent   : public OverlayItemComponent,
                                 public ComponentListener
    {
    public:
        enum Type    { left, right, top, bottom };

        //==============================================================================
        SizeGuideComponent (EditorCanvasBase* canvas_, const ValueTree& state_, Type type_)
            : OverlayItemComponent (canvas_), state (state_), type (type_)
        {
            setAlwaysOnTop (true);
            canvas->addAndMakeVisible (this);
            setInterceptsMouseClicks (false, false);
        }

        //==============================================================================
        void paint (Graphics& g)
        {
            const float dashes[] = { 4.0f, 3.0f };

            g.setColour (resizableBorderColour);
            g.drawDashedLine (0.5f, 0.5f, getWidth() - 0.5f, getHeight() - 0.5f, dashes, 2, 1.0f);
        }

        //==============================================================================
        void updatePosition (const Rectangle<int>& bounds)
        {
            RelativeRectangle coords (canvas->getObjectCoords (state));
            RelativeCoordinate coord;
            Rectangle<int> r;

            switch (type)
            {
                case left:    coord = coords.left;   r.setBounds (bounds.getX(), 0, 1, bounds.getY()); break;
                case right:   coord = coords.right;  r.setBounds (bounds.getRight(), 0, 1, bounds.getY()); break;
                case top:     coord = coords.top;    r.setBounds (0, bounds.getY(), bounds.getX(), 1); break;
                case bottom:  coord = coords.bottom; r.setBounds (0, bounds.getBottom(), bounds.getX(), 1); break;
                default:      jassertfalse; break;
            }

            setBoundsInTargetSpace (r);
            label.update (getParentComponent(), coord.toString(), Colours::darkgrey, getX(), getY(), type != left, type != top);
        }

    private:
        ValueTree state;
        Type type;
        FloatingLabelComponent label;
    };

    void showSizeGuides()
    {
        if (sizeGuides.size() == 0 && canvas->hasSizeGuides())
        {
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::left));
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::right));
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::top));
            sizeGuides.add (new SizeGuideComponent (canvas, objectState, SizeGuideComponent::bottom));
        }
    }

    void hideSizeGuides()
    {
        sizeGuides.clear();
    }

private:
    ValueTree objectState;
    String objectId;
    ResizableBorderComponent::Zone dragZone;
    const int borderThickness;
    OwnedArray <SizeGuideComponent> sizeGuides;
    Rectangle<int> rotateArea;
    bool isDragging, canRotate, isRotating;

    const Rectangle<int> getCentreArea() const
    {
        return getLocalBounds().reduced (borderThickness, borderThickness);
    }

    void updateDragZone (const Point<int>& p)
    {
        ResizableBorderComponent::Zone newZone
            = ResizableBorderComponent::Zone::fromPositionOnBorder (getLocalBounds(),
                                                                    BorderSize (borderThickness), p);

        if (dragZone != newZone)
        {
            dragZone = newZone;
            setMouseCursor (newZone.getMouseCursor());
        }
    }
};

//==============================================================================
class EditorCanvasBase::OverlayComponent  : public Component,
                                            public LassoSource <SelectedItems::ItemType>,
                                            public ChangeListener
{
public:
    OverlayComponent (EditorCanvasBase* canvas_)
        : canvas (canvas_)
    {
        setWantsKeyboardFocus (true);
        getSelection().addChangeListener (this);
    }

    ~OverlayComponent()
    {
        getSelection().removeChangeListener (this);
        lasso = 0;
        resizers.clear();
        controlPoints.clear();
        deleteAllChildren();
    }

    //==============================================================================
    void mouseDown (const MouseEvent& e)
    {
        lasso = 0;
        mouseDownCompUID = SelectedItems::ItemType();
        isDraggingClickedComp = false;

        const MouseEvent e2 (e.getEventRelativeTo (canvas->getComponentHolder()));
        const SelectedItems::ItemType underMouse (canvas->findObjectIdAt (canvas->screenSpaceToObjectSpace (e2.getPosition())));

        if (e.mods.isPopupMenu())
        {
            if (underMouse.isNotEmpty() && ! getSelection().isSelected (underMouse))
            {
                canvas->enableResizingMode();
                getSelection().selectOnly (underMouse);
            }

            canvas->showPopupMenu (underMouse.isNotEmpty());
        }
        else
        {
            if (underMouse.isEmpty() || e.mods.isAltDown())
            {
                canvas->deselectNonDraggableObjects();
                addAndMakeVisible (lasso = new LassoComponent <SelectedItems::ItemType>());
                lasso->beginLasso (e, this);
            }
            else
            {
                mouseDownCompUID = underMouse;
                canvas->deselectNonDraggableObjects();
                canvas->enableResizingMode();
                mouseDownResult = getSelection().addToSelectionOnMouseDown (mouseDownCompUID, e.mods);

                updateResizeFrames();
                hideSizeGuides();
                showSizeGuides();
            }
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (lasso != 0)
        {
            lasso->dragLasso (e);
        }
        else
        {
            if ((! isDraggingClickedComp)
                && mouseDownCompUID.isNotEmpty()
                && (! e.mouseWasClicked())
                && (! e.mods.isPopupMenu())
                && e.getDistanceFromDragStart() > 7) // whenever this drag occurs, it's selecting the object
                                                     // and beginning a drag, so allow for more wobble than
                                                     // when when dragging an already-selected object
            {
                isDraggingClickedComp = true;
                canvas->enableResizingMode();
                getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, true, mouseDownResult);
                canvas->beginDrag (e.withNewPosition (e.getMouseDownPosition()),
                                   ResizableBorderComponent::Zone (ResizableBorderComponent::Zone::centre),
                                   false, Point<float>());
            }

            if (isDraggingClickedComp)
            {
                canvas->continueDrag (e);
                showSizeGuides();
            }
        }

        autoScrollForMouseEvent (e);
    }

    void mouseUp (const MouseEvent& e)
    {
        hideSizeGuides();

        if (lasso != 0)
        {
            lasso->endLasso();
            lasso = 0;

            if (e.mouseWasClicked())
                getSelection().deselectAll();
        }
        else if (! e.mods.isPopupMenu())
        {
            if (! isDraggingClickedComp)
                getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, ! e.mouseWasClicked(), mouseDownResult);
        }

        canvas->endDrag (e);
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        const MouseEvent e2 (e.getEventRelativeTo (canvas->getComponentHolder()));
        const SelectedItems::ItemType underMouse (canvas->findObjectIdAt (canvas->screenSpaceToObjectSpace (e2.getPosition())));

        if (underMouse.isNotEmpty())
        {
            const ValueTree state (canvas->getObjectState (underMouse));
            canvas->objectDoubleClicked (e2, state);
        }
    }

    void findLassoItemsInArea (Array <SelectedItems::ItemType>& itemsFound, const Rectangle<int>& area)
    {
        const Rectangle<int> sourceArea (area + relativePositionToOtherComponent (canvas->getComponentHolder(), Point<int>()));
        canvas->findLassoItemsInArea (itemsFound, canvas->screenSpaceToObjectSpace (sourceArea));
    }

    SelectedItems& getSelection()               { return canvas->getSelection(); }
    SelectedItems& getLassoSelection()          { return getSelection(); }

    void changeListenerCallback (void*)
    {
        update();
    }

    void modifierKeysChanged (const ModifierKeys&)
    {
        Desktop::getInstance().getMainMouseSource().triggerFakeMove();
    }

    void showSizeGuides()
    {
        if (canvas->hasSizeGuides())
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                ResizeFrame* resizer = dynamic_cast <ResizeFrame*> (getChildComponent(i));
                if (resizer != 0)
                    resizer->showSizeGuides();
            }
        }
    }

    void hideSizeGuides()
    {
        if (canvas->hasSizeGuides())
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                ResizeFrame* resizer = dynamic_cast <ResizeFrame*> (getChildComponent(i));
                if (resizer != 0)
                    resizer->hideSizeGuides();
            }
        }
    }

    void update()
    {
        updateResizeFrames();
        updateControlPoints();
    }

private:
    //==============================================================================
    EditorCanvasBase* canvas;
    ScopedPointer <LassoComponent <SelectedItems::ItemType> > lasso;
    bool mouseDownResult, isDraggingClickedComp;
    SelectedItems::ItemType mouseDownCompUID;
    OwnedArray <ResizeFrame> resizers;
    OwnedArray <OverlayItemComponent> controlPoints;

    void updateResizeFrames()
    {
        if (! canvas->isResizingMode())
        {
            resizers.clear();
            return;
        }

        SelectedItems& selection = getSelection();
        StringArray requiredIds;
        const int num = selection.getNumSelected();

        int i;
        for (i = 0; i < num; ++i)
            requiredIds.add (selection.getSelectedItem(i));

        for (i = resizers.size(); --i >= 0;)
        {
            ResizeFrame* resizer = resizers.getUnchecked(i);
            const int index = requiredIds.indexOf (resizer->getTargetObjectID());

            if (index >= 0)
            {
                if (resizer->updatePosition())
                {
                    requiredIds.remove (index);
                }
                else
                {
                    resizers.remove (i);
                    canvas->getSelection().deselect (requiredIds[i]);
                }
            }
            else
            {
                resizers.remove (i);
            }
        }

        for (i = requiredIds.size(); --i >= 0;)
        {
            const ValueTree state (canvas->getObjectState (requiredIds[i]));

            if (state.isValid()) // (the id may be a marker)
            {
                ResizeFrame* frame = new ResizeFrame (canvas, requiredIds[i], state);
                resizers.add (frame);
                addAndMakeVisible (frame);
                frame->updatePosition();
            }
        }
    }

    void updateControlPoints()
    {
        if (! canvas->isControlPointMode())
        {
            controlPoints.clear();
            return;
        }

        canvas->updateControlPointComponents (this, controlPoints);
    }
};

//==============================================================================
class EditorCanvasBase::DocumentResizeFrame    : public Component
{
public:
    DocumentResizeFrame (EditorCanvasBase* canvas_)
        : canvas (canvas_), resizerThickness (4)
    {
    }

    ~DocumentResizeFrame()
    {
    }

    void paint (Graphics& g)
    {
        const Rectangle<int> content (getContentArea());

        g.setColour (Colour::greyLevel (0.1f).withAlpha (0.3f));
        g.drawRect (content.expanded (1, 1), 1);

        const int bottomGap = getHeight() - content.getBottom();
        g.setFont (bottomGap - 5.0f);

        g.setColour (Colour::greyLevel (0.9f));
        g.drawText (String (content.getWidth()) + " x " + String (content.getHeight()),
                    0, 0, jmax (content.getRight(), jmin (60, getWidth())), getHeight(), Justification::bottomRight, false);
    }

    void mouseMove (const MouseEvent& e)
    {
        updateDragZone (e.getPosition());
    }

    void mouseDown (const MouseEvent& e)
    {
        updateDragZone (e.getPosition());
        dragStartBounds = canvas->getCanvasBounds();
        canvas->showSizeGuides();
    }

    void mouseDrag (const MouseEvent& e)
    {
        Rectangle<int> newBounds (dragStartBounds);

        if (dragZone.isDraggingRightEdge())
            newBounds.setWidth (jmax (1, newBounds.getWidth() + e.getDistanceFromDragStartX()));

        if (dragZone.isDraggingBottomEdge())
            newBounds.setHeight (jmax (1, newBounds.getHeight() + e.getDistanceFromDragStartY()));

        canvas->setCanvasBounds (newBounds);
    }

    void mouseUp (const MouseEvent& e)
    {
        canvas->hideSizeGuides();
        updateDragZone (e.getPosition());
    }

    void updateDragZone (const Point<int>& p)
    {
        ResizableBorderComponent::Zone newZone
            = ResizableBorderComponent::Zone::fromPositionOnBorder (getContentArea().expanded (resizerThickness, resizerThickness),
                                                                    BorderSize (0, 0, resizerThickness, resizerThickness), p);

        if (dragZone != newZone)
        {
            dragZone = newZone;
            setMouseCursor (newZone.getMouseCursor());
        }
    }

    bool hitTest (int x, int y)
    {
        if (! canvas->canResizeCanvas())
            return false;

        const Rectangle<int> content (getContentArea());

        return (x >= content.getRight() || y >= content.getBottom())
                 && (! content.contains (x, y))
                 && content.expanded (resizerThickness, resizerThickness).contains (x, y);
    }

private:
    EditorCanvasBase* canvas;
    ResizableBorderComponent::Zone dragZone;
    Rectangle<int> dragStartBounds;
    const int resizerThickness;

    const Rectangle<int> getContentArea() const     { return canvas->getContentArea(); }
};


//==============================================================================
EditorCanvasBase::EditorCanvasBase()
    : border (8, 8, 14, 14)
{
    //setOpaque (true);
    addChildComponent (&spacebarDragOverlay);
}

EditorCanvasBase::~EditorCanvasBase()
{
    jassert (overlay == 0);
}

void EditorCanvasBase::initialise()
{
    addAndMakeVisible (componentHolder = createComponentHolder());
    addAndMakeVisible (overlay = new OverlayComponent (this));
    overlay->addAndMakeVisible (resizeFrame = new DocumentResizeFrame (this));

    handleAsyncUpdate();
}

void EditorCanvasBase::shutdown()
{
    dragger = 0;
    resizeFrame = 0;
    overlay = 0;
    componentHolder = 0;
}

EditorPanelBase* EditorCanvasBase::getPanel() const
{
    return findParentComponentOfClass ((EditorPanelBase*) 0);
}

const Point<int> EditorCanvasBase::screenSpaceToObjectSpace (const Point<int>& p) const
{
    return p - scale.origin;
}

const Point<int> EditorCanvasBase::objectSpaceToScreenSpace (const Point<int>& p) const
{
    return p + scale.origin;
}

const Point<float> EditorCanvasBase::screenSpaceToObjectSpace (const Point<float>& p) const
{
    return p - scale.origin.toFloat();
}

const Point<float> EditorCanvasBase::objectSpaceToScreenSpace (const Point<float>& p) const
{
    return p + scale.origin.toFloat();
}

const Rectangle<int> EditorCanvasBase::screenSpaceToObjectSpace (const Rectangle<int>& r) const
{
    return r - scale.origin;
}

const Rectangle<int> EditorCanvasBase::objectSpaceToScreenSpace (const Rectangle<int>& r) const
{
    return r + scale.origin;
}

void EditorCanvasBase::enableResizingMode()
{
    enableControlPointMode (ValueTree::invalid);
}

void EditorCanvasBase::enableControlPointMode (const ValueTree& objectToEdit)
{
    if (controlPointEditingTarget != objectToEdit)
    {
        controlPointEditingTarget = objectToEdit;
        getSelection().deselectAll();
        overlay->update();
    }
}

bool EditorCanvasBase::isRotating() const
{
    return dragger != 0 && dragger->isRotating();
}

//==============================================================================
void EditorCanvasBase::paint (Graphics& g)
{
}

bool EditorCanvasBase::keyStateChanged (bool)
{
    return spacebarDragOverlay.updateVisibility();
}

bool EditorCanvasBase::keyPressed (const KeyPress& key)
{
    return key.isKeyCode (KeyPress::spaceKey); // required to consume the spacebar events and avoid a warning beep
}

void EditorCanvasBase::setScale (const Scale& newScale)
{
    jassertfalse;
}

const Rectangle<int> EditorCanvasBase::getContentArea() const
{
    return border.subtractedFrom (getLocalBounds());
}

//==============================================================================
void EditorCanvasBase::handleAsyncUpdate()
{
    documentChanged();

    const Rectangle<int> canvasBounds (getCanvasBounds());

    const Point<int> newOrigin (jmax (0, -canvasBounds.getX()), jmax (0, -canvasBounds.getY()));
    const int newWidth = jmax (canvasBounds.getWidth(), canvasBounds.getRight()) + border.getLeftAndRight();
    const int newHeight = jmax (canvasBounds.getHeight(), canvasBounds.getBottom()) + border.getTopAndBottom();

    if (scale.origin != newOrigin)
    {
        repaint();

        const Point<int> oldOrigin (scale.origin);
        scale.origin = newOrigin;

        setBounds (jmin (0, getX() + oldOrigin.getX() - scale.origin.getX()),
                   jmin (0, getY() + oldOrigin.getY() - scale.origin.getY()),
                   newWidth, newHeight);

        EditorPanelBase* panel = getPanel();
        if (panel != 0)
            panel->updateRulers();
    }
    else if (getWidth() != newWidth || getHeight() != newHeight)
    {
        setSize (newWidth, newHeight);
    }
    else
    {
        overlay->update();

        EditorPanelBase* panel = getPanel();
        if (panel != 0)
            panel->updateMarkers();
    }
}

void EditorCanvasBase::resized()
{
    componentHolder->setBounds (getContentArea());
    overlay->setBounds (getLocalBounds());
    resizeFrame->setBounds (getLocalBounds());
    spacebarDragOverlay.setBounds (getLocalBounds());
    overlay->update();
    handleUpdateNowIfNeeded();
}

//==============================================================================
void EditorCanvasBase::showSizeGuides()   { overlay->showSizeGuides(); }
void EditorCanvasBase::hideSizeGuides()   { overlay->hideSizeGuides(); }


//==============================================================================
void EditorCanvasBase::beginDrag (const MouseEvent& e, const ResizableBorderComponent::Zone& zone,
                                  bool isRotating, const Point<float>& rotationCentre)
{
    dragger = createDragOperation (screenSpaceToObjectSpace (e.getEventRelativeTo (overlay).getPosition()), overlay, zone, isRotating);
    dragger->setRotationCentre (rotationCentre);
    repaint();
}

void EditorCanvasBase::continueDrag (const MouseEvent& e)
{
    MouseEvent e2 (e.getEventRelativeTo (overlay));

    if (dragger != 0)
        dragger->drag (e2, screenSpaceToObjectSpace (e2.getPosition()));
}

void EditorCanvasBase::endDrag (const MouseEvent& e)
{
    if (dragger != 0)
    {
        MouseEvent e2 (e.getEventRelativeTo (overlay));
        dragger->drag (e2, screenSpaceToObjectSpace (e2.getPosition()));
        dragger = 0;

        getUndoManager().beginNewTransaction();

        repaint();
    }
}

//==============================================================================
EditorCanvasBase::OverlayItemComponent::OverlayItemComponent (EditorCanvasBase* canvas_)
    : canvas (canvas_)
{
}

EditorCanvasBase::OverlayItemComponent::~OverlayItemComponent()
{
}

void EditorCanvasBase::OverlayItemComponent::setBoundsInTargetSpace (const Rectangle<int>& r)
{
    setBounds (canvas->objectSpaceToScreenSpace (r)
                + canvas->getComponentHolder()->relativePositionToOtherComponent (getParentComponent(), Point<int>()));
}

const Point<float> EditorCanvasBase::OverlayItemComponent::pointToLocalSpace (const Point<float>& p) const
{
    return canvas->objectSpaceToScreenSpace (p)
            + (canvas->getComponentHolder()->relativePositionToOtherComponent (getParentComponent(), Point<int>())
                - getPosition()).toFloat();
}

//==============================================================================
EditorCanvasBase::SpacebarDragOverlay::SpacebarDragOverlay()
{
    setAlwaysOnTop (true);
    setMouseCursor (MouseCursor::DraggingHandCursor);
}

EditorCanvasBase::SpacebarDragOverlay::~SpacebarDragOverlay()
{
}

bool EditorCanvasBase::SpacebarDragOverlay::updateVisibility()
{
    bool isSpaceDown = KeyPress::isKeyCurrentlyDown (KeyPress::spaceKey);

    if (isSpaceDown == isVisible())
        return false;

    setVisible (isSpaceDown);
    return true;
}

void EditorCanvasBase::SpacebarDragOverlay::paint (Graphics&)
{
}

void EditorCanvasBase::SpacebarDragOverlay::mouseMove (const MouseEvent& e)
{
    updateVisibility();
}

void EditorCanvasBase::SpacebarDragOverlay::mouseDown (const MouseEvent& e)
{
    Viewport* vp = findParentComponentOfClass ((Viewport*) 0);

    if (vp != 0)
        dragStart = vp->getViewPosition();
}

void EditorCanvasBase::SpacebarDragOverlay::mouseDrag (const MouseEvent& e)
{
    Viewport* vp = findParentComponentOfClass ((Viewport*) 0);

    if (vp != 0)
        vp->setViewPosition (dragStart - Point<int> (e.getDistanceFromDragStartX(),
                                                     e.getDistanceFromDragStartY()));
}

void EditorCanvasBase::SpacebarDragOverlay::modifierKeysChanged (const ModifierKeys& modifiers)
{
}

//==============================================================================
EditorCanvasBase::Scale::Scale()
    : scale (1.0)
{
}
