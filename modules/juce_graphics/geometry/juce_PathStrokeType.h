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
    Describes a type of stroke used to render a solid outline along a path.

    A PathStrokeType object can be used directly to create the shape of an outline
    around a path, and is used by Graphics::strokePath to specify the type of
    stroke to draw.

    @see Path, Graphics::strokePath

    @tags{Graphics}
*/
class JUCE_API  PathStrokeType
{
public:
    //==============================================================================
    /** The type of shape to use for the corners between two adjacent line segments. */
    enum JointStyle
    {
        mitered,    /**< Indicates that corners should be drawn with sharp joints.
                         Note that for angles that curve back on themselves, drawing a
                         mitre could require extending the point too far away from the
                         path, so a mitre limit is imposed and any corners that exceed it
                         are drawn as bevelled instead. */
        curved,     /**< Indicates that corners should be drawn as rounded-off. */
        beveled     /**< Indicates that corners should be drawn with a line flattening their
                         outside edge. */
    };

    /** The type shape to use for the ends of lines. */
    enum EndCapStyle
    {
        butt,       /**< Ends of lines are flat and don't extend beyond the end point. */
        square,     /**< Ends of lines are flat, but stick out beyond the end point for half
                         the thickness of the stroke. */
        rounded     /**< Ends of lines are rounded-off with a circular shape. */
    };

    //==============================================================================
    /** Creates a stroke type with a given line-width, and default joint/end styles. */
    explicit PathStrokeType (float strokeThickness) noexcept;

    /** Creates a stroke type.

        @param strokeThickness      the width of the line to use
        @param jointStyle           the type of joints to use for corners
        @param endStyle             the type of end-caps to use for the ends of open paths.
    */
    PathStrokeType (float strokeThickness,
                    JointStyle jointStyle,
                    EndCapStyle endStyle = butt) noexcept;

    /** Creates a copy of another stroke type. */
    PathStrokeType (const PathStrokeType&) noexcept;

    /** Copies another stroke onto this one. */
    PathStrokeType& operator= (const PathStrokeType&) noexcept;

    /** Destructor. */
    ~PathStrokeType() noexcept;

    //==============================================================================
    /** Applies this stroke type to a path and returns the resultant stroke as another Path.

        @param destPath         the resultant stroked outline shape will be copied into this path.
                                Note that it's ok for the source and destination Paths to be
                                the same object, so you can easily turn a path into a stroked version
                                of itself.
        @param sourcePath       the path to use as the source
        @param transform        an optional transform to apply to the points from the source path
                                as they are being used
        @param extraAccuracy    if this is greater than 1.0, it will subdivide the path to
                                a higher resolution, which improves the quality if you'll later want
                                to enlarge the stroked path. So for example, if you're planning on drawing
                                the stroke at 3x the size that you're creating it, you should set this to 3.

        @see createDashedStroke
    */
    void createStrokedPath (Path& destPath,
                            const Path& sourcePath,
                            const AffineTransform& transform = AffineTransform(),
                            float extraAccuracy = 1.0f) const;


    //==============================================================================
    /** Applies this stroke type to a path, creating a dashed line.

        This is similar to createStrokedPath, but uses the array passed in to
        break the stroke up into a series of dashes.

        @param destPath         the resultant stroked outline shape will be copied into this path.
                                Note that it's ok for the source and destination Paths to be
                                the same object, so you can easily turn a path into a stroked version
                                of itself.
        @param sourcePath       the path to use as the source
        @param dashLengths      An array of alternating on/off lengths. E.g. { 2, 3, 4, 5 } will create
                                a line of length 2, then skip a length of 3, then add a line of length 4,
                                skip 5, and keep repeating this pattern.
        @param numDashLengths   The number of lengths in the dashLengths array. This should really be
                                an even number, otherwise the pattern will get out of step as it
                                repeats.
        @param transform        an optional transform to apply to the points from the source path
                                as they are being used
        @param extraAccuracy    if this is greater than 1.0, it will subdivide the path to
                                a higher resolution, which improves the quality if you'll later want
                                to enlarge the stroked path. So for example, if you're planning on drawing
                                the stroke at 3x the size that you're creating it, you should set this to 3.
    */
    void createDashedStroke (Path& destPath,
                             const Path& sourcePath,
                             const float* dashLengths,
                             int numDashLengths,
                             const AffineTransform& transform = AffineTransform(),
                             float extraAccuracy = 1.0f) const;

    //==============================================================================
    /** Applies this stroke type to a path and returns the resultant stroke as another Path.

        @param destPath             the resultant stroked outline shape will be copied into this path.
                                    Note that it's ok for the source and destination Paths to be
                                    the same object, so you can easily turn a path into a stroked version
                                    of itself.
        @param sourcePath           the path to use as the source
        @param arrowheadStartWidth  the width of the arrowhead at the start of the path
        @param arrowheadStartLength the length of the arrowhead at the start of the path
        @param arrowheadEndWidth    the width of the arrowhead at the end of the path
        @param arrowheadEndLength   the length of the arrowhead at the end of the path
        @param transform            an optional transform to apply to the points from the source path
                                    as they are being used
        @param extraAccuracy        if this is greater than 1.0, it will subdivide the path to
                                    a higher resolution, which improves the quality if you'll later want
                                    to enlarge the stroked path. So for example, if you're planning on drawing
                                    the stroke at 3x the size that you're creating it, you should set this to 3.
        @see createDashedStroke
    */
    void createStrokeWithArrowheads (Path& destPath,
                                     const Path& sourcePath,
                                     float arrowheadStartWidth, float arrowheadStartLength,
                                     float arrowheadEndWidth, float arrowheadEndLength,
                                     const AffineTransform& transform = AffineTransform(),
                                     float extraAccuracy = 1.0f) const;

    //==============================================================================
    /** Returns the stroke thickness. */
    float getStrokeThickness() const noexcept                   { return thickness; }

    /** Sets the stroke thickness. */
    void setStrokeThickness (float newThickness) noexcept       { thickness = newThickness; }

    /** Returns the joint style. */
    JointStyle getJointStyle() const noexcept                   { return jointStyle; }

    /** Sets the joint style. */
    void setJointStyle (JointStyle newStyle) noexcept           { jointStyle = newStyle; }

    /** Returns the end-cap style. */
    EndCapStyle getEndStyle() const noexcept                    { return endStyle; }

    /** Sets the end-cap style. */
    void setEndStyle (EndCapStyle newStyle) noexcept            { endStyle = newStyle; }

    //==============================================================================
    /** Compares the stroke thickness, joint and end styles of two stroke types. */
    bool operator== (const PathStrokeType&) const noexcept;

    /** Compares the stroke thickness, joint and end styles of two stroke types. */
    bool operator!= (const PathStrokeType&) const noexcept;

private:
    //==============================================================================
    float thickness;
    JointStyle jointStyle;
    EndCapStyle endStyle;

    JUCE_LEAK_DETECTOR (PathStrokeType)
};

} // namespace juce
