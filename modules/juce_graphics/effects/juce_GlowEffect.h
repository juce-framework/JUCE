/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_GLOWEFFECT_H_INCLUDED
#define JUCE_GLOWEFFECT_H_INCLUDED


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


#endif   // JUCE_GLOWEFFECT_H_INCLUDED
