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

namespace juce
{

GlowEffect::GlowEffect() {}
GlowEffect::~GlowEffect() {}

void GlowEffect::setGlowProperties (float newRadius, Colour newColour, Point<int> pos)
{
    radius = newRadius;
    colour = newColour;
    offset = pos;
}

void GlowEffect::applyEffect (Image& image, Graphics& g, float scaleFactor, float alpha)
{
    Image temp (image.getFormat(), image.getWidth(), image.getHeight(), true);

    ImageConvolutionKernel blurKernel (roundToInt (radius * scaleFactor * 2.0f));

    blurKernel.createGaussianBlur (radius);
    blurKernel.rescaleAllValues (radius);

    blurKernel.applyToImage (temp, image, image.getBounds());

    g.setColour (colour.withMultipliedAlpha (alpha));
    g.drawImageAt (temp, offset.x, offset.y, true);

    g.setOpacity (alpha);
    g.drawImageAt (image, offset.x, offset.y, false);
}

} // namespace juce
