/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

GlowEffect::GlowEffect()
  : radius (2.0f),
    colour (Colours::white)
{
}

GlowEffect::~GlowEffect()
{
}

void GlowEffect::setGlowProperties (const float newRadius,
                                    Colour newColour)
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
