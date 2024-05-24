/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ColourGradient::ColourGradient() noexcept  : isRadial (false)
{
   #if JUCE_DEBUG
    point1.setX (987654.0f);
    #define JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED jassert (! exactlyEqual (point1.x, 987654.0f));
   #else
    #define JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED
   #endif
}

ColourGradient::ColourGradient (const ColourGradient& other)
    : point1 (other.point1), point2 (other.point2), isRadial (other.isRadial), colours (other.colours)
{}

ColourGradient::ColourGradient (ColourGradient&& other) noexcept
    : point1 (other.point1), point2 (other.point2), isRadial (other.isRadial),
      colours (std::move (other.colours))
{}

ColourGradient& ColourGradient::operator= (const ColourGradient& other)
{
    point1 = other.point1;
    point2 = other.point2;
    isRadial = other.isRadial;
    colours = other.colours;
    return *this;
}

ColourGradient& ColourGradient::operator= (ColourGradient&& other) noexcept
{
    point1 = other.point1;
    point2 = other.point2;
    isRadial = other.isRadial;
    colours = std::move (other.colours);
    return *this;
}

ColourGradient::ColourGradient (Colour colour1, float x1, float y1,
                                Colour colour2, float x2, float y2, bool radial)
    : ColourGradient (colour1, Point<float> (x1, y1),
                      colour2, Point<float> (x2, y2), radial)
{
}

ColourGradient::ColourGradient (Colour colour1, Point<float> p1,
                                Colour colour2, Point<float> p2, bool radial)
    : point1 (p1),
      point2 (p2),
      isRadial (radial)
{
    colours.add (ColourPoint { 0.0, colour1 },
                 ColourPoint { 1.0, colour2 });
}

ColourGradient::~ColourGradient() {}

ColourGradient ColourGradient::vertical (Colour c1, float y1, Colour c2, float y2)
{
    return { c1, 0, y1, c2, 0, y2, false };
}

ColourGradient ColourGradient::horizontal (Colour c1, float x1, Colour c2, float x2)
{
    return { c1, x1, 0, c2, x2, 0, false };
}

bool ColourGradient::operator== (const ColourGradient& other) const noexcept
{
    return point1 == other.point1 && point2 == other.point2
            && isRadial == other.isRadial
            && colours == other.colours;
}

bool ColourGradient::operator!= (const ColourGradient& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
void ColourGradient::clearColours()
{
    colours.clear();
}

int ColourGradient::addColour (const double proportionAlongGradient, Colour colour)
{
    // must be within the two end-points
    jassert (proportionAlongGradient >= 0 && proportionAlongGradient <= 1.0);

    if (proportionAlongGradient <= 0)
    {
        colours.set (0, { 0.0, colour });
        return 0;
    }

    auto pos = jmin (1.0, proportionAlongGradient);

    int i;
    for (i = 0; i < colours.size(); ++i)
        if (colours.getReference (i).position > pos)
            break;

    colours.insert (i, { pos, colour });
    return i;
}

void ColourGradient::removeColour (int index)
{
    jassert (index > 0 && index < colours.size() - 1);
    colours.remove (index);
}

void ColourGradient::multiplyOpacity (const float multiplier) noexcept
{
    for (auto& c : colours)
        c.colour = c.colour.withMultipliedAlpha (multiplier);
}

//==============================================================================
int ColourGradient::getNumColours() const noexcept
{
    return colours.size();
}

double ColourGradient::getColourPosition (int index) const noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        return colours.getReference (index).position;

    return 0;
 }

Colour ColourGradient::getColour (int index) const noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        return colours.getReference (index).colour;

    return {};
}

void ColourGradient::setColour (int index, Colour newColour) noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        colours.getReference (index).colour = newColour;
}

Colour ColourGradient::getColourAtPosition (double position) const noexcept
{
    jassert (approximatelyEqual (colours.getReference (0).position, 0.0)); // the first colour specified has to go at position 0

    if (position <= 0 || colours.size() <= 1)
        return colours.getReference (0).colour;

    int i = colours.size() - 1;
    while (position < colours.getReference (i).position)
        --i;

    auto& p1 = colours.getReference (i);

    if (i >= colours.size() - 1)
        return p1.colour;

    auto& p2 = colours.getReference (i + 1);

    return p1.colour.interpolatedWith (p2.colour, (float) ((position - p1.position) / (p2.position - p1.position)));
}

//==============================================================================
void ColourGradient::createLookupTable (PixelARGB* const lookupTable, const int numEntries) const noexcept
{
    JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED // Trying to use this object without setting its coordinates?
    jassert (colours.size() >= 2);
    jassert (numEntries > 0);
    jassert (approximatelyEqual (colours.getReference (0).position, 0.0)); // The first colour specified has to go at position 0

    int index = 0;

    for (int j = 0; j < colours.size() - 1; ++j)
    {
        const auto& o = colours.getReference (j + 0);
        const auto& p = colours.getReference (j + 1);
        const auto numToDo = roundToInt (p.position * (numEntries - 1)) - index;
        const auto pix1 = o.colour.getNonPremultipliedPixelARGB();
        const auto pix2 = p.colour.getNonPremultipliedPixelARGB();

        for (auto i = 0; i < numToDo; ++i)
        {
            auto blended = pix1;
            blended.tween (pix2, (uint32) ((i << 8) / numToDo));
            blended.premultiply();

            jassert (0 <= index && index < numEntries);
            lookupTable[index++] = blended;
        }
    }

    std::fill (lookupTable + index, lookupTable + numEntries, colours.getLast().colour.getPixelARGB());
}

int ColourGradient::createLookupTable (const AffineTransform& transform, HeapBlock<PixelARGB>& lookupTable) const
{
    JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED // Trying to use this object without setting its coordinates?
    jassert (colours.size() >= 2);

    auto numEntries = jlimit (1, jmax (1, (colours.size() - 1) << 8),
                              3 * (int) point1.transformedBy (transform)
                                              .getDistanceFrom (point2.transformedBy (transform)));
    lookupTable.jmalloc (numEntries);
    createLookupTable (lookupTable, numEntries);
    return numEntries;
}

bool ColourGradient::isOpaque() const noexcept
{
    for (auto& c : colours)
        if (! c.colour.isOpaque())
            return false;

    return true;
}

bool ColourGradient::isInvisible() const noexcept
{
    for (auto& c : colours)
        if (! c.colour.isTransparent())
            return false;

    return true;
}

bool ColourGradient::ColourPoint::operator== (ColourPoint other) const noexcept
{
    const auto tie = [] (const ColourPoint& p) { return std::tie (p.position, p.colour); };
    return tie (*this) == tie (other);
}

bool ColourGradient::ColourPoint::operator!= (ColourPoint other) const noexcept
{
    return ! operator== (other);
}

} // namespace juce
