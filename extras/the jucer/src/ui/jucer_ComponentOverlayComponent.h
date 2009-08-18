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

#ifndef __JUCER_COMPONENTOVERLAYCOMPONENT_JUCEHEADER__
#define __JUCER_COMPONENTOVERLAYCOMPONENT_JUCEHEADER__

#include "../model/jucer_JucerDocument.h"


//==============================================================================
/**
*/
class ComponentOverlayComponent  : public Component,
                                   public ComponentListener,
                                   public ChangeListener,
                                   public ComponentBoundsConstrainer
{
public:
    //==============================================================================
    ComponentOverlayComponent (Component* const targetComponent,
                               ComponentLayout& layout);

    ~ComponentOverlayComponent();

    //==============================================================================
    virtual void showPopupMenu();

    //==============================================================================
    void paint (Graphics& g);
    void resized();

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);

    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);

    void changeListenerCallback (void*);

    void resizeStart();
    void resizeEnd();

    void updateBoundsToMatchTarget();

    void checkBounds (int& x, int& y, int& w, int& h,
                      const Rectangle& previousBounds,
                      const Rectangle& limits,
                      const bool isStretchingTop,
                      const bool isStretchingLeft,
                      const bool isStretchingBottom,
                      const bool isStretchingRight);

    void applyBoundsToComponent (Component* component, int x, int y, int w, int h);

    //==============================================================================
    Component* const target;
    const int borderThickness;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ResizableBorderComponent* border;

    ComponentDeletionWatcher deletionWatcher;
    ComponentLayout& layout;

    bool selected, dragging, mouseDownSelectStatus;
    double originalAspectRatio;
};



#endif   // __JUCER_COMPONENTOVERLAYCOMPONENT_JUCEHEADER__
