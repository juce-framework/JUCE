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

#ifndef __JUCER_EDITORCANVAS_H_EF886D17__
#define __JUCER_EDITORCANVAS_H_EF886D17__

#include "../../utility/jucer_MarkerListBase.h"
class EditorPanelBase;


//==============================================================================
class EditorCanvasBase   : public Component,
                           public ValueTree::Listener,
                           public AsyncUpdater
{
public:
    //==============================================================================
    EditorCanvasBase();
    ~EditorCanvasBase();

    void initialise();
    void shutdown();

    //==============================================================================
    typedef SelectedItemSet<String> SelectedItems;

    //==============================================================================
    void paint (Graphics& g);
    void resized();

    const Rectangle<int> getContentArea() const;
    void drawXAxis (Graphics& g, const Rectangle<int>& r);
    void drawYAxis (Graphics& g, const Rectangle<int>& r);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&)         { triggerAsyncUpdate(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)   { triggerAsyncUpdate(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)    {}

    //==============================================================================
    void showSizeGuides();
    void hideSizeGuides();

    //==============================================================================
    virtual void documentChanged() = 0;
    virtual const Rectangle<int> getCanvasBounds() = 0;
    virtual void setCanvasBounds (const Rectangle<int>& newBounds) = 0;
    virtual bool canResizeCanvas() const = 0;
    virtual MarkerListBase& getMarkerList (bool isX) = 0;

    virtual const SelectedItems::ItemType findObjectIdAt (const Point<int>& position) = 0;
    virtual void showPopupMenu (bool isClickOnSelectedObject) = 0;
    virtual void objectDoubleClicked (const MouseEvent& e, const ValueTree& state) = 0;

    virtual const ValueTree getObjectState (const String& objectId) = 0;
    virtual const Rectangle<int> getObjectPosition (const ValueTree& state) = 0;

    virtual bool hasSizeGuides() const = 0;
    virtual RelativeRectangle getObjectCoords (const ValueTree& state) = 0;
    virtual SelectedItems& getSelection() = 0;
    virtual UndoManager& getUndoManager() = 0;
    virtual void deselectNonDraggableObjects() = 0;
    virtual void findLassoItemsInArea (Array <SelectedItems::ItemType>& itemsFound, const Rectangle<int>& area) = 0;
    virtual Component* createComponentHolder() = 0;

    class DragOperation
    {
    public:
        DragOperation() {}
        virtual ~DragOperation() {}

        virtual void drag (const MouseEvent& e) = 0;
        virtual bool dragItem (ValueTree& v, const Point<int>& distance, const Rectangle<float>& originalPos) = 0;
    };

    virtual DragOperation* createDragOperation (const MouseEvent& e,
                                                Component* snapGuideParentComponent,
                                                const ResizableBorderComponent::Zone& zone) = 0;

    void beginDrag (const MouseEvent& e, const ResizableBorderComponent::Zone& zone);
    void continueDrag (const MouseEvent& e);
    void endDrag (const MouseEvent& e);

    //==============================================================================
    Component* getComponentHolder() const       { return componentHolder; }
    EditorPanelBase* getPanel() const;

    const Point<int>& getOrigin() const throw()                             { return origin; }
    const Point<int> screenSpaceToObjectSpace (const Point<int>& p) const;
    const Point<int> objectSpaceToScreenSpace (const Point<int>& p) const;
    const Rectangle<int> screenSpaceToObjectSpace (const Rectangle<int>& r) const;
    const Rectangle<int> objectSpaceToScreenSpace (const Rectangle<int>& r) const;

    //==============================================================================
    class OverlayItemComponent  : public Component
    {
    public:
        OverlayItemComponent (EditorCanvasBase* canvas_);
        ~OverlayItemComponent();

        void setBoundsInTargetSpace (const Rectangle<int>& r);

    protected:
        EditorCanvasBase* canvas;
    };

protected:
    //==============================================================================
    const BorderSize border;
    Point<int> origin;
    double scaleFactor;

    friend class OverlayItemComponent;
    class ResizeFrame;
    class MarkerComponent;
    class DocumentResizeFrame;
    class OverlayComponent;

    //==============================================================================
    Component* componentHolder;
    OverlayComponent* overlay;
    DocumentResizeFrame* resizeFrame;
    ScopedPointer<DragOperation> dragger;

    void handleAsyncUpdate();
};


#endif  // __JUCER_EDITORCANVAS_H_EF886D17__
