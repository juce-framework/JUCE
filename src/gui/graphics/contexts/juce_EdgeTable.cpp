/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "juce_EdgeTable.h"
#include "../geometry/juce_PathIterator.h"

const int juce_edgeTableDefaultEdgesPerLine = 32;

//==============================================================================
EdgeTable::EdgeTable (const int top_, const int height_) throw()
   : top (top_),
     height (height_),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine << 1) + 1)
{
    table = (int*) juce_calloc (height * lineStrideElements * sizeof (int));
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

    const int tableSize = height * lineStrideElements * sizeof (int);
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
        int* const newTable = (int*) juce_malloc (height * newLineStrideElements * sizeof (int));

        for (int i = 0; i < height; ++i)
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
    jassert (y >= 0 && y < height)

    int* lineStart = table + lineStrideElements * y;
    int n = lineStart[0];

    if (n >= maxEdgesPerLine)
    {
        remapTableForNumEdges (maxEdgesPerLine + juce_edgeTableDefaultEdgesPerLine);
        jassert (n < maxEdgesPerLine);
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
void EdgeTable::addPath (const Path& path, const AffineTransform& transform) throw()
{
    const int bottomLimit = height << 8;

    PathFlatteningIterator iter (path, transform);

    while (iter.next())
    {
        int y1 = roundFloatToInt (iter.y1 * 256.0f) - (top << 8);
        int y2 = roundFloatToInt (iter.y2 * 256.0f) - (top << 8);

        if (y1 != y2)
        {
            const int oldY1 = y1;
            const double x1 = 256.0 * iter.x1;
            const double x2 = 256.0 * iter.x2;
            const double multiplier = (x2 - x1) / (y2 - y1);
            int winding = -1;

            if (y1 > y2)
            {
                swapVariables (y1, y2);
                winding = 1;
            }

            if (y1 < 0)
                y1 = 0;

            if (y2 > bottomLimit)
                y2 = bottomLimit;

            const int stepSize = jlimit (1, 256, 256 / (1 + abs ((int) multiplier)));

            while (y1 < y2)
            {
                const int step = jmin (stepSize, y2 - y1, 256 - (y1 & 255));

                addEdgePoint (roundDoubleToInt (x1 + multiplier * (y1 - oldY1)),
                              y1 >> 8, winding * step);

                y1 += step;
            }
        }
    }

    if (! path.isUsingNonZeroWinding())
    {
        int* lineStart = table;

        for (int i = height; --i >= 0;)
        {
            int* line = lineStart;
            lineStart += lineStrideElements;
            int num = *line;
            int level = 0;
            int lastCorrected = 0;

            while (--num >= 0)
            {
                line += 2;
                level += *line;
                int corrected = abs (level);
                if (corrected >> 8)
                {
                    corrected &= 511;
                    if (corrected >> 8)
                        corrected = 511 - corrected;
                }

                *line = corrected - lastCorrected;
                lastCorrected = corrected;
            }
        }
    }
}

/*void EdgeTable::clipToRectangle (const Rectangle& r) throw()
{
}

void EdgeTable::intersectWith (const EdgeTable& other)
{
}

void EdgeTable::generateFromImageAlpha (Image& image, int x, int y) throw()
{
}*/

END_JUCE_NAMESPACE
