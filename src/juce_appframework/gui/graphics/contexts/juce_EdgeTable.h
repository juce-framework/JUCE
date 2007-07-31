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

#ifndef __JUCE_EDGETABLE_JUCEHEADER__
#define __JUCE_EDGETABLE_JUCEHEADER__

#include "../geometry/juce_AffineTransform.h"
#include "../../../../juce_core/containers/juce_MemoryBlock.h"
class Path;

static const int juce_edgeTableDefaultEdgesPerLine = 10;


//==============================================================================
/**
    A table of horizontal scan-line segments - used for rasterising Paths.

    @see Path, Graphics
*/
class JUCE_API  EdgeTable
{
public:
    //==============================================================================
    /** Indicates the quality at which the edge table should be generated.

        Higher values will have better quality anti-aliasing, but will take
        longer to generate the edge table and to render it.
    */
    enum OversamplingLevel
    {
        Oversampling_none       = 0,    /**< No vertical anti-aliasing at all. */
        Oversampling_4times     = 2,    /**< Anti-aliased with 4 levels of grey - good enough for normal use. */
        Oversampling_16times    = 4,    /**< Anti-aliased with 16 levels of grey - very good quality but slower. */
        Oversampling_256times   = 8     /**< Anti-aliased with 256 levels of grey - best quality, but too slow for
                                             normal user-interface use. */
    };

    /** Creates an empty edge table ready to have paths added.

        A table is created with a fixed vertical size, and only sections of paths
        which lie within their range will be added to the table.

        @param topY                     the lowest y co-ordinate that the table can contain
        @param height                   the number of horizontal lines it can contain
        @param verticalOversampling     the amount of oversampling used for anti-aliasing
        @param expectedEdgesPerLine     used to optimise the table's internal data usage - it's not
                                        worth changing this except for very special purposes
    */
    EdgeTable (const int topY,
               const int height,
               const OversamplingLevel verticalOversampling = Oversampling_4times,
               const int expectedEdgesPerLine = juce_edgeTableDefaultEdgesPerLine) throw();

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
                  const int clipLeft,
                  int clipTop,
                  const int clipRight,
                  int clipBottom,
                  const int subPixelXOffset) const
    {
        if (clipTop < top)
            clipTop = top;

        if (clipBottom > top + height)
            clipBottom = top + height;

        const int* singleLine = table + lineStrideElements
                                          * ((clipTop - top) << (int) oversampling);

        int mergedLineAllocation = 128;
        MemoryBlock temp (mergedLineAllocation * (2 * sizeof (int)));
        int* mergedLine = (int*) temp.getData();

        const int timesOverSampling = 1 << (int) oversampling;

        for (int y = clipTop; y < clipBottom; ++y)
        {
            int numMergedPoints = 0;

            // sort all the oversampled lines into a single merged line ready to draw..
            for (int over = timesOverSampling; --over >= 0;)
            {
                const int* l = singleLine;
                singleLine += lineStrideElements;

                int num = *l;
                jassert (num >= 0);

                if (num > 0)
                {
                    if (numMergedPoints + num >= mergedLineAllocation)
                    {
                        mergedLineAllocation = (numMergedPoints + num + 0x100) & ~0xff;
                        temp.setSize (mergedLineAllocation * (2 * sizeof (int)), false);
                        mergedLine = (int*) temp.getData();
                    }

                    while (--num >= 0)
                    {
                        const int x = *++l;
                        const int winding = *++l;

                        int n = numMergedPoints << 1;

                        while (n > 0)
                        {
                            const int cx = mergedLine [n - 2];

                            if (cx <= x)
                                break;

                            mergedLine [n] = cx;
                            --n;
                            mergedLine [n + 2] = mergedLine [n];
                            --n;
                        }

                        mergedLine [n] = x;
                        mergedLine [n + 1] = winding;

                        ++numMergedPoints;
                    }
                }
            }

            if (--numMergedPoints > 0)
            {
                const int* line = mergedLine;
                int x = subPixelXOffset + *line;
                int level = *++line;
                int levelAccumulator = 0;

                iterationCallback.setEdgeTableYPos (y);

                while (--numMergedPoints >= 0)
                {
                    const int endX = subPixelXOffset + *++line;
                    jassert (endX >= x);

                    const int absLevel = abs (level);
                    int endOfRun = (endX >> 8);

                    if (endOfRun == (x >> 8))
                    {
                        // small segment within the same pixel, so just save it for the next
                        // time round..
                        levelAccumulator += (endX - x) * absLevel;
                    }
                    else
                    {
                        // plot the fist pixel of this segment, including any accumulated
                        // levels from smaller segments that haven't been drawn yet
                        levelAccumulator += (0xff - (x & 0xff)) * absLevel;

                        levelAccumulator >>= 8;
                        if (levelAccumulator > 0xff)
                            levelAccumulator = 0xff;

                        x >>= 8;

                        if (x >= clipRight)
                        {
                            levelAccumulator = 0;
                            break;
                        }

                        if (x >= clipLeft && x < clipRight && levelAccumulator > 0)
                            iterationCallback.handleEdgeTablePixel (x, levelAccumulator);

                        if (++x >= clipRight)
                        {
                            levelAccumulator = 0;
                            break;
                        }

                        // if there's a segment of solid pixels, do it all in one go..
                        if (absLevel > 0 && endOfRun > x)
                        {
                            if (x < clipLeft)
                                x = clipLeft;

                            if (endOfRun > clipRight)
                                endOfRun = clipRight;

                            const int numPix = endOfRun - x;

                            if (numPix > 0)
                                iterationCallback.handleEdgeTableLine (x, numPix,
                                                                       jmin (absLevel, 0xff));
                        }

                        // save the bit at the end to be drawn next time round the loop.
                        levelAccumulator = (endX & 0xff) * absLevel;
                    }

                    level += *++line;
                    x = endX;
                }

                if (levelAccumulator > 0)
                {
                    levelAccumulator >>= 8;
                    if (levelAccumulator > 0xff)
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
    OversamplingLevel oversampling;

    // this will assume that the y co-ord is within bounds, and will avoid checking
    // this for speed.
    void addEdgePoint (const int x, const int y, const int winding) throw();

    void remapTableForNumEdges (const int newNumEdgesPerLine) throw();
};


#endif   // __JUCE_EDGETABLE_JUCEHEADER__
