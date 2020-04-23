/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../jucer_JucerDocument.h"
#include "../jucer_PaintRoutine.h"
#include "jucer_SnapGridPainter.h"
class JucerDocumentEditor;

//==============================================================================
class PaintRoutineEditor  : public Component,
                            public ChangeListener,
                            public LassoSource <PaintElement*>,
                            public FileDragAndDropTarget
{
public:
    //==============================================================================
    PaintRoutineEditor (PaintRoutine& graphics,
                        JucerDocument& document,
                        JucerDocumentEditor* const docHolder);
    ~PaintRoutineEditor();

    //==============================================================================
    void paint (Graphics& g);
    void paintOverChildren (Graphics& g);
    void resized();
    void changeListenerCallback (ChangeBroadcaster*);

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void visibilityChanged();

    void findLassoItemsInArea (Array <PaintElement*>& results, const Rectangle<int>& area);

    SelectedItemSet <PaintElement*>& getLassoSelection();

    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& filenames, int x, int y);

    Rectangle<int> getComponentArea() const;

    //==============================================================================
    void refreshAllElements();

private:
    PaintRoutine& graphics;
    JucerDocument& document;
    JucerDocumentEditor* const documentHolder;
    LassoComponent <PaintElement*> lassoComp;
    SnapGridPainter grid;
    Image componentOverlay;
    float componentOverlayOpacity;

    Colour currentBackgroundColour;

    void removeAllElementComps();
    void updateComponentOverlay();
    void updateChildBounds();
};
