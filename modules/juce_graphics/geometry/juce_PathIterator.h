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

//==============================================================================
/**
    Flattens a Path object into a series of straight-line sections.

    Use one of these to iterate through a Path object, and it will convert
    all the curves into line sections so it's easy to render or perform
    geometric operations on.

    @see Path

    @tags{Graphics}
*/
class JUCE_API  PathFlatteningIterator  final
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
                            const AffineTransform& transform = AffineTransform(),
                            float tolerance = Path::defaultToleranceForMeasurement);

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

private:
    //==============================================================================
    const Path& path;
    const AffineTransform transform;
    const float* source;
    const float toleranceSquared;
    float subPathCloseX = 0, subPathCloseY = 0;
    const bool isIdentityTransform;

    HeapBlock<float> stackBase { 32 };
    float* stackPos;
    size_t stackSize = 32;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PathFlatteningIterator)
};

} // namespace juce
