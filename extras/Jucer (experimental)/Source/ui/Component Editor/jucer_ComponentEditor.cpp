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
        : editor (editor_), borderThickness (4)
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
        g.setColour (Colour::greyLevel (0.9f));
        g.drawRect (0, 0, getWidth(), getHeight(), borderThickness);
    }

    void resized()
    {
        componentHolder->setBounds (borderThickness, borderThickness,
                                    getWidth() - borderThickness * 2, getHeight() - borderThickness * 2);

        overlay->setBounds (componentHolder->getBounds());
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
    const int borderThickness;

    //==============================================================================
    class ComponentResizeFrame    : public Component,
                                    public ComponentListener
    {
    public:
        ComponentResizeFrame (ComponentCanvas& canvas_,
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

        bool hitTest (int x, int y)
        {
            return ! getCentreArea().contains (x, y);
        }

        uint32 getTargetComponentUID() const  { return component == 0 ? 0 : component->getComponentUID(); }

    private:
        ComponentCanvas& canvas;
        Component::SafePointer<Component> component;
        ResizableBorderComponent::Zone dragZone;
        const int borderThickness;

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
                    canvas.getDocument().beginDrag (canvas.getSelectedComps(), e,
                                                    ResizableBorderComponent::Zone (ResizableBorderComponent::Zone::centre));
                }

                canvas.getDocument().continueDrag (e);
            }
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
            else if (! e.mods.isPopupMenu())
            {
                if (! isDraggingClickedComp)
                    canvas.getSelection().addToSelectionOnMouseUp (mouseDownCompUID, e.mods, ! e.mouseWasClicked(), mouseDownResult);
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

        ComponentDocument::SelectedItems& getLassoSelection()   { return canvas.getSelection(); }

        void changeListenerCallback (void*)
        {
            updateSelectedComponentResizeFrames();
        }

        void modifierKeysChanged (const ModifierKeys&)
        {
            Desktop::getInstance().getMainMouseSource().triggerFakeMove();
        }

    private:
        ComponentCanvas& canvas;
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
        jassertfalse //xxx
        return true;

    case CommandIDs::redo:
        jassertfalse //xxx
        return true;

    default:
        break;
    }

    return DocumentEditorComponent::perform (info);
}
