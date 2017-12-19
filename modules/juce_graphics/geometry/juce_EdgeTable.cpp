/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
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

const int juce_edgeTableDefaultEdgesPerLine = 32;

//==============================================================================
EdgeTable::EdgeTable (Rectangle<int> area, const Path& path, const AffineTransform& transform)
   : bounds (area),
     // this is a very vague heuristic to make a rough guess at a good table size
     // for a given path, such that it's big enough to mostly avoid remapping, but also
     // not so big that it's wasteful for simple paths.
     maxEdgesPerLine (jmax (juce_edgeTableDefaultEdgesPerLine / 2,
                            4 * (int) std::sqrt (path.data.size()))),
     lineStrideElements (maxEdgesPerLine * 2 + 1)
{
    allocate();
    int* t = table;

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }

    auto leftLimit   = bounds.getX() * 256;
    auto topLimit    = bounds.getY() * 256;
    auto rightLimit  = bounds.getRight() * 256;
    auto heightLimit = bounds.getHeight() * 256;

    PathFlatteningIterator iter (path, transform);

    while (iter.next())
    {
        auto y1 = roundToInt (iter.y1 * 256.0f);
        auto y2 = roundToInt (iter.y2 * 256.0f);

        if (y1 != y2)
        {
            y1 -= topLimit;
            y2 -= topLimit;

            auto startY = y1;
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
                auto stepSize = jlimit (1, 256, 256 / (1 + (int) std::abs (multiplier)));

                do
                {
                    auto step = jmin (stepSize, y2 - y1, 256 - (y1 & 255));
                    auto x = roundToInt (startX + multiplier * ((y1 + (step >> 1)) - startY));

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

EdgeTable::EdgeTable (Rectangle<int> rectangleToAdd)
   : bounds (rectangleToAdd),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements (juce_edgeTableDefaultEdgesPerLine * 2 + 1)
{
    allocate();
    table[0] = 0;

    auto x1 = rectangleToAdd.getX() << 8;
    auto x2 = rectangleToAdd.getRight() << 8;
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

EdgeTable::EdgeTable (const RectangleList<int>& rectanglesToAdd)
   : bounds (rectanglesToAdd.getBounds()),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements (juce_edgeTableDefaultEdgesPerLine * 2 + 1),
     needToCheckEmptiness (true)
{
    allocate();
    clearLineSizes();

    for (auto& r : rectanglesToAdd)
    {
        auto x1 = r.getX() << 8;
        auto x2 = r.getRight() << 8;
        auto y = r.getY() - bounds.getY();

        for (int j = r.getHeight(); --j >= 0;)
            addEdgePointPair (x1, x2, y++, 255);
    }

    sanitiseLevels (true);
}

EdgeTable::EdgeTable (const RectangleList<float>& rectanglesToAdd)
   : bounds (rectanglesToAdd.getBounds().getSmallestIntegerContainer()),
     maxEdgesPerLine (rectanglesToAdd.getNumRectangles() * 2),
     lineStrideElements (rectanglesToAdd.getNumRectangles() * 4 + 1)
{
    bounds.setHeight (bounds.getHeight() + 1);
    allocate();
    clearLineSizes();

    for (auto& r : rectanglesToAdd)
    {
        auto x1 = roundToInt (r.getX() * 256.0f);
        auto x2 = roundToInt (r.getRight() * 256.0f);

        auto y1 = roundToInt (r.getY() * 256.0f) - (bounds.getY() << 8);
        auto y2 = roundToInt (r.getBottom() * 256.0f) - (bounds.getY() << 8);

        if (x2 <= x1 || y2 <= y1)
            continue;

        auto y = y1 >> 8;
        auto lastLine = y2 >> 8;

        if (y == lastLine)
        {
            addEdgePointPair (x1, x2, y, y2 - y1);
        }
        else
        {
            addEdgePointPair (x1, x2, y++, 255 - (y1 & 255));

            while (y < lastLine)
                addEdgePointPair (x1, x2, y++, 255);

            jassert (y < bounds.getHeight());
            addEdgePointPair (x1, x2, y, y2 & 255);
        }
    }

    sanitiseLevels (true);
}

EdgeTable::EdgeTable (Rectangle<float> rectangleToAdd)
   : bounds ((int) std::floor (rectangleToAdd.getX()),
             roundToInt (rectangleToAdd.getY() * 256.0f) >> 8,
             2 + (int) rectangleToAdd.getWidth(),
             2 + (int) rectangleToAdd.getHeight()),
     maxEdgesPerLine (juce_edgeTableDefaultEdgesPerLine),
     lineStrideElements ((juce_edgeTableDefaultEdgesPerLine * 2) + 1)
{
    jassert (! rectangleToAdd.isEmpty());
    allocate();
    table[0] = 0;

    auto x1 = roundToInt (rectangleToAdd.getX()      * 256.0f);
    auto x2 = roundToInt (rectangleToAdd.getRight()  * 256.0f);
    auto y1 = roundToInt (rectangleToAdd.getY()      * 256.0f) - (bounds.getY() << 8);
    auto y2 = roundToInt (rectangleToAdd.getBottom() * 256.0f) - (bounds.getY() << 8);
    jassert (y1 < 256);

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
    needToCheckEmptiness = other.needToCheckEmptiness;

    allocate();
    copyEdgeTableData (table, lineStrideElements, other.table, lineStrideElements, bounds.getHeight());
    return *this;
}

EdgeTable::~EdgeTable()
{
}

//==============================================================================
static size_t getEdgeTableAllocationSize (int lineStride, int height) noexcept
{
    // (leave an extra line at the end for use as scratch space)
    return (size_t) (lineStride * (2 + jmax (0, height)));
}

void EdgeTable::allocate()
{
    table.malloc (getEdgeTableAllocationSize (lineStrideElements, bounds.getHeight()));
}

void EdgeTable::clearLineSizes() noexcept
{
    int* t = table;

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }
}

void EdgeTable::copyEdgeTableData (int* dest, int destLineStride, const int* src, int srcLineStride, int numLines) noexcept
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

    for (int y = bounds.getHeight(); --y >= 0;)
    {
        auto num = lineStart[0];

        if (num > 0)
        {
            auto* items = reinterpret_cast<LineItem*> (lineStart + 1);
            auto* itemsEnd = items + num;

            // sort the X coords
            std::sort (items, itemsEnd);

            auto* src = items;
            auto correctedNum = num;
            int level = 0;

            while (src < itemsEnd)
            {
                level += src->level;
                auto x = src->x;
                ++src;

                while (src < itemsEnd && src->x == x)
                {
                    level += src->level;
                    ++src;
                    --correctedNum;
                }

                auto corrected = std::abs (level);

                if (corrected >> 8)
                {
                    if (useNonZeroWinding)
                    {
                        corrected = 255;
                    }
                    else
                    {
                        corrected &= 511;

                        if (corrected >> 8)
                            corrected = 511 - corrected;
                    }
                }

                items->x = x;
                items->level = corrected;
                ++items;
            }

            lineStart[0] = correctedNum;
            (items - 1)->level = 0; // force the last level to 0, just in case something went wrong in creating the table
        }

        lineStart += lineStrideElements;
    }
}

void EdgeTable::remapTableForNumEdges (const int newNumEdgesPerLine)
{
    if (newNumEdgesPerLine != maxEdgesPerLine)
    {
        maxEdgesPerLine = newNumEdgesPerLine;

        jassert (bounds.getHeight() > 0);
        auto newLineStrideElements = maxEdgesPerLine * 2 + 1;

        HeapBlock<int> newTable (getEdgeTableAllocationSize (newLineStrideElements, bounds.getHeight()));

        copyEdgeTableData (newTable, newLineStrideElements, table, lineStrideElements, bounds.getHeight());

        table.swapWith (newTable);
        lineStrideElements = newLineStrideElements;
    }
}

inline void EdgeTable::remapWithExtraSpace (int numPoints)
{
    remapTableForNumEdges (numPoints * 2);
    jassert (numPoints < maxEdgesPerLine);
}

void EdgeTable::optimiseTable()
{
    int maxLineElements = 0;

    for (int i = bounds.getHeight(); --i >= 0;)
        maxLineElements = jmax (maxLineElements, table[i * lineStrideElements]);

    remapTableForNumEdges (maxLineElements);
}

void EdgeTable::addEdgePoint (const int x, const int y, const int winding)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* line = table + lineStrideElements * y;
    auto numPoints = line[0];

    if (numPoints >= maxEdgesPerLine)
    {
        remapWithExtraSpace (numPoints);
        line = table + lineStrideElements * y;
    }

    line[0] = numPoints + 1;
    line += numPoints * 2;
    line[1] = x;
    line[2] = winding;
}

void EdgeTable::addEdgePointPair (int x1, int x2, int y, int winding)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* line = table + lineStrideElements * y;
    auto numPoints = line[0];

    if (numPoints + 1 >= maxEdgesPerLine)
    {
        remapWithExtraSpace (numPoints + 1);
        line = table + lineStrideElements * y;
    }

    line[0] = numPoints + 2;
    line += numPoints * 2;
    line[1] = x1;
    line[2] = winding;
    line[3] = x2;
    line[4] = -winding;
}

void EdgeTable::translate (float dx, int dy) noexcept
{
    bounds.translate ((int) std::floor (dx), dy);

    int* lineStart = table;
    auto intDx = (int) (dx * 256.0f);

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        auto* line = lineStart;
        lineStart += lineStrideElements;
        auto num = *line++;

        while (--num >= 0)
        {
            *line += intDx;
            line += 2;
        }
    }
}

void EdgeTable::multiplyLevels (float amount)
{
    int* lineStart = table;
    auto multiplier = (int) (amount * 256.0f);

    for (int y = 0; y < bounds.getHeight(); ++y)
    {
        auto numPoints = lineStart[0];
        auto* item = reinterpret_cast<LineItem*> (lineStart + 1);
        lineStart += lineStrideElements;

        while (--numPoints > 0)
        {
            item->level = jmin (255, (item->level * multiplier) >> 8);
            ++item;
        }
    }
}

void EdgeTable::intersectWithEdgeTableLine (const int y, const int* const otherLine)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* srcLine = table + lineStrideElements * y;
    auto srcNum1 = *srcLine;

    if (srcNum1 == 0)
        return;

    auto srcNum2 = *otherLine;

    if (srcNum2 == 0)
    {
        *srcLine = 0;
        return;
    }

    auto right = bounds.getRight() << 8;

    // optimise for the common case where our line lies entirely within a
    // single pair of points, as happens when clipping to a simple rect.
    if (srcNum2 == 2 && otherLine[2] >= 255)
    {
        clipEdgeTableLineToRange (srcLine, otherLine[1], jmin (right, otherLine[3]));
        return;
    }

    bool isUsingTempSpace = false;

    const int* src1 = srcLine + 1;
    auto x1 = *src1++;

    const int* src2 = otherLine + 1;
    auto x2 = *src2++;

    int destIndex = 0, destTotal = 0;
    int level1 = 0, level2 = 0;
    int lastX = std::numeric_limits<int>::min(), lastLevel = 0;

    while (srcNum1 > 0 && srcNum2 > 0)
    {
        int nextX;

        if (x1 <= x2)
        {
            if (x1 == x2)
            {
                level2 = *src2++;
                x2 = *src2++;
                --srcNum2;
            }

            nextX = x1;
            level1 = *src1++;
            x1 = *src1++;
            --srcNum1;
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

            auto nextLevel = (level1 * (level2 + 1)) >> 8;
            jassert (isPositiveAndBelow (nextLevel, 256));

            if (nextLevel != lastLevel)
            {
                if (destTotal >= maxEdgesPerLine)
                {
                    srcLine[0] = destTotal;

                    if (isUsingTempSpace)
                    {
                        auto tempSize = (size_t) srcNum1 * 2 * sizeof (int);
                        auto oldTemp = static_cast<int*> (alloca (tempSize));
                        memcpy (oldTemp, src1, tempSize);

                        remapTableForNumEdges (jmax (256, destTotal * 2));
                        srcLine = table + lineStrideElements * y;

                        auto* newTemp = table + lineStrideElements * bounds.getHeight();
                        memcpy (newTemp, oldTemp, tempSize);
                        src1 = newTemp;
                    }
                    else
                    {
                        remapTableForNumEdges (jmax (256, destTotal * 2));
                        srcLine = table + lineStrideElements * y;
                    }
                }

                ++destTotal;
                lastLevel = nextLevel;

                if (! isUsingTempSpace)
                {
                    isUsingTempSpace = true;
                    auto* temp = table + lineStrideElements * bounds.getHeight();
                    memcpy (temp, src1, (size_t) srcNum1 * 2 * sizeof (int));
                    src1 = temp;
                }

                srcLine[++destIndex] = nextX;
                srcLine[++destIndex] = nextLevel;
            }
        }
    }

    if (lastLevel > 0)
    {
        if (destTotal >= maxEdgesPerLine)
        {
            srcLine[0] = destTotal;
            remapTableForNumEdges (jmax (256, destTotal * 2));
            srcLine = table + lineStrideElements * y;
        }

        ++destTotal;
        srcLine[++destIndex] = right;
        srcLine[++destIndex] = 0;
    }

    srcLine[0] = destTotal;
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

        auto itemsRemoved = (int) (lastItem - (dest + 1)) / 2;

        if (itemsRemoved > 0)
        {
            dest[0] -= itemsRemoved;
            memmove (dest + 1, lastItem, (size_t) dest[0] * (sizeof (int) * 2));
        }

        dest[1] = x1;
    }
}


//==============================================================================
void EdgeTable::clipToRectangle (Rectangle<int> r)
{
    auto clipped = r.getIntersection (bounds);

    if (clipped.isEmpty())
    {
        needToCheckEmptiness = false;
        bounds.setHeight (0);
    }
    else
    {
        auto top = clipped.getY() - bounds.getY();
        auto bottom = clipped.getBottom() - bounds.getY();

        if (bottom < bounds.getHeight())
            bounds.setHeight (bottom);

        for (int i = 0; i < top; ++i)
            table[lineStrideElements * i] = 0;

        if (clipped.getX() > bounds.getX() || clipped.getRight() < bounds.getRight())
        {
            auto x1 = clipped.getX() << 8;
            auto x2 = jmin (bounds.getRight(), clipped.getRight()) << 8;
            int* line = table + lineStrideElements * top;

            for (int i = bottom - top; --i >= 0;)
            {
                if (line[0] != 0)
                    clipEdgeTableLineToRange (line, x1, x2);

                line += lineStrideElements;
            }
        }

        needToCheckEmptiness = true;
    }
}

void EdgeTable::excludeRectangle (Rectangle<int> r)
{
    auto clipped = r.getIntersection (bounds);

    if (! clipped.isEmpty())
    {
        auto top = clipped.getY() - bounds.getY();
        auto bottom = clipped.getBottom() - bounds.getY();

        const int rectLine[] = { 4, std::numeric_limits<int>::min(), 255,
                                 clipped.getX() << 8, 0,
                                 clipped.getRight() << 8, 255,
                                 std::numeric_limits<int>::max(), 0 };

        for (int i = top; i < bottom; ++i)
            intersectWithEdgeTableLine (i, rectLine);

        needToCheckEmptiness = true;
    }
}

void EdgeTable::clipToEdgeTable (const EdgeTable& other)
{
    auto clipped = other.bounds.getIntersection (bounds);

    if (clipped.isEmpty())
    {
        needToCheckEmptiness = false;
        bounds.setHeight (0);
    }
    else
    {
        auto top = clipped.getY() - bounds.getY();
        auto bottom = clipped.getBottom() - bounds.getY();

        if (bottom < bounds.getHeight())
            bounds.setHeight (bottom);

        if (clipped.getRight() < bounds.getRight())
            bounds.setRight (clipped.getRight());

        for (int i = 0; i < top; ++i)
            table[lineStrideElements * i] = 0;

        auto* otherLine = other.table + other.lineStrideElements * (clipped.getY() - other.bounds.getY());

        for (int i = top; i < bottom; ++i)
        {
            intersectWithEdgeTableLine (i, otherLine);
            otherLine += other.lineStrideElements;
        }

        needToCheckEmptiness = true;
    }
}

void EdgeTable::clipLineToMask (int x, int y, const uint8* mask, int maskStride, int numPixels)
{
    y -= bounds.getY();

    if (y < 0 || y >= bounds.getHeight())
        return;

    needToCheckEmptiness = true;

    if (numPixels <= 0)
    {
        table[lineStrideElements * y] = 0;
        return;
    }

    auto* tempLine = static_cast<int*> (alloca ((size_t) (numPixels * 2 + 4) * sizeof (int)));
    int destIndex = 0, lastLevel = 0;

    while (--numPixels >= 0)
    {
        auto alpha = *mask;
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
    if (needToCheckEmptiness)
    {
        needToCheckEmptiness = false;
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

} // namespace juce
