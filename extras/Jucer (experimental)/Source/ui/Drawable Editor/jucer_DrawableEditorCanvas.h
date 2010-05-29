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
            jassert (drawable != 0);
            getComponentHolder()->repaint();
        }
        else
        {
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
            getDocument().addNewItemMenuItems (m);
            const int r = m.show();
            getDocument().performNewItemMenuItem (r);
        }
    }

    void objectDoubleClicked (const MouseEvent& e, const ValueTree& state)
    {
        if (state.hasType (DrawablePath::valueTreeType)
             || state.hasType (DrawableImage::valueTreeType)
             || state.hasType (DrawableText::valueTreeType))
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
              controlPointNum (controlPointNum_), isDragging (false), mouseDownResult (false), selected (false)
        {
            selectionId = getControlPointId (drawableState, controlPointNum);
        }

        ~ControlPointComponent()
        {
        }

        void paint (Graphics& g)
        {
            g.setColour (Colour (selected ? 0xaaaaaaaa : 0xaa333333));
            g.drawRect (0, 0, getWidth(), getHeight());

            g.setColour (Colour (selected ? 0xaa000000 : 0x99ffffff));
            g.fillRect (1, 1, getWidth() - 2, getHeight() - 2);
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
                                   ResizableBorderComponent::Zone (ResizableBorderComponent::Zone::centre));
            }

            if (isDragging)
            {
                canvas->continueDrag (e.getEventRelativeTo (getParentComponent()));
                autoScrollForMouseEvent (e);
            }
        }

        void mouseUp (const MouseEvent& e)
        {
            if (isDragging)
            {
                canvas->endDrag (e.getEventRelativeTo (getParentComponent()));
            }
        }

        void mouseDoubleClick (const MouseEvent& e)
        {
        }

        void updatePosition (ControlPoint& point, RelativeCoordinate::NamedCoordinateFinder* nameFinder)
        {
            const Point<float> p (point.getPosition().resolve (nameFinder));
            setBoundsInTargetSpace (Rectangle<int> (roundToInt (p.getX()) - 2, roundToInt (p.getY()) - 2, 7, 7));

            const bool nowSelected = canvas->getSelection().isSelected (selectionId);

            if (selected != nowSelected)
            {
                selected = nowSelected;
                repaint();
            }
        }

    private:
        ValueTree drawableState;
        int controlPointNum;
        bool isDragging, mouseDownResult, selected;
        String selectionId;
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
        item.getAllControlPoints (points);

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
            for (int i = drawable->getNumDrawables(); --i >= 0;)
            {
                Drawable* d = drawable->getDrawable (i);

                if (d->getBounds().intersects (floatArea))
                    itemsFound.add (d->getName());
            }
        }
    }

    bool isControlPointId (const String& itemId)
    {
        return itemId.containsChar ('/');
    }

    static const String getControlPointId (const ValueTree& drawableState, int index)
    {
        return Drawable::ValueTreeWrapperBase (drawableState).getID() + "/" + String (index);
    }

    //==============================================================================
    class ObjectDragOperation  : public EditorDragOperation
    {
    public:
        ObjectDragOperation (DrawableEditorCanvas* canvas_,
                             const MouseEvent& e, const Point<int>& mousePos,
                             Component* snapGuideParentComp_,
                             const ResizableBorderComponent::Zone& zone_)
            : EditorDragOperation (canvas_, e, mousePos, snapGuideParentComp_, zone_), drawableCanvas (canvas_)
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
                                   Drawable* drawable_,
                                   const MouseEvent& e, const Point<int>& mousePos,
                                   Component* snapGuideParentComp_,
                                   const ResizableBorderComponent::Zone& zone_)
            : EditorDragOperation (canvas_, e, mousePos, snapGuideParentComp_, zone_),
              drawableCanvas (canvas_), drawableItem (drawableItem_), drawable (drawable_)
        {
            drawableItem.getAllControlPoints (points);
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
            int index = state [Ids::id_].toString().fromFirstOccurrenceOf ("/", false, false).getIntValue();
            ControlPoint* cp = points[index];
            if (cp == 0)
                return Rectangle<float>();

            Point<float> p (cp->getPosition().resolve (drawable->getParent()));
            return Rectangle<float> (p, p);
        }

        void setObjectPosition (ValueTree& state, const Rectangle<float>& newBounds)
        {
            int index = state [Ids::id_].toString().fromFirstOccurrenceOf ("/", false, false).getIntValue();
            ControlPoint* cp = points[index];
            if (cp != 0)
            {
                RelativePoint p (cp->getPosition());
                p.moveToAbsolute (newBounds.getPosition(), drawable->getParent());
                cp->setPosition (p, getDocument().getUndoManager());
            }
        }

        float getMarkerPosition (const ValueTree& marker, bool isX)
        {
            return 0;
        }

    private:
        DrawableEditorCanvas* drawableCanvas;
        DrawableTypeInstance drawableItem;
        Drawable* drawable;
    };

    //==============================================================================
    DragOperation* createDragOperation (const MouseEvent& e, Component* snapGuideParentComponent,
                                        const ResizableBorderComponent::Zone& zone)
    {
        Array<ValueTree> selected, unselected;
        EditorDragOperation* drag = 0;

        if (isControlPointMode())
        {
            Drawable* d = drawable->getDrawableWithName (Drawable::ValueTreeWrapperBase (controlPointEditingTarget).getID());
            DrawableTypeInstance item (getDocument(), controlPointEditingTarget);

            ControlPointDragOperation* cpd = new ControlPointDragOperation (this, item, d, e, e.getPosition() - origin, snapGuideParentComponent, zone);
            drag = cpd;

            for (int i = 0; i < cpd->points.size(); ++i)
            {
                const String pointId (getControlPointId (item.getState(), i));

                ValueTree v (Ids::controlPoint);
                v.setProperty (Ids::id_, pointId, 0);

                if (editor.getSelection().isSelected (pointId))
                    selected.add (v);
                else
                    unselected.add (v);
            }
        }
        else
        {
            drag = new ObjectDragOperation (this, e, e.getPosition() - origin, snapGuideParentComponent, zone);

            DrawableComposite::ValueTreeWrapper mainGroup (getDocument().getRootDrawableNode());

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
