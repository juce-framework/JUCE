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

#include "juce_DropShadowEffect.h"
#include "../imaging/juce_Image.h"
#include "../colour/juce_PixelFormats.h"

#if JUCE_MSVC
  #pragma optimize ("t", on)  // try to avoid slowing everything down in debug builds
#endif


//==============================================================================
DropShadowEffect::DropShadowEffect()
  : offsetX (0),
    offsetY (0),
    radius (4),
    opacity (0.6f)
{
}

DropShadowEffect::~DropShadowEffect()
{
}

void DropShadowEffect::setShadowProperties (const float newRadius,
                                            const float newOpacity,
                                            const int newShadowOffsetX,
                                            const int newShadowOffsetY)
{
    radius = jmax (1.1f, newRadius);
    offsetX = newShadowOffsetX;
    offsetY = newShadowOffsetY;
    opacity = newOpacity;
}

void DropShadowEffect::applyEffect (Image& image, Graphics& g)
{
    const int w = image.getWidth();
    const int h = image.getHeight();

    int lineStride, pixelStride;
    const PixelARGB* srcPixels = (const PixelARGB*) image.lockPixelDataReadOnly (0, 0, image.getWidth(), image.getHeight(), lineStride, pixelStride);

    Image shadowImage (Image::SingleChannel, w, h, false);
    int destStride, destPixelStride;
    uint8* const shadowChannel = (uint8*) shadowImage.lockPixelDataReadWrite (0, 0, w, h, destStride, destPixelStride);

    const int filter = roundFloatToInt (63.0f / radius);
    const int radiusMinus1 = roundFloatToInt ((radius - 1.0f) * 63.0f);

    for (int x = w; --x >= 0;)
    {
        int shadowAlpha = 0;

        const PixelARGB* src = srcPixels + x;
        uint8* shadowPix = shadowChannel + x;

        for (int y = h; --y >= 0;)
        {
            shadowAlpha = ((shadowAlpha * radiusMinus1 + (src->getAlpha() << 6)) * filter) >> 12;

            *shadowPix = (uint8) shadowAlpha;
            src = (const PixelARGB*) (((const uint8*) src) + lineStride);
            shadowPix += destStride;
        }
    }

    for (int y = h; --y >= 0;)
    {
        int shadowAlpha = 0;
        uint8* shadowPix = shadowChannel + y * destStride;

        for (int x = w; --x >= 0;)
        {
            shadowAlpha = ((shadowAlpha * radiusMinus1 + (*shadowPix << 6)) * filter) >> 12;
            *shadowPix++ = (uint8) shadowAlpha;
        }
    }

    image.releasePixelDataReadOnly (srcPixels);
    shadowImage.releasePixelDataReadWrite (shadowChannel);

    g.setColour (Colours::black.withAlpha (opacity));
    g.drawImageAt (&shadowImage, offsetX, offsetY, true);

    g.setOpacity (1.0f);
    g.drawImageAt (&image, 0, 0);
}

END_JUCE_NAMESPACE
