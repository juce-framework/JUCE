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
class ComponentCanvas   : public Component,
                          public ValueTree::Listener
{
public:
    ComponentCanvas (ComponentEditor& editor_)
        : editor (editor_)
    {
        setOpaque (true);
        addAndMakeVisible (componentHolder = new Component());
        addAndMakeVisible (overlay = new OverlayComponent (*this));

        setSize (500, 500);

        getDocument().getRoot().addListener (this);
        updateComponents();
    }

    ~ComponentCanvas()
    {
        getDocument().getRoot().removeListener (this);
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
    }

    void resized()
    {
        componentHolder->setSize (getWidth(), getHeight());
        overlay->setSize (getWidth(), getHeight());
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
            else
            {
                doc.updateComponent (c);
            }
        }
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
        Array <Component*> comps;
        for (int i = 0; i < selection.getNumSelected(); ++i)
        {
            Component* c = getComponentForUID (selection.getSelectedItem (i));
            jassert (c != 0);
            if (c != 0)
                comps.add (c);
        }

        return comps;
    }

private:
    ComponentEditor& editor;

    //==============================================================================
    class ComponentSelectorFrame    : public Component,
                                      public ComponentListener
    {
    public:
        ComponentSelectorFrame (ComponentCanvas& canvas_,
                                Component* componentToAttachTo)
            : canvas (canvas_),
              component (componentToAttachTo),
              borderThickness (4)
        {
            componentMovedOrResized (*componentToAttachTo, true, true);
            componentToAttachTo->addComponentListener (this);
        }

        ~ComponentSelectorFrame()
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
            repaint();
            updateDragZone (e.getPosition());
        }

        void mouseExit (const MouseEvent& e)
        {
            repaint();
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
                canvas.getDocument().beginDrag (canvas.getSelectedComps(), e, dragZone);
            }
        }

        void mouseDrag (const MouseEvent& e)
        {
            if (component != 0)
                canvas.getDocument().continueDrag (e);
        }

        void mouseUp (const MouseEvent& e)
        {
            if (component != 0)
                canvas.getDocument().endDrag (e);

            updateDragZone (e.getPosition());
        }

        void componentMovedOrResized (Component&, bool wasMoved, bool wasResized)
        {
            if (component != 0)
                setBounds (component->getBounds().expanded (borderThickness, borderThickness));
        }

        void resized()
        {
        }

        uint32 getTargetComponentUID() const  { return component == 0 ? 0 : component->getComponentUID(); }

    private:
        ComponentCanvas& canvas;
        Component::SafePointer<Component> component;
        int dragZone;
        const int borderThickness;

        void updateDragZone (const Point<int>& p)
        {
            int newZone = 0;

            Rectangle<int> r (0, 0, getWidth(), getHeight());
            r = r.reduced (borderThickness, borderThickness);

            if (! r.contains (p))
            {
                const int bw = jmax (borderThickness, proportionOfWidth (0.1f), jmin (10, proportionOfWidth (0.33f)));
                const int bh = jmax (borderThickness, proportionOfHeight (0.1f), jmin (10, proportionOfHeight (0.33f)));

                if (p.getX() < bw)
                    newZone |= ComponentDocument::zoneL;
                else if (p.getX() >= getWidth() - bw)
                    newZone |= ComponentDocument::zoneR;

                if (p.getY() < bh)
                    newZone |= ComponentDocument::zoneT;
                else if (p.getY() >= getHeight() - bh)
                    newZone |= ComponentDocument::zoneB;
            }

            if (dragZone != newZone)
            {
                dragZone = newZone;

                MouseCursor::StandardCursorType mc = MouseCursor::NormalCursor;

                switch (newZone)
                {
                    case (ComponentDocument::zoneL | ComponentDocument::zoneT):   mc = MouseCursor::TopLeftCornerResizeCursor; break;
                    case ComponentDocument::zoneT:                                mc = MouseCursor::TopEdgeResizeCursor; break;
                    case (ComponentDocument::zoneR | ComponentDocument::zoneT):   mc = MouseCursor::TopRightCornerResizeCursor; break;
                    case ComponentDocument::zoneL:                                mc = MouseCursor::LeftEdgeResizeCursor; break;
                    case ComponentDocument::zoneR:                                mc = MouseCursor::RightEdgeResizeCursor; break;
                    case (ComponentDocument::zoneL | ComponentDocument::zoneB):   mc = MouseCursor::BottomLeftCornerResizeCursor; break;
                    case ComponentDocument::zoneB:                                mc = MouseCursor::BottomEdgeResizeCursor; break;
                    case (ComponentDocument::zoneR | ComponentDocument::zoneB):   mc = MouseCursor::BottomRightCornerResizeCursor; break;
                    default:                                                      mc = MouseCursor::NormalCursor; break;
                }

                setMouseCursor (mc);
            }
        }
    };

    //==============================================================================
    class OverlayComponent  : public Component,
                              public LassoSource <ComponentDocument::SelectedItems::ItemType>,
                              public ChangeListener
    {
    public:
        OverlayComponent (ComponentCanvas& canvas_)
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
                }
            }
        }

        void mouseDrag (const MouseEvent& e)
        {
            if (lasso != 0)
                lasso->dragLasso (e);
        }

        void mouseUp (const MouseEvent& e)
        {
            if (lasso != 0)
            {
                lasso->endLasso();
                lasso = 0;

                if (e.mouseWasClicked())
                    canvas.getSelection().deselectAll();
            }
            else
            {
                canvas.getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, ! e.mouseWasClicked(), mouseDownResult);
            }
        }

        void findLassoItemsInArea (Array <ComponentDocument::SelectedItems::ItemType>& itemsFound, int x, int y, int width, int height)
        {
            const Rectangle<int> lassoArea (x, y, width, height);

            for (int i = canvas.getComponentHolder()->getNumChildComponents(); --i >= 0;)
            {
                Component* c = canvas.getComponentHolder()->getChildComponent(i);
                if (c != this && c->getBounds().intersects (lassoArea))
                    itemsFound.add (c->getComponentUID());
            }
        }

        ComponentDocument::SelectedItems& getLassoSelection()   { return canvas.getSelection(); }

        void changeListenerCallback (void*)
        {
            updateSelectedComponentOverlays();
        }

    private:
        ComponentCanvas& canvas;
        ScopedPointer <LassoComponent <ComponentDocument::SelectedItems::ItemType> > lasso;
        bool mouseDownResult;
        uint32 mouseDownCompUID;

        ComponentSelectorFrame* getSelectorFrameFor (Component* c) const
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                ComponentSelectorFrame* overlay = dynamic_cast <ComponentSelectorFrame*> (getChildComponent(i));
                if (overlay != 0 && overlay->getTargetComponentUID() == c->getComponentUID())
                    return overlay;
            }

            return 0;
        }

        void updateSelectedComponentOverlays()
        {
            ComponentDocument::SelectedItems& selection = canvas.getSelection();

            int i;
            for (i = getNumChildComponents(); --i >= 0;)
            {
                ComponentSelectorFrame* overlay = dynamic_cast <ComponentSelectorFrame*> (getChildComponent(i));

                if (overlay != 0 && ! selection.isSelected (overlay->getTargetComponentUID()))
                    delete overlay;
            }

            for (i = canvas.getComponentHolder()->getNumChildComponents(); --i >= 0;)
            {
                Component* c = canvas.getComponentHolder()->getChildComponent(i);

                if (c != this && selection.isSelected (c->getComponentUID()) && getSelectorFrameFor (c) == 0)
                    addAndMakeVisible (new ComponentSelectorFrame (canvas, c));
            }
        }
    };

    Component* componentHolder;
    OverlayComponent* overlay;
    ComponentDocument::SelectedItems selection;

    Component* getComponentForUID (const uint32 uid) const
    {
        for (int i = getNumChildComponents(); --i >= 0;)
            if (componentHolder->getChildComponent (i)->getComponentUID() == uid)
                return componentHolder->getChildComponent (i);

        return 0;
    }
};


//==============================================================================
ComponentEditor::ComponentEditor (OpenDocumentManager::Document* document,
                                  Project* project_, ComponentDocument* componentDocument_)
    : DocumentEditorComponent (document),
      project (project_),
      componentDocument (componentDocument_)
{
    jassert (componentDocument != 0);

    setOpaque (true);

    addAndMakeVisible (viewport = new Viewport());
    viewport->setViewedComponent (new ComponentCanvas (*this));
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
    viewport->setBounds (0, 0, getWidth(), getHeight());
}
