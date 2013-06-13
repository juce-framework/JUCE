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

const int juce_edgeTableDefaultEdgesPerLine = 32;

//==============================================================================
EdgeTable::EdgeTable (const Rectangle<int>& bounds_,
                      const Path& path, const AffineTransform& transform)
   : bounds (bounds_),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine << 1) + 1),
     needToCheckEmptinesss (true)
{
    table.malloc ((size_t) ((bounds.getHeight() + 1) * lineStrideElements));
    int* t = table;

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }

    const int leftLimit   = bounds.getX() << 8;
    const int topLimit    = bounds.getY() << 8;
    const int rightLimit  = bounds.getRight() << 8;
    const int heightLimit = bounds.getHeight() << 8;

    PathFlatteningIterator iter (path, transform);

    while (iter.next())
    {
        int y1 = roundToInt (iter.y1 * 256.0f);
        int y2 = roundToInt (iter.y2 * 256.0f);

        if (y1 != y2)
        {
            y1 -= topLimit;
            y2 -= topLimit;

            const int startY = y1;
            int direction = -1;

            if (y1 > y2)
            {
                std::swap (y1, y2);
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
                const int stepSize = jlimit (1, 256, 256 / (1 + (int) std::abs (multiplier)));

                do
                {
                    const int step = jmin (stepSize, y2 - y1, 256 - (y1 & 255));
                    int x = roundToInt (startX + multiplier * ((y1 + (step >> 1)) - startY));

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

    sanitiseLevels (path.isUsingNonZeroWinding());
}

EdgeTable::EdgeTable (const Rectangle<int>& rectangleToAdd)
   : bounds (rectangleToAdd),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine << 1) + 1),
     needToCheckEmptinesss (true)
{
    table.malloc ((size_t) (jmax (1, bounds.getHeight()) * lineStrideElements));
    table[0] = 0;

    const int x1 = rectangleToAdd.getX() << 8;
    const int x2 = rectangleToAdd.getRight() << 8;

    int* t = table;
    for (int i = rectangleToAdd.getHeight(); --i >= 0;)
    {
        t[0] = 2;
        t[1] = x1;
        t[2] = 255;
        t[3] = x2;
        t[4] = 0;
        t += lineStrideElements;
    }
}

EdgeTable::EdgeTable (const RectangleList& rectanglesToAdd)
   : bounds (rectanglesToAdd.getBounds()),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine << 1) + 1),
     needToCheckEmptinesss (true)
{
    table.malloc ((size_t) (jmax (1, bounds.getHeight()) * lineStrideElements));

    int* t = table;
    for (int i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }

    for (const Rectangle<int>* r = rectanglesToAdd.begin(), * const e = rectanglesToAdd.end(); r != e; ++r)
    {
        const int x1 = r->getX() << 8;
        const int x2 = r->getRight() << 8;
        int y = r->getY() - bounds.getY();

        for (int j = r->getHeight(); --j >= 0;)
        {
            addEdgePoint (x1, y, 255);
            addEdgePoint (x2, y, -255);
            ++y;
        }
    }

    sanitiseLevels (true);
}

EdgeTable::EdgeTable (const Rectangle<float>& rectangleToAdd)
   : bounds (Rectangle<int> ((int) std::floor (rectangleToAdd.getX()),
                             roundToInt (rectangleToAdd.getY() * 256.0f) >> 8,
                             2 + (int) rectangleToAdd.getWidth(),
                             2 + (int) rectangleToAdd.getHeight())),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine << 1) + 1),
     needToCheckEmptinesss (true)
{
    jassert (! rectangleToAdd.isEmpty());
    table.malloc ((size_t) (jmax (1, bounds.getHeight()) * lineStrideElements));
    table[0] = 0;

    const int x1 = roundToInt (rectangleToAdd.getX() * 256.0f);
    const int x2 = roundToInt (rectangleToAdd.getRight() * 256.0f);

    int y1 = roundToInt (rectangleToAdd.getY() * 256.0f) - (bounds.getY() << 8);
    jassert (y1 < 256);
    int y2 = roundToInt (rectangleToAdd.getBottom() * 256.0f) - (bounds.getY() << 8);

    if (x2 <= x1 || y2 <= y1)
    {
        bounds.setHeight (0);
        return;
    }

    int lineY = 0;
    int* t = table;

    if ((y1 >> 8) == (y2 >> 8))
    {
        t[0] = 2;
        t[1] = x1;
        t[2] = y2 - y1;
        t[3] = x2;
        t[4] = 0;
        ++lineY;
        t += lineStrideElements;
    }
    else
    {
        t[0] = 2;
        t[1] = x1;
        t[2] = 255 - (y1 & 255);
        t[3] = x2;
        t[4] = 0;
        ++lineY;
        t += lineStrideElements;

        while (lineY < (y2 >> 8))
        {
            t[0] = 2;
            t[1] = x1;
            t[2] = 255;
            t[3] = x2;
            t[4] = 0;
            ++lineY;
            t += lineStrideElements;
        }

        jassert (lineY < bounds.getHeight());
        t[0] = 2;
        t[1] = x1;
        t[2] = y2 & 255;
        t[3] = x2;
        t[4] = 0;
        ++lineY;
        t += lineStrideElements;
    }

    while (lineY < bounds.getHeight())
    {
        t[0] = 0;
        t += lineStrideElements;
        ++lineY;
    }
}

EdgeTable::EdgeTable (const EdgeTable& other)
{
    operator= (other);
}

EdgeTable& EdgeTable::operator= (const EdgeTable& other)
{
    bounds = other.bounds;
    maxEdgesPerLine = other.maxEdgesPerLine;
    lineStrideElements = other.lineStrideElements;
    needToCheckEmptinesss = other.needToCheckEmptinesss;

    table.malloc ((size_t) (jmax (1, bounds.getHeight()) * lineStrideElements));
    copyEdgeTableData (table, lineStrideElements, other.table, lineStrideElements, bounds.getHeight());
    return *this;
}

EdgeTable::~EdgeTable()
{
}

//==============================================================================
void EdgeTable::copyEdgeTableData (int* dest, const int destLineStride, const int* src, const int srcLineStride, int numLines) noexcept
{
    while (--numLines >= 0)
    {
        memcpy (dest, src, (size_t) (src[0] * 2 + 1) * sizeof (int));
        src += srcLineStride;
        dest += destLineStride;
    }
}

void EdgeTable::sanitiseLevels (const bool useNonZeroWinding) noexcept
{
    // Convert the table from relative windings to absolute levels..
    int* lineStart = table;

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        int* line = lineStart;
        lineStart += lineStrideElements;

        int num = *line;
        if (num == 0)
            continue;

        int level = 0;

        if (useNonZeroWinding)
        {
            while (--num > 0)
            {
                line += 2;
                level += *line;
                int corrected = abs (level);
                if (corrected >> 8)
                    corrected = 255;

                *line = corrected;
            }
        }
        else
        {
            while (--num > 0)
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

                *line = corrected;
            }
        }

        line[2] = 0; // force the last level to 0, just in case something went wrong in creating the table
    }
}

void EdgeTable::remapTableForNumEdges (const int newNumEdgesPerLine)
{
    if (newNumEdgesPerLine != maxEdgesPerLine)
    {
        maxEdgesPerLine = newNumEdgesPerLine;

        jassert (bounds.getHeight() > 0);
        const int newLineStrideElements = maxEdgesPerLine * 2 + 1;

        HeapBlock <int> newTable ((size_t) (bounds.getHeight() * newLineStrideElements));

        copyEdgeTableData (newTable, newLineStrideElements, table, lineStrideElements, bounds.getHeight());

        table.swapWith (newTable);
        lineStrideElements = newLineStrideElements;
    }
}

void EdgeTable::optimiseTable()
{
    int maxLineElements = 0;

    for (int i = bounds.getHeight(); --i >= 0;)
        maxLineElements = jmax (maxLineElements, table [i * lineStrideElements]);

    remapTableForNumEdges (maxLineElements);
}

void EdgeTable::addEdgePoint (const int x, const int y, const int winding)
{
    jassert (y >= 0 && y < bounds.getHeight());

    int* line = table + lineStrideElements * y;
    const int numPoints = line[0];
    int n = numPoints << 1;

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

        memmove (line + (n + 3), line + (n + 1), sizeof (int) * (size_t) ((numPoints << 1) - n));
    }

    line [n + 1] = x;
    line [n + 2] = winding;
    line[0]++;
}

void EdgeTable::translate (float dx, const int dy) noexcept
{
    bounds.translate ((int) std::floor (dx), dy);

    int* lineStart = table;
    const int intDx = (int) (dx * 256.0f);

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        int* line = lineStart;
        lineStart += lineStrideElements;
        int num = *line++;

        while (--num >= 0)
        {
            *line += intDx;
            line += 2;
        }
    }
}

void EdgeTable::intersectWithEdgeTableLine (const int y, const int* otherLine)
{
    jassert (y >= 0 && y < bounds.getHeight());

    int* dest = table + lineStrideElements * y;
    if (dest[0] == 0)
        return;

    int otherNumPoints = *otherLine;
    if (otherNumPoints == 0)
    {
        *dest = 0;
        return;
    }

    const int right = bounds.getRight() << 8;

    // optimise for the common case where our line lies entirely within a
    // single pair of points, as happens when clipping to a simple rect.
    if (otherNumPoints == 2 && otherLine[2] >= 255)
    {
        clipEdgeTableLineToRange (dest, otherLine[1], jmin (right, otherLine[3]));
        return;
    }

    ++otherLine;
    const size_t lineSizeBytes = (size_t) (dest[0] * 2 + 1) * sizeof (int);
    int* temp = static_cast<int*> (alloca (lineSizeBytes));
    memcpy (temp, dest, lineSizeBytes);

    const int* src1 = temp;
    int srcNum1 = *src1++;
    int x1 = *src1++;

    const int* src2 = otherLine;
    int srcNum2 = otherNumPoints;
    int x2 = *src2++;

    int destIndex = 0, destTotal = 0;
    int level1 = 0, level2 = 0;
    int lastX = std::numeric_limits<int>::min(), lastLevel = 0;

    while (srcNum1 > 0 && srcNum2 > 0)
    {
        int nextX;

        if (x1 < x2)
        {
            nextX = x1;
            level1 = *src1++;
            x1 = *src1++;
            --srcNum1;
        }
        else if (x1 == x2)
        {
            nextX = x1;
            level1 = *src1++;
            level2 = *src2++;
            x1 = *src1++;
            x2 = *src2++;
            --srcNum1;
            --srcNum2;
        }
        else
        {
            nextX = x2;
            level2 = *src2++;
            x2 = *src2++;
            --srcNum2;
        }

        if (nextX > lastX)
        {
            if (nextX >= right)
                break;

            lastX = nextX;

            const int nextLevel = (level1 * (level2 + 1)) >> 8;
            jassert (isPositiveAndBelow (nextLevel, (int) 256));

            if (nextLevel != lastLevel)
            {
                if (destTotal >= maxEdgesPerLine)
                {
                    dest[0] = destTotal;
                    remapTableForNumEdges (maxEdgesPerLine + juce_edgeTableDefaultEdgesPerLine);
                    dest = table + lineStrideElements * y;
                }

                ++destTotal;
                lastLevel = nextLevel;
                dest[++destIndex] = nextX;
                dest[++destIndex] = nextLevel;
            }
        }
    }

    if (lastLevel > 0)
    {
        if (destTotal >= maxEdgesPerLine)
        {
            dest[0] = destTotal;
            remapTableForNumEdges (maxEdgesPerLine + juce_edgeTableDefaultEdgesPerLine);
            dest = table + lineStrideElements * y;
        }

        ++destTotal;
        dest[++destIndex] = right;
        dest[++destIndex] = 0;
    }

    dest[0] = destTotal;

#if JUCE_DEBUG
    int last = std::numeric_limits<int>::min();
    for (int i = 0; i < dest[0]; ++i)
    {
        jassert (dest[i * 2 + 1] > last);
        last = dest[i * 2 + 1];
    }

    jassert (dest [dest[0] * 2] == 0);
#endif
}

void EdgeTable::clipEdgeTableLineToRange (int* dest, const int x1, const int x2) noexcept
{
    int* lastItem = dest + (dest[0] * 2 - 1);

    if (x2 < lastItem[0])
    {
        if (x2 <= dest[1])
        {
            dest[0] = 0;
            return;
        }

        while (x2 < lastItem[-2])
        {
            --(dest[0]);
            lastItem -= 2;
        }

        lastItem[0] = x2;
        lastItem[1] = 0;
    }

    if (x1 > dest[1])
    {
        while (lastItem[0] > x1)
            lastItem -= 2;

        const int itemsRemoved = (int) (lastItem - (dest + 1)) / 2;

        if (itemsRemoved > 0)
        {
            dest[0] -= itemsRemoved;
            memmove (dest + 1, lastItem, (size_t) dest[0] * (sizeof (int) * 2));
        }

        dest[1] = x1;
    }
}


//==============================================================================
void EdgeTable::clipToRectangle (const Rectangle<int>& r)
{
    const Rectangle<int> clipped (r.getIntersection (bounds));

    if (clipped.isEmpty())
    {
        needToCheckEmptinesss = false;
        bounds.setHeight (0);
    }
    else
    {
        const int top = clipped.getY() - bounds.getY();
        const int bottom = clipped.getBottom() - bounds.getY();

        if (bottom < bounds.getHeight())
            bounds.setHeight (bottom);

        for (int i = top; --i >= 0;)
            table [lineStrideElements * i] = 0;

        if (clipped.getX() > bounds.getX() || clipped.getRight() < bounds.getRight())
        {
            const int x1 = clipped.getX() << 8;
            const int x2 = jmin (bounds.getRight(), clipped.getRight()) << 8;
            int* line = table + lineStrideElements * top;

            for (int i = bottom - top; --i >= 0;)
            {
                if (line[0] != 0)
                    clipEdgeTableLineToRange (line, x1, x2);

                line += lineStrideElements;
            }
        }

        needToCheckEmptinesss = true;
    }
}

void EdgeTable::excludeRectangle (const Rectangle<int>& r)
{
    const Rectangle<int> clipped (r.getIntersection (bounds));

    if (! clipped.isEmpty())
    {
        const int top = clipped.getY() - bounds.getY();
        const int bottom = clipped.getBottom() - bounds.getY();

        const int rectLine[] = { 4, std::numeric_limits<int>::min(), 255,
                                 clipped.getX() << 8, 0,
                                 clipped.getRight() << 8, 255,
                                 std::numeric_limits<int>::max(), 0 };

        for (int i = top; i < bottom; ++i)
            intersectWithEdgeTableLine (i, rectLine);

        needToCheckEmptinesss = true;
    }
}

void EdgeTable::clipToEdgeTable (const EdgeTable& other)
{
    const Rectangle<int> clipped (other.bounds.getIntersection (bounds));

    if (clipped.isEmpty())
    {
        needToCheckEmptinesss = false;
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

        int i = 0;
        for (i = top; --i >= 0;)
            table [lineStrideElements * i] = 0;

        const int* otherLine = other.table + other.lineStrideElements * (clipped.getY() - other.bounds.getY());

        for (i = top; i < bottom; ++i)
        {
            intersectWithEdgeTableLine (i, otherLine);
            otherLine += other.lineStrideElements;
        }

        needToCheckEmptinesss = true;
    }
}

void EdgeTable::clipLineToMask (int x, int y, const uint8* mask, int maskStride, int numPixels)
{
    y -= bounds.getY();

    if (y < 0 || y >= bounds.getHeight())
        return;

    needToCheckEmptinesss = true;

    if (numPixels <= 0)
    {
        table [lineStrideElements * y] = 0;
        return;
    }

    int* tempLine = static_cast<int*> (alloca ((size_t) (numPixels * 2 + 4) * sizeof (int)));
    int destIndex = 0, lastLevel = 0;

    while (--numPixels >= 0)
    {
        const int alpha = *mask;
        mask += maskStride;

        if (alpha != lastLevel)
        {
            tempLine[++destIndex] = (x << 8);
            tempLine[++destIndex] = alpha;
            lastLevel = alpha;
        }

        ++x;
    }

    if (lastLevel > 0)
    {
        tempLine[++destIndex] = (x << 8);
        tempLine[++destIndex] = 0;
    }

    tempLine[0] = destIndex >> 1;

    intersectWithEdgeTableLine (y, tempLine);
}

bool EdgeTable::isEmpty() noexcept
{
    if (needToCheckEmptinesss)
    {
        needToCheckEmptinesss = false;
        int* t = table;

        for (int i = bounds.getHeight(); --i >= 0;)
        {
            if (t[0] > 1)
                return false;

            t += lineStrideElements;
        }

        bounds.setHeight (0);
    }

    return bounds.getHeight() == 0;
}
