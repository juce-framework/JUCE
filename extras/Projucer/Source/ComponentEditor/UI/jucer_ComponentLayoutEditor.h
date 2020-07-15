/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ComponentOverlayComponent.h"
#include "../jucer_JucerDocument.h"
#include "jucer_SnapGridPainter.h"

//==============================================================================
class ComponentLayoutEditor  : public Component,
                               public FileDragAndDropTarget,
                               public DragAndDropTarget,
                               public LassoSource<Component*>,
                               private ChangeListener
{
public:
    //==============================================================================
    ComponentLayoutEditor (JucerDocument&, ComponentLayout&);
    ~ComponentLayoutEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void visibilityChanged() override;

    void mouseDown (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;
    void mouseUp (const MouseEvent&) override;
    bool keyPressed (const KeyPress&) override;

    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped (const StringArray& filenames, int x, int y) override;

    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails) override;
    void itemDropped (const SourceDetails& dragSourceDetails) override;


    ComponentLayout& getLayout() const noexcept                 { return layout; }

    void findLassoItemsInArea (Array <Component*>& results, const Rectangle<int>& area) override;

    SelectedItemSet<Component*>& getLassoSelection() override;

    //==============================================================================
    void refreshAllComponents();
    void updateOverlayPositions();

    ComponentOverlayComponent* getOverlayCompFor (Component*) const;

    Rectangle<int> getComponentArea() const;
    Image createComponentLayerSnapshot() const;

private:
    void changeListenerCallback (ChangeBroadcaster*) override;

    JucerDocument& document;
    ComponentLayout& layout;
    Component* subCompHolder;

    LassoComponent<Component*> lassoComp;
    SnapGridPainter grid;
    bool firstResize;
};
