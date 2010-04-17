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
#include "jucer_ComponentEditor.h"


//==============================================================================
class SizeGuideComponent   : public Component,
                             public ComponentListener
{
public:
    enum Type
    {
        left, right, top, bottom
    };

    SizeGuideComponent (ComponentDocument& document_, const ValueTree& state_, Component* component_,
                        Component& parentForOverlays, Type type_)
        : document (document_), state (state_), component (component_), type (type_),
          font (10.0f)
    {
        component->addComponentListener (this);

        setAlwaysOnTop (true);
        parentForOverlays.addAndMakeVisible (this);
        updatePosition();
    }

    ~SizeGuideComponent()
    {
        if (component != 0)
            component->removeComponentListener (this);
    }

    void updatePosition()
    {
        RectangleCoordinates coords (document.getCoordsFor (state));
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

        Rectangle<int> bounds (Rectangle<int> (p1, p2).expanded (2, 2).getUnion (textArea));
        bounds.setPosition (component->getParentComponent()->relativePositionToOtherComponent (getParentComponent(), bounds.getPosition()));
        setBounds (bounds);

        lineEnd1 = component->getParentComponent()->relativePositionToOtherComponent (this, p1);
        lineEnd2 = component->getParentComponent()->relativePositionToOtherComponent (this, p2);
        textArea.setPosition (component->getParentComponent()->relativePositionToOtherComponent (this, textArea.getPosition()));
        repaint();
    }

    void paint (Graphics& g)
    {
        const float dashes[] = { 4.0f, 3.0f };

        g.setColour (Colours::grey.withAlpha (0.4f));
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

private:
    ComponentDocument& document;
    ValueTree state;
    Component* component;
    Type type;
    Font font;
    Point<int> lineEnd1, lineEnd2;
    Rectangle<int> textArea;
};

//==============================================================================
class ComponentEditor::Canvas   : public Component,
                                  public ValueTree::Listener,
                                  public Timer
{
public:
    Canvas (ComponentEditor& editor_)
        : editor (editor_), border (14), resizerThickness (4),
          dragStartWidth (0), dragStartHeight (0)
    {
        setOpaque (true);
        addAndMakeVisible (componentHolder = new Component());
        addAndMakeVisible (overlay = new OverlayComponent (*this));

        setSize (500, 500);

        getDocument().getRoot().addListener (this);
        updateComponents();
    }

    ~Canvas()
    {
        dragger = 0;
        getDocument().getRoot().removeListener (this);
        componentHolder->deleteAllChildren();
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
        g.setColour (Colour::greyLevel (0.9f));

        g.drawRect (getContentArea().expanded (resizerThickness, resizerThickness), resizerThickness);

        g.setFont (border.getBottom() - 5.0f);
        g.setColour (Colours::grey);
        g.drawText (String (componentHolder->getWidth()) + " x " + String (componentHolder->getHeight()),
                    0, 0, jmax (getWidth() - border.getRight(), jmin (60, getWidth())), getHeight(), Justification::bottomRight, false);

        g.setFont (border.getTop() - 5.0f);
        g.setColour (Colours::darkgrey);

        g.drawHorizontalLine (border.getTop() - 1, 2.0f, (float) getWidth() - border.getRight());
        g.drawVerticalLine (border.getLeft() - 1, 2.0f, (float) getHeight() - border.getBottom());
        drawXAxis (g, Rectangle<int> (border.getLeft(), 0, componentHolder->getWidth(), border.getTop()));
        drawYAxis (g, Rectangle<int> (0, border.getTop(), border.getLeft(), componentHolder->getHeight()));
    }

    void drawXAxis (Graphics& g, const Rectangle<int>& r)
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

    void drawYAxis (Graphics& g, const Rectangle<int>& r)
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

    void resized()
    {
        componentHolder->setBounds (getContentArea());
        overlay->setBounds (componentHolder->getBounds());
        updateComponents();
    }

    void zoom (float newScale, const Point<int>& centre)
    {
    }

    ComponentEditor& getEditor()                            { return editor; }
    ComponentDocument& getDocument()                        { return editor.getDocument(); }
    ComponentDocument::SelectedItems& getSelection()        { return selection; }

    Component* findComponentFor (const ValueTree& state)
    {
        ComponentDocument& doc = getDocument();

        for (int i = componentHolder->getNumChildComponents(); --i >= 0;)
        {
            Component* c = componentHolder->getChildComponent (i);

            if (doc.isStateForComponent (state, c))
                return c;
        }

        return 0;
    }

    void updateComponents()
    {
        ComponentDocument& doc = getDocument();
        setSize ((int) doc.getCanvasWidth().getValue() + border.getLeftAndRight(),
                 (int) doc.getCanvasHeight().getValue() + border.getTopAndBottom());

        int i;
        for (i = componentHolder->getNumChildComponents(); --i >= 0;)
        {
            Component* c = componentHolder->getChildComponent (i);

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
            Component* c = findComponentFor (v);

            if (c == 0)
            {
                c = doc.createComponent (i);
                componentHolder->addAndMakeVisible (c);
            }

            doc.updateComponent (c);
        }

        startTimer (500);
    }

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const var::identifier& property)
    {
        updateComponents();
    }

    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)
    {
        updateComponents();
    }

    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)
    {
    }

    Component* getComponentHolder() const       { return componentHolder; }

    const Array<Component*> getSelectedComps() const
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

    void getSelectedItemProperties (Array <PropertyComponent*>& props)
    {
        //xxx needs to handle multiple selections..

        if (selection.getNumSelected() == 1)
        {
            Component* c = getComponentForUID (selection.getSelectedItem (0));
            getDocument().getComponentProperties (props, c);
        }
    }

    void timerCallback()
    {
        stopTimer();

        if (! Component::isMouseButtonDownAnywhere())
            getDocument().beginNewTransaction();
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
        showSizeGuides();
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
        hideSizeGuides();
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

    void showSizeGuides()   { overlay->showSizeGuides(); }
    void hideSizeGuides()   { overlay->hideSizeGuides(); }

    //==============================================================================
    class DragOperation
    {
    public:
        DragOperation (Canvas& canvas_,
                       const Array<Component*>& items,
                       const MouseEvent& e,
                       const ResizableBorderComponent::Zone& zone_)
            : canvas (canvas_),
              zone (zone_)
        {
            for (int i = 0; i < items.size(); ++i)
            {
                Component* comp = items.getUnchecked(i);
                jassert (comp != 0);

                const ValueTree v (getDocument().getComponentState (comp));
                draggedComponents.add (v);

                Rectangle<int> pos;

                {
                    RectangleCoordinates relativePos (getDocument().getCoordsFor (v));
                    ScopedPointer<Coordinate::MarkerResolver> markers (getDocument().createMarkerResolver (v));
                    pos = relativePos.resolve (*markers);
                    originalPositions.add (pos);
                }

                const Rectangle<float> floatPos ((float) pos.getX(), (float) pos.getY(),
                                                 (float) pos.getWidth(), (float) pos.getHeight());

                if (zone.isDraggingWholeObject() || zone.isDraggingLeftEdge())
                    verticalSnapPositions.add (SnapLine (floatPos.getX(), floatPos.getY(), floatPos.getBottom()));

                if (zone.isDraggingWholeObject() || (zone.isDraggingLeftEdge() && zone.isDraggingRightEdge()))
                    verticalSnapPositions.add (SnapLine (floatPos.getCentreX(), floatPos.getY(), floatPos.getBottom()));

                if (zone.isDraggingWholeObject() || zone.isDraggingRightEdge())
                    verticalSnapPositions.add (SnapLine (floatPos.getRight(), floatPos.getY(), floatPos.getBottom()));

                if (zone.isDraggingWholeObject() || zone.isDraggingTopEdge())
                    horizontalSnapPositions.add (SnapLine (floatPos.getY(), floatPos.getX(), floatPos.getRight()));

                if (zone.isDraggingWholeObject() || (zone.isDraggingTopEdge() && zone.isDraggingBottomEdge()))
                    horizontalSnapPositions.add (SnapLine (floatPos.getCentreY(), floatPos.getX(), floatPos.getRight()));

                if (zone.isDraggingWholeObject() || zone.isDraggingBottomEdge())
                    horizontalSnapPositions.add (SnapLine (floatPos.getBottom(), floatPos.getX(), floatPos.getRight()));
            }

            verticalSnapTargets.add (SnapLine (0, 0, 10000.0f));
            verticalSnapTargets.add (SnapLine (getDocument().getCanvasWidth().getValue(), 0, 10000.0f));

            if (zone.isDraggingWholeObject() || (zone.isDraggingLeftEdge() && zone.isDraggingRightEdge()))
                verticalSnapTargets.add (SnapLine ((float) getDocument().getCanvasWidth().getValue() / 2.0f, 0, 10000.0f));

            horizontalSnapTargets.add (SnapLine (0, 0, 10000.0f));
            horizontalSnapTargets.add (SnapLine (getDocument().getCanvasHeight().getValue(), 0, 10000.0f));

            if (zone.isDraggingWholeObject() || (zone.isDraggingTopEdge() && zone.isDraggingBottomEdge()))
                horizontalSnapTargets.add (SnapLine ((float) getDocument().getCanvasHeight().getValue() / 2.0f, 0, 10000.0f));

            getDocument().beginNewTransaction();
        }

        ~DragOperation()
        {
            getDocument().beginNewTransaction();
        }

        struct SnapLine
        {
            SnapLine (float position_, float start_, float end_)
                : position (position_), start (start_), end (end_)
            {}

            float position, start, end;
        };

        class SnapGuideComponent  : public Component
        {
        public:
            SnapGuideComponent (const SnapLine& line_, bool isVertical_, Component* parent)
                : line (line_), isVertical (isVertical_)
            {
                if (isVertical)
                    setBounds (roundToInt (line.position), roundToInt (line.start), 1, roundToInt (line.end - line.start));
                else
                    setBounds (roundToInt (line.start), roundToInt (line.position), roundToInt (line.end - line.start), 1);

                parent->addAndMakeVisible (this);
            }

            void paint (Graphics& g)
            {
                g.fillAll (Colours::blue.withAlpha (0.3f));
            }

        private:
            const SnapLine line;
            const bool isVertical;

            SnapGuideComponent (const SnapGuideComponent&);
            SnapGuideComponent& operator= (const SnapGuideComponent&);
        };

        void drag (const MouseEvent& e)
        {
            const float snapThreshold = 8.0f;

            getDocument().getUndoManager()->undoCurrentTransactionOnly();

            Point<int> distance (e.getOffsetFromDragStart());
            snapGuides.clear();

            SnapLine snap (0, 0, 0);
            const float snapX = findBestSnapDistance (verticalSnapTargets, getVerticalSnapPositions (distance), snap);
            if (fabsf (snapX) < snapThreshold)
            {
                distance = Point<int> (distance.getX() + snapX, distance.getY());

                if (snap.position != 0)
                    snapGuides.add (new SnapGuideComponent (snap, true, canvas.overlay));
            }

            const float snapY = findBestSnapDistance (horizontalSnapTargets, getHorizontalSnapPositions (distance), snap);
            if (fabsf (snapY) < snapThreshold)
            {
                distance = Point<int> (distance.getX(), distance.getY() + snapY);

                if (snap.position != 0)
                    snapGuides.add (new SnapGuideComponent (snap, false, canvas.overlay));
            }

            for (int n = 50;;)
            {
                // Need to repeatedly apply the new positions until they all settle down, in case some of
                // the coords are relative to each other..
                bool anyUpdated = false;

                for (int i = 0; i < draggedComponents.size(); ++i)
                    if (dragItem (draggedComponents.getReference(i), distance, originalPositions.getReference(i)))
                        anyUpdated = true;

                if (! anyUpdated)
                    break;

                if (--n == 0)
                {
                    jassertfalse;
                    break;
                }
            }
        }

        bool dragItem (ValueTree& v, const Point<int>& distance, const Rectangle<int>& originalPos)
        {
            const Rectangle<int> newBounds (zone.resizeRectangleBy (originalPos, distance));

            RectangleCoordinates pr (getDocument().getCoordsFor (v));
            ScopedPointer<Coordinate::MarkerResolver> markers (getDocument().createMarkerResolver (v));

            pr.moveToAbsolute (newBounds, *markers);

            return getDocument().setCoordsFor (v, pr);
        }

        const Array<SnapLine> getVerticalSnapPositions (const Point<int>& distance) const
        {
            Array<SnapLine> p (verticalSnapPositions);
            for (int i = p.size(); --i >= 0;)
                p.getReference(i).position += distance.getX();

            return p;
        }

        const Array<SnapLine> getHorizontalSnapPositions (const Point<int>& distance) const
        {
            Array<SnapLine> p (horizontalSnapPositions);
            for (int i = p.size(); --i >= 0;)
                p.getReference(i).position += distance.getY();

            return p;
        }

        static float findBestSnapDistance (const Array<SnapLine>& targets, const Array<SnapLine>& sources, SnapLine& line)
        {
            if (targets.size() == 0 || sources.size() == 0)
                return 0.0f;

            float best = 1.0e10f;
            float absBest = fabsf (best);
            line = SnapLine (1.0e10f, 0, 0);

            for (int i = 0; i < targets.size(); ++i)
            {
                for (int j = 0; j < sources.size(); ++j)
                {
                    SnapLine& target = targets.getReference(i);
                    SnapLine& source = sources.getReference(j);
                    const float diff = target.position - source.position;
                    const float absDiff = fabsf (diff);

                    if (absDiff < absBest)
                    {
                        line = SnapLine (target.position, jmin (target.start, source.start), jmax (target.end, source.end));
                        best = diff;
                        absBest = absDiff;
                    }
                }
            }

            jassert (absBest < 1.0e10f);
            return best;
        }

    private:
        Canvas& canvas;
        Array <ValueTree> draggedComponents;
        Array <Rectangle<int> > originalPositions;

        Array <SnapLine> verticalSnapPositions, horizontalSnapPositions;
        Array <SnapLine> verticalSnapTargets, horizontalSnapTargets;
        const ResizableBorderComponent::Zone zone;
        OwnedArray<Component> snapGuides;

        ComponentDocument& getDocument() throw()         { return canvas.getDocument(); }
    };

    void beginDrag (const MouseEvent& e, const ResizableBorderComponent::Zone& zone)
    {
        dragger = new DragOperation (*this, getSelectedComps(), e, zone);
    }

    void continueDrag (const MouseEvent& e)
    {
        if (dragger != 0)
            dragger->drag (e);
    }

    void endDrag (const MouseEvent& e)
    {
        if (dragger != 0)
        {
            dragger->drag (e);
            dragger = 0;
        }
    }

private:
    ComponentEditor& editor;
    const BorderSize border;
    const int resizerThickness;
    ScopedPointer <DragOperation> dragger;
    ResizableBorderComponent::Zone dragZone;
    int dragStartWidth, dragStartHeight;

    const Rectangle<int> getContentArea() const
    {
        return border.subtractedFrom (getLocalBounds());
    }

    //==============================================================================
    class ComponentResizeFrame    : public Component,
                                    public ComponentListener
    {
    public:
        ComponentResizeFrame (Canvas& canvas_,
                              Component* componentToAttachTo)
            : canvas (canvas_),
              component (componentToAttachTo),
              borderThickness (4)
        {
            componentMovedOrResized (*componentToAttachTo, true, true);
            componentToAttachTo->addComponentListener (this);
        }

        ~ComponentResizeFrame()
        {
            if (component != 0)
                component->removeComponentListener (this);
        }

        void paint (Graphics& g)
        {
            g.setColour (Colours::red.withAlpha (0.1f));
            g.drawRect (0, 0, getWidth(), getHeight(), borderThickness);
        }

        void mouseEnter (const MouseEvent& e)
        {
            //repaint();
            updateDragZone (e.getPosition());
        }

        void mouseExit (const MouseEvent& e)
        {
            //repaint();
            updateDragZone (e.getPosition());
        }

        void mouseMove (const MouseEvent& e)
        {
            updateDragZone (e.getPosition());
        }

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

        void componentMovedOrResized (Component&, bool wasMoved, bool wasResized)
        {
            if (component != 0)
                setBounds (component->getBounds().expanded (borderThickness, borderThickness));
        }

        bool hitTest (int x, int y)
        {
            return ! getCentreArea().contains (x, y);
        }

        uint32 getTargetComponentUID() const  { return component == 0 ? 0 : component->getComponentUID(); }

        void showSizeGuides()
        {
            if (sizeGuides.size() == 0)
            {
                const ValueTree v (canvas.getDocument().getComponentState (component));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, canvas, SizeGuideComponent::left));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, canvas, SizeGuideComponent::right));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, canvas, SizeGuideComponent::top));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, canvas, SizeGuideComponent::bottom));
            }
        }

        void hideSizeGuides()
        {
            sizeGuides.clear();
        }

    private:
        Canvas& canvas;
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
    class OverlayComponent  : public Component,
                              public LassoSource <ComponentDocument::SelectedItems::ItemType>,
                              public ChangeListener
    {
    public:
        OverlayComponent (Canvas& canvas_)
            : canvas (canvas_)
        {
            setAlwaysOnTop (true);
            setWantsKeyboardFocus (true);
            canvas.getSelection().addChangeListener (this);
        }

        ~OverlayComponent()
        {
            canvas.getSelection().removeChangeListener (this);

            lasso = 0;
            deleteAllChildren();
        }

        void mouseDown (const MouseEvent& e)
        {
            lasso = 0;
            mouseDownCompUID = 0;
            isDraggingClickedComp = false;

            if (e.mods.isPopupMenu())
            {
                PopupMenu m;
                canvas.getDocument().addNewComponentMenuItems (m);

                const int r = m.show();
                canvas.getDocument().performNewComponentMenuItem (r);
            }
            else
            {
                Component* underMouse = canvas.getComponentHolder()->getComponentAt (e.x, e.y);

                if (underMouse == canvas.getComponentHolder())
                    underMouse = 0;

                if (underMouse == 0 || e.mods.isAltDown())
                {
                    addAndMakeVisible (lasso = new LassoComponent <ComponentDocument::SelectedItems::ItemType>());
                    lasso->beginLasso (e, this);
                }
                else
                {
                    mouseDownCompUID = underMouse->getComponentUID();
                    mouseDownResult = canvas.getSelection().addToSelectionOnMouseDown (mouseDownCompUID, e.mods);

                    updateSelectedComponentResizeFrames();
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

        ComponentDocument::SelectedItems& getLassoSelection()   { return canvas.getSelection(); }

        void changeListenerCallback (void*)
        {
            updateSelectedComponentResizeFrames();
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

    private:
        Canvas& canvas;
        ScopedPointer <LassoComponent <ComponentDocument::SelectedItems::ItemType> > lasso;
        bool mouseDownResult, isDraggingClickedComp;
        uint32 mouseDownCompUID;

        ComponentResizeFrame* getSelectorFrameFor (Component* c) const
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                ComponentResizeFrame* resizer = dynamic_cast <ComponentResizeFrame*> (getChildComponent(i));
                if (resizer != 0 && resizer->getTargetComponentUID() == c->getComponentUID())
                    return resizer;
            }

            return 0;
        }

        void updateSelectedComponentResizeFrames()
        {
            ComponentDocument::SelectedItems& selection = canvas.getSelection();

            int i;
            for (i = getNumChildComponents(); --i >= 0;)
            {
                ComponentResizeFrame* resizer = dynamic_cast <ComponentResizeFrame*> (getChildComponent(i));

                if (resizer != 0 && ! selection.isSelected (resizer->getTargetComponentUID()))
                    delete resizer;
            }

            for (i = canvas.getComponentHolder()->getNumChildComponents(); --i >= 0;)
            {
                Component* c = canvas.getComponentHolder()->getChildComponent(i);

                if (c != this && selection.isSelected (c->getComponentUID()) && getSelectorFrameFor (c) == 0)
                    addAndMakeVisible (new ComponentResizeFrame (canvas, c));
            }
        }
    };

    Component* componentHolder;
    OverlayComponent* overlay;
    ComponentDocument::SelectedItems selection;

    Component* getComponentForUID (const uint32 uid) const
    {
        for (int i = componentHolder->getNumChildComponents(); --i >= 0;)
            if (componentHolder->getChildComponent (i)->getComponentUID() == uid)
                return componentHolder->getChildComponent (i);

        return 0;
    }
};

//==============================================================================
class ComponentEditor::InfoPanel  : public Component,
                                    public ChangeListener
{
public:
    InfoPanel (ComponentEditor& editor_)
      : editor (editor_)
    {
        setOpaque (true);

        addAndMakeVisible (props = new PropertyPanel());

        editor.getCanvas()->getSelection().addChangeListener (this);
    }

    ~InfoPanel()
    {
        editor.getCanvas()->getSelection().removeChangeListener (this);

        props->clear();
        deleteAllChildren();
    }

    void changeListenerCallback (void*)
    {
        Array <PropertyComponent*> newComps;
        editor.getCanvas()->getSelectedItemProperties (newComps);

        props->clear();
        props->addProperties (newComps);
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colour::greyLevel (0.92f));
    }

    void resized()
    {
        props->setSize (getWidth(), getHeight());
    }

private:
    ComponentEditor& editor;
    PropertyPanel* props;
};


//==============================================================================
ComponentEditor::ComponentEditor (OpenDocumentManager::Document* document,
                                  Project* project_, ComponentDocument* componentDocument_)
    : DocumentEditorComponent (document),
      project (project_),
      componentDocument (componentDocument_),
      infoPanel (0)
{
    setOpaque (true);

    addAndMakeVisible (viewport = new Viewport());

    if (document != 0)
    {
        viewport->setViewedComponent (new Canvas (*this));
        addAndMakeVisible (infoPanel = new InfoPanel (*this));
    }
}

ComponentEditor::~ComponentEditor()
{
    deleteAndZero (infoPanel);
    deleteAllChildren();
}

void ComponentEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void ComponentEditor::resized()
{
    const int infoPanelWidth = 200;
    viewport->setBounds (0, 0, getWidth() - infoPanelWidth, getHeight());

    if (infoPanel != 0)
        infoPanel->setBounds (getWidth() - infoPanelWidth, 0, infoPanelWidth, getHeight());
}

ComponentEditor::Canvas* ComponentEditor::getCanvas() const
{
    return dynamic_cast <Canvas*> (viewport->getViewedComponent());
}

void ComponentEditor::getAllCommands (Array <CommandID>& commands)
{
    DocumentEditorComponent::getAllCommands (commands);

    const CommandID ids[] = { CommandIDs::undo,
                              CommandIDs::redo };

    commands.addArray (ids, numElementsInArray (ids));
}

void ComponentEditor::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    result.setActive (document != 0);

    switch (commandID)
    {
    case CommandIDs::undo:
        result.setInfo ("Undo", "Undoes the last change",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::redo:
        result.setInfo ("Redo", "Redoes the last change",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
        result.defaultKeypresses.add (KeyPress ('y', ModifierKeys::commandModifier, 0));
        break;

    default:
        DocumentEditorComponent::getCommandInfo (commandID, result);
        break;
    }
}

bool ComponentEditor::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::undo:
        getDocument().getUndoManager()->undo();
        return true;

    case CommandIDs::redo:
        getDocument().getUndoManager()->redo();
        return true;

    default:
        break;
    }

    return DocumentEditorComponent::perform (info);
}
