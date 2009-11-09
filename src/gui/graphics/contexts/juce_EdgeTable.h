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

#ifndef __JUCE_EDGETABLE_JUCEHEADER__
#define __JUCE_EDGETABLE_JUCEHEADER__

#include "../geometry/juce_AffineTransform.h"
#include "../../../containers/juce_MemoryBlock.h"
class Path;
class Image;
class Rectangle;

//==============================================================================
/**
    A table of horizontal scan-line segments - used for rasterising Paths.

    @see Path, Graphics
*/
class JUCE_API  EdgeTable
{
public:
    //==============================================================================
    /** Creates an empty edge table ready to have paths added.

        A table is created with a fixed vertical size, and only sections of paths
        which lie within their range will be added to the table.

        @param topY                     the lowest y co-ordinate that the table can contain
        @param height                   the number of horizontal lines it can contain
    */
    EdgeTable (const int topY, const int height) throw();

    /** Creates a copy of another edge table. */
    EdgeTable (const EdgeTable& other) throw();

    /** Copies from another edge table. */
    const EdgeTable& operator= (const EdgeTable& other) throw();

    /** Destructor. */
    ~EdgeTable() throw();

    //==============================================================================
    /** Adds edges to the table for a path.

        This will add horizontal lines to the edge table for any parts of the path
        which lie within the vertical bounds for which this table was created.

        @param path         the path to add
        @param transform    an optional transform to apply to the path while it's
                            being added
    */
    void addPath (const Path& path,
                  const AffineTransform& transform) throw();

    /*void clipToRectangle (const Rectangle& r) throw();
    void intersectWith (const EdgeTable& other);
    void generateFromImageAlpha (Image& image, int x, int y) throw();*/

    /** Reduces the amount of space the table has allocated.

        This will shrink the table down to use as little memory as possible - useful for
        read-only tables that get stored and re-used for rendering.
    */
    void optimiseTable() throw();


    //==============================================================================
    /** Iterates the lines in the table, for rendering.

        This function will iterate each line in the table, and call a user-defined class
        to render each pixel or continuous line of pixels that the table contains.

        @param iterationCallback    this templated class must contain the following methods:
                                        @code
                                        inline void setEdgeTableYPos (int y);
                                        inline void handleEdgeTablePixel (int x, int alphaLevel) const;
                                        inline void handleEdgeTableLine (int x, int width, int alphaLevel) const;
                                        @endcode
                                        (these don't necessarily have to be 'const', but it might help it go faster)
        @param clipLeft             the left-hand edge of the rectangle which should be iterated
        @param clipTop              the top edge of the rectangle which should be iterated
        @param clipRight            the right-hand edge of the rectangle which should be iterated
        @param clipBottom           the bottom edge of the rectangle which should be iterated
        @param subPixelXOffset      a fraction of 1 pixel by which to shift the table rightwards, in the range 0 to 255
    */
    template <class EdgeTableIterationCallback>
    void iterate (EdgeTableIterationCallback& iterationCallback,
                  const int clipLeft, int clipTop,
                  const int clipRight, int clipBottom,
                  const int subPixelXOffset) const throw()
    {
        if (clipTop < top)
            clipTop = top;

        if (clipBottom > top + height)
            clipBottom = top + height;

        const int* lineStart = table + lineStrideElements * (clipTop - top);

        for (int y = clipTop; y < clipBottom; ++y)
        {
            const int* line = lineStart;
            lineStart += lineStrideElements;
            int numPoints = line[0];

            if (--numPoints > 0)
            {
                int x = subPixelXOffset + *++line;
                int level = *++line;
                int levelAccumulator = 0;

                iterationCallback.setEdgeTableYPos (y);

                while (--numPoints >= 0)
                {
                    int correctedLevel = abs (level);
                    if (correctedLevel >> 8)
                    {
                        if (nonZeroWinding)
                        {
                            correctedLevel = 0xff;
                        }
                        else
                        {
                            correctedLevel &= 511;
                            if (correctedLevel >> 8)
                                correctedLevel = 511 - correctedLevel;
                        }
                    }

                    const int endX = subPixelXOffset + *++line;
                    jassert (endX >= x);
                    int endOfRun = (endX >> 8);

                    if (endOfRun == (x >> 8))
                    {
                        // small segment within the same pixel, so just save it for the next
                        // time round..
                        levelAccumulator += (endX - x) * correctedLevel;
                    }
                    else
                    {
                        // plot the fist pixel of this segment, including any accumulated
                        // levels from smaller segments that haven't been drawn yet
                        levelAccumulator += (0xff - (x & 0xff)) * correctedLevel;

                        x >>= 8;
                        if (x >= clipRight)
                        {
                            levelAccumulator = 0;
                            break;
                        }

                        if (x >= clipLeft)
                        {
                            levelAccumulator >>= 8;
                            if (levelAccumulator > 0)
                                iterationCallback.handleEdgeTablePixel (x, jmin (0xff, levelAccumulator));
                        }

                        if (++x >= clipRight)
                        {
                            levelAccumulator = 0;
                            break;
                        }

                        // if there's a segment of solid pixels, do it all in one go..
                        if (correctedLevel > 0 && endOfRun > x)
                        {
                            if (x < clipLeft)
                                x = clipLeft;

                            if (endOfRun > clipRight)
                                endOfRun = clipRight;

                            const int numPix = endOfRun - x;

                            if (numPix > 0)
                                iterationCallback.handleEdgeTableLine (x, numPix,
                                                                       jmin (correctedLevel, 0xff));
                        }

                        // save the bit at the end to be drawn next time round the loop.
                        levelAccumulator = (endX & 0xff) * correctedLevel;
                    }

                    level += *++line;
                    x = endX;
                }

                if (levelAccumulator > 0)
                {
                    levelAccumulator >>= 8;
                    if (levelAccumulator >> 8)
                        levelAccumulator = 0xff;

                    x >>= 8;
                    if (x >= clipLeft && x < clipRight)
                        iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                }
            }
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    // table line format: number of points; point0 x, point0 levelDelta, point1 x, point1 levelDelta, etc
    int* table;
    int top, height, maxEdgesPerLine, lineStrideElements;
    bool nonZeroWinding;

    void addEdgePoint (const int x, const int y, const int winding) throw();
    void remapTableForNumEdges (const int newNumEdgesPerLine) throw();
};


#endif   // __JUCE_EDGETABLE_JUCEHEADER__
