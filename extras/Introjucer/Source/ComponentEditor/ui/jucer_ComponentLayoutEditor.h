/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef __JUCER_COMPONENTLAYOUTEDITOR_JUCEHEADER__
#define __JUCER_COMPONENTLAYOUTEDITOR_JUCEHEADER__

#include "jucer_ComponentOverlayComponent.h"
#include "../jucer_JucerDocument.h"
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
    void changeListenerCallback (ChangeBroadcaster*);

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    bool keyPressed (const KeyPress& key);

    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& filenames, int x, int y);

    ComponentLayout& getLayout() const noexcept                 { return layout; }

    void findLassoItemsInArea (Array <Component*>& results, const Rectangle<int>& area);

    SelectedItemSet <Component*>& getLassoSelection();

    //==============================================================================
    void refreshAllComponents();
    void updateOverlayPositions();

    ComponentOverlayComponent* getOverlayCompFor (Component*) const;

    Rectangle<int> getComponentArea() const;
    Image createComponentLayerSnapshot() const;

private:
    JucerDocument& document;
    ComponentLayout& layout;
    Component* subCompHolder;

    LassoComponent <Component*> lassoComp;
    SnapGridPainter grid;
    bool firstResize;
};


#endif   // __JUCER_COMPONENTLAYOUTEDITOR_JUCEHEADER__
