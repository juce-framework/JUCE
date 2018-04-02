/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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

ColorGradient::ColorGradient() noexcept  : isRadial (false)
{
   #if JUCE_DEBUG
    point1.setX (987654.0f);
    #define JUCE_COLORGRADIENT_CHECK_COORDS_INITIALIZED   jassert (point1.x != 987654.0f);
   #else
    #define JUCE_COLORGRADIENT_CHECK_COORDS_INITIALIZED
   #endif
}

ColorGradient::ColorGradient (const ColorGradient& other)
    : point1 (other.point1), point2 (other.point2), isRadial (other.isRadial), colors (other.colors)
{}

ColorGradient::ColorGradient (ColorGradient&& other) noexcept
    : point1 (other.point1), point2 (other.point2), isRadial (other.isRadial),
      colors (static_cast<Array<ColorPoint>&&> (other.colors))
{}

ColorGradient& ColorGradient::operator= (const ColorGradient& other)
{
    point1 = other.point1;
    point2 = other.point2;
    isRadial = other.isRadial;
    colors = other.colors;
    return *this;
}

ColorGradient& ColorGradient::operator= (ColorGradient&& other) noexcept
{
    point1 = other.point1;
    point2 = other.point2;
    isRadial = other.isRadial;
    colors = static_cast<Array<ColorPoint>&&> (other.colors);
    return *this;
}

ColorGradient::ColorGradient (Color color1, float x1, float y1,
                                Color color2, float x2, float y2, bool radial)
    : ColorGradient (color1, Point<float> (x1, y1),
                      color2, Point<float> (x2, y2), radial)
{
}

ColorGradient::ColorGradient (Color color1, Point<float> p1,
                                Color color2, Point<float> p2, bool radial)
    : point1 (p1),
      point2 (p2),
      isRadial (radial)
{
    colors.add (ColorPoint { 0.0, color1 },
                 ColorPoint { 1.0, color2 });
}

ColorGradient::~ColorGradient() {}

ColorGradient ColorGradient::vertical (Color c1, float y1, Color c2, float y2)
{
    return { c1, 0, y1, c2, 0, y2, false };
}

ColorGradient ColorGradient::horizontal (Color c1, float x1, Color c2, float x2)
{
    return { c1, x1, 0, c2, x2, 0, false };
}

bool ColorGradient::operator== (const ColorGradient& other) const noexcept
{
    return point1 == other.point1 && point2 == other.point2
            && isRadial == other.isRadial
            && colors == other.colors;
}

bool ColorGradient::operator!= (const ColorGradient& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
void ColorGradient::clearColors()
{
    colors.clear();
}

int ColorGradient::addColor (const double proportionAlongGradient, Color color)
{
    // must be within the two end-points
    jassert (proportionAlongGradient >= 0 && proportionAlongGradient <= 1.0);

    if (proportionAlongGradient <= 0)
    {
        colors.set (0, { 0.0, color });
        return 0;
    }

    auto pos = jmin (1.0, proportionAlongGradient);

    int i;
    for (i = 0; i < colors.size(); ++i)
        if (colors.getReference(i).position > pos)
            break;

    colors.insert (i, { pos, color });
    return i;
}

void ColorGradient::removeColor (int index)
{
    jassert (index > 0 && index < colors.size() - 1);
    colors.remove (index);
}

void ColorGradient::multiplyOpacity (const float multiplier) noexcept
{
    for (auto& c : colors)
        c.color = c.color.withMultipliedAlpha (multiplier);
}

//==============================================================================
int ColorGradient::getNumColors() const noexcept
{
    return colors.size();
}

double ColorGradient::getColorPosition (int index) const noexcept
{
    if (isPositiveAndBelow (index, colors.size()))
        return colors.getReference (index).position;

    return 0;
 }

Color ColorGradient::getColor (int index) const noexcept
{
    if (isPositiveAndBelow (index, colors.size()))
        return colors.getReference (index).color;

    return {};
}

void ColorGradient::setColor (int index, Color newColor) noexcept
{
    if (isPositiveAndBelow (index, colors.size()))
        colors.getReference (index).color = newColor;
}

Color ColorGradient::getColorAtPosition (double position) const noexcept
{
    jassert (colors.getReference(0).position == 0.0); // the first color specified has to go at position 0

    if (position <= 0 || colors.size() <= 1)
        return colors.getReference(0).color;

    int i = colors.size() - 1;
    while (position < colors.getReference(i).position)
        --i;

    auto& p1 = colors.getReference (i);

    if (i >= colors.size() - 1)
        return p1.color;

    auto& p2 = colors.getReference (i + 1);

    return p1.color.interpolatedWith (p2.color, (float) ((position - p1.position) / (p2.position - p1.position)));
}

//==============================================================================
void ColorGradient::createLookupTable (PixelARGB* const lookupTable, const int numEntries) const noexcept
{
    JUCE_COLORGRADIENT_CHECK_COORDS_INITIALIZED // Trying to use this object without setting its coordinates?
    jassert (colors.size() >= 2);
    jassert (numEntries > 0);
    jassert (colors.getReference(0).position == 0.0); // The first color specified has to go at position 0

    auto pix1 = colors.getReference (0).color.getPixelARGB();
    int index = 0;

    for (int j = 1; j < colors.size(); ++j)
    {
        auto& p = colors.getReference (j);
        auto numToDo = roundToInt (p.position * (numEntries - 1)) - index;
        auto pix2 = p.color.getPixelARGB();

        for (int i = 0; i < numToDo; ++i)
        {
            jassert (index >= 0 && index < numEntries);

            lookupTable[index] = pix1;
            lookupTable[index].tween (pix2, (uint32) ((i << 8) / numToDo));
            ++index;
        }

        pix1 = pix2;
    }

    while (index < numEntries)
        lookupTable [index++] = pix1;
}

int ColorGradient::createLookupTable (const AffineTransform& transform, HeapBlock<PixelARGB>& lookupTable) const
{
    JUCE_COLORGRADIENT_CHECK_COORDS_INITIALIZED // Trying to use this object without setting its coordinates?
    jassert (colors.size() >= 2);

    auto numEntries = jlimit (1, jmax (1, (colors.size() - 1) << 8),
                              3 * (int) point1.transformedBy (transform)
                                              .getDistanceFrom (point2.transformedBy (transform)));
    lookupTable.malloc (numEntries);
    createLookupTable (lookupTable, numEntries);
    return numEntries;
}

bool ColorGradient::isOpaque() const noexcept
{
    for (auto& c : colors)
        if (! c.color.isOpaque())
            return false;

    return true;
}

bool ColorGradient::isInvisible() const noexcept
{
    for (auto& c : colors)
        if (! c.color.isTransparent())
            return false;

    return true;
}

bool ColorGradient::ColorPoint::operator== (ColorPoint other) const noexcept
{
    return position == other.position && color == other.color;
}

bool ColorGradient::ColorPoint::operator!= (ColorPoint other) const noexcept
{
    return position != other.position || color != other.color;
}

} // namespace juce
