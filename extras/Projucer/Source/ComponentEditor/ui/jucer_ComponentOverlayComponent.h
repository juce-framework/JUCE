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

#ifndef JUCER_COMPONENTOVERLAYCOMPONENT_H_INCLUDED
#define JUCER_COMPONENTOVERLAYCOMPONENT_H_INCLUDED

#include "../jucer_JucerDocument.h"


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
    void paint (Graphics&) override;
    void resized() override;

    void mouseDown (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;
    void mouseUp (const MouseEvent&) override;

    void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;

    void changeListenerCallback (ChangeBroadcaster*) override;

    void resizeStart() override;
    void resizeEnd() override;

    void updateBoundsToMatchTarget();

    void checkBounds (Rectangle<int>& bounds,
                      const Rectangle<int>& previousBounds,
                      const Rectangle<int>& limits,
                      bool isStretchingTop,
                      bool isStretchingLeft,
                      bool isStretchingBottom,
                      bool isStretchingRight) override;

    void applyBoundsToComponent (Component*, const Rectangle<int>&) override;

    //==============================================================================
    Component::SafePointer<Component> target;
    const int borderThickness;

private:
    ScopedPointer<ResizableBorderComponent> border;

    ComponentLayout& layout;

    bool selected, dragging, mouseDownSelectStatus;
    double originalAspectRatio;
};



#endif   // JUCER_COMPONENTOVERLAYCOMPONENT_H_INCLUDED
