/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6255 6263 6386)

EdgeTable::EdgeTable (Rectangle<int> area, const Path& path, const AffineTransform& transform)
   : bounds (area),
     // this is a very vague heuristic to make a rough guess at a good table size
     // for a given path, such that it's big enough to mostly avoid remapping, but also
     // not so big that it's wasteful for simple paths.
     maxEdgesPerLine (jmax (defaultEdgesPerLine / 2,
                            4 * (int) std::sqrt (path.data.size()))),
     lineStrideElements (maxEdgesPerLine * 2 + 1)
{
    allocate();
    int* t = table.data();

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }

    auto leftLimit   = scale * static_cast<int64_t> (bounds.getX());
    auto topLimit    = scale * static_cast<int64_t> (bounds.getY());
    auto rightLimit  = scale * static_cast<int64_t> (bounds.getRight());
    auto heightLimit = scale * static_cast<int64_t> (bounds.getHeight());

    PathFlatteningIterator iter (path, transform);

    while (iter.next())
    {
        const auto scaleIterY = [] (auto y)
        {
            return static_cast<int64_t> (y * 256.0f + (y >= 0 ? 0.5f : -0.5f));
        };

        auto y1 = scaleIterY (iter.y1);
        auto y2 = scaleIterY (iter.y2);

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
                auto stepSize = static_cast<int64_t> (jlimit (1, 256, 256 / (1 + (int) std::abs (multiplier))));

                do
                {
                    auto step = jmin (stepSize, y2 - y1, 256 - (y1 & 255));
                    auto x = static_cast<int64_t> (startX + multiplier * static_cast<double> ((y1 + (step >> 1)) - startY));
                    auto clampedX = static_cast<int> (jlimit (leftLimit, rightLimit - 1, x));

                    addEdgePoint (clampedX, static_cast<int> (y1 / scale), static_cast<int> (direction * step));
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
     maxEdgesPerLine (defaultEdgesPerLine),
     lineStrideElements (defaultEdgesPerLine * 2 + 1)
{
    allocate();
    table[0] = 0;

    auto x1 = scale * rectangleToAdd.getX();
    auto x2 = scale * rectangleToAdd.getRight();
    int* t = table.data();

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
     maxEdgesPerLine (defaultEdgesPerLine),
     lineStrideElements (defaultEdgesPerLine * 2 + 1),
     needToCheckEmptiness (true)
{
    allocate();
    clearLineSizes();

    for (auto& r : rectanglesToAdd)
    {
        auto x1 = scale * r.getX();
        auto x2 = scale * r.getRight();
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
        auto x1 = roundToInt ((float) scale * r.getX());
        auto x2 = roundToInt ((float) scale * r.getRight());

        auto y1 = roundToInt ((float) scale * r.getY())      - (bounds.getY() * scale);
        auto y2 = roundToInt ((float) scale * r.getBottom()) - (bounds.getY() * scale);

        if (x2 <= x1 || y2 <= y1)
            continue;

        auto y        = y1 / scale;
        auto lastLine = y2 / scale;

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
             roundToInt (rectangleToAdd.getY() * 256.0f) / scale,
             2 + (int) rectangleToAdd.getWidth(),
             2 + (int) rectangleToAdd.getHeight()),
     maxEdgesPerLine (defaultEdgesPerLine),
     lineStrideElements ((defaultEdgesPerLine * 2) + 1)
{
    jassert (! rectangleToAdd.isEmpty());
    allocate();
    table[0] = 0;

    auto x1 = roundToInt ((float) scale * rectangleToAdd.getX());
    auto x2 = roundToInt ((float) scale * rectangleToAdd.getRight());
    auto y1 = roundToInt ((float) scale * rectangleToAdd.getY())      - (bounds.getY() * scale);
    auto y2 = roundToInt ((float) scale * rectangleToAdd.getBottom()) - (bounds.getY() * scale);
    jassert (y1 < 256);

    if (x2 <= x1 || y2 <= y1)
    {
        bounds.setHeight (0);
        return;
    }

    int lineY = 0;
    int* t = table.data();

    if ((y1 / scale) == (y2 / scale))
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

        while (lineY < (y2 / scale))
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

static void copyEdgeTableData (int* dest,
                               size_t destLineStride,
                               const int* src,
                               size_t srcLineStride,
                               size_t numLines) noexcept
{
    for (size_t line = 0; line < numLines; ++line)
    {
        const auto* srcLine = src + line * srcLineStride;
        std::copy (srcLine, srcLine + *srcLine * 2 + 1, dest + line * destLineStride);
    }
}

//==============================================================================
static size_t getEdgeTableAllocationSize (int lineStride, int height) noexcept
{
    // (leave an extra line at the end for use as scratch space)
    return (size_t) (lineStride * (2 + jmax (0, height)));
}

void EdgeTable::allocate()
{
    table = CopyableHeapBlock<int> (getEdgeTableAllocationSize (lineStrideElements, bounds.getHeight()));
}

void EdgeTable::clearLineSizes() noexcept
{
    int* t = table.data();

    for (int i = bounds.getHeight(); --i >= 0;)
    {
        *t = 0;
        t += lineStrideElements;
    }
}

void EdgeTable::sanitiseLevels (const bool useNonZeroWinding) noexcept
{
    // Convert the table from relative windings to absolute levels..
    int* lineStart = table.data();

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

                if (corrected / scale)
                {
                    if (useNonZeroWinding)
                    {
                        corrected = 255;
                    }
                    else
                    {
                        corrected &= 511;

                        if (corrected / scale)
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
        jassert (newNumEdgesPerLine > maxEdgesPerLine);
        maxEdgesPerLine = newNumEdgesPerLine;

        jassert (bounds.getHeight() > 0);
        auto newLineStrideElements = maxEdgesPerLine * 2 + 1;

        CopyableHeapBlock<int> newTable (getEdgeTableAllocationSize (newLineStrideElements, bounds.getHeight()));
        copyEdgeTableData (newTable.data(),
                           (size_t) newLineStrideElements,
                           table.data(),
                           (size_t) lineStrideElements,
                           (size_t) bounds.getHeight());

        table = std::move (newTable);
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
        maxLineElements = jmax (maxLineElements, table[(size_t) i * (size_t) lineStrideElements]);

    remapTableForNumEdges (maxLineElements);
}

void EdgeTable::addEdgePoint (const int x, const int y, const int winding)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* line = table.data() + lineStrideElements * y;
    auto numPoints = line[0];

    if (numPoints >= maxEdgesPerLine)
    {
        remapWithExtraSpace (numPoints);
        line = table.data() + lineStrideElements * y;
    }

    line[0] = numPoints + 1;
    line += numPoints * 2;
    line[1] = x;
    line[2] = winding;
}

void EdgeTable::addEdgePointPair (int x1, int x2, int y, int winding)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* line = table.data() + lineStrideElements * y;
    auto numPoints = line[0];

    if (numPoints + 1 >= maxEdgesPerLine)
    {
        remapWithExtraSpace (numPoints + 1);
        line = table.data() + lineStrideElements * y;
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

    int* lineStart = table.data();
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
    int* lineStart = table.data();
    auto multiplier = (int) (amount * 256.0f);

    for (int y = 0; y < bounds.getHeight(); ++y)
    {
        auto numPoints = lineStart[0];

        for (auto i = 0; i < numPoints; ++i)
        {
            auto* ptr = lineStart + 1 + (2 * i);
            auto item = readUnaligned<LineItem> (ptr);
            item.level = jmin (255, (item.level * multiplier) / scale);
            writeUnaligned (ptr, item);
        }
    }
}

void EdgeTable::intersectWithEdgeTableLine (const int y, const int* const otherLine)
{
    jassert (y >= 0 && y < bounds.getHeight());

    auto* srcLine = table.data() + lineStrideElements * y;
    const auto srcNum1 = *srcLine;

    if (srcNum1 == 0)
        return;

    const auto srcNum2 = *otherLine;

    if (srcNum2 == 0)
    {
        *srcLine = 0;
        return;
    }

    Span srcLine1 { srcLine   + 1, (size_t) srcNum1 * 2 };
    Span srcLine2 { otherLine + 1, (size_t) srcNum2 * 2 };

    const auto popHead = [] (auto& s)
    {
        if (s.empty())
            return 0;

        const auto result = s.front();
        s = Span { s.data() + 1, s.size() - 1 };
        return result;
    };

    const auto reseat = [] (auto& s, auto* ptr)
    {
        s = Span { ptr, s.size() };
    };

    const auto right = bounds.getRight() * scale;

    // optimise for the common case where our line lies entirely within a
    // single pair of points, as happens when clipping to a simple rect.
    if (srcLine2.size() == 4 && srcLine2[1] >= 255)
    {
        clipEdgeTableLineToRange (srcLine, srcLine2[0], jmin (right, srcLine2[2]));
        return;
    }

    bool isUsingTempSpace = false;

    auto x1 = popHead (srcLine1);
    auto x2 = popHead (srcLine2);

    int destIndex = 0, destTotal = 0;
    int level1 = 0, level2 = 0;
    int lastLevel = 0;

    while (! srcLine1.empty() && ! srcLine2.empty())
    {
        int nextX;

        if (x1 <= x2)
        {
            if (x1 == x2)
            {
                level2 = popHead (srcLine2);
                x2 = popHead (srcLine2);
            }

            nextX = x1;
            level1 = popHead (srcLine1);
            x1 = popHead (srcLine1);
        }
        else
        {
            nextX = x2;
            level2 = popHead (srcLine2);
            x2 = popHead (srcLine2);
        }

        if (right <= nextX)
            break;

        const auto nextLevel = (level1 * (level2 + 1)) / scale;

        if (std::exchange (lastLevel, nextLevel) != nextLevel)
        {
            jassert (isPositiveAndBelow (nextLevel, 256));

            if (destTotal >= maxEdgesPerLine)
            {
                srcLine[0] = destTotal;

                if (isUsingTempSpace)
                {
                    auto* stackBuffer = static_cast<int*> (alloca (sizeof (int) * srcLine1.size()));
                    std::copy (srcLine1.begin(), srcLine1.end(), stackBuffer);

                    remapTableForNumEdges (jmax (256, destTotal * 2));
                    srcLine = table.data() + lineStrideElements * y;

                    reseat (srcLine1, table.data() + lineStrideElements * bounds.getHeight());
                    std::copy (stackBuffer, stackBuffer + srcLine1.size(), srcLine1.data());
                }
                else
                {
                    remapTableForNumEdges (jmax (256, destTotal * 2));
                    srcLine = table.data() + lineStrideElements * y;
                }
            }

            ++destTotal;

            if (! isUsingTempSpace)
            {
                isUsingTempSpace = true;
                auto* temp = table.data() + lineStrideElements * bounds.getHeight();
                std::copy (srcLine1.begin(), srcLine1.end(), temp);
                reseat (srcLine1, temp);
            }

            srcLine[++destIndex] = nextX;
            srcLine[++destIndex] = nextLevel;
        }
    }

    if (lastLevel > 0)
    {
        if (destTotal >= maxEdgesPerLine)
        {
            srcLine[0] = destTotal;
            remapTableForNumEdges (jmax (256, destTotal * 2));
            srcLine = table.data() + lineStrideElements * y;
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
            table[(size_t) lineStrideElements * (size_t) i] = 0;

        if (clipped.getX() > bounds.getX() || clipped.getRight() < bounds.getRight())
        {
            auto x1 = scale * clipped.getX();
            auto x2 = scale * jmin (bounds.getRight(), clipped.getRight());
            int* line = table.data() + lineStrideElements * top;

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
                                 scale * clipped.getX(), 0,
                                 scale * clipped.getRight(), 255,
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
            table[(size_t) lineStrideElements * (size_t) i] = 0;

        auto* otherLine = other.table.data() + other.lineStrideElements * (clipped.getY() - other.bounds.getY());

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
        table[(size_t) lineStrideElements * (size_t) y] = 0;
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
            tempLine[++destIndex] = (x * scale);
            tempLine[++destIndex] = alpha;
            lastLevel = alpha;
        }

        ++x;
    }

    if (lastLevel > 0)
    {
        tempLine[++destIndex] = (x * scale);
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
        int* t = table.data();

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

JUCE_END_IGNORE_WARNINGS_MSVC

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class EdgeTableTests : public UnitTest
{
public:
    EdgeTableTests() : UnitTest ("EdgeTable", UnitTestCategories::graphics) {}

    void runTest() override
    {
        beginTest ("The result of clipToEdgeTable() shouldn't contain any point that's not present in both operands");
        {
            // The way this EdgeTable is constructed is significant in triggering a certain corner
            // case.
            const auto edgeTableContainingAPath = [&]
            {
                RectangleList<int> rl;
                rl.add (Rectangle<int> (6, 1, 1, 4));
                rl.add (Rectangle<int> (1, 1, 5, 5));
                EdgeTable rectListEdgeTable { rl };

                Path p;
                p.startNewSubPath (2.0f, 6.0f);
                p.lineTo (2.0f, 1.0f);
                p.lineTo (6.0f, 1.0f);
                p.lineTo (6.0f, 6.0f);
                p.closeSubPath();

                const EdgeTable pathEdgeTable { Rectangle<int> { 1, 1, 6, 5 }, p, {} };

                rectListEdgeTable.clipToEdgeTable (pathEdgeTable);
                return rectListEdgeTable;
            }();

            const EdgeTable edgeTableFromRectangle (Rectangle<float> (1.0f, 1.0f, 6.0f, 5.0f));

            const auto intersection = [&]
            {
                auto result = edgeTableFromRectangle;
                result.clipToEdgeTable (edgeTableContainingAPath);
                return result;
            }();

            expect (! contains (edgeTableContainingAPath, { 6, 2 }),
                    "The path doesn't enclose the point (6, 2) so it's EdgeTable shouldn't contain it");

            expect (contains (edgeTableFromRectangle, { 6, 2 }),
                    "The Rectangle covers the point (6, 2) so it's EdgeTable should contain it");

            expect (! contains (intersection, { 6, 2 }),
                    "The intersecting EdgeTable shouldn't contain (6, 2) because one of its constituents doesn't contain it either");
        }
    }

private:
    class EdgeTableFiller
    {
    public:
        EdgeTableFiller (int w, int h)
            : width (w), height (h), data ((size_t) (w * h))
        {
        }

        void setEdgeTableYPos (int yIn)
        {
            y = yIn;
        }

        void handleEdgeTablePixelFull (int x)
        {
            if (! (y < height && x < width))
                return;

            auto* ptr = data.data() + width * y + x;
            *ptr = 1;
        }

        void handleEdgeTablePixel (int x, int)
        {
            handleEdgeTablePixelFull (x);
        }

        void handleEdgeTableLineFull (int x, int w)
        {
            if (! (y < height && x < width))
                return;

            auto* ptr = data.data() + width * y + x;
            std::fill (ptr, ptr + std::min (w, width - x), 1);
        }

        void handleEdgeTableLine (int x, int w, int)
        {
            handleEdgeTableLineFull (x, w);
        }

        void handleEdgeTableRectangleFull (int x, int yIn, int w, int h) noexcept
        {
            for (int j = yIn; j < std::min (yIn + h, height); ++j)
            {
                auto* ptr = data.data() + width * j + x;
                std::fill (ptr, ptr + std::min (w, width - x), 1);
            }
        }

        void handleEdgeTableRectangle (int x, int yIn, int w, int h, int) noexcept
        {
            handleEdgeTableRectangleFull (x, yIn, w, h);
        }

        int get (int x, int yIn) const
        {
            const auto index = (size_t) (width * yIn + x);

            if (index >= data.size())
                return 0;

            return data[index];
        }

    private:
        const int width, height = 0;
        std::vector<int> data;
        int y = 0;
    };

    static bool contains (const EdgeTable& et, Point<int> p)
    {
        EdgeTableFiller filler { p.getX() + 2, p.getY() + 2 };
        et.iterate (filler);

        return filler.get (p.getX(), p.getY()) == 1;
    }
};

static EdgeTableTests edgeTableTests;

#endif

} // namespace juce
