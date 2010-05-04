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


//==============================================================================
class DrawableEditorCanvas  : public EditorCanvasBase,
                              public Timer
{
public:
    DrawableEditorCanvas (DrawableEditor& editor_)
        : editor (editor_)
    {
        initialise();
        editor.getDocument().getRoot().addListener (this);
    }

    ~DrawableEditorCanvas()
    {
        editor.getDocument().getRoot().removeListener (this);
        shutdown();
    }

    Component* createComponentHolder()
    {
        return new DrawableComponent (this);
    }

    void updateComponents()
    {
        drawable = Drawable::createFromValueTree (getEditor().getDocument().getRootDrawableNode());
        getComponentHolder()->repaint();
        startTimer (500);
    }

    int getCanvasWidth()             { return getDocument().getCanvasWidth().getValue(); }
    int getCanvasHeight()            { return getDocument().getCanvasHeight().getValue(); }
    void setCanvasWidth (int w)      { getDocument().getCanvasWidth() = w; }
    void setCanvasHeight (int h)     { getDocument().getCanvasHeight() = h; }

    MarkerListBase& getMarkerList (bool isX)
    {
        return getDocument().getMarkerList (isX);
    }

    const SelectedItems::ItemType findObjectIdAt (const Point<int>& position)
    {
        return String::empty;
    }

    void showPopupMenu (const Point<int>& position)
    {
        PopupMenu m;

        if (findObjectIdAt (position).isNotEmpty())
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
//            getDocument().addNewComponentMenuItems (m);
  //          const int r = m.show();
    //        getDocument().performNewComponentMenuItem (r);
        }
    }

    const ValueTree getObjectState (const String& objectId)
    {
        return ValueTree();
    }

    const Rectangle<int> getObjectPosition (const ValueTree& state)
    {
        return Rectangle<int>();//getDocument().getCoordsFor (state).resolve (getDocument());
    }

    RectangleCoordinates getObjectCoords (const ValueTree& state)
    {
        return RectangleCoordinates();
//        return getDocument().getCoordsFor (state);
    }

    SelectedItems& getSelection()
    {
        return editor.getSelection();
    }

    void deselectNonDraggableObjects()
    {
    }

    void findLassoItemsInArea (Array <SelectedItems::ItemType>& itemsFound, const Rectangle<int>& area)
    {
    }

    //==============================================================================
    class DragOperation  : public EditorDragOperation
    {
    public:
        DragOperation (DrawableEditorCanvas* canvas_,
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
        DrawableDocument& getDocument() throw()                 { return static_cast <DrawableEditorCanvas*> (canvas)->getDocument(); }

        int getCanvasWidth()                                    { return getDocument().getCanvasWidth().getValue(); }
        int getCanvasHeight()                                   { return getDocument().getCanvasHeight().getValue(); }

        UndoManager& getUndoManager()                           { return *getDocument().getUndoManager(); }

        const Rectangle<float> getObjectPosition (const ValueTree& state)
        {
            return Rectangle<float> ();
        }

        bool setObjectPosition (ValueTree& state, const Rectangle<float>& newBounds)
        {
            return false;
        }
    };

    DragOperation* createDragOperation (const MouseEvent& e, Component* snapGuideParentComponent,
                                        const ResizableBorderComponent::Zone& zone)
    {
        DragOperation* d = new DragOperation (this, e, snapGuideParentComponent, zone);

        Array<ValueTree> selected, unselected;

        /*for (int i = getDocument().getNumComponents(); --i >= 0;)
        {
            const ValueTree v (getDocument().getComponent (i));
            if (editor.getSelection().isSelected (v [ComponentDocument::idProperty]))
                selected.add (v);
            else
                unselected.add (v);
        }*/

        d->initialise (selected, unselected);
        return d;
    }

    UndoManager& getUndoManager()
    {
        return *getDocument().getUndoManager();
    }

    DrawableEditor& getEditor() throw()                         { return editor; }
    DrawableDocument& getDocument() throw()                     { return editor.getDocument(); }

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
            g.fillAll (Colours::white);
            canvas->drawable->draw (g, 1.0f);
        }

    private:
        DrawableEditorCanvas* canvas;
        DrawableEditor& getEditor() const   { return canvas->getEditor(); }
    };

    ScopedPointer<Drawable> drawable;

private:
    //==============================================================================
    DrawableEditor& editor;
};


#endif   // __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__
