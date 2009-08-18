/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_DROPSHADOWEFFECT_JUCEHEADER__
#define __JUCE_DROPSHADOWEFFECT_JUCEHEADER__

#include "juce_ImageEffectFilter.h"


//==============================================================================
/**
    An effect filter that adds a drop-shadow behind the image's content.

    (This will only work on images/components that aren't opaque, of course).

    When added to a component, this effect will draw a soft-edged
    shadow based on what gets drawn inside it. The shadow will also
    be applied to the component's children.

    For speed, this doesn't use a proper gaussian blur, but cheats by
    using a simple bilinear filter. If you need a really high-quality
    shadow, check out ImageConvolutionKernel::createGaussianBlur()

    @see Component::setComponentEffect
*/
class JUCE_API  DropShadowEffect  : public ImageEffectFilter
{
public:
    //==============================================================================
    /** Creates a default drop-shadow effect.

        To customise the shadow's appearance, use the setShadowProperties()
        method.
    */
    DropShadowEffect();

    /** Destructor. */
    ~DropShadowEffect();

    //==============================================================================
    /** Sets up parameters affecting the shadow's appearance.

        @param newRadius        the (approximate) radius of the blur used
        @param newOpacity       the opacity with which the shadow is rendered
        @param newShadowOffsetX allows the shadow to be shifted in relation to the
                                component's contents
        @param newShadowOffsetY allows the shadow to be shifted in relation to the
                                component's contents
    */
    void setShadowProperties (const float newRadius,
                              const float newOpacity,
                              const int newShadowOffsetX,
                              const int newShadowOffsetY);


    //==============================================================================
    /** @internal */
    void applyEffect (Image& sourceImage, Graphics& destContext);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int offsetX, offsetY;
    float radius, opacity;
};


#endif   // __JUCE_DROPSHADOWEFFECT_JUCEHEADER__
