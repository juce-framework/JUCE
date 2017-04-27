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


//==============================================================================
/**
    A component effect that adds a coloured blur around the component's contents.

    (This will only work on non-opaque components).

    @see Component::setComponentEffect, DropShadowEffect
*/
class JUCE_API  GlowEffect  : public ImageEffectFilter
{
public:
    //==============================================================================
    /** Creates a default 'glow' effect.

        To customise its appearance, use the setGlowProperties() method.
    */
    GlowEffect();

    /** Destructor. */
    ~GlowEffect();

    //==============================================================================
    /** Sets the glow's radius and colour.

        The radius is how large the blur should be, and the colour is
        used to render it (for a less intense glow, lower the colour's
        opacity).
    */
    void setGlowProperties (float newRadius,
                            Colour newColour);


    //==============================================================================
    /** @internal */
    void applyEffect (Image& sourceImage, Graphics& destContext, float scaleFactor, float alpha);

private:
    //==============================================================================
    float radius;
    Colour colour;

    JUCE_LEAK_DETECTOR (GlowEffect)
};
