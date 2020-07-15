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

#include "../jucer_JucerDocument.h"

//==============================================================================
class ComponentOverlayComponent  : public Component,
                                   private ComponentBoundsConstrainer,
                                   private ComponentListener,
                                   private ChangeListener
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

    void updateBoundsToMatchTarget();

    //==============================================================================
    Component::SafePointer<Component> target;
    const int borderThickness;

private:
    void resizeStart() override;
    void resizeEnd() override;

    void checkBounds (Rectangle<int>& bounds,
                      const Rectangle<int>& previousBounds,
                      const Rectangle<int>& limits,
                      bool isStretchingTop,
                      bool isStretchingLeft,
                      bool isStretchingBottom,
                      bool isStretchingRight) override;

    void applyBoundsToComponent (Component&, Rectangle<int>) override;

    void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;

    void changeListenerCallback (ChangeBroadcaster*) override;

    std::unique_ptr<ResizableBorderComponent> border;

    ComponentLayout& layout;

    bool selected, dragging, mouseDownSelectStatus;
    double originalAspectRatio;
};
