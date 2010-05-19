/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ColourGradient.h"


//==============================================================================
ColourGradient::ColourGradient() throw()
{
#if JUCE_DEBUG
    point1.setX (987654.0f);
#endif
}

ColourGradient::ColourGradient (const Colour& colour1,
                                const float x1_,
                                const float y1_,
                                const Colour& colour2,
                                const float x2_,
                                const float y2_,
                                const bool isRadial_)
    : point1 (x1_, y1_),
      point2 (x2_, y2_),
      isRadial (isRadial_)
{
    colours.add (ColourPoint (0, colour1));
    colours.add (ColourPoint (1 << 16, colour2));
}

ColourGradient::~ColourGradient()
{
}

bool ColourGradient::operator== (const ColourGradient& other) const throw()
{
    return point1 == other.point1 && point2 == other.point2
            && isRadial == other.isRadial
            && colours == other.colours;
}

bool ColourGradient::operator!= (const ColourGradient& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
void ColourGradient::clearColours()
{
    colours.clear();
}

void ColourGradient::addColour (const double proportionAlongGradient,
                                const Colour& colour)
{
    // must be within the two end-points
    jassert (proportionAlongGradient >= 0 && proportionAlongGradient <= 1.0);

    const uint32 pos = jlimit (0, 65535, roundToInt (proportionAlongGradient * 65536.0));

    int i;
    for (i = 0; i < colours.size(); ++i)
        if (colours.getReference(i).position > pos)
            break;

    colours.insert (i, ColourPoint (pos, colour));
}

void ColourGradient::multiplyOpacity (const float multiplier) throw()
{
    for (int i = 0; i < colours.size(); ++i)
    {
        Colour& c = colours.getReference(i).colour;
        c = c.withMultipliedAlpha (multiplier);
    }
}

//==============================================================================
int ColourGradient::getNumColours() const throw()
{
    return colours.size();
}

double ColourGradient::getColourPosition (const int index) const throw()
{
    if (((unsigned int) index) < (unsigned int) colours.size())
        return jlimit (0.0, 1.0, colours.getReference (index).position / 65535.0);

    return 0;
 }

const Colour ColourGradient::getColour (const int index) const throw()
{
    if (((unsigned int) index) < (unsigned int) colours.size())
        return colours.getReference (index).colour;

    return Colour();
}

const Colour ColourGradient::getColourAtPosition (const float position) const throw()
{
    jassert (colours.getReference(0).position == 0); // the first colour specified has to go at position 0

    const int integerPos = jlimit (0, 65535, roundToInt (position * 65536.0f));

    if (integerPos <= 0 || colours.size() <= 1)
        return getColour (0);

    int i = colours.size() - 1;
    while (integerPos < (int) colours.getReference(i).position)
        --i;

    if (i >= colours.size() - 1)
        return colours.getReference(i).colour;

    const ColourPoint& p1 = colours.getReference (i);
    const ColourPoint& p2 = colours.getReference (i + 1);

    return p1.colour.interpolatedWith (p2.colour, (integerPos - p1.position) / (float) (p2.position - p1.position));
}

//==============================================================================
int ColourGradient::createLookupTable (const AffineTransform& transform, HeapBlock <PixelARGB>& lookupTable) const
{
#if JUCE_DEBUG
    // trying to use the object without setting its co-ordinates? Have a careful read of
    // the comments for the constructors.
    jassert (point1.getX() != 987654.0f);
#endif

    const int numEntries = jlimit (1, (colours.size() - 1) << 8,
                                   3 * (int) point1.transformedBy (transform)
                                                .getDistanceFrom (point2.transformedBy (transform)));
    lookupTable.malloc (numEntries);

    if (colours.size() >= 2)
    {
        jassert (colours.getReference(0).position == 0); // the first colour specified has to go at position 0

        PixelARGB pix1 (colours.getReference (0).colour.getPixelARGB());
        int index = 0;

        for (int j = 1; j < colours.size(); ++j)
        {
            const ColourPoint& p = colours.getReference (j);
            const int numToDo = ((p.position * (numEntries - 1)) >> 16) - index;
            const PixelARGB pix2 (p.colour.getPixelARGB());

            for (int i = 0; i < numToDo; ++i)
            {
                jassert (index >= 0 && index < numEntries);

                lookupTable[index] = pix1;
                lookupTable[index].tween (pix2, (i << 8) / numToDo);
                ++index;
            }

            pix1 = pix2;
        }

        while (index < numEntries)
            lookupTable [index++] = pix1;
    }
    else
    {
        jassertfalse; // no colours specified!
    }

    return numEntries;
}

bool ColourGradient::isOpaque() const throw()
{
    for (int i = 0; i < colours.size(); ++i)
        if (! colours.getReference(i).colour.isOpaque())
            return false;

    return true;
}

bool ColourGradient::isInvisible() const throw()
{
    for (int i = 0; i < colours.size(); ++i)
        if (! colours.getReference(i).colour.isTransparent())
            return false;

    return true;
}


END_JUCE_NAMESPACE
