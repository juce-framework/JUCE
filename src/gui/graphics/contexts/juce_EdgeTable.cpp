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
EdgeTable::EdgeTable (const Rectangle& bounds_,
                      const Path& path, const AffineTransform& transform) throw()
   : bounds (bounds_),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine << 1) + 1)
{
    table = (int*) juce_malloc ((bounds.getHeight() + 1) * lineStrideElements * sizeof (int));
    int* t = table;
    for (int i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }

    const int topLimit = bounds.getY() << 8;
    const int heightLimit = bounds.getHeight() << 8;
    const int leftLimit = bounds.getX() << 8;
    const int rightLimit = bounds.getRight() << 8;

    PathFlatteningIterator iter (path, transform);

    while (iter.next())
    {
        int y1 = roundFloatToInt (iter.y1 * 256.0f);
        int y2 = roundFloatToInt (iter.y2 * 256.0f);

        if (y1 != y2)
        {
            y1 -= topLimit;
            y2 -= topLimit;

            const int startY = y1;
            int direction = -1;

            if (y1 > y2)
            {
                swapVariables (y1, y2);
                direction = 1;
            }

            if (y1 < 0)
                y1 = 0;

            if (y2 > heightLimit)
                y2 = heightLimit;

            if (y1 < y2)
            {
                const double startX = 256.0f * iter.x1;
                const double multiplier = (iter.x2 - iter.x1) / (iter.y2 - iter.y1);
                const int stepSize = jlimit (1, 256, 256 / (1 + (int) fabs (multiplier)));

                do
                {
                    const int step = jmin (stepSize, y2 - y1, 256 - (y1 & 255));
                    int x = roundDoubleToInt (startX + multiplier * (y1 - startY));

                    if (x < leftLimit)
                        x = leftLimit;
                    else if (x >= rightLimit)
                        x = rightLimit - 1;

                    addEdgePoint (x, y1 >> 8, direction * step);
                    y1 += step;
                }
                while (y1 < y2);
            }
        }
    }

    if (! path.isUsingNonZeroWinding())
    {
        int* lineStart = table;

        for (int i = bounds.getHeight(); --i >= 0;)
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

EdgeTable::EdgeTable (const Rectangle& rectangleToAdd) throw()
   : bounds (rectangleToAdd),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine << 1) + 1)
{
    table = (int*) juce_malloc (jmax (1, bounds.getHeight()) * lineStrideElements * sizeof (int));
    *table = 0;

    const int x1 = rectangleToAdd.getX();
    const int x2 = rectangleToAdd.getRight();

    int* t = table;
    for (int i = rectangleToAdd.getHeight(); --i >= 0;)
    {
        t[0] = 2;
        t[1] = x1;
        t[2] = 256;
        t[3] = x2;
        t[4] = -256;
        t += lineStrideElements;
    }
}

EdgeTable::EdgeTable (const EdgeTable& other) throw()
   : table (0)
{
    operator= (other);
}

const EdgeTable& EdgeTable::operator= (const EdgeTable& other) throw()
{
    juce_free (table);

    bounds = other.bounds;
    maxEdgesPerLine = other.maxEdgesPerLine;
    lineStrideElements = other.lineStrideElements;

    const int tableSize = jmax (1, bounds.getHeight()) * lineStrideElements * sizeof (int);
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

        jassert (bounds.getHeight() > 0);
        const int newLineStrideElements = maxEdgesPerLine * 2 + 1;
        int* const newTable = (int*) juce_malloc (bounds.getHeight() * newLineStrideElements * sizeof (int));

        for (int i = 0; i < bounds.getHeight(); ++i)
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

    for (int i = bounds.getHeight(); --i >= 0;)
        maxLineElements = jmax (maxLineElements, table [i * lineStrideElements]);

    remapTableForNumEdges (maxLineElements);
}

void EdgeTable::addEdgePoint (const int x, const int y, const int winding) throw()
{
    jassert (y >= 0 && y < bounds.getHeight());

    int* line = table + lineStrideElements * y;
    const int numPoints = line[0];
    int n = line[0] << 1;

    if (n > 0)
    {
        while (n > 0)
        {
            const int cx = line [n - 1];

            if (cx <= x)
            {
                if (cx == x)
                {
                    line [n] += winding;
                    return;
                }

                break;
            }

            n -= 2;
        }

        if (numPoints >= maxEdgesPerLine)
        {
            remapTableForNumEdges (maxEdgesPerLine + juce_edgeTableDefaultEdgesPerLine);
            jassert (numPoints < maxEdgesPerLine);
            line = table + lineStrideElements * y;
        }

        memmove (line + (n + 3), line + (n + 1), sizeof (int) * ((numPoints << 1) - n));
    }

    line [n + 1] = x;
    line [n + 2] = winding;
    line[0]++;
}

void EdgeTable::clearLineSection (const int y, int minX, int maxX) throw()
{
    jassert (y >= 0 && y < bounds.getHeight());
    jassert (maxX > minX);

    int* lineStart = table + lineStrideElements * y;
    const int totalPoints = *lineStart;
    int* line = lineStart;
    int level = 0;
    int num = totalPoints;

    while (--num >= 0)
    {
        int x = *++line;

        if (x < minX)
        {
            level += *++line;
            continue;
        }

        if (x > maxX)
        {
            if (level != 0)
            {
                addEdgePoint (minX, y, -level);
                addEdgePoint (maxX, y, level);
            }

            return;
        }

        if (level != 0)
        {
            const int oldLevel = level;
            level += line[1];
            line[0] = minX;
            line[1] = -oldLevel;
            line += 2;
        }
        else
        {
            ++num;
        }

        int* cutPoint = line;
        int numToDelete = 0;

        while (--num >= 0)
        {
            x = *line++;

            if (x <= maxX)
            {
                level += *line++;
                ++numToDelete;
                continue;
            }

            break;
        }

        if (num < 0)
        {
            lineStart[0] -= numToDelete;
        }
        else
        {
            if (--numToDelete > 0)
            {
                char* startOfSrcSection = (char*) (cutPoint + numToDelete * 2);
                memmove (cutPoint, startOfSrcSection,
                         ((char*) (lineStart + (1 + 2 * totalPoints))) - startOfSrcSection);

                lineStart[0] -= numToDelete;
            }

            if (numToDelete < 0)
            {
                addEdgePoint (maxX, y, level);
            }
            else
            {
                cutPoint[0] = maxX;
                cutPoint[1] = level;
            }
        }

        break;
    }
}

void EdgeTable::intersectWithEdgeTableLine (const int y, const int* otherLine) throw()
{
//    int otherNum = otherLine[0];
}

//==============================================================================
/*void EdgeTable::clipToRectangle (const Rectangle& r) throw()
{
    const Rectangle clipped (r.getIntersection (bounds));

    if (clipped.isEmpty())
    {
        bounds.setHeight (0);
    }
    else
    {
        const int top = clipped.getY() - bounds.getY();
        const int bottom = clipped.getBottom() - bounds.getY();

        if (bottom < bounds.getHeight())
            bounds.setHeight (bottom);

        if (clipped.getRight() < bounds.getRight())
            bounds.setRight (clipped.getRight());

        for (int i = top; --i >= 0;)
            table [lineStrideElements * i] = 0;

        if (clipped.getX() > bounds.getX())
            for (int i = top; i < bottom; ++i)
                clearLineSection (i, bounds.getX(), clipped.getX());
    }
}

void EdgeTable::excludeRectangle (const Rectangle& r) throw()
{
    const Rectangle clipped (r.getIntersection (bounds));

    if (! clipped.isEmpty())
    {
        const int top = clipped.getY() - bounds.getY();
        const int bottom = clipped.getBottom() - bounds.getY();

        for (int i = top; i < bottom; ++i)
            clearLineSection (i, clipped.getX(), clipped.getRight());
    }
}

void EdgeTable::clipToEdgeTable (const EdgeTable& other)
{
    const Rectangle clipped (other.bounds.getIntersection (bounds));

    if (clipped.isEmpty())
    {
        bounds.setHeight (0);
    }
    else
    {
        const int top = clipped.getY() - bounds.getY();
        const int bottom = clipped.getBottom() - bounds.getY();

        if (bottom < bounds.getHeight())
            bounds.setHeight (bottom);

        if (clipped.getRight() < bounds.getRight())
            bounds.setRight (clipped.getRight());

        for (int i = top; --i >= 0;)
            table [lineStrideElements * i] = 0;

        const int* otherLine = other.table + other.lineStrideElements * (top + (bounds.getY() - other.bounds.getY()));

        for (int i = top; i < bottom; ++i)
        {
            intersectWithEdgeTableLine (i, otherLine);
            otherLine += other.lineStrideElements;
        }
    }
}

void EdgeTable::clipToImageAlpha (Image& image, int x, int y) throw()
{
}
*/
END_JUCE_NAMESPACE
