/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ComponentDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays a grid of lights.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2019

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ComponentDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/**
    This class represents one of the individual lights in our grid.
*/
class ToggleLightComponent  : public Component
{
public:
    ToggleLightComponent() {}

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
    bool isOn = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleLightComponent)
};

//==============================================================================
/**
    This is the parent class that holds multiple ToggleLightComponents in a grid.
*/
class ToggleLightGridComponent  : public Component
{
public:
    ToggleLightGridComponent()
    {
        // Adds the child light components and makes them visible
        // within this component.
        // (they currently rely on having a default constructor
        // so they don't have to be individually initialised)
        for (auto i = 0; i < numX * numY; ++i)
            addAndMakeVisible (toggleLights[i]);
    }

    void resized() override
    {
        // This creates a grid of rectangles to use as the bounds
        // for all of our lights. The grid is defined with the
        // width and height of this component.

        auto stepX = getWidth()  / numX;
        auto stepY = getHeight() / numY;

        for (auto x = 0; x < numX; ++x)
        {
            for (auto y = 0; y < numY; ++y)
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

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class ComponentDemo   : public Component
{
public:
    //==============================================================================
    ComponentDemo()
    {
        // add the light grid to out main component.
        addAndMakeVisible (lightGrid);

        setSize (600, 600);
    }

    void paint (Graphics&) override {}

    void resized() override
    {
        // set the size of the grid to fill the whole window.
        lightGrid.setBounds (getLocalBounds());
    }

private:
    //==============================================================================
    ToggleLightGridComponent lightGrid;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentDemo)
};
