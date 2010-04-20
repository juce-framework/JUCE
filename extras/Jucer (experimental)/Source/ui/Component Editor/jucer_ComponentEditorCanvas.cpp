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
#include "jucer_ComponentEditorCanvas.h"
#include "jucer_ComponentEditor.h"

const float snapDistance = 8.0f;
static const Colour alignmentMarkerColour (0x77ff0000);
static const Colour resizableBorderColour (0x7066aaff);

#include "jucer_ComponentDragOperation.h"


//==============================================================================
class ComponentEditorCanvas::ComponentResizeFrame    : public ComponentEditorCanvas::OverlayItemComponent,
                                                       public ComponentListener
{
public:
    ComponentResizeFrame (ComponentEditorCanvas& canvas_, Component* componentToAttachTo)
        : OverlayItemComponent (canvas_),
          component (componentToAttachTo),
          borderThickness (4)
    {
        component->addComponentListener (this);
    }

    ~ComponentResizeFrame()
    {
        if (component != 0)
            component->removeComponentListener (this);
    }

    void paint (Graphics& g)
    {
        g.setColour (resizableBorderColour);
        g.drawRect (0, 0, getWidth(), getHeight(), borderThickness);
    }

    void componentMovedOrResized (Component&, bool, bool)       { updatePosition(); }

    void mouseEnter (const MouseEvent& e)                       { updateDragZone (e.getPosition()); }
    void mouseExit (const MouseEvent& e)                        { updateDragZone (e.getPosition()); }
    void mouseMove (const MouseEvent& e)                        { updateDragZone (e.getPosition()); }

    void mouseDown (const MouseEvent& e)
    {
        jassert (component != 0);

        if (component != 0)
        {
            updateDragZone (e.getPosition());
            canvas.beginDrag (e, dragZone);
            canvas.showSizeGuides();
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (component != 0)
            canvas.continueDrag (e);
    }

    void mouseUp (const MouseEvent& e)
    {
        canvas.hideSizeGuides();

        if (component != 0)
            canvas.endDrag (e);

        updateDragZone (e.getPosition());
    }

    bool hitTest (int x, int y)
    {
        return ! getCentreArea().contains (x, y);
    }

    void updatePosition()
    {
        if (component != 0)
            setBoundsInTargetSpace (component->getBounds().expanded (borderThickness, borderThickness));
    }

    uint32 getTargetComponentUID() const  { return component == 0 ? 0 : component->getComponentUID(); }

    //==============================================================================
    class SizeGuideComponent   : public OverlayItemComponent,
                                 public ComponentListener
    {
    public:
        enum Type    { left, right, top, bottom };

        //==============================================================================
        SizeGuideComponent (ComponentEditorCanvas& canvas_, const ValueTree& state_, Component* component_, Type type_)
            : OverlayItemComponent (canvas_), state (state_), component (component_), type (type_),
              font (10.0f)
        {
            component->addComponentListener (this);

            setAlwaysOnTop (true);
            canvas.addAndMakeVisible (this);
            updatePosition();
        }

        ~SizeGuideComponent()
        {
            if (component != 0)
                component->removeComponentListener (this);
        }

        //==============================================================================
        void paint (Graphics& g)
        {
            const float dashes[] = { 4.0f, 3.0f };

            g.setColour (resizableBorderColour);
            g.drawDashedLine (lineEnd1.getX() + 0.5f, lineEnd1.getY() + 0.5f,
                              lineEnd2.getX() + 0.5f, lineEnd2.getY() + 0.5f,
                              dashes, 2, 1.0f);

            g.setFont (font);
            g.setColour (Colours::white);

            for (int y = -1; y <= 1; ++y)
                for (int x = -1; x <= 1; ++x)
                    g.drawText (getName(), textArea.getX() + x, textArea.getY() + y, textArea.getWidth(), textArea.getHeight(), Justification::centred, 1);

            g.setColour (Colours::black);
            g.drawText (getName(), textArea.getX(), textArea.getY(), textArea.getWidth(), textArea.getHeight(), Justification::centred, 1);
        }

        void componentMovedOrResized (Component&, bool, bool)
        {
            updatePosition();
        }

        void componentBeingDeleted (Component&)
        {
            setVisible (false);
            component = 0;
        }

        //==============================================================================
        void updatePosition()
        {
            RectangleCoordinates coords (getDocument().getCoordsFor (state));
            Coordinate coord (false);
            bool isHorizontal = false;

            switch (type)
            {
                case left:      coord = coords.left; isHorizontal = true; break;
                case right:     coord = coords.right; isHorizontal = true; break;
                case top:       coord = coords.top; break;
                case bottom:    coord = coords.bottom; break;
                default:        jassertfalse; break;
            }

            setName (coord.toString());

            int textW = (int) font.getStringWidth (getName());
            int textH = (int) font.getHeight();

            Point<int> p1, p2;

            switch (type)
            {
            case left:
                p1 = Point<int> (component->getX(), 0);
                p2 = Point<int> (component->getX(), component->getY());
                textArea.setBounds (p1.getX() - textW - 2, 4, textW, textH);
                break;

            case right:
                p1 = Point<int> (component->getRight(), 0);
                p2 = Point<int> (component->getRight(), component->getY());
                textArea.setBounds (p1.getX() + 2, 4, textW, textH);
                break;

            case top:
                p1 = Point<int> (0, component->getY());
                p2 = Point<int> (component->getX(), component->getY());
                textArea.setBounds (4, p1.getY() - textH - 2, textW, textH);
                break;

            case bottom:
                p1 = Point<int> (0, component->getBottom());
                p2 = Point<int> (component->getX(), component->getBottom());
                textArea.setBounds (4, p1.getY() + 2, textW, textH);
                break;

            default:
                jassertfalse;
                break;
            }

            setBoundsInTargetSpace (Rectangle<int> (p1, p2).expanded (2, 2).getUnion (textArea));

            lineEnd1 = component->getParentComponent()->relativePositionToOtherComponent (this, p1);
            lineEnd2 = component->getParentComponent()->relativePositionToOtherComponent (this, p2);
            textArea.setPosition (component->getParentComponent()->relativePositionToOtherComponent (this, textArea.getPosition()));
            repaint();
        }

    private:
        ValueTree state;
        Component* component;
        Type type;
        Font font;
        Point<int> lineEnd1, lineEnd2;
        Rectangle<int> textArea;
    };

    void showSizeGuides()
    {
        if (sizeGuides.size() == 0)
        {
            const ValueTree v (getDocument().getComponentState (component));
            sizeGuides.add (new SizeGuideComponent (canvas, v, component, SizeGuideComponent::left));
            sizeGuides.add (new SizeGuideComponent (canvas, v, component, SizeGuideComponent::right));
            sizeGuides.add (new SizeGuideComponent (canvas, v, component, SizeGuideComponent::top));
            sizeGuides.add (new SizeGuideComponent (canvas, v, component, SizeGuideComponent::bottom));
        }
    }

    void hideSizeGuides()
    {
        sizeGuides.clear();
    }

private:
    Component::SafePointer<Component> component;
    ResizableBorderComponent::Zone dragZone;
    const int borderThickness;
    OwnedArray <SizeGuideComponent> sizeGuides;

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
class ComponentEditorCanvas::MarkerComponent   : public ComponentEditorCanvas::OverlayItemComponent,
                                                 public ValueTree::Listener
{
public:
    MarkerComponent (ComponentEditorCanvas& canvas_, const ValueTree& marker_, bool isX_, int headSize_)
        : OverlayItemComponent (canvas_), marker (marker_), isX (isX_), headSize (headSize_ - 2),
          dragStartPos (0), isDragging (false)
    {
        marker.addListener (this);
    }

    ~MarkerComponent()
    {
        marker.removeListener (this);
    }

    void paint (Graphics& g)
    {
        g.setColour (Colours::darkgreen);
        g.fillPath (path);
    }

    void updatePosition()
    {
        ComponentDocument& doc = getDocument();
        Coordinate coord (doc.getMarkerList (isX).getCoordinate (marker));
        const int pos = roundToInt (coord.resolve (doc.getMarkerList (isX)));
        const int width = 10;

        if (isX)
            setBoundsInTargetSpace (Rectangle<int> (pos - width, -headSize, width * 2, getParentHeight()));
        else
            setBoundsInTargetSpace (Rectangle<int> (-headSize, pos - width, getParentWidth(), width * 2));
    }

    bool hitTest (int x, int y)
    {
        return (isX ? y : x) < headSize;
    }

    void resized()
    {
        const float lineThickness = 1.0f;
        path.clear();

        if (isX)
        {
            const float centre = getWidth() / 2 + 0.5f;
            path.addLineSegment (centre, 2.0f, centre, getHeight() + 1.0f, lineThickness);
            path.addTriangle (1.0f, 0.0f, centre * 2.0f - 1.0f, 0.0f, centre, headSize + 1.0f);
        }
        else
        {
            const float centre = getHeight() / 2 + 0.5f;
            path.addLineSegment (2.0f, centre, getWidth() + 1.0f, centre, lineThickness);
            path.addTriangle (0.0f, centre * 2.0f - 1.0f, 0.0f, 1.0f, headSize + 1.0f, centre);
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        if (e.mods.isPopupMenu())
        {
            isDragging = false;
        }
        else
        {
            isDragging = true;
            getDocument().beginNewTransaction();

            ComponentDocument& doc = getDocument();
            Coordinate coord (doc.getMarkerList(isX).getCoordinate (marker));
            dragStartPos = coord.resolve (doc.getMarkerList (isX));
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (isDragging)
        {
            ComponentDocument& doc = getDocument();
            doc.getUndoManager()->undoCurrentTransactionOnly();

            Coordinate coord (doc.getMarkerList (isX).getCoordinate (marker));
            coord.moveToAbsolute (jmax (0.0, dragStartPos + (isX ? e.getDistanceFromDragStartX()
                                                                 : e.getDistanceFromDragStartY())), 
                                  doc.getMarkerList (isX));
            doc.getMarkerList(isX).setCoordinate (marker, coord);
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        getDocument().beginNewTransaction();
    }

    void mouseEnter (const MouseEvent& e)
    {
    }

    void mouseExit (const MouseEvent& e)
    {
    }

    void valueTreePropertyChanged (ValueTree&, const var::identifier&)    { updatePosition(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)   {}
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)    {}

    ValueTree marker;
    const bool isX;

private:
    const int headSize;
    Path path;
    double dragStartPos;
    bool isDragging;
};


//==============================================================================
class ComponentEditorCanvas::OverlayComponent  : public Component,
                                                 public LassoSource <ComponentDocument::SelectedItems::ItemType>,
                                                 public ChangeListener,
                                                 public ValueTree::Listener
{
public:
    OverlayComponent (ComponentEditorCanvas& canvas_)
        : canvas (canvas_)
    {
        setWantsKeyboardFocus (true);
        canvas.getSelection().addChangeListener (this);

        markerRootX = getDocument().getMarkerListX().getGroup();
        markerRootY = getDocument().getMarkerListY().getGroup();
        markerRootX.addListener (this);
        markerRootY.addListener (this);
    }

    ~OverlayComponent()
    {
        markerRootX.removeListener (this);
        markerRootY.removeListener (this);

        canvas.getSelection().removeChangeListener (this);

        lasso = 0;
        deleteAllChildren();
    }

    //==============================================================================
    void mouseDown (const MouseEvent& e)
    {
        lasso = 0;
        mouseDownCompUID = 0;
        isDraggingClickedComp = false;

        if (e.mods.isPopupMenu())
        {
            PopupMenu m;
            getDocument().addNewComponentMenuItems (m);

            const int r = m.show();
            getDocument().performNewComponentMenuItem (r);
        }
        else
        {
            Component* underMouse = 0;

            for (int i = canvas.getComponentHolder()->getNumChildComponents(); --i >= 0;)
            {
                Component* const c = canvas.getComponentHolder()->getChildComponent(i);
                if (c->getBounds().contains (e.getPosition()))
                {
                    underMouse = c;
                    break;
                }
            }

            if (underMouse == 0 || e.mods.isAltDown())
            {
                addAndMakeVisible (lasso = new LassoComponent <ComponentDocument::SelectedItems::ItemType>());
                lasso->beginLasso (e, this);
            }
            else
            {
                mouseDownCompUID = underMouse->getComponentUID();
                mouseDownResult = canvas.getSelection().addToSelectionOnMouseDown (mouseDownCompUID, e.mods);

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
        else if (mouseDownCompUID != 0 && (! e.mouseWasClicked()) && (! e.mods.isPopupMenu()))
        {
            if (! isDraggingClickedComp)
            {
                isDraggingClickedComp = true;
                canvas.getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, true, mouseDownResult);
                canvas.beginDrag (e, ResizableBorderComponent::Zone (ResizableBorderComponent::Zone::centre));
            }

            canvas.continueDrag (e);
            showSizeGuides();
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        hideSizeGuides();

        if (lasso != 0)
        {
            lasso->endLasso();
            lasso = 0;

            if (e.mouseWasClicked())
                canvas.getSelection().deselectAll();
        }
        else if (! e.mods.isPopupMenu())
        {
            if (! isDraggingClickedComp)
                canvas.getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, ! e.mouseWasClicked(), mouseDownResult);
        }

        canvas.endDrag (e);
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        const BorderSize& border = canvas.border;
        const Rectangle<int> xAxis (border.getLeft(), 0, getWidth() - border.getLeftAndRight(), border.getTop());
        const Rectangle<int> yAxis (0, border.getTop(), border.getLeft(), getHeight() - border.getTopAndBottom());

        if (xAxis.contains (e.x, e.y))
        {
            getDocument().getMarkerListX().createMarker ("Marker", e.x - xAxis.getX());
        }
        else if (yAxis.contains (e.x, e.y))
        {
            getDocument().getMarkerListY().createMarker ("Marker", e.y - yAxis.getY());
        }
    }

    void findLassoItemsInArea (Array <ComponentDocument::SelectedItems::ItemType>& itemsFound, int x, int y, int width, int height)
    {
        const Rectangle<int> lassoArea (x, y, width, height);

        for (int i = canvas.getComponentHolder()->getNumChildComponents(); --i >= 0;)
        {
            Component* c = canvas.getComponentHolder()->getChildComponent(i);
            if (c->getBounds().intersects (lassoArea))
                itemsFound.add (c->getComponentUID());
        }
    }

    ComponentDocument::SelectedItems& getLassoSelection()       { return canvas.getSelection(); }

    void resized()
    {
        updateMarkers();
    }

    void changeListenerCallback (void*)
    {
        updateResizeFrames();
    }

    void modifierKeysChanged (const ModifierKeys&)
    {
        Desktop::getInstance().getMainMouseSource().triggerFakeMove();
    }

    void showSizeGuides()
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            ComponentResizeFrame* resizer = dynamic_cast <ComponentResizeFrame*> (getChildComponent(i));
            if (resizer != 0)
                resizer->showSizeGuides();
        }
    }

    void hideSizeGuides()
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            ComponentResizeFrame* resizer = dynamic_cast <ComponentResizeFrame*> (getChildComponent(i));
            if (resizer != 0)
                resizer->hideSizeGuides();
        }
    }

    void valueTreePropertyChanged (ValueTree&, const var::identifier&)    { updateMarkers(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)   { updateMarkers(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)    {}

private:
    //==============================================================================
    ComponentEditorCanvas& canvas;
    ValueTree markerRootX, markerRootY;
    ScopedPointer <LassoComponent <ComponentDocument::SelectedItems::ItemType> > lasso;
    bool mouseDownResult, isDraggingClickedComp;
    uint32 mouseDownCompUID;

    ComponentDocument& getDocument()            { return canvas.getDocument(); }

    Component* getComponentWithUID (const int uid) const
    {
        for (int i = canvas.getComponentHolder()->getNumChildComponents(); --i >= 0;)
        {
            Component* c = canvas.getComponentHolder()->getChildComponent(i);

            if (c->getComponentUID() == uid)
                return c;
        }

        return 0;
    }

    void updateResizeFrames()
    {
        ComponentDocument::SelectedItems& selection = canvas.getSelection();

        Array<int> requiredIds;
        int i;
        for (i = selection.getNumSelected(); --i >= 0;)
            requiredIds.add (selection.getSelectedItem(i));

        for (i = getNumChildComponents(); --i >= 0;)
        {
            ComponentResizeFrame* resizer = dynamic_cast <ComponentResizeFrame*> (getChildComponent(i));

            if (resizer != 0)
            {
                if (selection.isSelected (resizer->getTargetComponentUID()))
                    requiredIds.removeValue (resizer->getTargetComponentUID());
                else
                    delete resizer;
            }
        }

        for (i = requiredIds.size(); --i >= 0;)
        {
            Component* c = getComponentWithUID (requiredIds.getUnchecked(i));

            if (c != 0)
            {
                ComponentResizeFrame* frame = new ComponentResizeFrame (canvas, c);
                addAndMakeVisible (frame);
                frame->updatePosition();
            }
        }
    }

    void updateMarkers (bool isX)
    {
        ComponentDocument& doc = getDocument();
        Array<ValueTree> requiredMarkers;

        int i;
        for (i = doc.getMarkerList (isX).size(); --i >= 0;)
            requiredMarkers.add (doc.getMarkerList (isX).getMarker (i));

        for (i = getNumChildComponents(); --i >= 0;)
        {
            MarkerComponent* marker = dynamic_cast <MarkerComponent*> (getChildComponent(i));

            if (marker != 0 && marker->isX == isX)
            {
                if (requiredMarkers.contains (marker->marker))
                {
                    marker->updatePosition();
                    requiredMarkers.removeValue (marker->marker);
                }
                else
                {
                    delete marker;
                }
            }
        }

        for (i = requiredMarkers.size(); --i >= 0;)
        {
            MarkerComponent* marker = new MarkerComponent (canvas, requiredMarkers.getReference(i),
                                                           isX, isX ? canvas.border.getTop()
                                                                    : canvas.border.getLeft());
            addAndMakeVisible (marker);
            marker->updatePosition();
        }
    }

    void updateMarkers()
    {
        updateMarkers (true);
        updateMarkers (false);
    }
};

//==============================================================================
class ComponentEditorCanvas::ComponentHolder    : public Component
{
public:
    ComponentHolder() {}
    ~ComponentHolder() {}

    void updateComponents (ComponentDocument& doc, ComponentDocument::SelectedItems& selection)
    {
        int i;
        for (i = getNumChildComponents(); --i >= 0;)
        {
            Component* c = getChildComponent (i);

            if (! doc.containsComponent (c))
            {
                selection.deselect (c->getComponentUID());
                delete c;
            }
        }

        const int num = doc.getNumComponents();
        for (i = 0; i < num; ++i)
        {
            const ValueTree v (doc.getComponent (i));
            Component* c = getComponentForState (doc, v);

            if (c == 0)
            {
                c = doc.createComponent (i);
                addAndMakeVisible (c);
            }

            doc.updateComponent (c);
        }
    }

    Component* getComponentForState (ComponentDocument& doc, const ValueTree& state)
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            Component* const c = getChildComponent (i);

            if (doc.isStateForComponent (state, c))
                return c;
        }

        return 0;
    }
};

//==============================================================================
class ComponentEditorCanvas::WholeComponentResizer    : public Component
{
public:
    WholeComponentResizer (ComponentEditorCanvas& canvas_)
        : canvas (canvas_), dragStartWidth (0), dragStartHeight (0), resizerThickness (4)
    {
    }

    ~WholeComponentResizer()
    {
    }

    void paint (Graphics& g)
    {
        const Rectangle<int> content (getContentArea());

        g.setColour (Colour::greyLevel (0.7f).withAlpha (0.4f));
        g.drawRect (content.expanded (resizerThickness, resizerThickness), resizerThickness);

        const int bottomGap = getHeight() - content.getBottom();
        g.setFont (bottomGap - 5.0f);

        g.setColour (Colours::grey);
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
        dragStartWidth = getDocument().getCanvasWidth().getValue();
        dragStartHeight = getDocument().getCanvasHeight().getValue();
        canvas.showSizeGuides();
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (dragZone.isDraggingRightEdge())
            getDocument().getCanvasWidth() = jmax (1, dragStartWidth + e.getDistanceFromDragStartX());

        if (dragZone.isDraggingBottomEdge())
            getDocument().getCanvasHeight() = jmax (1, dragStartHeight + e.getDistanceFromDragStartY());
    }

    void mouseUp (const MouseEvent& e)
    {
        canvas.hideSizeGuides();
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
        const Rectangle<int> content (getContentArea());

        return (x >= content.getRight() || y >= content.getBottom())
                 && (! content.contains (x, y))
                 && content.expanded (resizerThickness, resizerThickness).contains (x, y);
    }

private:
    ComponentEditorCanvas& canvas;
    ResizableBorderComponent::Zone dragZone;
    int dragStartWidth, dragStartHeight;
    const int resizerThickness;

    ComponentDocument& getDocument()                { return canvas.getDocument(); }
    const Rectangle<int> getContentArea() const     { return canvas.getContentArea(); }
};


//==============================================================================
ComponentEditorCanvas::ComponentEditorCanvas (ComponentEditor& editor_)
    : editor (editor_), border (14)
{
    setOpaque (true);
    addAndMakeVisible (componentHolder = new ComponentHolder());
    addAndMakeVisible (overlay = new OverlayComponent (*this));
    overlay->addAndMakeVisible (resizeFrame = new WholeComponentResizer (*this));

    setSize (500, 500);

    getDocument().getRoot().addListener (this);
    updateComponents();
}

ComponentEditorCanvas::~ComponentEditorCanvas()
{
    dragger = 0;
    getDocument().getRoot().removeListener (this);
    componentHolder->deleteAllChildren();
    deleteAllChildren();
}

//==============================================================================
ComponentEditor& ComponentEditorCanvas::getEditor()                            { return editor; }
ComponentDocument& ComponentEditorCanvas::getDocument()                        { return editor.getDocument(); }
ComponentDocument::SelectedItems& ComponentEditorCanvas::getSelection()        { return selection; }
Component* ComponentEditorCanvas::getComponentHolder() const                   { return componentHolder; }

void ComponentEditorCanvas::timerCallback()
{
    stopTimer();

    if (! Component::isMouseButtonDownAnywhere())
        getDocument().beginNewTransaction();
}

//==============================================================================
void ComponentEditorCanvas::paint (Graphics& g)
{
    g.fillAll (Colours::white);

    g.setFont (border.getTop() - 5.0f);
    g.setColour (Colours::darkgrey);

    g.drawHorizontalLine (border.getTop() - 1, 2.0f, (float) getWidth() - border.getRight());
    g.drawVerticalLine (border.getLeft() - 1, 2.0f, (float) getHeight() - border.getBottom());
    drawXAxis (g, Rectangle<int> (border.getLeft(), 0, componentHolder->getWidth(), border.getTop()));
    drawYAxis (g, Rectangle<int> (0, border.getTop(), border.getLeft(), componentHolder->getHeight()));
}

void ComponentEditorCanvas::drawXAxis (Graphics& g, const Rectangle<int>& r)
{
    TickIterator ticks (0, r.getWidth(), 1.0, 10, 50);
    float pos, tickLength;
    String label;

    while (ticks.getNextTick (pos, tickLength, label))
    {
        if (pos > 0)
        {
            g.drawVerticalLine (r.getX() + (int) pos, r.getBottom() - tickLength * r.getHeight(), (float) r.getBottom());
            g.drawSingleLineText (label, r.getX() + (int) pos + 2, (int) r.getBottom() - 6);
        }
    }
}

void ComponentEditorCanvas::drawYAxis (Graphics& g, const Rectangle<int>& r)
{
    TickIterator ticks (0, r.getHeight(), 1.0, 10, 80);
    float pos, tickLength;
    String label;

    while (ticks.getNextTick (pos, tickLength, label))
    {
        if (pos > 0)
        {
            g.drawHorizontalLine (r.getY() + (int) pos, r.getRight() - tickLength * r.getWidth(), (float) r.getRight());
            g.drawTextAsPath (label, AffineTransform::rotation (float_Pi / -2.0f)
                                                     .translated (r.getRight() - 6.0f, r.getY() + pos - 2.0f));
        }
    }
}

const Rectangle<int> ComponentEditorCanvas::getContentArea() const
{
    return border.subtractedFrom (getLocalBounds());
}

//==============================================================================
void ComponentEditorCanvas::resized()
{
    componentHolder->setBounds (getContentArea());
    overlay->setBounds (getLocalBounds());
    resizeFrame->setBounds (getLocalBounds());
    updateComponents();
}

void ComponentEditorCanvas::updateComponents()
{
    setSize ((int) getDocument().getCanvasWidth().getValue() + border.getLeftAndRight(),
             (int) getDocument().getCanvasHeight().getValue() + border.getTopAndBottom());

    componentHolder->updateComponents (getDocument(), selection);
    startTimer (500);
}

//==============================================================================
void ComponentEditorCanvas::getSelectedItemProperties (Array <PropertyComponent*>& props)
{
    //xxx needs to handle multiple selections..

    if (selection.getNumSelected() == 1)
    {
        Component* c = getComponentForUID (selection.getSelectedItem (0));
        jassert (c != 0);
        getDocument().getComponentProperties (props, c);
    }
}

//==============================================================================
void ComponentEditorCanvas::showSizeGuides()   { overlay->showSizeGuides(); }
void ComponentEditorCanvas::hideSizeGuides()   { overlay->hideSizeGuides(); }


//==============================================================================
Component* ComponentEditorCanvas::getComponentForUID (const uint32 uid) const
{
    for (int i = componentHolder->getNumChildComponents(); --i >= 0;)
        if (componentHolder->getChildComponent (i)->getComponentUID() == uid)
            return componentHolder->getChildComponent (i);

    return 0;
}

const Array<Component*> ComponentEditorCanvas::getSelectedComps() const
{
    Array<Component*> comps;

    for (int i = 0; i < selection.getNumSelected(); ++i)
    {
        Component* c = getComponentForUID (selection.getSelectedItem (i));
        jassert (c != 0);
        if (c != 0)
            comps.add (c);
    }

    return comps;
}

const Array<Component*> ComponentEditorCanvas::getUnselectedComps() const
{
    Array<Component*> comps;

    for (int i = componentHolder->getNumChildComponents(); --i >= 0;)
        if (! selection.isSelected (componentHolder->getChildComponent(i)->getComponentUID()))
            comps.add (componentHolder->getChildComponent(i));

    return comps;
}

//==============================================================================
void ComponentEditorCanvas::beginDrag (const MouseEvent& e, const ResizableBorderComponent::Zone& zone)
{
    dragger = new DragOperation (*this, getSelectedComps(), getUnselectedComps(), e, overlay, zone);
}

void ComponentEditorCanvas::continueDrag (const MouseEvent& e)
{
    if (dragger != 0)
        dragger->drag (e);
}

void ComponentEditorCanvas::endDrag (const MouseEvent& e)
{
    if (dragger != 0)
    {
        dragger->drag (e);
        dragger = 0;
    }
}

//==============================================================================
ComponentEditorCanvas::OverlayItemComponent::OverlayItemComponent (ComponentEditorCanvas& canvas_)
    : canvas (canvas_)
{
}

void ComponentEditorCanvas::OverlayItemComponent::setBoundsInTargetSpace (const Rectangle<int>& r)
{
    setBounds (r + canvas.getComponentHolder()->relativePositionToOtherComponent (getParentComponent(), Point<int>()));
}
