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

#ifndef __JUCER_PAINTROUTINEEDITOR_JUCEHEADER__
#define __JUCER_PAINTROUTINEEDITOR_JUCEHEADER__

#include "../model/jucer_JucerDocument.h"
#include "../model/jucer_PaintRoutine.h"
#include "jucer_SnapGridPainter.h"
class JucerDocumentHolder;


//==============================================================================
/**
*/
class PaintRoutineEditor  : public Component,
                            public ChangeListener,
                            public LassoSource <PaintElement*>,
                            public FileDragAndDropTarget
{
public:
    //==============================================================================
    PaintRoutineEditor (PaintRoutine& graphics,
                        JucerDocument& document,
                        JucerDocumentHolder* const docHolder);
    ~PaintRoutineEditor();

    //==============================================================================
    void paint (Graphics& g);
    void paintOverChildren (Graphics& g);
    void resized();
    void changeListenerCallback (void*);

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void visibilityChanged();

    void findLassoItemsInArea (Array <PaintElement*>& results,
                               int x, int y, int w, int h);

    SelectedItemSet <PaintElement*>& getLassoSelection();

    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& filenames, int x, int y);

    const Rectangle getComponentArea() const;

    //==============================================================================
    void refreshAllElements();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PaintRoutine& graphics;
    JucerDocument& document;
    JucerDocumentHolder* const documentHolder;
    LassoComponent <PaintElement*> lassoComp;
    SnapGridPainter grid;
    Image* componentOverlay;
    float componentOverlayOpacity;

    Colour currentBackgroundColour;

    void removeAllElementComps();
    void updateComponentOverlay();
    void updateChildBounds();
};


#endif   // __JUCER_PAINTROUTINEEDITOR_JUCEHEADER__
