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


const float snapDistance = 8.0f;
static const Colour resizableBorderColour (0x7066aaff);
static const Colour alignmentMarkerColour (0x77ff0000);


//==============================================================================
class SizeGuideComponent   : public Component,
                             public ComponentListener
{
public:
    enum Type    { left, right, top, bottom };

    //==============================================================================
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
    //==============================================================================
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

    //==============================================================================
    ComponentEditor& getEditor()                            { return editor; }
    ComponentDocument& getDocument()                        { return editor.getDocument(); }
    ComponentDocument::SelectedItems& getSelection()        { return selection; }
    Component* getComponentHolder() const                   { return componentHolder; }

    void timerCallback()
    {
        stopTimer();

        if (! Component::isMouseButtonDownAnywhere())
            getDocument().beginNewTransaction();
    }

    //==============================================================================
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

    const Rectangle<int> getContentArea() const
    {
        return border.subtractedFrom (getLocalBounds());
    }

    //==============================================================================
    void resized()
    {
        componentHolder->setBounds (getContentArea());
        overlay->setBounds (componentHolder->getBounds());
        updateComponents();
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
            Component* c = getComponentForState (v);

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

    //==============================================================================
    void getSelectedItemProperties (Array <PropertyComponent*>& props)
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
                       const Array<Component*>& itemsToSnapTo,
                       const MouseEvent& e,
                       const ResizableBorderComponent::Zone& zone_)
            : canvas (canvas_),
              zone (zone_)
        {
            int i;
            for (i = 0; i < items.size(); ++i)
            {
                jassert (items.getUnchecked(i) != 0);
                const ValueTree v (getDocument().getComponentState (items.getUnchecked(i)));
                draggedComponents.add (v);
                const Rectangle<float> floatPos (getComponentPosition (v));

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

            if (isDraggingLeftRight())
            {
                verticalSnapTargets.add (SnapLine (0, 0, 10000.0f));
                verticalSnapTargets.add (SnapLine (getDocument().getCanvasWidth().getValue(), 0, 10000.0f));

                if (zone.isDraggingWholeObject() || (zone.isDraggingLeftEdge() && zone.isDraggingRightEdge()))
                    verticalSnapTargets.add (SnapLine ((float) getDocument().getCanvasWidth().getValue() / 2.0f, 0, 10000.0f));
            }

            if (isDraggingUpDown())
            {
                horizontalSnapTargets.add (SnapLine (0, 0, 10000.0f));
                horizontalSnapTargets.add (SnapLine (getDocument().getCanvasHeight().getValue(), 0, 10000.0f));

                if (zone.isDraggingWholeObject() || (zone.isDraggingTopEdge() && zone.isDraggingBottomEdge()))
                    horizontalSnapTargets.add (SnapLine ((float) getDocument().getCanvasHeight().getValue() / 2.0f, 0, 10000.0f));
            }

            for (i = 0; i < itemsToSnapTo.size(); ++i)
            {
                jassert (itemsToSnapTo.getUnchecked(i) != 0);
                const ValueTree v (getDocument().getComponentState (itemsToSnapTo.getUnchecked(i)));
                const Rectangle<float> floatPos (getComponentPosition (v));

                if (isDraggingLeftRight())
                {
                    verticalSnapTargets.add (SnapLine (floatPos.getX(), floatPos.getY(), floatPos.getBottom()));
                    verticalSnapTargets.add (SnapLine (floatPos.getRight(), floatPos.getY(), floatPos.getBottom()));
                }

                if (zone.isDraggingWholeObject() || (zone.isDraggingLeftEdge() && zone.isDraggingRightEdge()))
                    verticalSnapTargets.add (SnapLine (floatPos.getCentreX(), floatPos.getY(), floatPos.getBottom()));

                if (isDraggingUpDown())
                {
                    horizontalSnapTargets.add (SnapLine (floatPos.getY(), floatPos.getX(), floatPos.getRight()));
                    horizontalSnapTargets.add (SnapLine (floatPos.getBottom(), floatPos.getX(), floatPos.getRight()));
                }

                if (zone.isDraggingWholeObject() || (zone.isDraggingTopEdge() && zone.isDraggingBottomEdge()))
                    horizontalSnapTargets.add (SnapLine (floatPos.getCentreY(), floatPos.getX(), floatPos.getRight()));
            }

            mergeSnapLines (verticalSnapTargets);
            mergeSnapLines (horizontalSnapTargets);

            getDocument().beginNewTransaction();
        }

        ~DragOperation()
        {
            getDocument().beginNewTransaction();
        }

        //==============================================================================
        struct SnapLine
        {
            SnapLine() : position (0), start (0), end (0)  {}

            SnapLine (const float position_, const float start_, const float end_)
                : position (position_), start (start_), end (end_)
            {}

            float position, start, end;
        };

        //==============================================================================
        class AlignmentHintComponent  : public Component
        {
        public:
            AlignmentHintComponent (const SnapLine& line_, bool isVertical_, Component* parent)
                : line (line_), isVertical (isVertical_)
            {
                const int extraEndLength = 5;
                setAlwaysOnTop (true);

                if (isVertical)
                    setBounds (roundToInt (line.position), roundToInt (line.start) - extraEndLength, 1, roundToInt (line.end - line.start) + extraEndLength * 2);
                else
                    setBounds (roundToInt (line.start) - extraEndLength, roundToInt (line.position), roundToInt (line.end - line.start) + extraEndLength * 2, 1);

                parent->addAndMakeVisible (this);
            }

            void paint (Graphics& g)
            {
                g.fillAll (alignmentMarkerColour);
            }

        private:
            const SnapLine line;
            const bool isVertical;

            AlignmentHintComponent (const AlignmentHintComponent&);
            AlignmentHintComponent& operator= (const AlignmentHintComponent&);
        };

        //==============================================================================
        void drag (const MouseEvent& e)
        {
            getDocument().getUndoManager()->undoCurrentTransactionOnly();

            Point<int> distance (e.getOffsetFromDragStart());
            if (! isDraggingLeftRight())
                distance = Point<int> (0, distance.getY());

            if (! isDraggingUpDown())
                distance = Point<int> (distance.getX(), 0);

            snapGuides.clear();

            performSnap (verticalSnapTargets, getVerticalSnapPositions (distance), true, distance);
            performSnap (horizontalSnapTargets, getHorizontalSnapPositions (distance), false, distance);

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

        //==============================================================================
    private:
        Canvas& canvas;
        Array <ValueTree> draggedComponents;
        Array <Rectangle<int> > originalPositions;
        Array <SnapLine> verticalSnapPositions, horizontalSnapPositions;
        Array <SnapLine> verticalSnapTargets, horizontalSnapTargets;
        const ResizableBorderComponent::Zone zone;
        OwnedArray<Component> snapGuides;

        ComponentDocument& getDocument() throw()         { return canvas.getDocument(); }

        const Rectangle<float> getComponentPosition (const ValueTree& state)
        {
            RectangleCoordinates relativePos (getDocument().getCoordsFor (state));
            ScopedPointer<Coordinate::MarkerResolver> markers (getDocument().createMarkerResolver (state));
            const Rectangle<int> intPos (relativePos.resolve (*markers));
            originalPositions.add (intPos);

            return Rectangle<float> ((float) intPos.getX(), (float) intPos.getY(),
                                     (float) intPos.getWidth(), (float) intPos.getHeight());
        }

        static void mergeSnapLines (Array <SnapLine>& lines)
        {
            for (int i = lines.size(); --i > 0;)
            {
                SnapLine& s1 = lines.getReference(i);

                for (int j = i; --j >= 0;)
                {
                    SnapLine& s2 = lines.getReference(j);

                    if (s1.position == s2.position)
                    {
                        s2.start = jmin (s1.start, s2.start);
                        s2.end = jmax (s1.end, s2.end);
                        lines.remove (i);
                    }
                }
            }
        }

        void performSnap (const Array<SnapLine>& targets, const Array<SnapLine>& sources, bool isVertical, Point<int>& distance)
        {
            if (targets.size() == 0 || sources.size() == 0)
                return;

            float best = std::numeric_limits<float>::max();
            float absBest = fabsf (best);
            Array <SnapLine> lines;

            for (int i = 0; i < targets.size(); ++i)
            {
                const SnapLine& target = targets.getReference(i);

                for (int j = 0; j < sources.size(); ++j)
                {
                    const SnapLine& source = sources.getReference(j);
                    const float diff = target.position - source.position;
                    const float absDiff = fabsf (diff);

                    if (absDiff <= absBest)
                    {
                        if (absDiff < absBest)
                            lines.clearQuick();

                        lines.add (SnapLine (target.position, jmin (target.start, source.start), jmax (target.end, source.end)));
                        best = diff;
                        absBest = absDiff;
                    }
                }
            }

            jassert (absBest < std::numeric_limits<float>::max());

            if (absBest < snapDistance)
            {
                distance += isVertical ? Point<int> (roundToInt (best), 0) : Point<int> (0, roundToInt (best));

                for (int i = lines.size(); --i >= 0;)
                    if (lines.getReference(i).position != 0)
                        snapGuides.add (new AlignmentHintComponent (lines.getReference(i), isVertical, canvas.overlay));
            }
        }

        const Array<SnapLine> getVerticalSnapPositions (const Point<int>& distance) const
        {
            Array<SnapLine> p (verticalSnapPositions);
            for (int i = p.size(); --i >= 0;)
            {
                SnapLine& s = p.getReference(i);
                s.position += distance.getX();
                s.start += distance.getY();
                s.end += distance.getY();
            }

            return p;
        }

        const Array<SnapLine> getHorizontalSnapPositions (const Point<int>& distance) const
        {
            Array<SnapLine> p (horizontalSnapPositions);
            for (int i = p.size(); --i >= 0;)
            {
                SnapLine& s = p.getReference(i);
                s.position += distance.getY();
                s.start += distance.getX();
                s.end += distance.getX();
            }

            return p;
        }

        bool isDraggingLeftRight() const        { return zone.isDraggingWholeObject() || zone.isDraggingLeftEdge() || zone.isDraggingRightEdge(); }
        bool isDraggingUpDown() const           { return zone.isDraggingWholeObject() || zone.isDraggingTopEdge() || zone.isDraggingBottomEdge(); }

        DragOperation (const DragOperation&);
        DragOperation& operator= (const DragOperation&);
    };

    //==============================================================================
    void beginDrag (const MouseEvent& e, const ResizableBorderComponent::Zone& zone)
    {
        dragger = new DragOperation (*this, getSelectedComps(), getUnselectedComps(), e, zone);
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
            g.setColour (resizableBorderColour);
            g.drawRect (0, 0, getWidth(), getHeight(), borderThickness);
        }

        void mouseEnter (const MouseEvent& e)           { updateDragZone (e.getPosition()); }
        void mouseExit (const MouseEvent& e)            { updateDragZone (e.getPosition()); }
        void mouseMove (const MouseEvent& e)            { updateDragZone (e.getPosition()); }

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

        //==============================================================================
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

        ComponentDocument::SelectedItems& getLassoSelection()       { return canvas.getSelection(); }

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
        //==============================================================================
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

    //==============================================================================
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

    Component* getComponentForState (const ValueTree& state)
    {
        ComponentDocument& doc = getDocument();

        for (int i = componentHolder->getNumChildComponents(); --i >= 0;)
        {
            Component* const c = componentHolder->getChildComponent (i);

            if (doc.isStateForComponent (state, c))
                return c;
        }

        return 0;
    }

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

    const Array<Component*> getUnselectedComps() const
    {
        Array<Component*> comps;

        for (int i = componentHolder->getNumChildComponents(); --i >= 0;)
            if (! selection.isSelected (componentHolder->getChildComponent(i)->getComponentUID()))
                comps.add (componentHolder->getChildComponent(i));

        return comps;
    }
};

//==============================================================================
class ComponentEditor::ClassInfoHolder  : public Component
{
public:
    ClassInfoHolder (ComponentEditor& editor_)
        : editor (editor_)
    {
        addAndMakeVisible (panel = new PropertyPanelWithTooltips());

        Array <PropertyComponent*> props;
        editor.getDocument().createClassProperties (props);
        panel->getPanel()->addSection ("Component Properties", props, true);
    }

    ~ClassInfoHolder()
    {
        deleteAllChildren();
    }

    void resized()
    {
        panel->setBounds (getLocalBounds());
    }

private:
    ComponentEditor& editor;
    PropertyPanelWithTooltips* panel;
};

//==============================================================================
class ComponentEditor::LayoutEditorHolder  : public Component
{
public:
    LayoutEditorHolder (ComponentEditor& editor_)
        : editor (editor_), infoPanel (0)
    {
        addAndMakeVisible (viewport = new Viewport());
    }

    ~LayoutEditorHolder()
    {
        deleteAndZero (infoPanel);
        deleteAllChildren();
    }

    void createCanvas()
    {
        viewport->setViewedComponent (new Canvas (editor));
        addAndMakeVisible (infoPanel = new InfoPanel (editor));
    }

    void resized()
    {
        const int infoPanelWidth = 200;
        viewport->setBounds (0, 0, getWidth() - infoPanelWidth, getHeight());

        if (infoPanel != 0)
            infoPanel->setBounds (getWidth() - infoPanelWidth, 0, infoPanelWidth, getHeight());
    }

    Viewport* getViewport() const   { return viewport; }

private:
    class InfoPanel  : public Component,
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

    ComponentEditor& editor;
    Viewport* viewport;
    InfoPanel* infoPanel;
};

//==============================================================================
class ComponentEditor::BackgroundEditorHolder  : public Component
{
public:
    BackgroundEditorHolder (ComponentEditor& editor_)
        : editor (editor_)
    {
    }

    ~BackgroundEditorHolder()
    {
    }

private:
    ComponentEditor& editor;
};

//==============================================================================
class ComponentEditor::CodeEditorHolder  : public Component
{
public:
    CodeEditorHolder (ComponentEditor& editor_)
        : editor (editor_)
    {
    }

    ~CodeEditorHolder()
    {
    }

private:
    ComponentEditor& editor;
};

//==============================================================================
ComponentEditor::ComponentEditor (OpenDocumentManager::Document* document,
                                  Project* project_, ComponentDocument* componentDocument_)
    : DocumentEditorComponent (document),
      project (project_),
      componentDocument (componentDocument_),
      classInfoHolder (0),
      layoutEditorHolder (0),
      backgroundEditorHolder (0),
      codeEditorHolder (0)
{
    setOpaque (true);

    if (componentDocument != 0)
    {
        classInfoHolder = new ClassInfoHolder (*this);
        layoutEditorHolder = new LayoutEditorHolder (*this);
        backgroundEditorHolder = new BackgroundEditorHolder (*this);
        codeEditorHolder = new CodeEditorHolder (*this);
        layoutEditorHolder->createCanvas();
    }

    addAndMakeVisible (tabs = new TabbedComponent (TabbedButtonBar::TabsAtRight));
    tabs->setTabBarDepth (22);

    tabs->addTab ("Class Settings", Colour::greyLevel (0.88f), classInfoHolder, true);
    tabs->addTab ("Components", Colours::white, layoutEditorHolder, true);
    tabs->addTab ("Background", Colours::white, backgroundEditorHolder, true);
    tabs->addTab ("Source Code", Colours::white, codeEditorHolder, true);

    tabs->setCurrentTabIndex (1);
}

ComponentEditor::~ComponentEditor()
{
    deleteAllChildren();
}

void ComponentEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void ComponentEditor::resized()
{
    tabs->setBounds (getLocalBounds());
}

ComponentEditor::Canvas* ComponentEditor::getCanvas() const
{
    return dynamic_cast <Canvas*> (getViewport()->getViewedComponent());
}

Viewport* ComponentEditor::getViewport() const
{
    return layoutEditorHolder->getViewport();
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
