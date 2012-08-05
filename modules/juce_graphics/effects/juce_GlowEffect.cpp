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

GlowEffect::GlowEffect()
  : radius (2.0f),
    colour (Colours::white)
{
}

GlowEffect::~GlowEffect()
{
}

void GlowEffect::setGlowProperties (const float newRadius,
                                    const Colour& newColour)
{
    radius = newRadius;
    colour = newColour;
}

void GlowEffect::applyEffect (Image& image, Graphics& g, float scaleFactor, float alpha)
{
    Image temp (image.getFormat(), image.getWidth(), image.getHeight(), true);

    ImageConvolutionKernel blurKernel (roundToInt (radius * scaleFactor * 2.0f));

    blurKernel.createGaussianBlur (radius);
    blurKernel.rescaleAllValues (radius);

    blurKernel.applyToImage (temp, image, image.getBounds());

    g.setColour (colour.withMultipliedAlpha (alpha));
    g.drawImageAt (temp, 0, 0, true);

    g.setOpacity (alpha);
    g.drawImageAt (image, 0, 0, false);
}
