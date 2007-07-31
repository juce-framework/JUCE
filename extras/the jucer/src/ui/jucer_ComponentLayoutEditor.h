/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
    bool filesDropped (const StringArray& filenames, int x, int y);

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
