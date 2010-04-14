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
                        Component* parentForOverlays, Type type_)
        : document (document_), state (state_), component (component_), type (type_),
          font (10.0f)
    {
        component->addComponentListener (this);

        setAlwaysOnTop (true);
        parentForOverlays->addAndMakeVisible (this);
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

        ScopedPointer<Coordinate::MarkerResolver> markers (document.createMarkerResolver (state, component->getParentComponent()));
        int anchor1 = roundToInt (coord.getAnchorPoint1().resolve (*markers));
        int anchor2 = roundToInt (coord.getAnchorPoint2().resolve (*markers));

        Point<int> p1, p2;

        switch (type)
        {
            case left:      p1 = Point<int> (component->getX(), component->getY() + component->proportionOfHeight (0.33f));
                            p2 = Point<int> (anchor1, p1.getY()); break;
            case right:     p1 = Point<int> (component->getRight(), component->getY() + component->proportionOfHeight (0.66f));
                            p2 = Point<int> (anchor1, p1.getY()); break;
            case top:       p1 = Point<int> (component->getX() + component->proportionOfWidth (0.33f), component->getY());
                            p2 = Point<int> (p1.getX(), anchor1); break;
            case bottom:    p1 = Point<int> (component->getX() + component->proportionOfWidth (0.66f), component->getBottom());
                            p2 = Point<int> (p1.getX(), anchor1); break;
            default:        jassertfalse; break;
        }

        Rectangle<int> bounds (Rectangle<int> (p1, p2).expanded (4, 4));
        Point<int> textPos ((p1.getX() + p2.getX()) / 2,
                            (p1.getY() + p2.getY()) / 2);
        int textW = (int) font.getStringWidth (getName());
        int textH = (int) font.getHeight();
        Rectangle<int> textRect (textPos.getX() - textW / 2, textPos.getY() - textH / 2, textW, textH);

        if (isHorizontal)
            textRect = textRect - Point<int> (0, textH / 2 + 4);

        bounds = bounds.getUnion (textRect);
        setBounds (bounds);

        lineEnd1 = p1 - bounds.getPosition();
        lineEnd2 = p2 - bounds.getPosition();
        textArea = textRect - bounds.getPosition();
        repaint();
    }

    void paint (Graphics& g)
    {
        Path p;
        p.addLineSegment ((float) lineEnd1.getX(), (float) lineEnd1.getY(), (float) lineEnd2.getX(), (float) lineEnd2.getY(), 1.6f);
        const float startBlobSize = 2.0f;
        p.addEllipse (lineEnd1.getX() - startBlobSize, lineEnd1.getY() - startBlobSize, startBlobSize * 2.0f, startBlobSize * 2.0f);
        const float endBlobSize = 4.0f;
        p.addEllipse (lineEnd2.getX() - endBlobSize, lineEnd2.getY() - endBlobSize, endBlobSize * 2.0f, endBlobSize * 2.0f);

        g.setColour (Colours::black.withAlpha (0.3f));
        g.fillPath (p);

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
        : editor (editor_), borderThickness (4)
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
        componentHolder->setBounds (getLocalBounds().reduced (borderThickness, borderThickness));
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
        dragStartSize = getBounds();
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (dragZone.isDraggingRightEdge() || dragZone.isDraggingBottomEdge())
        {
            showSizeGuides();
            setSize (jmax (0, dragStartSize.getWidth() + e.getDistanceFromDragStartX()),
                     jmax (0, dragStartSize.getHeight() + e.getDistanceFromDragStartY()));
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        hideSizeGuides();
        updateDragZone (e.getPosition());
    }

    void updateDragZone (const Point<int>& p)
    {
        ResizableBorderComponent::Zone newZone
            = ResizableBorderComponent::Zone::fromPositionOnBorder (getLocalBounds(),
                                                                    BorderSize (borderThickness), p);

        newZone = ResizableBorderComponent::Zone (newZone.getZoneFlags()
                                                   & (ResizableBorderComponent::Zone::right | ResizableBorderComponent::Zone::bottom));

        if (dragZone != newZone)
        {
            dragZone = newZone;
            setMouseCursor (newZone.getMouseCursor());
        }
    }

    void showSizeGuides()   { overlay->showSizeGuides(); }
    void hideSizeGuides()   { overlay->hideSizeGuides(); }

private:
    ComponentEditor& editor;
    const int borderThickness;
    ResizableBorderComponent::Zone dragZone;
    Rectangle<int> dragStartSize;

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
                canvas.getDocument().beginDrag (canvas.getSelectedComps(), e, getParentComponent(), dragZone);
            }
        }

        void mouseDrag (const MouseEvent& e)
        {
            if (component != 0)
            {
                canvas.showSizeGuides();
                canvas.getDocument().continueDrag (e);
            }
        }

        void mouseUp (const MouseEvent& e)
        {
            canvas.hideSizeGuides();

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

        void showSizeGuides()
        {
            if (sizeGuides.size() == 0)
            {
                const ValueTree v (canvas.getDocument().getComponentState (component));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, getParentComponent(), SizeGuideComponent::left));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, getParentComponent(), SizeGuideComponent::right));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, getParentComponent(), SizeGuideComponent::top));
                sizeGuides.add (new SizeGuideComponent (canvas.getDocument(), v, component, getParentComponent(), SizeGuideComponent::bottom));
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
                    canvas.getDocument().beginDrag (canvas.getSelectedComps(), e, getParentComponent(),
                                                    ResizableBorderComponent::Zone (ResizableBorderComponent::Zone::centre));
                }

                canvas.getDocument().continueDrag (e);
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

            canvas.getDocument().endDrag (e);
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
