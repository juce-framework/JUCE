/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
    Defines a drop-shadow effect.
*/
struct JUCE_API  DropShadow
{
    /** Creates a default drop-shadow effect. */
    DropShadow() noexcept;

    /** Creates a drop-shadow object with the given parameters. */
    DropShadow (const Colour& shadowColour, int radius, const Point<int>& offset) noexcept;

    /** Renders a drop-shadow based on the alpha-channel of the given image. */
    void drawForImage (Graphics& g, const Image& srcImage) const;

    /** Renders a drop-shadow based on the shape of a path. */
    void drawForPath (Graphics& g, const Path& path) const;

    /** Renders a drop-shadow for a rectangle.
        Note that for speed, this approximates the shadow using gradients.
    */
    void drawForRectangle (Graphics& g, const Rectangle<int>& area) const;

    /** The colour with which to render the shadow.
        In most cases you'll probably want to leave this as black with an alpha
        value of around 0.5
    */
    Colour colour;

    /** The approximate spread of the shadow. */
    int radius;

    /** The offset of the shadow. */
    Point<int> offset;
};

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
        To customise the shadow's appearance, use the setShadowProperties() method.
    */
    DropShadowEffect();

    /** Destructor. */
    ~DropShadowEffect();

    //==============================================================================
    /** Sets up parameters affecting the shadow's appearance. */
    void setShadowProperties (const DropShadow& newShadow);

    //==============================================================================
    /** @internal */
    void applyEffect (Image& sourceImage, Graphics& destContext, float scaleFactor, float alpha);


private:
    //==============================================================================
    DropShadow shadow;

    JUCE_LEAK_DETECTOR (DropShadowEffect)
};


#endif   // __JUCE_DROPSHADOWEFFECT_JUCEHEADER__
