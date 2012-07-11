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

#if JUCE_MSVC && JUCE_DEBUG
 #pragma optimize ("t", on)
#endif

static void blurSingleChannelImage (uint8* const data, const int width, const int height,
                                    const int lineStride, const int repetitions) noexcept
{
    uint8* line = data;

    for (int y = height; --y >= 0;)
    {
        for (int i = repetitions; --i >= 0;)
        {
            uint8* p = line;
            *p++ = (((int) p[0]) + p[1]) / 2;

            for (int x = width - 2; --x >= 0;)
                *p++ = (((int) p[-1]) + p[0] + p[1] + 1) / 3;

            *p = (((int) p[0]) + p[-1]) / 2;
        }

        line += lineStride;
    }

    for (int i = repetitions; --i >= 0;)
    {
        line = data;

        {
            uint8* p1 = line;
            uint8* p2 = line + lineStride;

            for (int x = width; --x >= 0;)
                *p1++ = (((int) *p1) + *p2++) / 2;
        }

        line += lineStride;

        for (int y = height - 2; --y >= 0;)
        {
            uint8* p1 = line;
            uint8* p2 = line - lineStride;
            uint8* p3 = line + lineStride;

            for (int x = width; --x >= 0;)
                *p1++ = (((int) *p1) + *p2++ + *p3++ + 1) / 3;

            line += lineStride;
        }

        uint8* p1 = line;
        uint8* p2 = line - lineStride;

        for (int x = width; --x >= 0;)
            *p1++ = (((int) *p1) + *p2++) / 2;
    }
}

static void blurSingleChannelImage (Image& image, int radius)
{
    const Image::BitmapData bm (image, Image::BitmapData::readWrite);
    blurSingleChannelImage (bm.data, bm.width, bm.height, bm.lineStride, 2 * radius);
}

#if JUCE_MSVC && JUCE_DEBUG
 #pragma optimize ("", on)  // resets optimisations to the project defaults
#endif

//==============================================================================
DropShadow::DropShadow() noexcept
    : colour (0x90000000), radius (4)
{
}

DropShadow::DropShadow (const Colour& shadowColour, const int r, const Point<int>& o) noexcept
    : colour (shadowColour), radius (r), offset (o)
{
    jassert (radius > 0);
}

void DropShadow::drawForImage (Graphics& g, const Image& srcImage) const
{
    jassert (radius > 0);

    if (srcImage.isValid())
    {
        Image shadowImage (srcImage.convertedToFormat (Image::SingleChannel));
        shadowImage.duplicateIfShared();

        blurSingleChannelImage (shadowImage, radius);

        g.setColour (colour);
        g.drawImageAt (shadowImage, offset.x, offset.y, true);
    }
}

void DropShadow::drawForPath (Graphics& g, const Path& path) const
{
    jassert (radius > 0);

    const Rectangle<int> area (path.getBounds().translated ((float) offset.x, (float) offset.y)
                                               .getSmallestIntegerContainer()
                                               .getIntersection (g.getClipBounds())
                                               .expanded (radius + 1, radius + 1));

    if (area.getWidth() > 2 && area.getHeight() > 2)
    {
        Image renderedPath (Image::SingleChannel, area.getWidth(), area.getHeight(), true);

        {
            Graphics g2 (renderedPath);
            g2.setColour (Colours::white);
            g2.fillPath (path, AffineTransform::translation ((float) (offset.x - area.getX()),
                                                             (float) (offset.y - area.getY())));
        }

        blurSingleChannelImage (renderedPath, radius);

        g.setColour (colour);
        g.drawImageAt (renderedPath, area.getX(), area.getY(), true);
    }
}

//==============================================================================
DropShadowEffect::DropShadowEffect() {}
DropShadowEffect::~DropShadowEffect() {}

void DropShadowEffect::setShadowProperties (const DropShadow& newShadow)
{
    shadow = newShadow;
}

void DropShadowEffect::applyEffect (Image& image, Graphics& g, float scaleFactor, float alpha)
{
    DropShadow s (shadow);
    s.radius = roundToInt (s.radius * scaleFactor);
    s.colour = s.colour.withMultipliedAlpha (alpha);
    s.offset.x = roundToInt (s.offset.x * scaleFactor);
    s.offset.y = roundToInt (s.offset.y * scaleFactor);

    s.drawForImage (g, image);

    g.setOpacity (alpha);
    g.drawImageAt (image, 0, 0);
}
