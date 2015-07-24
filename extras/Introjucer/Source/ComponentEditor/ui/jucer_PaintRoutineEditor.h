/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_PAINTROUTINEEDITOR_H_INCLUDED
#define JUCER_PAINTROUTINEEDITOR_H_INCLUDED

#include "../jucer_JucerDocument.h"
#include "../jucer_PaintRoutine.h"
#include "jucer_SnapGridPainter.h"
class JucerDocumentEditor;


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


#endif   // JUCER_PAINTROUTINEEDITOR_H_INCLUDED
