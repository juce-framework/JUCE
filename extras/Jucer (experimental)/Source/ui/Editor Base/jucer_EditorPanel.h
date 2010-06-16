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

#ifndef __JUCER_EDITORPANEL_H_8E192A99__
#define __JUCER_EDITORPANEL_H_8E192A99__

#include "../../utility/jucer_TickIterator.h"
#include "jucer_EditorCanvas.h"


//==============================================================================
class EditorPanelBase  : public Component
{
public:
    EditorPanelBase()
        : rulerX (true), rulerY (false), markersVisible (true), snappingEnabled (true), canvas (0)
    {
        setOpaque (true);

        background = ImageCache::getFromMemory (BinaryData::brushed_aluminium_png, BinaryData::brushed_aluminium_pngSize);

        addAndMakeVisible (&toolbar);
        toolbar.setStyle (Toolbar::textOnly);

        addAndMakeVisible (&viewport);
        addAndMakeVisible (&rulerX);
        addAndMakeVisible (&rulerY);
        addAndMakeVisible (&tooltipBar);

        addChildComponent (&tree);
        tree.setRootItemVisible (true);
        tree.setMultiSelectEnabled (true);
        tree.setDefaultOpenness (true);
        tree.setColour (TreeView::backgroundColourId, Colour::greyLevel (0.92f));
        tree.setIndentSize (15);
    }

    ~EditorPanelBase()
    {
        jassert (infoPanel == 0); // remember to call shutdown()
    }

    void initialise (EditorCanvasBase* canvas_, ToolbarItemFactory& toolbarFactory, TreeViewItem* treeRootItem)
    {
        canvas = canvas_;
        toolbar.addDefaultItems (toolbarFactory);
        viewport.setViewedComponent (canvas);
        addAndMakeVisible (infoPanel = new InfoPanel (this));
        tree.setRootItem (treeRootItem);
        resized();
    }

    void shutdown()
    {
        tree.deleteRootItem();
        infoPanel = 0;
    }

    //==============================================================================
    void showOrHideProperties()
    {
        infoPanel->setVisible (! infoPanel->isVisible());
        resized();
    }

    bool arePropertiesVisible() const       { return infoPanel->isVisible(); }

    void showOrHideTree()
    {
        tree.setVisible (! tree.isVisible());
        resized();
    }

    bool isTreeVisible() const              { return tree.isVisible(); }

    void showOrHideMarkers()
    {
        markersVisible = ! markersVisible;
        commandManager->commandStatusChanged();
    }

    bool areMarkersVisible() const          { return markersVisible; }

    void toggleSnapping()
    {
        snappingEnabled = ! snappingEnabled;
        commandManager->commandStatusChanged();
    }

    bool isSnappingEnabled() const          { return snappingEnabled; }

    //==============================================================================
    virtual SelectedItemSet<String>& getSelection() = 0;
    virtual void getSelectedItemProperties (Array<PropertyComponent*>& newComps) = 0;

    static int getRulerThickness() throw()   { return 16; }

    void paint (Graphics& g)
    {
        g.setTiledImageFill (background, 0, 0, 1.0f);
        g.fillAll();
    }

    void resized()
    {
        Rectangle<int> area (getLocalBounds());

        toolbar.setBounds (area.removeFromTop (22));

        if (infoPanel != 0 && infoPanel->isVisible())
        {
            Rectangle<int> panel (area.removeFromRight (200));
            tooltipBar.setBounds (panel.removeFromBottom (30));
            infoPanel->setBounds (panel);
        }
        else
        {
            tooltipBar.setBounds (area.removeFromBottom (18));
        }

        if (tree.isVisible())
            tree.setBounds (area.removeFromLeft (200));

        Rectangle<int> ry (area.removeFromLeft (getRulerThickness()));
        ry.removeFromTop (getRulerThickness());
        rulerY.setBounds (ry);
        rulerX.setBounds (area.removeFromTop (getRulerThickness()));
        viewport.setBounds (area);
        updateRulers();
    }

    void updateRulers()
    {
        if (canvas != 0)
        {
            rulerX.update (canvas->getScale(), canvas->getComponentHolder());
            rulerY.update (canvas->getScale(), canvas->getComponentHolder());
        }

        updateMarkers();
    }

    void updateMarkers()
    {
        if (canvas != 0)
        {
            const int vw = viewport.getMaximumVisibleWidth();
            const int vh = viewport.getMaximumVisibleHeight();
            rulerX.updateMarkers (canvas->getMarkerList (true), canvas, vw, vh);
            rulerY.updateMarkers (canvas->getMarkerList (false), canvas, vw, vh);
        }
    }

private:
    //==============================================================================
    class InfoPanel  : public Component,
                       public ChangeListener
    {
    public:
        InfoPanel (EditorPanelBase* owner_)
          : owner (owner_)
        {
            setOpaque (true);

            addAndMakeVisible (props = new PropertyPanel());

            owner->getSelection().addChangeListener (this);
        }

        ~InfoPanel()
        {
            owner->getSelection().removeChangeListener (this);
            props->clear();
            deleteAllChildren();
        }

        void changeListenerCallback (void*)
        {
            Array <PropertyComponent*> newComps;
            owner->getSelectedItemProperties (newComps);

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
        EditorPanelBase* owner;
        PropertyPanel* props;
    };

    //==============================================================================
    class RulerComponent    : public Component
    {
    public:
        RulerComponent (const bool isX_)
            : isX (isX_), range (0.0, 100.0), canvas (0)
        {
        }

        ~RulerComponent()
        {
        }

        void update (const EditorCanvasBase::Scale& scale, Component* contentHolder)
        {
            const Point<int> origin (contentHolder->relativePositionToOtherComponent (this, scale.origin));
            const double start = isX ? origin.getX() : origin.getY();

            const Range<double> newRange (-start * scale.scale,
                                          ((isX ? getWidth() : getHeight()) - start) * scale.scale);

            if (range != newRange)
            {
                range = newRange;
                repaint();
            }
        }

        void updateMarkers (MarkerListBase& markerList, EditorCanvasBase* canvas_,
                            const int viewportWidth, const int viewportHeight)
        {
            canvas = canvas_;
            const int num = markerList.size();

            Array<ValueTree> requiredMarkers;
            requiredMarkers.ensureStorageAllocated (num);

            int i;
            for (i = 0; i < num; ++i)
                requiredMarkers.add (markerList.getMarker (i));

            for (i = markers.size(); --i >= 0;)
            {
                MarkerComponent* marker = markers.getUnchecked (i);
                const int index = requiredMarkers.indexOf (marker->marker);

                if (index >= 0)
                {
                    marker->updatePosition (viewportWidth, viewportHeight);
                    requiredMarkers.removeValue (marker->marker);
                }
                else
                {
                    if (marker->isMouseButtonDown())
                        marker->setBounds (-1, -1, 1, 1);
                    else
                        markers.remove (i);
                }
            }

            for (i = requiredMarkers.size(); --i >= 0;)
            {
                MarkerComponent* marker = new MarkerComponent (*this, canvas, requiredMarkers.getReference(i),
                                                               markerList.isSpecialMarker (requiredMarkers.getReference(i)), isX);
                markers.add (marker);
                getParentComponent()->addAndMakeVisible (marker);
                marker->updatePosition (viewportWidth, viewportHeight);
            }
        }

        void paint (Graphics& g)
        {
            g.setFont (10.0f);
            g.setColour (Colour::greyLevel (0.9f));

            TickIterator ticks (range.getStart(), range.getEnd(), range.getLength() / (isX ? getWidth() : getHeight()),
                                10, isX ? 50 : 80);

            float pos, tickLength;
            String label;

            while (ticks.getNextTick (pos, tickLength, label))
            {
                if (pos > 0)
                {
                    if (isX)
                    {
                        g.drawVerticalLine ((int) pos, getHeight() - tickLength * getHeight(), (float) getHeight());
                        g.drawSingleLineText (label, (int) pos + 2, getHeight() - 6);
                    }
                    else
                    {
                        g.drawHorizontalLine ((int) pos, getWidth() - tickLength * getWidth(), (float) getWidth());
                        g.drawTextAsPath (label, AffineTransform::rotation (float_Pi / -2.0f)
                                                                 .translated (getWidth() - 6.0f, pos - 2.0f));
                    }
                }
            }
        }

        void mouseDoubleClick (const MouseEvent& e)
        {
            if (isX)
                canvas->getMarkerList (true).createMarker (canvas->getMarkerList (true).getNonexistentMarkerName ("Marker"),
                                                           xToPosition (e.x));
            else
                canvas->getMarkerList (false).createMarker (canvas->getMarkerList (false).getNonexistentMarkerName ("Marker"),
                                                            xToPosition (e.y));
        }

        double xToPosition (const int x) const
        {
            return range.getStart() + x * range.getLength() / (isX ? getWidth() : getHeight());
        }

        int positionToX (const double position) const
        {
            const float proportion = (float) ((position - range.getStart()) / range.getLength());
            return isX ? proportionOfWidth (proportion) : proportionOfHeight (proportion);
        }

        //==============================================================================
        class MarkerComponent   : public Component,
                                  public ChangeListener
        {
        public:
            MarkerComponent (RulerComponent& ruler_, EditorCanvasBase* const canvas_,
                             const ValueTree& marker_, bool isSpecial_, bool isX_)
                : ruler (ruler_), canvas (canvas_), marker (marker_), isX (isX_), headSize (getRulerThickness() - 2),
                  isDragging (false), isSpecial (isSpecial_), isSelected (false)
            {
                updateSelectionState();

                canvas->getSelection().addChangeListener (this);
            }

            ~MarkerComponent()
            {
                canvas->getSelection().removeChangeListener (this);
            }

            void paint (Graphics& g)
            {
                if (isSelected || isMouseOverOrDragging())
                {
                    g.setColour (Colours::white.withAlpha (0.5f));
                    g.strokePath (path, PathStrokeType (isSelected ? 2.5f : 1.5f));
                }

                Colour c (isSpecial ? Colours::darkgreen : Colours::darkgrey);

                if (isSelected)
                    c = c.overlaidWith (Colours::red.withAlpha (0.5f));

                g.setColour (c.withAlpha (isMouseOverOrDragging() ? 0.95f : 0.6f));
                g.fillPath (path);
            }

            void updatePosition (const int viewportWidth, const int viewportHeight)
            {
                RelativeCoordinate coord (getMarkerList().getCoordinate (marker));
                const double pos = coord.resolve (&getMarkerList());

                if (! ruler.range.contains (pos))
                {
                    setVisible (false);
                }
                else
                {
                    setVisible (true);

                    Point<int> anchorPoint;
                    if (isX)
                        anchorPoint.setXY (ruler.positionToX (pos), ruler.getHeight());
                    else
                        anchorPoint.setXY (ruler.getWidth(), ruler.positionToX (pos));

                    Component* const parent = getParentComponent();
                    anchorPoint = ruler.relativePositionToOtherComponent (parent, anchorPoint);

                    const int width = 8;
                    if (isX)
                        setBounds (anchorPoint.getX() - width, anchorPoint.getY() - headSize, width * 2, viewportHeight + headSize);
                    else
                        setBounds (anchorPoint.getX() - headSize, anchorPoint.getY() - width, viewportWidth + headSize, width * 2);
                }

                labelText = "name: " + getMarkerList().getName (marker) + "\nposition: " + coord.toString();
                updateLabel();
            }

            void updateLabel()
            {
                if (isMouseOverOrDragging() && isVisible() && (getWidth() > 1 || getHeight() > 1))
                    label.update (getParentComponent(), labelText, Colours::darkgreen,
                                  isX ? getBounds().getCentreX() : getX() + headSize,
                                  isX ? getY() + headSize : getBounds().getCentreY(), true, true);
                else
                    label.remove();
            }

            bool hitTest (int x, int y)
            {
                return (isX ? y : x) < headSize;
            }

            void resized()
            {
                const float lineThickness = 1.0f;
                path.clear();

                if (isX)
                {
                    float w = getWidth() / 2.0f;
                    const float centre = w + 0.5f;
                    w -= 2.0f;
                    path.startNewSubPath (centre - w, 1.0f);
                    path.lineTo (centre + w, 1.0f);
                    path.lineTo (centre + lineThickness / 2.0f, headSize);
                    path.lineTo (centre + lineThickness / 2.0f, (float) getHeight());
                    path.lineTo (centre - lineThickness / 2.0f, (float) getHeight());
                    path.lineTo (centre - lineThickness / 2.0f, headSize);
                    path.closeSubPath();
                }
                else
                {
                    float w = getHeight() / 2.0f;
                    const float centre = w + 0.5f;
                    w -= 2.0f;
                    path.startNewSubPath (1.0f, centre + w);
                    path.lineTo (1.0f, centre - w);
                    path.lineTo (headSize, centre - lineThickness / 2.0f);
                    path.lineTo ((float) getWidth(), centre - lineThickness / 2.0f);
                    path.lineTo ((float) getWidth(), centre + lineThickness / 2.0f);
                    path.lineTo (headSize, centre + lineThickness / 2.0f);
                    path.closeSubPath();
                }

                updateLabel();
            }

            void mouseDown (const MouseEvent& e)
            {
                mouseDownPos = e.getMouseDownPosition();
                toFront (false);
                updateLabel();

                canvas->getSelection().selectOnly (getMarkerList().getId (marker));

                if (e.mods.isPopupMenu())
                {
                    isDragging = false;
                }
                else
                {
                    isDragging = true;
                    canvas->getUndoManager().beginNewTransaction();
                }
            }

            void mouseDrag (const MouseEvent& e)
            {
                if (isDragging)
                {
                    autoScrollForMouseEvent (e.getEventRelativeTo (canvas), isX, ! isX);
                    canvas->getUndoManager().undoCurrentTransactionOnly();

                    Rectangle<int> axis;
                    if (isX)
                        axis.setBounds (0, 0, getParentWidth(), headSize);
                    else
                        axis.setBounds (0, 0, headSize, getParentHeight());

                    if (axis.expanded (isX ? 500 : 30, isX ? 30 : 500).contains (e.x, e.y))
                    {
                        RelativeCoordinate coord (getMarkerList().getCoordinate (marker));

                        MouseEvent rulerEvent (e.getEventRelativeTo (&ruler));
                        int rulerPos = isX ? (rulerEvent.x + getWidth() / 2 - mouseDownPos.getX())
                                           : (rulerEvent.y + getHeight() / 2 - mouseDownPos.getY());

                        coord.moveToAbsolute (canvas->limitMarkerPosition (ruler.xToPosition (rulerPos)), &getMarkerList());
                        getMarkerList().setCoordinate (marker, coord);

                        canvas->handleUpdateNowIfNeeded();
                    }
                    else
                    {
                        getMarkerList().deleteMarker (marker);
                    }
                }
            }

            void mouseUp (const MouseEvent& e)
            {
                canvas->getUndoManager().beginNewTransaction();
                updateLabel();
            }

            void mouseEnter (const MouseEvent& e)
            {
                updateLabel();
                repaint();
            }

            void mouseExit (const MouseEvent& e)
            {
                updateLabel();
                repaint();
            }

            void updateSelectionState()
            {
                bool nowSelected = canvas->getSelection().isSelected (getMarkerList().getId (marker));

                if (isSelected != nowSelected)
                {
                    isSelected = nowSelected;
                    repaint();
                }
            }

            void changeListenerCallback (void*)
            {
                updateSelectionState();
            }

            MarkerListBase& getMarkerList()      { return canvas->getMarkerList (isX); }

            ValueTree marker;
            const bool isX;

        private:
            RulerComponent& ruler;
            EditorCanvasBase* canvas;
            const int headSize;
            Path path;
            bool isSpecial, isDragging, isSelected;
            FloatingLabelComponent label;
            String labelText;
            Point<int> mouseDownPos;
        };

        Range<double> range;

    private:
        const bool isX;
        OwnedArray <MarkerComponent> markers;
        EditorCanvasBase* canvas;
    };

    //==============================================================================
    class CanvasViewport  : public Viewport
    {
    public:
        CanvasViewport()
            : canvas (0)
        {
            setOpaque (true);
        }

        ~CanvasViewport()
        {
        }

        void paint (Graphics& g)
        {
            if (canvas == 0)
                canvas = dynamic_cast <EditorCanvasBase*> (getViewedComponent());

            if (canvas != 0)
                canvas->fillBackground (g);
        }

        void paintOverChildren (Graphics& g)
        {
            drawRecessedShadows (g, getMaximumVisibleWidth(), getMaximumVisibleHeight(), 14);
        }

        void visibleAreaChanged (int, int , int, int)
        {
            EditorPanelBase* p = dynamic_cast <EditorPanelBase*> (getParentComponent());

            if (p != 0)
                p->updateRulers();
        }

    private:
        EditorCanvasBase* canvas;
    };

    //==============================================================================
    class TooltipBar    : public Component,
                          public Timer
    {
    public:
        TooltipBar()
            : lastComp (0)
        {
            label.setColour (Label::textColourId, Colour::greyLevel (0.15f));
            label.setColour (Label::backgroundColourId, Colour::greyLevel (0.75f));
            label.setFont (Font (13.0f));
            label.setJustificationType (Justification::centredLeft);

            addAndMakeVisible (&label);
        }

        ~TooltipBar()
        {
        }

        void timerCallback()
        {
            Component* const newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

            if (newComp != lastComp)
            {
                lastComp = newComp;
                label.setText (findTip (newComp), false);
            }
        }

        void resized()
        {
            label.setBounds (getLocalBounds());
        }

        void visibilityChanged()
        {
            if (isVisible())
                startTimer (150);
            else
                stopTimer();
        }

    private:
        Label label;
        Component* lastComp;

        const String findTip (Component* c)
        {
            while (c != 0 && c != this)
            {
                TooltipClient* const tc = dynamic_cast <TooltipClient*> (c);
                if (tc != 0)
                {
                    const String tip (tc->getTooltip());

                    if (tip.isNotEmpty())
                        return tip;
                }

                c = c->getParentComponent();
            }

            return String::empty;
        }

        TooltipBar (const TooltipBar&);
        TooltipBar& operator= (const TooltipBar&);
    };

    //==============================================================================
    Toolbar toolbar;
    CanvasViewport viewport;
    RulerComponent rulerX, rulerY;
    ScopedPointer<InfoPanel> infoPanel;
    TreeView tree;
    TooltipBar tooltipBar;

    EditorCanvasBase* canvas;
    bool markersVisible, snappingEnabled;
    Image background;
};


#endif  // __JUCER_EDITORPANEL_H_8E192A99__
