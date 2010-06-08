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

#ifndef __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__
#define __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__

#include "jucer_DrawableEditor.h"
#include "../../model/Drawable/jucer_DrawableTypeHandler.h"


//==============================================================================
class DrawableEditorCanvas  : public EditorCanvasBase,
                              public FileDragAndDropTarget,
                              public Timer
{
public:
    //==============================================================================
    DrawableEditorCanvas (DrawableEditor& editor_)
        : editor (editor_)
    {
        initialise();
        getDocument().getRoot().addListener (this);
    }

    ~DrawableEditorCanvas()
    {
        getDocument().getRoot().removeListener (this);
        shutdown();
    }

    //==============================================================================
    UndoManager& getUndoManager() throw()                       { return *getDocument().getUndoManager(); }
    DrawableEditor& getEditor() throw()                         { return editor; }
    DrawableDocument& getDocument() throw()                     { return editor.getDocument(); }

    Component* createComponentHolder()
    {
        return new DrawableComponent (this);
    }

    void documentChanged()
    {
        DrawableDocument& doc = getDocument();

        if (drawable == 0)
        {
            Drawable* newDrawable = Drawable::createFromValueTree (doc.getRootDrawableNode().getState(), &doc);
            drawable = dynamic_cast <DrawableComposite*> (newDrawable);
            drawable->resetBoundingBoxToContentArea();
            jassert (drawable != 0);
            getComponentHolder()->repaint();
        }
        else
        {
            doc.getRootDrawableNode().resetBoundingBoxToContentArea (0);
            const Rectangle<float> damage (drawable->refreshFromValueTree (doc.getRootDrawableNode().getState(), &doc));

            getComponentHolder()->repaint (objectSpaceToScreenSpace (damage.getSmallestIntegerContainer()));
        }

        startTimer (500);
    }

    const Rectangle<int> getCanvasBounds()
    {
        return drawable->getBounds().getSmallestIntegerContainer();
    }

    void setCanvasBounds (const Rectangle<int>& newBounds)  {}
    bool canResizeCanvas() const                            { return false; }

    //==============================================================================
    const ValueTree getObjectState (const String& objectId)
    {
        return getDocument().findDrawableState (objectId, false);
    }

    const SelectedItems::ItemType findObjectIdAt (const Point<int>& position)
    {
        if (drawable != 0)
        {
            for (int i = drawable->getNumDrawables(); --i >= 0;)
            {
                Drawable* d = drawable->getDrawable (i);

                if (d->hitTest ((float) position.getX(), (float) position.getY()))
                    return d->getName();
            }
        }

        return String::empty;
    }

    void showPopupMenu (bool isClickOnSelectedObject)
    {
        PopupMenu m;

        if (isClickOnSelectedObject)
        {
            m.addCommandItem (commandManager, CommandIDs::toFront);
            m.addCommandItem (commandManager, CommandIDs::toBack);
            m.addSeparator();
            m.addCommandItem (commandManager, StandardApplicationCommandIDs::del);
            const int r = m.show();
            (void) r;
        }
        else
        {
            editor.showNewShapeMenu (0);
        }
    }

    void objectDoubleClicked (const MouseEvent& e, const ValueTree& state)
    {
        if (state.hasType (DrawablePath::valueTreeType)
             || state.hasType (DrawableImage::valueTreeType)
             || state.hasType (DrawableText::valueTreeType)
             || state.hasType (DrawableComposite::valueTreeType))
        {
            enableControlPointMode (state);
        }
        else if (state.hasType (DrawableComposite::valueTreeType))
        {
            // xxx
        }
    }

    bool hasSizeGuides() const  { return false; }

    void getObjectPositionDependencies (const ValueTree& state, Array<ValueTree>& deps)
    {
        DrawableDocument& doc = getDocument();
        DrawableTypeInstance item (doc, state);

        OwnedArray <ControlPoint> points;
        item.getAllControlPoints (points);

        StringArray anchors;
        for (int i = 0; i < points.size(); ++i)
        {
            const RelativePoint p (points.getUnchecked(i)->getPosition());
            anchors.addIfNotAlreadyThere (p.x.getAnchorName1());
            anchors.addIfNotAlreadyThere (p.x.getAnchorName2());
            anchors.addIfNotAlreadyThere (p.y.getAnchorName1());
            anchors.addIfNotAlreadyThere (p.y.getAnchorName2());
        }

        for (int i = 0; i < anchors.size(); ++i)
        {
            const String anchor (anchors[i]);
            if (anchor.isNotEmpty() && ! anchor.startsWith ("parent."))
            {
                const ValueTree v (doc.findDrawableState (anchor.upToFirstOccurrenceOf (".", false, false), false));

                if (v.isValid())
                    deps.add (v);
            }
        }
    }

    const Rectangle<float> getObjectPositionFloat (const ValueTree& state)
    {
        if (drawable != 0)
        {
            Drawable* d = drawable->getDrawableWithName (Drawable::ValueTreeWrapperBase (state).getID());

            if (d != 0)
                return d->getBounds();
        }

        return Rectangle<float>();
    }

    void setObjectPositionFloat (const ValueTree& state, const Rectangle<float>& newPos)
    {
        if (drawable != 0)
        {
            Drawable* d = drawable->getDrawableWithName (Drawable::ValueTreeWrapperBase (state).getID());

            if (d != 0)
            {
                d->refreshFromValueTree (state, &getDocument());
                DrawableTypeInstance di (getDocument(), state);
                di.setBounds (d, newPos);
            }
        }
    }

    const Rectangle<int> getObjectPosition (const ValueTree& state)
    {
        return getObjectPositionFloat (state).getSmallestIntegerContainer();
    }

    void transformObject (ValueTree& state, const AffineTransform& transform)
    {
        if (drawable != 0)
        {
            Drawable* d = drawable->getDrawableWithName (Drawable::ValueTreeWrapperBase (state).getID());

            if (d != 0)
            {
                d->refreshFromValueTree (state, &getDocument());
                DrawableTypeInstance di (getDocument(), state);
                di.applyTransform (d, transform);
            }
        }
    }

    RelativeRectangle getObjectCoords (const ValueTree& state)
    {
        return RelativeRectangle();
    }

    //==============================================================================
    class ControlPointComponent  : public OverlayItemComponent
    {
    public:
        ControlPointComponent (DrawableEditorCanvas* canvas, const ValueTree& drawableState_, int controlPointNum_)
            : OverlayItemComponent (canvas), drawableState (drawableState_),
              controlPointNum (controlPointNum_), isDragging (false), mouseDownResult (false), selected (false),
              sizeNormal (7), sizeOver (11)
        {
            setRepaintsOnMouseActivity (true);
        }

        ~ControlPointComponent()
        {
        }

        void paint (Graphics& g)
        {
            Rectangle<int> r (getLocalBounds());

            if (! isMouseOverOrDragging())
                r = r.reduced ((sizeOver - sizeNormal) / 2, (sizeOver - sizeNormal) / 2);

            g.setColour (Colour (selected ? 0xaaaaaaaa : 0xaa333333));
            g.drawRect (r);

            g.setColour (Colour (selected ? 0xaa000000 : 0x99ffffff));
            g.fillRect (r.reduced (1, 1));
        }

        bool hitTest (int x, int y)
        {
            if (isMouseOverOrDragging())
                return true;

            return getLocalBounds().reduced ((sizeOver - sizeNormal) / 2, (sizeOver - sizeNormal) / 2).contains (x, y);
        }

        void mouseDown (const MouseEvent& e)
        {
            isDragging = false;

            if (e.mods.isPopupMenu())
            {
                canvas->showPopupMenu (true);
            }
            else
            {
                mouseDownResult = canvas->getSelection().addToSelectionOnMouseDown (selectionId, e.mods);
            }
        }

        void mouseDrag (const MouseEvent& e)
        {
            if (! (isDragging || e.mouseWasClicked() || e.mods.isPopupMenu()))
            {
                canvas->getSelection().addToSelectionOnMouseUp (selectionId, e.mods, true, mouseDownResult);

                isDragging = true;
                canvas->beginDrag (e.withNewPosition (e.getMouseDownPosition()).getEventRelativeTo (getParentComponent()),
                                   ResizableBorderComponent::Zone (ResizableBorderComponent::Zone::centre), false, Point<float>());
            }

            if (isDragging)
            {
                canvas->continueDrag (e.getEventRelativeTo (getParentComponent()));
                autoScrollForMouseEvent (e);
            }
        }

        void mouseUp (const MouseEvent& e)
        {
            if (! e.mods.isPopupMenu())
            {
                if (isDragging)
                    canvas->endDrag (e.getEventRelativeTo (getParentComponent()));
                else
                    canvas->getSelection().addToSelectionOnMouseUp (selectionId, e.mods, false, mouseDownResult);
            }
        }

        void mouseDoubleClick (const MouseEvent& e)
        {
        }

        class LineComponent  : public OverlayItemComponent
        {
        public:
            LineComponent (EditorCanvasBase* canvas)
                : OverlayItemComponent (canvas)
            {}

            ~LineComponent() {}

            void setLine (const Line<float>& newLine)
            {
                if (line != newLine)
                {
                    line = newLine;
                    setBoundsInTargetSpace (Rectangle<float> (line.getStart(), line.getEnd())
                                                .getSmallestIntegerContainer().expanded (2, 2));
                    repaint();
                }
            }

            void paint (Graphics& g)
            {
                g.setColour (Colours::black.withAlpha (0.6f));
                g.drawLine (Line<float> (pointToLocalSpace (line.getStart()),
                                         pointToLocalSpace (line.getEnd())), 1.0f);
            }

            bool hitTest (int, int)
            {
                return false;
            }

        private:
            Line<float> line;
        };

        void updatePosition (ControlPoint& point, RelativeCoordinate::NamedCoordinateFinder* nameFinder)
        {
            selectionId = point.getID();

            const Point<float> p (point.getPosition().resolve (nameFinder));
            setBoundsInTargetSpace (Rectangle<int> (roundToInt (p.getX()) - sizeOver / 2,
                                                    roundToInt (p.getY()) - sizeOver / 2,
                                                    sizeOver, sizeOver));

            const bool nowSelected = canvas->getSelection().isSelected (selectionId);

            if (selected != nowSelected)
            {
                selected = nowSelected;
                repaint();
            }

            if (point.hasLine())
            {
                if (line == 0)
                {
                    line = new LineComponent (canvas);
                    getParentComponent()->addAndMakeVisible (line, 0);
                }

                line->setLine (Line<float> (p, point.getEndOfLine().resolve (nameFinder)));
            }
            else
            {
                line = 0;
            }
        }

    private:
        ValueTree drawableState;
        int controlPointNum;
        bool isDragging, mouseDownResult, selected;
        String selectionId;
        ScopedPointer <LineComponent> line;
        const int sizeNormal, sizeOver;
    };

    void updateControlPointComponents (Component* parent, OwnedArray<OverlayItemComponent>& comps)
    {
        if (drawable == 0)
        {
            comps.clear();
            return;
        }

        DrawableTypeInstance item (getDocument(), controlPointEditingTarget);
        OwnedArray <ControlPoint> points;
        item.getVisibleControlPoints (points, getSelection());

        Drawable* d = drawable->getDrawableWithName (Drawable::ValueTreeWrapperBase (controlPointEditingTarget).getID());
        DrawableComposite* parentDrawable = d->getParent();

        comps.removeRange (points.size(), comps.size());

        BigInteger requiredIndexes;
        requiredIndexes.setRange (0, points.size(), true);

        for (int i = 0; i < points.size(); ++i)
        {
            ControlPointComponent* c = dynamic_cast <ControlPointComponent*> (comps[i]);

            if (c == 0)
            {
                c = new ControlPointComponent (this, controlPointEditingTarget, i);
                comps.set (i, c);
                parent->addAndMakeVisible (c);
            }

            c->updatePosition (*points.getUnchecked(i), parentDrawable);
        }
    }

    //==============================================================================
    MarkerListBase& getMarkerList (bool isX)
    {
        return getDocument().getMarkerList (isX);
    }

    double limitMarkerPosition (double pos)
    {
        return pos;
    }

    //==============================================================================
    SelectedItems& getSelection()
    {
        return editor.getSelection();
    }

    void deselectNonDraggableObjects()
    {
    }

    void findLassoItemsInArea (Array <SelectedItems::ItemType>& itemsFound, const Rectangle<int>& area)
    {
        const Rectangle<float> floatArea (area.toFloat());

        if (drawable != 0)
        {
            if (isControlPointMode())
            {
                DrawableTypeInstance item (getDocument(), controlPointEditingTarget);
                OwnedArray <ControlPoint> points;
                item.getVisibleControlPoints (points, getSelection());

                const Rectangle<float> floatArea (area.toFloat());

                for (int i = 0; i < points.size(); ++i)
                {
                    const Point<float> p (points.getUnchecked(i)->getPosition().resolve (drawable));

                    if (floatArea.contains (p))
                        itemsFound.add (points.getUnchecked(i)->getID());
                }
            }
            else
            {
                for (int i = drawable->getNumDrawables(); --i >= 0;)
                {
                    Drawable* d = drawable->getDrawable (i);

                    if (d->getBounds().intersects (floatArea))
                        itemsFound.add (d->getName());
                }
            }
        }
    }

    bool isControlPointId (const String& itemId)
    {
        return itemId.containsChar ('/');
    }

    //==============================================================================
    class ObjectDragOperation  : public EditorDragOperation
    {
    public:
        ObjectDragOperation (DrawableEditorCanvas* canvas_, const Point<int>& mousePos,
                             Component* snapGuideParentComp_, const ResizableBorderComponent::Zone& zone_, bool isRotating)
            : EditorDragOperation (canvas_, mousePos, snapGuideParentComp_, zone_, isRotating),
              drawableCanvas (canvas_)
        {
        }

        ~ObjectDragOperation() {}

    protected:
        DrawableDocument& getDocument() throw()                 { return drawableCanvas->getDocument(); }

        void getSnapPointsX (Array<float>& points, bool /*includeCentre*/)  { points.add (0.0f); }
        void getSnapPointsY (Array<float>& points, bool /*includeCentre*/)  { points.add (0.0f); }

        UndoManager& getUndoManager()                           { return *getDocument().getUndoManager(); }

        void getObjectDependencies (const ValueTree& state, Array<ValueTree>& deps)
        {
            drawableCanvas->getObjectPositionDependencies (state, deps);
        }

        const Rectangle<float> getObjectPosition (const ValueTree& state)
        {
            return drawableCanvas->getObjectPositionFloat (state);
        }

        void setObjectPosition (ValueTree& state, const Rectangle<float>& newBounds)
        {
            drawableCanvas->setObjectPositionFloat (state, newBounds);
        }

        void transformObject (ValueTree& state, const AffineTransform& transform)
        {
            drawableCanvas->transformObject (state, transform);
        }

        float getMarkerPosition (const ValueTree& marker, bool isX)
        {
            return 0;
        }

    private:
        DrawableEditorCanvas* drawableCanvas;
    };

    //==============================================================================
    class ControlPointDragOperation  : public EditorDragOperation
    {
    public:
        ControlPointDragOperation (DrawableEditorCanvas* canvas_,
                                   const DrawableTypeInstance& drawableItem_,
                                   DrawableComposite* drawable_,
                                   const Point<int>& mousePos,
                                   Component* snapGuideParentComp_,
                                   const ResizableBorderComponent::Zone& zone_)
            : EditorDragOperation (canvas_, mousePos, snapGuideParentComp_, zone_, false),
              drawableCanvas (canvas_), drawableItem (drawableItem_), drawable (drawable_)
        {
            drawableItem.getVisibleControlPoints (points, canvas_->getSelection());
        }

        ~ControlPointDragOperation() {}

        OwnedArray <ControlPoint> points;

    protected:
        DrawableDocument& getDocument() throw()                 { return drawableCanvas->getDocument(); }

        void getSnapPointsX (Array<float>& points, bool /*includeCentre*/)  { points.add (0.0f); }
        void getSnapPointsY (Array<float>& points, bool /*includeCentre*/)  { points.add (0.0f); }

        UndoManager& getUndoManager()                           { return *getDocument().getUndoManager(); }

        void getObjectDependencies (const ValueTree& state, Array<ValueTree>& deps)
        {
            drawableCanvas->getObjectPositionDependencies (drawableItem.getState(), deps);
        }

        const Rectangle<float> getObjectPosition (const ValueTree& state)
        {
            int index = state [Ids::id_];
            ControlPoint* cp = points[index];
            if (cp == 0)
                return Rectangle<float>();

            Point<float> p (cp->getPosition().resolve (drawable));
            return Rectangle<float> (p, p);
        }

        void setObjectPosition (ValueTree& state, const Rectangle<float>& newBounds)
        {
            int index = state [Ids::id_];
            ControlPoint* cp = points[index];
            if (cp != 0)
            {
                RelativePoint p (cp->getPosition());
                p.moveToAbsolute (newBounds.getPosition(), drawable);
                cp->setPosition (p, getDocument().getUndoManager());
            }
        }

        void transformObject (ValueTree& state, const AffineTransform& transform)
        {
        }

        float getMarkerPosition (const ValueTree& marker, bool isX)
        {
            return 0;
        }

    private:
        DrawableEditorCanvas* drawableCanvas;
        DrawableTypeInstance drawableItem;
        DrawableComposite* drawable;
    };

    //==============================================================================
    bool canRotate() const  { return true; }

    DragOperation* createDragOperation (const Point<int>& mouseDownPos, Component* snapGuideParentComponent,
                                        const ResizableBorderComponent::Zone& zone, bool isRotating)
    {
        Array<ValueTree> selected, unselected;
        EditorDragOperation* drag = 0;

        if (isControlPointMode())
        {
            DrawableTypeInstance item (getDocument(), controlPointEditingTarget);
            ControlPointDragOperation* cpd = new ControlPointDragOperation (this, item, drawable, mouseDownPos, snapGuideParentComponent, zone);
            drag = cpd;

            for (int i = 0; i < cpd->points.size(); ++i)
            {
                const String pointId (cpd->points.getUnchecked(i)->getID());

                ValueTree v (Ids::controlPoint);
                v.setProperty (Ids::id_, i, 0);

                if (editor.getSelection().isSelected (pointId))
                    selected.add (v);
                else
                    unselected.add (v);
            }
        }
        else
        {
            DrawableComposite::ValueTreeWrapper mainGroup (getDocument().getRootDrawableNode());
            drag = new ObjectDragOperation (this, mouseDownPos, snapGuideParentComponent, zone, isRotating);

            for (int i = mainGroup.getNumDrawables(); --i >= 0;)
            {
                const ValueTree v (mainGroup.getDrawableState (i));

                if (editor.getSelection().isSelected (v[Drawable::ValueTreeWrapperBase::idProperty]))
                    selected.add (v);
                else
                    unselected.add (v);
            }
        }

        drag->initialise (selected, unselected);
        return drag;
    }

    void timerCallback()
    {
        stopTimer();

        if (! Component::isMouseButtonDownAnywhere())
            getUndoManager().beginNewTransaction();
    }

    //==============================================================================
    bool isInterestedInFileDrag (const StringArray& files)
    {
        for (int i = files.size(); --i >= 0;)
            if (File (files[i]).hasFileExtension ("svg;jpg;jpeg;gif;png"))
                return true;

        return false;
    }

    void filesDropped (const StringArray& files, int x, int y)
    {
        for (int i = files.size(); --i >= 0;)
        {
            const File f (files[i]);

            if (f.hasFileExtension ("svg"))
            {
                ValueTree newItem (getDocument().insertSVG (f, screenSpaceToObjectSpace (Point<int> (x, y).toFloat())));

                if (newItem.isValid())
                    getSelection().selectOnly (Drawable::ValueTreeWrapperBase (newItem).getID());
            }
            else if (f.hasFileExtension ("jpg;jpeg;gif;png"))
            {
            }
        }
    }

    //==============================================================================
    class DrawableComponent   : public Component
    {
    public:
        DrawableComponent (DrawableEditorCanvas* canvas_)
            : canvas (canvas_)
        {
            setOpaque (true);
        }

        ~DrawableComponent()
        {
        }

        void updateDrawable()
        {
            repaint();
        }

        void paint (Graphics& g)
        {
            canvas->handleUpdateNowIfNeeded();
            g.fillAll (Colours::white);

            const Point<int> origin (canvas->getOrigin());
            g.setOrigin (origin.getX(), origin.getY());

            if (origin.getX() > 0)
            {
                g.setColour (Colour::greyLevel (0.87f));
                g.drawVerticalLine (0, -10000.0f, 10000.0f);
            }

            if (origin.getY() > 0)
            {
                g.setColour (Colour::greyLevel (0.87f));
                g.drawHorizontalLine (0, -10000.0f, 10000.0f);
            }

            canvas->drawable->draw (g, 1.0f);
        }

    private:
        DrawableEditorCanvas* canvas;
        DrawableEditor& getEditor() const   { return canvas->getEditor(); }
    };

    ScopedPointer<DrawableComposite> drawable;

private:
    //==============================================================================
    DrawableEditor& editor;
};


#endif   // __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__
