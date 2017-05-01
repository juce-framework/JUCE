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

/**
    This class represents one of the individual lights in our grid.
*/
class ToggleLightComponent  : public Component
{
public:
    ToggleLightComponent (String name = "light")
        : Component (name),
          isOn (false)
    {
    }

    void paint (Graphics& g) override
    {
        // Only shows the red ellipse when the button is on.
        if (isOn)
        {
            g.setColour (getLookAndFeel().findColour (Slider::thumbColourId));
            g.fillEllipse (getLocalBounds().toFloat());
        }
    }

    void mouseEnter (const MouseEvent&) override
    {
        // button toggles state on mouse over.
        isOn = ! isOn;
        repaint();
    }

private:
    // member variables for the Component
    bool isOn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleLightComponent)
};
