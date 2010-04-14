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

        //ScopedPointer<Coordinate::MarkerResolver> markers (document.createMarkerResolver (state, component->getParentComponent()));
        //int anchor1 = roundToInt (coord.getAnchorPoint1().resolve (*markers));
        //int anchor2 = roundToInt (coord.getAnchorPoint2().resolve (*markers));

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
static const double tickSizes[] = { 1.0, 2.0, 5.0,
                                    10.0, 20.0, 50.0,
                                    100.0, 200.0, 500.0, 1000.0 };

class TickIterator
{
public:
    TickIterator (const double startValue_, const double endValue_, const double valuePerPixel_,
                  int minPixelsPerTick, int minWidthForLabels)
        : startValue (startValue_),
          endValue (endValue_),
          valuePerPixel (valuePerPixel_)
    {
        tickLevelIndex  = findLevelIndexForValue (valuePerPixel * minPixelsPerTick);
        labelLevelIndex = findLevelIndexForValue (valuePerPixel * minWidthForLabels);

        tickPosition = pixelsToValue (-minWidthForLabels);
        tickPosition = snapValueDown (tickPosition, tickLevelIndex);
    }

    bool getNextTick (float& pixelX, float& tickLength, String& label)
    {
        const double tickUnits = tickSizes [tickLevelIndex];
        tickPosition += tickUnits;

        const int totalLevels = sizeof (tickSizes) / sizeof (*tickSizes);
        int highestIndex = tickLevelIndex;

        while (++highestIndex < totalLevels)
        {
            const double ticksAtThisLevel = tickPosition / tickSizes [highestIndex];

            if (fabs (ticksAtThisLevel - floor (ticksAtThisLevel + 0.5)) > 0.000001)
                break;
        }

        --highestIndex;

        if (highestIndex >= labelLevelIndex)
            label = getDescriptionOfValue (tickPosition, labelLevelIndex);
        else
            label = String::empty;

        tickLength = (highestIndex + 1 - tickLevelIndex) / (float) (totalLevels + 1 - tickLevelIndex);
        pixelX = valueToPixels (tickPosition);

        return tickPosition < endValue;
    }

private:
    double tickPosition;
    int tickLevelIndex, labelLevelIndex;
    const double startValue, endValue, valuePerPixel;

    int findLevelIndexForValue (const double value) const
    {
        int i;
        for (i = 0; i < sizeof (tickSizes) / sizeof (*tickSizes); ++i)
            if (tickSizes [i] >= value)
                break;

        return i;
    }

    double pixelsToValue (int pixels) const
    {
        return startValue + pixels * valuePerPixel;
    }

    float valueToPixels (double value) const
    {
        return (float) ((value - startValue) / valuePerPixel);
    }

    static double snapValueToNearest (const double t, const int valueLevelIndex)
    {
        const double unitsPerInterval = tickSizes [valueLevelIndex];
        return unitsPerInterval * floor (t / unitsPerInterval + 0.5);
    }

    static double snapValueDown (const double t, const int valueLevelIndex)
    {
        const double unitsPerInterval = tickSizes [valueLevelIndex];
        return unitsPerInterval * floor (t / unitsPerInterval);
    }

    static inline int roundDoubleToInt (const double value)
    {
        union { int asInt[2]; double asDouble; } n;
        n.asDouble = value + 6755399441055744.0;

    #if TARGET_RT_BIG_ENDIAN
        return n.asInt [1];
    #else
        return n.asInt [0];
    #endif
    }

    static const String getDescriptionOfValue (const double value, const int valueLevelIndex)
    {
        return String (roundToInt (value));
    }
};


//==============================================================================
class ComponentEditor::Canvas   : public Component,
                                  public ValueTree::Listener,
                                  public Timer
{
public:
    Canvas (ComponentEditor& editor_)
        : editor (editor_), border (14), resizerThickness (4)
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

        g.drawRect (getContentArea().expanded (resizerThickness, resizerThickness), resizerThickness);

        g.setFont (border.getBottom() - 5.0f);
        g.setColour (Colours::grey);
        g.drawText (String (componentHolder->getWidth()) + " x " + String (componentHolder->getHeight()),
                    0, 0, getWidth() - border.getRight(), getHeight(), Justification::bottomRight, false);

        g.setFont (border.getTop() - 5.0f);
        g.setColour (Colours::darkgrey);

        const float x = border.getLeft();
        const float y = border.getTop();

        g.drawHorizontalLine (y, 2.0f, getWidth() - border.getRight());
        g.drawVerticalLine (x, 2.0f, getHeight() - border.getBottom());

        {
            TickIterator ticks (0, componentHolder->getWidth(), 1.0, 10, 50);

            float pos, tickLength;
            String label;

            while (ticks.getNextTick (pos, tickLength, label))
            {
                if (pos > 0)
                {
                    g.drawVerticalLine (x + pos, y - tickLength * y, y);
                    g.drawSingleLineText (label, x + pos + 2, y - 6);
                }
            }
        }

        {
            TickIterator ticks (0, componentHolder->getHeight(), 1.0, 10, 80);

            float pos, tickLength;
            String label;

            while (ticks.getNextTick (pos, tickLength, label))
            {
                if (pos > 0)
                {
                    g.drawHorizontalLine (y + pos, x - tickLength * x, x);

                    g.drawTextAsPath (label, AffineTransform::rotation (float_Pi / -2.0f)
                                                             .translated (x - 6, y + pos - 2));
                }
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
        showSizeGuides();
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (dragZone.isDraggingRightEdge() || dragZone.isDraggingBottomEdge())
        {
            setSize (dragZone.isDraggingRightEdge() ? jmax (0, dragStartSize.getWidth() + e.getDistanceFromDragStartX())
                                                    : dragStartSize.getWidth(),
                     dragZone.isDraggingBottomEdge() ? jmax (0, dragStartSize.getHeight() + e.getDistanceFromDragStartY())
                                                     : dragStartSize.getHeight());
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

private:
    ComponentEditor& editor;
    const BorderSize border;
    const int resizerThickness;
    ResizableBorderComponent::Zone dragZone;
    Rectangle<int> dragStartSize;

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
                canvas.getDocument().beginDrag (canvas.getSelectedComps(), e, getParentComponent(), dragZone);
                canvas.showSizeGuides();
            }
        }

        void mouseDrag (const MouseEvent& e)
        {
            if (component != 0)
                canvas.getDocument().continueDrag (e);
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
