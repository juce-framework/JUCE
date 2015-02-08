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

static inline void blurDataTriplets (uint8* d, int num, const int delta) noexcept
{
    uint32 last = d[0];
    d[0] = (uint8) ((d[0] + d[delta] + 1) / 3);
    d += delta;

    num -= 2;

    do
    {
        const uint32 newLast = d[0];
        d[0] = (uint8) ((last + d[0] + d[delta] + 1) / 3);
        d += delta;
        last = newLast;
    }
    while (--num > 0);

    d[0] = (uint8) ((last + d[0] + 1) / 3);
}

static void blurSingleChannelImage (uint8* const data, const int width, const int height,
                                    const int lineStride, const int repetitions) noexcept
{
    jassert (width > 2 && height > 2);

    for (int y = 0; y < height; ++y)
        for (int i = repetitions; --i >= 0;)
            blurDataTriplets (data + lineStride * y, width, 1);

    for (int x = 0; x < width; ++x)
        for (int i = repetitions; --i >= 0;)
            blurDataTriplets (data + x, height, lineStride);
}

static void blurSingleChannelImage (Image& image, int radius)
{
    const Image::BitmapData bm (image, Image::BitmapData::readWrite);
    blurSingleChannelImage (bm.data, bm.width, bm.height, bm.lineStride, 2 * radius);
}

//==============================================================================
DropShadow::DropShadow() noexcept
    : colour (0x90000000), radius (4)
{
}

DropShadow::DropShadow (Colour shadowColour, const int r, Point<int> o) noexcept
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

    const Rectangle<int> area ((path.getBounds().getSmallestIntegerContainer() + offset)
                                   .expanded (radius + 1)
                                   .getIntersection (g.getClipBounds().expanded (radius + 1)));

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

static void drawShadowSection (Graphics& g, ColourGradient& cg, Rectangle<float> area,
                               bool isCorner, float centreX, float centreY, float edgeX, float edgeY)
{
    cg.point1 = area.getRelativePoint (centreX, centreY);
    cg.point2 = area.getRelativePoint (edgeX, edgeY);
    cg.isRadial = isCorner;

    g.setGradientFill (cg);
    g.fillRect (area);
}

void DropShadow::drawForRectangle (Graphics& g, const Rectangle<int>& targetArea) const
{
    ColourGradient cg (colour, 0, 0, colour.withAlpha (0.0f), 0, 0, false);

    for (float i = 0.05f; i < 1.0f; i += 0.1f)
        cg.addColour (1.0 - i, colour.withMultipliedAlpha (i * i));

    const float radiusInset = (radius + 1) / 2.0f;
    const float expandedRadius = radius + radiusInset;

    const Rectangle<float> area (targetArea.toFloat().reduced (radiusInset) + offset.toFloat());

    Rectangle<float> r (area.expanded (expandedRadius));
    Rectangle<float> top (r.removeFromTop (expandedRadius));
    Rectangle<float> bottom (r.removeFromBottom (expandedRadius));

    drawShadowSection (g, cg, top.removeFromLeft  (expandedRadius), true, 1.0f, 1.0f, 0, 1.0f);
    drawShadowSection (g, cg, top.removeFromRight (expandedRadius), true, 0, 1.0f, 1.0f, 1.0f);
    drawShadowSection (g, cg, top, false, 0, 1.0f, 0, 0);

    drawShadowSection (g, cg, bottom.removeFromLeft  (expandedRadius), true, 1.0f, 0, 0, 0);
    drawShadowSection (g, cg, bottom.removeFromRight (expandedRadius), true, 0, 0, 1.0f, 0);
    drawShadowSection (g, cg, bottom, false, 0, 0, 0, 1.0f);

    drawShadowSection (g, cg, r.removeFromLeft  (expandedRadius), false, 1.0f, 0, 0, 0);
    drawShadowSection (g, cg, r.removeFromRight (expandedRadius), false, 0, 0, 1.0f, 0);

    g.setColour (colour);
    g.fillRect (area);
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
