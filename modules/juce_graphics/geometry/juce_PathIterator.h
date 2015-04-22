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

#ifndef JUCE_PATHITERATOR_H_INCLUDED
#define JUCE_PATHITERATOR_H_INCLUDED


//==============================================================================
/**
    Flattens a Path object into a series of straight-line sections.

    Use one of these to iterate through a Path object, and it will convert
    all the curves into line sections so it's easy to render or perform
    geometric operations on.

    @see Path
*/
class JUCE_API  PathFlatteningIterator
{
public:
    //==============================================================================
    /** Creates a PathFlatteningIterator.

        After creation, use the next() method to initialise the fields in the
        object with the first line's position.

        @param path         the path to iterate along
        @param transform    a transform to apply to each point in the path being iterated
        @param tolerance    the amount by which the curves are allowed to deviate from the lines
                            into which they are being broken down - a higher tolerance contains
                            less lines, so can be generated faster, but will be less smooth.
    */
    PathFlatteningIterator (const Path& path,
                            const AffineTransform& transform = AffineTransform::identity,
                            float tolerance = defaultTolerance);

    /** Destructor. */
    ~PathFlatteningIterator();

    //==============================================================================
    /** Fetches the next line segment from the path.

        This will update the member variables x1, y1, x2, y2, subPathIndex and closesSubPath
        so that they describe the new line segment.

        @returns false when there are no more lines to fetch.
    */
    bool next();

    float x1;  /**< The x position of the start of the current line segment. */
    float y1;  /**< The y position of the start of the current line segment. */
    float x2;  /**< The x position of the end of the current line segment. */
    float y2;  /**< The y position of the end of the current line segment. */

    /** Indicates whether the current line segment is closing a sub-path.

        If the current line is the one that connects the end of a sub-path
        back to the start again, this will be true.
    */
    bool closesSubPath;

    /** The index of the current line within the current sub-path.

        E.g. you can use this to see whether the line is the first one in the
        subpath by seeing if it's 0.
    */
    int subPathIndex;

    /** Returns true if the current segment is the last in the current sub-path. */
    bool isLastInSubpath() const noexcept;

    /** This is the default value that should be used for the tolerance value (see the constructor parameters). */
    static const float defaultTolerance;

private:
    //==============================================================================
    const Path& path;
    const AffineTransform transform;
    float* points;
    const float toleranceSquared;
    float subPathCloseX, subPathCloseY;
    const bool isIdentityTransform;

    HeapBlock <float> stackBase;
    float* stackPos;
    size_t index, stackSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PathFlatteningIterator)
};


#endif   // JUCE_PATHITERATOR_H_INCLUDED
