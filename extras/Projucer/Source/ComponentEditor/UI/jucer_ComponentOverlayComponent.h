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

//==============================================================================
class ComponentOverlayComponent  : public Component,
                                   public ComponentListener,
                                   public ChangeListener,
                                   public ComponentBoundsConstrainer
{
public:
    //==============================================================================
    ComponentOverlayComponent (Component* const targetComponent,
                               ComponentLayout& layout);

    ~ComponentOverlayComponent() override;

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

    void applyBoundsToComponent (Component&, Rectangle<int>) override;

    //==============================================================================
    Component::SafePointer<Component> target;
    const int borderThickness;

private:
    std::unique_ptr<ResizableBorderComponent> border;

    ComponentLayout& layout;

    bool selected, dragging, mouseDownSelectStatus;
    double originalAspectRatio;
};
