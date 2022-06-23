/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ComponentDragger::ComponentDragger() {}
ComponentDragger::~ComponentDragger() {}

//==============================================================================
void ComponentDragger::startDraggingComponent (Component* const componentToDrag, const MouseEvent& e)
{
    jassert (componentToDrag != nullptr);
    jassert (e.mods.isAnyMouseButtonDown()); // The event has to be a drag event!

    if (componentToDrag != nullptr)
        mouseDownWithinTarget = e.getEventRelativeTo (componentToDrag).getMouseDownPosition();
}

void ComponentDragger::dragComponent (Component* const componentToDrag, const MouseEvent& e,
                                      ComponentBoundsConstrainer* const constrainer)
{
    jassert (componentToDrag != nullptr);
    jassert (e.mods.isAnyMouseButtonDown()); // The event has to be a drag event!

    if (componentToDrag != nullptr)
    {
        auto bounds = componentToDrag->getBounds();

        // If the component is a window, multiple mouse events can get queued while it's in the same position,
        // so their coordinates become wrong after the first one moves the window, so in that case, we'll use
        // the current mouse position instead of the one that the event contains...
        if (componentToDrag->isOnDesktop())
            bounds += componentToDrag->getLocalPoint (nullptr, e.source.getScreenPosition()).roundToInt() - mouseDownWithinTarget;
        else
            bounds += e.getEventRelativeTo (componentToDrag).getPosition() - mouseDownWithinTarget;

        if (constrainer != nullptr)
            constrainer->setBoundsForComponent (componentToDrag, bounds, false, false, false, false);
        else
            componentToDrag->setBounds (bounds);
    }
}

} // namespace juce
