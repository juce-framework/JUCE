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

#ifndef __JUCER_COMPONENTEDITORCANVAS_H_37C33B56__
#define __JUCER_COMPONENTEDITORCANVAS_H_37C33B56__


//==============================================================================
class ComponentEditorCanvas   : public EditorCanvasBase,
                                public Timer
{
public:
    //==============================================================================
    ComponentEditorCanvas (ComponentEditor& editor_)
        : editor (editor_)
    {
        initialise();
        getDocument().getRoot().addListener (this);
    }

    ~ComponentEditorCanvas()
    {
        getDocument().getRoot().removeListener (this);
        shutdown();
    }

    ComponentEditor& getEditor()         { return editor; }
    ComponentDocument& getDocument()     { return editor.getDocument(); }

    void timerCallback()
    {
        stopTimer();

        if (! Component::isMouseButtonDownAnywhere())
            getDocument().beginNewTransaction();
    }

    Component* createComponentHolder()
    {
        return new Component();
    }

    static Component* findComponentForState (Component* compHolder, ComponentDocument& doc, const ValueTree& state)
    {
        for (int i = compHolder->getNumChildComponents(); --i >= 0;)
        {
            Component* const c = compHolder->getChildComponent (i);
            if (doc.isStateForComponent (state, c))
                return c;
        }

        return 0;
    }

    static void updateComponentsIn (Component* compHolder, ComponentDocument& doc, SelectedItems& selection)
    {
        int i;
        for (i = compHolder->getNumChildComponents(); --i >= 0;)
        {
            Component* c = compHolder->getChildComponent (i);

            if (! doc.containsComponent (c))
            {
                selection.deselect (ComponentDocument::getJucerIDFor (c));
                delete c;
            }
        }

        Array <Component*> componentsInOrder;

        const int num = doc.getNumComponents();
        for (i = 0; i < num; ++i)
        {
            const ValueTree v (doc.getComponent (i));
            Component* c = findComponentForState (compHolder, doc, v);

            if (c == 0)
            {
                c = doc.createComponent (i);
                compHolder->addAndMakeVisible (c);
            }

            doc.updateComponent (c);
            componentsInOrder.add (c);
        }

        // Make sure the z-order is correct..
        if (num > 0)
        {
            componentsInOrder.getLast()->toFront (false);

            for (i = num - 1; --i >= 0;)
                componentsInOrder.getUnchecked(i)->toBehind (componentsInOrder.getUnchecked (i + 1));
        }
    }

    void updateComponents()
    {
        updateComponentsIn (getComponentHolder(), getDocument(), editor.getSelection());
        startTimer (500);
    }

    int getCanvasWidth()             { return getDocument().getCanvasWidth().getValue(); }
    int getCanvasHeight()            { return getDocument().getCanvasHeight().getValue(); }
    void setCanvasWidth (int w)      { getDocument().getCanvasWidth() = w; }
    void setCanvasHeight (int h)     { getDocument().getCanvasHeight() = h; }

    ComponentDocument::MarkerList& getMarkerList (bool isX)
    {
        return getDocument().getMarkerList (isX);
    }

    const SelectedItems::ItemType findObjectIdAt (const Point<int>& position)
    {
        for (int i = getComponentHolder()->getNumChildComponents(); --i >= 0;)
        {
            Component* const c = getComponentHolder()->getChildComponent(i);
            if (c->getBounds().contains (position))
                return ComponentDocument::getJucerIDFor (c);
        }

        return String::empty;
    }

    void showPopupMenu (const Point<int>& position)
    {
        if (findObjectIdAt (position).isNotEmpty())
        {
            PopupMenu m;
            m.addCommandItem (commandManager, CommandIDs::toFront);
            m.addCommandItem (commandManager, CommandIDs::toBack);
            m.addSeparator();
            m.addCommandItem (commandManager, StandardApplicationCommandIDs::del);
            const int r = m.show();
            (void) r;
        }
        else
        {
            editor.showNewComponentMenu (0);
        }
    }

    const ValueTree getObjectState (const String& objectId)
    {
        return getDocument().getComponentWithID (objectId);
    }

    const Rectangle<int> getObjectPosition (const ValueTree& state)
    {
        return getDocument().getCoordsFor (state).resolve (getDocument());
    }

    RectangleCoordinates getObjectCoords (const ValueTree& state)
    {
        return getDocument().getCoordsFor (state);
    }

    SelectedItems& getSelection()
    {
        return editor.getSelection();
    }

    void deselectNonDraggableObjects()
    {
        editor.deselectNonComponents();
    }

    void findLassoItemsInArea (Array <SelectedItems::ItemType>& itemsFound, const Rectangle<int>& area)
    {
        for (int i = getComponentHolder()->getNumChildComponents(); --i >= 0;)
        {
            Component* c = getComponentHolder()->getChildComponent(i);
            if (c->getBounds().intersects (area))
                itemsFound.add (ComponentDocument::getJucerIDFor (c));
        }
    }

    //==============================================================================
    class DragOperation  : public EditorDragOperation
    {
    public:
        DragOperation (ComponentEditorCanvas* canvas_,
                       const MouseEvent& e,
                       Component* snapGuideParentComp_,
                       const ResizableBorderComponent::Zone& zone_)
            : EditorDragOperation (canvas_, e, snapGuideParentComp_, zone_)
        {
        }

        ~DragOperation()
        {
            getUndoManager().beginNewTransaction();
        }

    protected:
        ComponentDocument& getDocument() throw()                { return static_cast <ComponentEditorCanvas*> (canvas)->getDocument(); }

        int getCanvasWidth()                                    { return getDocument().getCanvasWidth().getValue(); }
        int getCanvasHeight()                                   { return getDocument().getCanvasHeight().getValue(); }

        UndoManager& getUndoManager()                           { return *getDocument().getUndoManager(); }

        const Rectangle<float> getObjectPosition (const ValueTree& state)
        {
            ComponentDocument& doc = getDocument();
            RectangleCoordinates relativePos (doc.getCoordsFor (state));
            const Rectangle<int> intPos (relativePos.resolve (doc));

            return intPos.toFloat();
        }

        bool setObjectPosition (ValueTree& state, const Rectangle<float>& newBounds)
        {
            ComponentDocument& doc = getDocument();
            RectangleCoordinates pr (doc.getCoordsFor (state));
            pr.moveToAbsolute (newBounds, doc);

            return doc.setCoordsFor (state, pr);
        }
    };

    DragOperation* createDragOperation (const MouseEvent& e, Component* snapGuideParentComponent,
                                        const ResizableBorderComponent::Zone& zone)
    {
        DragOperation* d = new DragOperation (this, e, snapGuideParentComponent, zone);

        Array<ValueTree> selected, unselected;

        for (int i = getDocument().getNumComponents(); --i >= 0;)
        {
            const ValueTree v (getDocument().getComponent (i));
            if (editor.getSelection().isSelected (v [ComponentDocument::idProperty]))
                selected.add (v);
            else
                unselected.add (v);
        }

        d->initialise (selected, unselected);
        return d;
    }

    UndoManager& getUndoManager()
    {
        return *getDocument().getUndoManager();
    }

private:
    //==============================================================================
    ComponentEditor& editor;
};


#endif  // __JUCER_COMPONENTEDITORCANVAS_H_37C33B56__
