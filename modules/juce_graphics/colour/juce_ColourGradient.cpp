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

ColourGradient::ColourGradient() noexcept
{
   #if JUCE_DEBUG
    point1.setX (987654.0f);
    #define JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED   jassert (point1.x != 987654.0f);
   #else
    #define JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED
   #endif
}

ColourGradient::ColourGradient (Colour colour1, const float x1, const float y1,
                                Colour colour2, const float x2, const float y2,
                                const bool radial)
    : point1 (x1, y1),
      point2 (x2, y2),
      isRadial (radial)
{
    colours.add (ColourPoint (0.0, colour1));
    colours.add (ColourPoint (1.0, colour2));
}

ColourGradient::~ColourGradient()
{
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

    const double pos = jlimit (0.0, 1.0, proportionAlongGradient);

    int i;
    for (i = 0; i < colours.size(); ++i)
        if (colours.getReference(i).position > pos)
            break;

    colours.insert (i, ColourPoint (pos, colour));
    return i;
}

void ColourGradient::removeColour (int index)
{
    jassert (index > 0 && index < colours.size() - 1);
    colours.remove (index);
}

void ColourGradient::multiplyOpacity (const float multiplier) noexcept
{
    for (int i = 0; i < colours.size(); ++i)
    {
        Colour& c = colours.getReference(i).colour;
        c = c.withMultipliedAlpha (multiplier);
    }
}

//==============================================================================
int ColourGradient::getNumColours() const noexcept
{
    return colours.size();
}

double ColourGradient::getColourPosition (const int index) const noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        return colours.getReference (index).position;

    return 0;
 }

Colour ColourGradient::getColour (const int index) const noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        return colours.getReference (index).colour;

    return Colour();
}

void ColourGradient::setColour (int index, Colour newColour) noexcept
{
    if (isPositiveAndBelow (index, colours.size()))
        colours.getReference (index).colour = newColour;
}

Colour ColourGradient::getColourAtPosition (const double position) const noexcept
{
    jassert (colours.getReference(0).position == 0); // the first colour specified has to go at position 0

    if (position <= 0 || colours.size() <= 1)
        return colours.getReference(0).colour;

    int i = colours.size() - 1;
    while (position < colours.getReference(i).position)
        --i;

    const ColourPoint& p1 = colours.getReference (i);

    if (i >= colours.size() - 1)
        return p1.colour;

    const ColourPoint& p2 = colours.getReference (i + 1);

    return p1.colour.interpolatedWith (p2.colour, (float) ((position - p1.position) / (p2.position - p1.position)));
}

//==============================================================================
void ColourGradient::createLookupTable (PixelARGB* const lookupTable, const int numEntries) const noexcept
{
    JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED // Trying to use this object without setting its coordinates?
    jassert (colours.size() >= 2);
    jassert (numEntries > 0);
    jassert (colours.getReference(0).position == 0); // The first colour specified has to go at position 0

    PixelARGB pix1 (colours.getReference (0).colour.getPixelARGB());
    int index = 0;

    for (int j = 1; j < colours.size(); ++j)
    {
        const ColourPoint& p = colours.getReference (j);
        const int numToDo = roundToInt (p.position * (numEntries - 1)) - index;
        const PixelARGB pix2 (p.colour.getPixelARGB());

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

int ColourGradient::createLookupTable (const AffineTransform& transform, HeapBlock <PixelARGB>& lookupTable) const
{
    JUCE_COLOURGRADIENT_CHECK_COORDS_INITIALISED // Trying to use this object without setting its coordinates?
    jassert (colours.size() >= 2);

    const int numEntries = jlimit (1, jmax (1, (colours.size() - 1) << 8),
                                   3 * (int) point1.transformedBy (transform)
                                                .getDistanceFrom (point2.transformedBy (transform)));
    lookupTable.malloc ((size_t) numEntries);
    createLookupTable (lookupTable, numEntries);
    return numEntries;
}

bool ColourGradient::isOpaque() const noexcept
{
    for (int i = 0; i < colours.size(); ++i)
        if (! colours.getReference(i).colour.isOpaque())
            return false;

    return true;
}

bool ColourGradient::isInvisible() const noexcept
{
    for (int i = 0; i < colours.size(); ++i)
        if (! colours.getReference(i).colour.isTransparent())
            return false;

    return true;
}

bool ColourGradient::ColourPoint::operator== (const ColourPoint& other) const noexcept
{
    return position == other.position && colour == other.colour;
}

bool ColourGradient::ColourPoint::operator!= (const ColourPoint& other) const noexcept
{
    return position != other.position || colour != other.colour;
}
