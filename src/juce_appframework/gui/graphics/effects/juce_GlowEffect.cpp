/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_GlowEffect.h"
#include "../../graphics/imaging/juce_ImageConvolutionKernel.h"


//==============================================================================
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

void GlowEffect::applyEffect (Image& image, Graphics& g)
{
    const int w = image.getWidth();
    const int h = image.getHeight();

    Image temp (image.getFormat(), w, h, true);

    ImageConvolutionKernel blurKernel (roundFloatToInt (radius * 2.0f));

    blurKernel.createGaussianBlur (radius);
    blurKernel.rescaleAllValues (radius);

    blurKernel.applyToImage (temp, &image, 0, 0, w, h);

    g.setColour (colour);
    g.drawImageAt (&temp, 0, 0, true);

    g.setOpacity (1.0f);
    g.drawImageAt (&image, 0, 0, false);
}

END_JUCE_NAMESPACE
