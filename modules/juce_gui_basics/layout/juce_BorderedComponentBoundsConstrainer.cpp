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

void BorderedComponentBoundsConstrainer::checkBounds (Rectangle<int>& bounds,
                                                      const Rectangle<int>& previousBounds,
                                                      const Rectangle<int>& limits,
                                                      bool isStretchingTop,
                                                      bool isStretchingLeft,
                                                      bool isStretchingBottom,
                                                      bool isStretchingRight)
{
    if (auto* decorated = getWrappedConstrainer())
    {
        const auto border = getAdditionalBorder();
        const auto requestedBounds = bounds;

        border.subtractFrom (bounds);
        decorated->checkBounds (bounds,
                                border.subtractedFrom (previousBounds),
                                limits,
                                isStretchingTop,
                                isStretchingLeft,
                                isStretchingBottom,
                                isStretchingRight);
        border.addTo (bounds);
        bounds = bounds.withPosition (requestedBounds.getPosition());

        if (isStretchingTop && ! isStretchingBottom)
            bounds = bounds.withBottomY (previousBounds.getBottom());

        if (! isStretchingTop && isStretchingBottom)
            bounds = bounds.withY (previousBounds.getY());

        if (isStretchingLeft && ! isStretchingRight)
            bounds = bounds.withRightX (previousBounds.getRight());

        if (! isStretchingLeft && isStretchingRight)
            bounds = bounds.withX (previousBounds.getX());
    }
    else
    {
        ComponentBoundsConstrainer::checkBounds (bounds,
                                                 previousBounds,
                                                 limits,
                                                 isStretchingTop,
                                                 isStretchingLeft,
                                                 isStretchingBottom,
                                                 isStretchingRight);
    }
}

} // namespace juce
