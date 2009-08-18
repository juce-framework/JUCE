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

#ifndef __JUCER_COMPONENTLAYOUTEDITOR_JUCEHEADER__
#define __JUCER_COMPONENTLAYOUTEDITOR_JUCEHEADER__

#include "jucer_ComponentOverlayComponent.h"
#include "../model/jucer_JucerDocument.h"
#include "jucer_SnapGridPainter.h"


//==============================================================================
/**
*/
class ComponentLayoutEditor  : public Component,
                               public ChangeListener,
                               public FileDragAndDropTarget,
                               public LassoSource <Component*>
{
public:
    //==============================================================================
    ComponentLayoutEditor (JucerDocument& document, ComponentLayout& layout);
    ~ComponentLayoutEditor();

    //==============================================================================
    void paint (Graphics& g);
    void resized();
    void visibilityChanged();
    void changeListenerCallback (void*);

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    bool keyPressed (const KeyPress& key);

    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& filenames, int x, int y);

    ComponentLayout& getLayout() const throw()                  { return layout; }

    void findLassoItemsInArea (Array <Component*>& results,
                               int x, int y, int w, int h);

    SelectedItemSet <Component*>& getLassoSelection();

    //==============================================================================
    void refreshAllComponents();
    void updateOverlayPositions();

    ComponentOverlayComponent* getOverlayCompFor (Component* comp) const;

    const Rectangle getComponentArea() const;

    Image* createComponentLayerSnapshot() const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    JucerDocument& document;
    ComponentLayout& layout;
    Component* subCompHolder;

    LassoComponent <Component*> lassoComp;
    SnapGridPainter grid;
    bool firstResize;
};


#endif   // __JUCER_COMPONENTLAYOUTEDITOR_JUCEHEADER__
