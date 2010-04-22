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

#ifndef __JUCER_COMPONENTEDITORCANVAS_H_37C33B56__
#define __JUCER_COMPONENTEDITORCANVAS_H_37C33B56__

#include "../../model/jucer_ComponentDocument.h"
#include "../jucer_DocumentEditorComponent.h"
class ComponentEditor;


//==============================================================================
class ComponentEditorCanvas   : public Component,
                                public ValueTree::Listener,
                                public Timer
{
public:
    //==============================================================================
    ComponentEditorCanvas (ComponentEditor& editor_);
    ~ComponentEditorCanvas();

    //==============================================================================
    ComponentEditor& getEditor();
    ComponentDocument& getDocument();

    typedef SelectedItemSet<String> SelectedItems;
    SelectedItems& getSelection();

    class ComponentHolder;
    ComponentHolder* getComponentHolder() const;

    //==============================================================================
    void timerCallback();
    void paint (Graphics& g);
    void resized();

    void updateComponents();
    const Rectangle<int> getContentArea() const;
    void drawXAxis (Graphics& g, const Rectangle<int>& r);
    void drawYAxis (Graphics& g, const Rectangle<int>& r);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const var::identifier&)    { updateComponents(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)   { updateComponents(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)    {}

    //==============================================================================
    const StringArray getSelectedIds() const;
    void getSelectedItemProperties (Array <PropertyComponent*>& props);
    void deleteSelection();
    void deselectNonComponents();
    void selectionToFront();
    void selectionToBack();

    //==============================================================================
    void showSizeGuides();
    void hideSizeGuides();

    //==============================================================================
    class DragOperation;

    void beginDrag (const MouseEvent& e, const ResizableBorderComponent::Zone& zone);
    void continueDrag (const MouseEvent& e);
    void endDrag (const MouseEvent& e);

private:
    ComponentEditor& editor;
    const BorderSize border;
    ScopedPointer <DragOperation> dragger;

    //==============================================================================
    class OverlayItemComponent  : public Component
    {
    public:
        OverlayItemComponent (ComponentEditorCanvas& canvas_);

        void setBoundsInTargetSpace (const Rectangle<int>& r);

        ComponentDocument& getDocument()        { return canvas.getDocument(); }

    protected:
        ComponentEditorCanvas& canvas;
    };

    friend class OverlayItemComponent;
    class ComponentResizeFrame;
    class MarkerComponent;
    class WholeComponentResizer;
    class OverlayComponent;

    //==============================================================================
    ComponentHolder* componentHolder;
    OverlayComponent* overlay;
    WholeComponentResizer* resizeFrame;
    SelectedItems selection;

    const Array<Component*> getSelectedComps() const;
    const Array<Component*> getUnselectedComps() const;
};


#endif  // __JUCER_COMPONENTEDITORCANVAS_H_37C33B56__
