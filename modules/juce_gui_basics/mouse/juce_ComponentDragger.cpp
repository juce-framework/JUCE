/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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
        const auto bounds = componentToDrag->getBounds();

        const auto setBounds = [&] (auto b)
        {
            if (constrainer != nullptr)
                constrainer->setBoundsForComponent (componentToDrag, b, false, false, false, false);
            else
                componentToDrag->setBounds (b);
        };

        if (auto* peer = componentToDrag->isOnDesktop() ? componentToDrag->getPeer() : nullptr)
        {
            // If the component is a window, multiple mouse events can get queued while it's in the same position,
            // so their coordinates become wrong after the first one moves the window, so in that case, we'll use
            // the current mouse position instead of the one that the event contains...

            const auto globalMouseDown = componentToDrag->localPointToGlobal (mouseDownWithinTarget.toFloat());
            const auto peerSpaceMouseDown = peer->globalToLocal (detail::ScalingHelpers::scaledScreenPosToUnscaled (globalMouseDown));
            const auto [multimonitor, logical] = detail::ComponentHelpers::getTopLeftForPeer (*peer, e.source.getScreenPosition(), peerSpaceMouseDown);
            const auto scope = peer->setMultimonitorPositionOverride (multimonitor.roundToInt());
            setBounds (bounds.withPosition (logical.roundToInt()));
        }
        else
        {
            setBounds (bounds + (e.getEventRelativeTo (componentToDrag).getPosition() - mouseDownWithinTarget));
        }
    }
}

} // namespace juce
