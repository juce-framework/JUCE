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

#include "juce_ColourGradient.h"


//==============================================================================
ColourGradient::ColourGradient() throw()
    : colours (4)
{
#ifdef JUCE_DEBUG
    x1 = 987654.0f;
#endif
}

ColourGradient::ColourGradient (const Colour& colour1,
                                const float x1_,
                                const float y1_,
                                const Colour& colour2,
                                const float x2_,
                                const float y2_,
                                const bool isRadial_) throw()
    : x1 (x1_),
      y1 (y1_),
      x2 (x2_),
      y2 (y2_),
      isRadial (isRadial_),
      colours (4)
{
    colours.add (0);
    colours.add (colour1.getPixelARGB().getARGB());

    colours.add (1 << 16);
    colours.add (colour2.getPixelARGB().getARGB());
}

ColourGradient::~ColourGradient() throw()
{
}

//==============================================================================
void ColourGradient::clearColours() throw()
{
    colours.clear();
}

void ColourGradient::addColour (const double proportionAlongGradient,
                                const Colour& colour) throw()
{
    // must be within the two end-points
    jassert (proportionAlongGradient >= 0 && proportionAlongGradient <= 1.0);

    const uint32 pos = jlimit (0, 65535, roundDoubleToInt (proportionAlongGradient * 65536.0));

    int i;
    for (i = 0; i < colours.size(); i += 2)
        if (colours.getUnchecked(i) > pos)
            break;

    colours.insert (i, pos);
    colours.insert (i + 1, colour.getPixelARGB().getARGB());
}

void ColourGradient::multiplyOpacity (const float multiplier) throw()
{
    for (int i = 1; i < colours.size(); i += 2)
    {
        PixelARGB pix (colours.getUnchecked(i));
        pix.multiplyAlpha (multiplier);
        colours.set (i, pix.getARGB());
    }
}

//==============================================================================
int ColourGradient::getNumColours() const throw()
{
    return colours.size() >> 1;
}

double ColourGradient::getColourPosition (const int index) const throw()
{
    return colours [index << 1];
}

const Colour ColourGradient::getColour (const int index) const throw()
{
    PixelARGB pix (colours [(index << 1) + 1]);
    pix.unpremultiply();
    return Colour (pix.getARGB());
}

//==============================================================================
PixelARGB* ColourGradient::createLookupTable (int& numEntries) const throw()
{
#ifdef JUCE_DEBUG
    // trying to use the object without setting its co-ordinates? Have a careful read of
    // the comments for the constructors.
    jassert (x1 != 987654.0f);
#endif

    const int numColours = colours.size() >> 1;

    float tx1 = x1, ty1 = y1, tx2 = x2, ty2 = y2;
    transform.transformPoint (tx1, ty1);
    transform.transformPoint (tx2, ty2);
    const double distance = juce_hypot (tx1 - tx2, ty1 - ty2);

    numEntries = jlimit (1, (numColours - 1) << 8, 3 * (int) distance);

    PixelARGB* const lookupTable = (PixelARGB*) juce_calloc (numEntries * sizeof (PixelARGB));

    if (numColours >= 2)
    {
        jassert (colours.getUnchecked (0) == 0); // the first colour specified has to go at position 0

        PixelARGB pix1 (colours.getUnchecked (1));
        int index = 0;

        for (int j = 2; j < colours.size(); j += 2)
        {
            const int numToDo = ((colours.getUnchecked (j) * numEntries) >> 16) - index;
            const PixelARGB pix2 (colours.getUnchecked (j + 1));

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
        jassertfalse // no colours specified!
    }

    return lookupTable;
}

bool ColourGradient::isOpaque() const throw()
{
    for (int i = 1; i < colours.size(); i += 2)
        if (PixelARGB (colours.getUnchecked(i)).getAlpha() < 0xff)
            return false;

    return true;
}

bool ColourGradient::isInvisible() const throw()
{
    for (int i = 1; i < colours.size(); i += 2)
        if (PixelARGB (colours.getUnchecked(i)).getAlpha() > 0)
            return false;

    return true;
}


END_JUCE_NAMESPACE
