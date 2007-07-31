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


#include "juce_EdgeTable.h"
#include "../geometry/juce_PathIterator.h"


//==============================================================================
EdgeTable::EdgeTable (const int top_,
                      const int height_,
                      const OversamplingLevel oversampling_,
                      const int expectedEdgesPerLine) throw()
   : top (top_),
     height (height_),
     maxEdgesPerLine (expectedEdgesPerLine),
     lineStrideElements ((expectedEdgesPerLine << 1) + 1),
     oversampling (oversampling_)
{
    table = (int*) juce_calloc ((height << (int)oversampling_)
                                  * lineStrideElements * sizeof (int));
}

EdgeTable::EdgeTable (const EdgeTable& other) throw()
   : table (0)
{
    operator= (other);
}

const EdgeTable& EdgeTable::operator= (const EdgeTable& other) throw()
{
    juce_free (table);

    top = other.top;
    height = other.height;
    maxEdgesPerLine = other.maxEdgesPerLine;
    lineStrideElements = other.lineStrideElements;
    oversampling = other.oversampling;

    const int tableSize = (height << (int)oversampling)
                            * lineStrideElements * sizeof (int);

    table = (int*) juce_malloc (tableSize);
    memcpy (table, other.table, tableSize);

    return *this;
}

EdgeTable::~EdgeTable() throw()
{
    juce_free (table);
}

//==============================================================================
void EdgeTable::remapTableForNumEdges (const int newNumEdgesPerLine) throw()
{
    if (newNumEdgesPerLine != maxEdgesPerLine)
    {
        maxEdgesPerLine = newNumEdgesPerLine;

        const int newLineStrideElements = maxEdgesPerLine * 2 + 1;
        int* const newTable = (int*) juce_malloc ((height << (int) oversampling)
                                                    * newLineStrideElements * sizeof (int));

        for (int i = 0; i < (height << (int) oversampling); ++i)
        {
            const int* srcLine = table + lineStrideElements * i;
            int* dstLine = newTable + newLineStrideElements * i;

            int num = *srcLine++;
            *dstLine++ = num;

            num <<= 1;
            while (--num >= 0)
                *dstLine++ = *srcLine++;
        }

        juce_free (table);
        table = newTable;
        lineStrideElements = newLineStrideElements;
    }
}

void EdgeTable::optimiseTable() throw()
{
    int maxLineElements = 0;

    for (int i = height; --i >= 0;)
        maxLineElements = jmax (maxLineElements,
                                table [i * lineStrideElements]);

    remapTableForNumEdges (maxLineElements);
}

//==============================================================================
void EdgeTable::addEdgePoint (const int x, const int y, const int winding) throw()
{
    jassert (y >= 0 && y < (height << oversampling))

    int* lineStart = table + lineStrideElements * y;
    int n = lineStart[0];

    if (n >= maxEdgesPerLine)
    {
        remapTableForNumEdges (maxEdgesPerLine + juce_edgeTableDefaultEdgesPerLine);
        lineStart = table + lineStrideElements * y;
    }

    n <<= 1;

    int* const line = lineStart + 1;

    while (n > 0)
    {
        const int cx = line [n - 2];

        if (cx <= x)
            break;

        line [n] = cx;
        line [n + 1] = line [n - 1];
        n -= 2;
    }

    line [n] = x;
    line [n + 1] = winding;

    lineStart[0]++;
}

//==============================================================================
void EdgeTable::addPath (const Path& path,
                         const AffineTransform& transform) throw()
{
    const int windingAmount = 256 / (1 << (int) oversampling);
    const float timesOversampling = (float) (1 << (int) oversampling);

    const int bottomLimit = (height << (int) oversampling);

    PathFlatteningIterator iter (path, transform);

    while (iter.next())
    {
        int y1 = roundFloatToInt (iter.y1 * timesOversampling) - (top << (int) oversampling);
        int y2 = roundFloatToInt (iter.y2 * timesOversampling) - (top << (int) oversampling);

        if (y1 != y2)
        {
            const double x1 = 256.0 * iter.x1;
            const double x2 = 256.0 * iter.x2;

            const double multiplier = (x2 - x1) / (y2 - y1);

            const int oldY1 = y1;
            int winding;

            if (y1 > y2)
            {
                swapVariables (y1, y2);
                winding = windingAmount;
            }
            else
            {
                winding = -windingAmount;
            }

            jassert (y1 < y2);

            if (y1 < 0)
                y1 = 0;

            if (y2 > bottomLimit)
                y2 = bottomLimit;

            while (y1 < y2)
            {
                addEdgePoint (roundDoubleToInt (x1 + multiplier * (y1 - oldY1)),
                              y1,
                              winding);

                ++y1;
            }
        }
    }

    if (! path.isUsingNonZeroWinding())
    {
        // if it's an alternate-winding path, we need to go through and
        // make sure all the windings are alternating.

        int* lineStart = table;

        for (int i = height << (int) oversampling; --i >= 0;)
        {
            int* line = lineStart;
            lineStart += lineStrideElements;

            int num = *line;

            while (--num >= 0)
            {
                line += 2;
                *line = abs (*line);

                if (--num >= 0)
                {
                    line += 2;
                    *line = -abs (*line);
                }
            }
        }
    }
}


END_JUCE_NAMESPACE
