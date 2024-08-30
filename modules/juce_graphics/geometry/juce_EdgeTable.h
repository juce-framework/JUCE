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

//==============================================================================
/**
    A table of horizontal scan-line segments - used for rasterising Paths.

    @see Path, Graphics

    @tags{Graphics}
*/
class JUCE_API  EdgeTable
{
public:
    //==============================================================================
    /** Creates an edge table containing a path.

        A table is created with a fixed vertical range, and only sections of the path
        which lie within this range will be added to the table.

        @param clipLimits               only the region of the path that lies within this area will be added
        @param pathToAdd                the path to add to the table
        @param transform                a transform to apply to the path being added
    */
    EdgeTable (Rectangle<int> clipLimits,
               const Path& pathToAdd,
               const AffineTransform& transform);

    /** Creates an edge table containing a rectangle. */
    explicit EdgeTable (Rectangle<int> rectangleToAdd);

    /** Creates an edge table containing a rectangle. */
    explicit EdgeTable (Rectangle<float> rectangleToAdd);

    /** Creates an edge table containing a rectangle list. */
    explicit EdgeTable (const RectangleList<int>& rectanglesToAdd);

    /** Creates an edge table containing a rectangle list. */
    explicit EdgeTable (const RectangleList<float>& rectanglesToAdd);

    //==============================================================================
    void clipToRectangle (Rectangle<int> r);
    void excludeRectangle (Rectangle<int> r);
    void clipToEdgeTable (const EdgeTable&);
    void clipLineToMask (int x, int y, const uint8* mask, int maskStride, int numPixels);
    bool isEmpty() noexcept;
    const Rectangle<int>& getMaximumBounds() const noexcept      { return bounds; }
    void translate (float dx, int dy) noexcept;

    /** Scales all the alpha-levels in the table by the given multiplier. */
    void multiplyLevels (float factor);

    /** Reduces the amount of space the table has allocated.

        This will shrink the table down to use as little memory as possible - useful for
        read-only tables that get stored and re-used for rendering.
    */
    void optimiseTable();


    //==============================================================================
    /** Iterates the lines in the table, for rendering.

        This function will iterate each line in the table, and call a user-defined class
        to render each pixel or continuous line of pixels that the table contains.

        @param iterationCallback    this templated class must contain the following methods:
                                        @code
                                        inline void setEdgeTableYPos (int y);
                                        inline void handleEdgeTablePixel (int x, int alphaLevel) const;
                                        inline void handleEdgeTablePixelFull (int x) const;
                                        inline void handleEdgeTableLine (int x, int width, int alphaLevel) const;
                                        inline void handleEdgeTableLineFull (int x, int width) const;
                                        @endcode
                                        (these don't necessarily have to be 'const', but it might help it go faster)
    */
    template <class EdgeTableIterationCallback>
    void iterate (EdgeTableIterationCallback& iterationCallback) const noexcept
    {
        const int* lineStart = table.data();

        for (int y = 0; y < bounds.getHeight(); ++y)
        {
            const int* line = lineStart;
            lineStart += lineStrideElements;
            int numPoints = line[0];

            if (--numPoints > 0)
            {
                int x = *++line;
                jassert ((x / scale) >= bounds.getX() && (x / scale) < bounds.getRight());
                int levelAccumulator = 0;

                iterationCallback.setEdgeTableYPos (bounds.getY() + y);

                while (--numPoints >= 0)
                {
                    const int level = *++line;
                    jassert (isPositiveAndBelow (level, scale));
                    const int endX = *++line;
                    jassert (endX >= x);
                    const int endOfRun = (endX / scale);

                    if (endOfRun == (x / scale))
                    {
                        // small segment within the same pixel, so just save it for the next
                        // time round..
                        levelAccumulator += (endX - x) * level;
                    }
                    else
                    {
                        // plot the fist pixel of this segment, including any accumulated
                        // levels from smaller segments that haven't been drawn yet
                        levelAccumulator += (0x100 - (x & 0xff)) * level;
                        levelAccumulator /= scale;
                        x /= scale;

                        if (levelAccumulator > 0)
                        {
                            if (levelAccumulator >= 255)
                                iterationCallback.handleEdgeTablePixelFull (x);
                            else
                                iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                        }

                        // if there's a run of similar pixels, do it all in one go..
                        if (level > 0)
                        {
                            jassert (endOfRun <= bounds.getRight());
                            const int numPix = endOfRun - ++x;

                            if (numPix > 0)
                                iterationCallback.handleEdgeTableLine (x, numPix, level);
                        }

                        // save the bit at the end to be drawn next time round the loop.
                        levelAccumulator = (endX & 0xff) * level;
                    }

                    x = endX;
                }

                levelAccumulator /= scale;

                if (levelAccumulator > 0)
                {
                    x /= scale;
                    jassert (x >= bounds.getX() && x < bounds.getRight());

                    if (levelAccumulator >= 255)
                        iterationCallback.handleEdgeTablePixelFull (x);
                    else
                        iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                }
            }
        }
    }

private:
    //==============================================================================
    static constexpr auto defaultEdgesPerLine = 32;
    static constexpr auto scale = 256;

    //==============================================================================
    // table line format: number of points; point0 x, point0 levelDelta, point1 x, point1 levelDelta, etc
    struct LineItem
    {
        int x, level;

        bool operator< (const LineItem& other) const noexcept   { return x < other.x; }
    };

    CopyableHeapBlock<int> table;
    Rectangle<int> bounds;
    int maxEdgesPerLine, lineStrideElements;
    bool needToCheckEmptiness = true;

    void allocate();
    void clearLineSizes() noexcept;
    void addEdgePoint (int x, int y, int winding);
    void addEdgePointPair (int x1, int x2, int y, int winding);
    void remapTableForNumEdges (int newNumEdgesPerLine);
    void remapWithExtraSpace (int numPointsNeeded);
    void intersectWithEdgeTableLine (int y, const int* otherLine);
    void clipEdgeTableLineToRange (int* line, int x1, int x2) noexcept;
    void sanitiseLevels (bool useNonZeroWinding) noexcept;

    JUCE_LEAK_DETECTOR (EdgeTable)
};

} // namespace juce
