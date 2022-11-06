/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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

 name:             AnimationAppDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple animation application.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AnimationAppDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class AnimationAppDemo   : public AnimatedAppComponent
{
public:
    //==============================================================================
    AnimationAppDemo()
    {
        setSize (800, 600);
        setSynchroniseToVBlank (true);
    }

    void update() override
    {
        // This function is called at the frequency specified by the setFramesPerSecond() call
        // in the constructor. You can use it to update counters, animate values, etc.
    }

    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        g.setColour (getLookAndFeel().findColour (Slider::thumbColourId));
        auto fishLength = 15;

        Path spinePath;

        for (auto i = 0; i < fishLength; ++i)
        {
            auto radius = 100 + 10 * std::sin ((float) getFrameCounter() * 0.1f + (float) i * 0.5f);

            Point<float> p ((float) getWidth()  / 2.0f + 1.5f * radius * std::sin ((float) getFrameCounter() * 0.02f + (float) i * 0.12f),
                            (float) getHeight() / 2.0f + 1.0f * radius * std::cos ((float) getFrameCounter() * 0.04f + (float) i * 0.12f));

            // draw the circles along the fish
            g.fillEllipse (p.x - (float) i, p.y - (float) i, 2.0f + 2.0f * (float) i, 2.0f + 2.0f * (float) i);

            if (i == 0)
                spinePath.startNewSubPath (p);  // if this is the first point, start a new path..
            else
                spinePath.lineTo (p);           // ...otherwise add the next point
        }

        // draw an outline around the path that we have created
        g.strokePath (spinePath, PathStrokeType (4.0f));
    }

    void resized() override
    {
        // This is called when this component is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationAppDemo)
};
