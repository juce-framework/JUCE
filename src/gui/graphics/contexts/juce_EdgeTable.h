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
#include "../geometry/juce_Rectangle.h"
#include "../../../containers/juce_MemoryBlock.h"
class Path;
class Image;


//==============================================================================
/**
    A table of horizontal scan-line segments - used for rasterising Paths.

    @see Path, Graphics
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
    EdgeTable (const Rectangle& clipLimits,
               const Path& pathToAdd,
               const AffineTransform& transform) throw();

    /** Creates an edge table containing a rectangle.
    */
    EdgeTable (const Rectangle& rectangleToAdd) throw();

    /** Creates a copy of another edge table. */
    EdgeTable (const EdgeTable& other) throw();

    /** Copies from another edge table. */
    const EdgeTable& operator= (const EdgeTable& other) throw();

    /** Destructor. */
    ~EdgeTable() throw();

    //==============================================================================
    /*void clipToRectangle (const Rectangle& r) throw();
    void excludeRectangle (const Rectangle& r) throw();
    void clipToEdgeTable (const EdgeTable& other);
    void clipToImageAlpha (Image& image, int x, int y) throw();*/

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
    */
    template <class EdgeTableIterationCallback>
    void iterate (EdgeTableIterationCallback& iterationCallback) const throw()
    {
        const int* lineStart = table;

        for (int y = 0; y < bounds.getHeight(); ++y)
        {
            const int* line = lineStart;
            lineStart += lineStrideElements;
            int numPoints = line[0];

            if (--numPoints > 0)
            {
                int x = *++line;
                jassert ((x >> 8) >= bounds.getX() && (x >> 8) < bounds.getRight());
                int level = *++line;
                int levelAccumulator = 0;

                iterationCallback.setEdgeTableYPos (y);

                while (--numPoints >= 0)
                {
                    int correctedLevel = abs (level);
                    if (correctedLevel >> 8)
                        correctedLevel = 0xff;

                    const int endX = *++line;
                    jassert (endX >= x);
                    const int endOfRun = (endX >> 8);

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
                        levelAccumulator >>= 8;
                        x >>= 8;

                        if (levelAccumulator > 0)
                        {
                            if (levelAccumulator >> 8)
                                levelAccumulator = 0xff;

                            iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                        }

                        // if there's a segment of solid pixels, do it all in one go..
                        if (correctedLevel > 0)
                        {
                            jassert (endOfRun <= bounds.getRight());
                            const int numPix = endOfRun - ++x;

                            if (numPix > 0)
                                iterationCallback.handleEdgeTableLine (x, numPix, correctedLevel);
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
                    jassert (x >= bounds.getX() && x < bounds.getRight());
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
    Rectangle bounds;
    int maxEdgesPerLine, lineStrideElements;

    void addEdgePoint (const int x, const int y, const int winding) throw();
    void remapTableForNumEdges (const int newNumEdgesPerLine) throw();
    void clearLineSection (const int y, int minX, int maxX) throw();
    void intersectWithEdgeTableLine (const int y, const int* otherLine) throw();
};


#endif   // __JUCE_EDGETABLE_JUCEHEADER__
