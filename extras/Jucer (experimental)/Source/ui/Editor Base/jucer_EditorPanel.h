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

    void paint (Graphics& g)
    {
        g.setTiledImageFill (background, 0, 0, 1.0f);
        g.fillAll();
    }

    void resized()
    {
        const int toolbarHeight = 22;

        toolbar.setBounds (0, 0, getWidth(), toolbarHeight);

        int contentL = 0, contentR = getWidth();

        if (infoPanel != 0 && infoPanel->isVisible())
        {
            contentR -= 200;
            infoPanel->setBounds (contentR, toolbar.getBottom(), getWidth() - contentR, getHeight() - toolbar.getBottom());
        }

        if (tree.isVisible())
        {
            contentL = 200;
            tree.setBounds (0, toolbar.getBottom(), contentL, getHeight() - toolbar.getBottom());
        }

        const int rulerThickness = 16;
        viewport.setBounds (contentL + rulerThickness, toolbar.getBottom() + rulerThickness,
                            contentR - contentL - rulerThickness,
                            getHeight() - toolbar.getBottom() - rulerThickness);

        rulerX.setBounds (viewport.getX(), viewport.getY() - rulerThickness, viewport.getWidth(), rulerThickness);
        rulerY.setBounds (viewport.getX() - rulerThickness, viewport.getY(), rulerThickness, viewport.getHeight());
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

        void updateMarkers (MarkerListBase& markerList, EditorCanvasBase* canvas_, const int viewportWidth, const int viewportHeight)
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
                                                               isX, isX ? getHeight() : getWidth());
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
        class MarkerComponent   : public Component
        {
        public:
            MarkerComponent (RulerComponent& ruler_, EditorCanvasBase* const canvas_,
                             const ValueTree& marker_, bool isX_, int headSize_)
                : ruler (ruler_), canvas (canvas_), marker (marker_), isX (isX_), headSize (headSize_ - 2),
                  isDragging (false)
            {
            }

            ~MarkerComponent()
            {
            }

            void paint (Graphics& g)
            {
                g.setColour (Colours::lightblue.withAlpha (isMouseOverOrDragging() ? 0.9f : 0.5f));
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
                    const float centre = getWidth() / 2 + 0.5f;
                    path.addLineSegment (Line<float> (centre, 2.0f, centre, getHeight() + 1.0f), lineThickness);
                    path.addTriangle (1.0f, 0.0f, centre * 2.0f - 1.0f, 0.0f, centre, headSize + 1.0f);
                }
                else
                {
                    const float centre = getHeight() / 2 + 0.5f;
                    path.addLineSegment (Line<float> (2.0f, centre, getWidth() + 1.0f, centre), lineThickness);
                    path.addTriangle (0.0f, centre * 2.0f - 1.0f, 0.0f, 1.0f, headSize + 1.0f, centre);
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

            MarkerListBase& getMarkerList()      { return canvas->getMarkerList (isX); }

            ValueTree marker;
            const bool isX;

        private:
            RulerComponent& ruler;
            EditorCanvasBase* canvas;
            const int headSize;
            Path path;
            bool isDragging;
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
    Toolbar toolbar;
    CanvasViewport viewport;
    RulerComponent rulerX, rulerY;
    ScopedPointer<InfoPanel> infoPanel;
    TreeView tree;
    EditorCanvasBase* canvas;
    bool markersVisible, snappingEnabled;
    Image background;
};


#endif  // __JUCER_EDITORPANEL_H_8E192A99__
