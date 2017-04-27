/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "ToggleLightComponent.h"

/**
    This is the parent class that holds multiple ToggleLightComponents in a grid.
*/
class ToggleLightGridComponent  : public Component
{
public:
    ToggleLightGridComponent (String name = "grid")
        : Component (name)
    {
        // Adds the child light components and makes them visible
        // within this component.
        // (they currently rely on having a default constructor
        // so they dont have to be individually initialised)
        for (int i = 0; i < numX * numY; ++i)
            addAndMakeVisible (toggleLights[i]);
    }

    void resized() override
    {
        // This creates a grid of rectangles to use as the bounds
        // for all of our lights. The grid is defined with the
        // width and height of this component.

        int stepX = getWidth()  / numX;
        int stepY = getHeight() / numY;

        for (int x = 0; x < numX; ++x)
        {
            for (int y = 0; y < numY; ++y)
            {
                // creates the rectangle     (x,         y,         width, height)
                Rectangle<int> elementBounds (x * stepX, y * stepY, stepX, stepY);

                // set the size and position of the Toggle light to this rectangle.
                toggleLights[x + numX * y].setBounds (elementBounds);
            }
        }
    }

private:
    // member variables for the Component
    static const int numX = 20;
    static const int numY = 20;

    ToggleLightComponent toggleLights [numX * numY];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleLightGridComponent)
};
