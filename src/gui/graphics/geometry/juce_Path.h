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

#ifndef __JUCE_PATH_JUCEHEADER__
#define __JUCE_PATH_JUCEHEADER__

#include "juce_AffineTransform.h"
#include "juce_Point.h"
#include "juce_Rectangle.h"
#include "../contexts/juce_Justification.h"
#include "../contexts/juce_EdgeTable.h"
#include "../../../containers/juce_Array.h"
#include "../../../io/streams/juce_InputStream.h"
#include "../../../io/streams/juce_OutputStream.h"
class Image;

//==============================================================================
/**
    A path is a sequence of lines and curves that may either form a closed shape
    or be open-ended.

    To use a path, you can create an empty one, then add lines and curves to it
    to create shapes, then it can be rendered by a Graphics context or used
    for geometric operations.

    e.g. @code
    Path myPath;

    myPath.startNewSubPath (10.0f, 10.0f);          // move the current position to (10, 10)
    myPath.lineTo (100.0f, 200.0f);                 // draw a line from here to (100, 200)
    myPath.quadraticTo (0.0f, 150.0f, 5.0f, 50.0f); // draw a curve that ends at (5, 50)
    myPath.closeSubPath();                          // close the subpath with a line back to (10, 10)

    // add an ellipse as well, which will form a second sub-path within the path..
    myPath.addEllipse (50.0f, 50.0f, 40.0f, 30.0f);

    // double the width of the whole thing..
    myPath.applyTransform (AffineTransform::scale (2.0f, 1.0f));

    // and draw it to a graphics context with a 5-pixel thick outline.
    g.strokePath (myPath, PathStrokeType (5.0f));

    @endcode

    A path object can actually contain multiple sub-paths, which may themselves
    be open or closed.

    @see PathFlatteningIterator, PathStrokeType, Graphics
*/
class JUCE_API  Path
{
public:
    //==============================================================================
    /** Creates an empty path. */
    Path() throw();

    /** Creates a copy of another path. */
    Path (const Path& other) throw();

    /** Destructor. */
    ~Path() throw();

    /** Copies this path from another one. */
    const Path& operator= (const Path& other) throw();

    //==============================================================================
    /** Returns true if the path doesn't contain any lines or curves. */
    bool isEmpty() const throw();

    /** Returns the smallest rectangle that contains all points within the path.
    */
    void getBounds (float& x, float& y,
                    float& w, float& h) const throw();

    /** Returns the smallest rectangle that contains all points within the path
        after it's been transformed with the given tranasform matrix.
    */
    void getBoundsTransformed (const AffineTransform& transform,
                               float& x, float& y,
                               float& w, float& h) const throw();

    /** Checks whether a point lies within the path.

        This is only relevent for closed paths (see closeSubPath()), and
        may produce false results if used on a path which has open sub-paths.

        The path's winding rule is taken into account by this method.

        The tolerence parameter is passed to the PathFlatteningIterator that
        is used to trace the path - for more info about it, see the notes for
        the PathFlatteningIterator constructor.

        @see closeSubPath, setUsingNonZeroWinding
    */
    bool contains (const float x,
                   const float y,
                   const float tolerence = 10.0f) const throw();

    /** Checks whether a line crosses the path.

        This will return positive if the line crosses any of the paths constituent
        lines or curves. It doesn't take into account whether the line is inside
        or outside the path, or whether the path is open or closed.

        The tolerence parameter is passed to the PathFlatteningIterator that
        is used to trace the path - for more info about it, see the notes for
        the PathFlatteningIterator constructor.
    */
    bool intersectsLine (const float x1, const float y1,
                         const float x2, const float y2,
                         const float tolerence = 10.0f) throw();

    //==============================================================================
    /** Removes all lines and curves, resetting the path completely. */
    void clear() throw();

    /** Begins a new subpath with a given starting position.

        This will move the path's current position to the co-ordinates passed in and
        make it ready to draw lines or curves starting from this position.

        After adding whatever lines and curves are needed, you can either
        close the current sub-path using closeSubPath() or call startNewSubPath()
        to move to a new sub-path, leaving the old one open-ended.

        @see lineTo, quadraticTo, cubicTo, closeSubPath
    */
    void startNewSubPath (const float startX,
                          const float startY) throw();

    /** Closes a the current sub-path with a line back to its start-point.

        When creating a closed shape such as a triangle, don't use 3 lineTo()
        calls - instead use two lineTo() calls, followed by a closeSubPath()
        to join the final point back to the start.

        This ensures that closes shapes are recognised as such, and this is
        important for tasks like drawing strokes, which needs to know whether to
        draw end-caps or not.

        @see startNewSubPath, lineTo, quadraticTo, cubicTo, closeSubPath
    */
    void closeSubPath() throw();

    /** Adds a line from the shape's last position to a new end-point.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a straight line.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, quadraticTo, cubicTo, closeSubPath
    */
    void lineTo (const float endX,
                 const float endY) throw();

    /** Adds a quadratic bezier curve from the shape's last position to a new position.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a quadratic spline with one control-point.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, lineTo, cubicTo, closeSubPath
    */
    void quadraticTo (const float controlPointX,
                      const float controlPointY,
                      const float endPointX,
                      const float endPointY) throw();

    /** Adds a cubic bezier curve from the shape's last position to a new position.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a cubic spline with two control-points.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, lineTo, quadraticTo, closeSubPath
    */
    void cubicTo (const float controlPoint1X,
                  const float controlPoint1Y,
                  const float controlPoint2X,
                  const float controlPoint2Y,
                  const float endPointX,
                  const float endPointY) throw();

    /** Returns the last point that was added to the path by one of the drawing methods.
    */
    const Point getCurrentPosition() const;

    //==============================================================================
    /** Adds a rectangle to the path.

        The rectangle is added as a new sub-path. (Any currently open paths will be
        left open).

        @see addRoundedRectangle, addTriangle
    */
    void addRectangle (const float x, const float y,
                       const float w, const float h) throw();

    /** Adds a rectangle to the path.

        The rectangle is added as a new sub-path. (Any currently open paths will be
        left open).

        @see addRoundedRectangle, addTriangle
    */
    void addRectangle (const Rectangle& rectangle) throw();

    /** Adds a rectangle with rounded corners to the path.

        The rectangle is added as a new sub-path. (Any currently open paths will be
        left open).

        @see addRectangle, addTriangle
    */
    void addRoundedRectangle (const float x, const float y,
                              const float w, const float h,
                              float cornerSize) throw();

    /** Adds a rectangle with rounded corners to the path.

        The rectangle is added as a new sub-path. (Any currently open paths will be
        left open).

        @see addRectangle, addTriangle
    */
    void addRoundedRectangle (const float x, const float y,
                              const float w, const float h,
                              float cornerSizeX,
                              float cornerSizeY) throw();

    /** Adds a triangle to the path.

        The triangle is added as a new closed sub-path. (Any currently open paths will be
        left open).

        Note that whether the vertices are specified in clockwise or anticlockwise
        order will affect how the triangle is filled when it overlaps other
        shapes (the winding order setting will affect this of course).
    */
    void addTriangle (const float x1, const float y1,
                      const float x2, const float y2,
                      const float x3, const float y3) throw();

    /** Adds a quadrilateral to the path.

        The quad is added as a new closed sub-path. (Any currently open paths will be
        left open).

        Note that whether the vertices are specified in clockwise or anticlockwise
        order will affect how the quad is filled when it overlaps other
        shapes (the winding order setting will affect this of course).
    */
    void addQuadrilateral (const float x1, const float y1,
                           const float x2, const float y2,
                           const float x3, const float y3,
                           const float x4, const float y4) throw();

    /** Adds an ellipse to the path.

        The shape is added as a new sub-path. (Any currently open paths will be
        left open).

        @see addArc
    */
    void addEllipse (const float x, const float y,
                     const float width, const float height) throw();

    /** Adds an elliptical arc to the current path.

        Note that when specifying the start and end angles, the curve will be drawn either clockwise
        or anti-clockwise according to whether the end angle is greater than the start. This means
        that sometimes you may need to use values greater than 2*Pi for the end angle.

        @param x            the left-hand edge of the rectangle in which the elliptical outline fits
        @param y            the top edge of the rectangle in which the elliptical outline fits
        @param width        the width of the rectangle in which the elliptical outline fits
        @param height       the height of the rectangle in which the elliptical outline fits
        @param fromRadians  the angle (clockwise) in radians at which to start the arc segment (where 0 is the
                            top-centre of the ellipse)
        @param toRadians    the angle (clockwise) in radians at which to end the arc segment (where 0 is the
                            top-centre of the ellipse). This angle can be greater than 2*Pi, so for example to
                            draw a curve clockwise from the 9 o'clock position to the 3 o'clock position via
                            12 o'clock, you'd use 1.5*Pi and 2.5*Pi as the start and finish points.
        @param startAsNewSubPath    if true, the arc will begin a new subpath from its starting point; if false,
                            it will be added to the current sub-path, continuing from the current postition

        @see addCentredArc, arcTo, addPieSegment, addEllipse
    */
    void addArc (const float x, const float y,
                 const float width, const float height,
                 const float fromRadians,
                 const float toRadians,
                 const bool startAsNewSubPath = false) throw();

    /** Adds an arc which is centred at a given point, and can have a rotation specified.

        Note that when specifying the start and end angles, the curve will be drawn either clockwise
        or anti-clockwise according to whether the end angle is greater than the start. This means
        that sometimes you may need to use values greater than 2*Pi for the end angle.

        @param centreX      the centre x of the ellipse
        @param centreY      the centre y of the ellipse
        @param radiusX      the horizontal radius of the ellipse
        @param radiusY      the vertical radius of the ellipse
        @param rotationOfEllipse    an angle by which the whole ellipse should be rotated about its centre, in radians (clockwise)
        @param fromRadians  the angle (clockwise) in radians at which to start the arc segment (where 0 is the
                            top-centre of the ellipse)
        @param toRadians    the angle (clockwise) in radians at which to end the arc segment (where 0 is the
                            top-centre of the ellipse). This angle can be greater than 2*Pi, so for example to
                            draw a curve clockwise from the 9 o'clock position to the 3 o'clock position via
                            12 o'clock, you'd use 1.5*Pi and 2.5*Pi as the start and finish points.
        @param startAsNewSubPath    if true, the arc will begin a new subpath from its starting point; if false,
                            it will be added to the current sub-path, continuing from the current postition

        @see addArc, arcTo
    */
    void addCentredArc (const float centreX, const float centreY,
                        const float radiusX, const float radiusY,
                        const float rotationOfEllipse,
                        const float fromRadians,
                        const float toRadians,
                        const bool startAsNewSubPath = false) throw();

    /** Adds a "pie-chart" shape to the path.

        The shape is added as a new sub-path. (Any currently open paths will be
        left open).

        Note that when specifying the start and end angles, the curve will be drawn either clockwise
        or anti-clockwise according to whether the end angle is greater than the start. This means
        that sometimes you may need to use values greater than 2*Pi for the end angle.

        @param x            the left-hand edge of the rectangle in which the elliptical outline fits
        @param y            the top edge of the rectangle in which the elliptical outline fits
        @param width        the width of the rectangle in which the elliptical outline fits
        @param height       the height of the rectangle in which the elliptical outline fits
        @param fromRadians  the angle (clockwise) in radians at which to start the arc segment (where 0 is the
                            top-centre of the ellipse)
        @param toRadians    the angle (clockwise) in radians at which to end the arc segment (where 0 is the
                            top-centre of the ellipse)
        @param innerCircleProportionalSize  if this is > 0, then the pie will be drawn as a curved band around a hollow
                            ellipse at its centre, where this value indicates the inner ellipse's size with
                            respect to the outer one.

        @see addArc
    */
    void addPieSegment (const float x, const float y,
                        const float width, const float height,
                        const float fromRadians,
                        const float toRadians,
                        const float innerCircleProportionalSize);

    /** Adds a line with a specified thickness.

        The line is added as a new closed sub-path. (Any currently open paths will be
        left open).

        @see addArrow
    */
    void addLineSegment (const float startX, const float startY,
                         const float endX, const float endY,
                         float lineThickness) throw();

    /** Adds a line with an arrowhead on the end.

        The arrow is added as a new closed sub-path. (Any currently open paths will be
        left open).
    */
    void addArrow (const float startX, const float startY,
                   const float endX, const float endY,
                   float lineThickness,
                   float arrowheadWidth,
                   float arrowheadLength) throw();

    /** Adds a star shape to the path.

    */
    void addStar (const float centreX,
                  const float centreY,
                  const int numberOfPoints,
                  const float innerRadius,
                  const float outerRadius,
                  const float startAngle = 0.0f);

    /** Adds a speech-bubble shape to the path.

        @param bodyX            the left of the main body area of the bubble
        @param bodyY            the top of the main body area of the bubble
        @param bodyW            the width of the main body area of the bubble
        @param bodyH            the height of the main body area of the bubble
        @param cornerSize       the amount by which to round off the corners of the main body rectangle
        @param arrowTipX        the x position that the tip of the arrow should connect to
        @param arrowTipY        the y position that the tip of the arrow should connect to
        @param whichSide        the side to connect the arrow to: 0 = top, 1 = left, 2 = bottom, 3 = right
        @param arrowPositionAlongEdgeProportional   how far along the edge of the main rectangle the
                                arrow's base should be - this is a proportional distance between 0 and 1.0
        @param arrowWidth       how wide the base of the arrow should be where it joins the main rectangle
    */
    void addBubble (float bodyX, float bodyY,
                    float bodyW, float bodyH,
                    float cornerSize,
                    float arrowTipX,
                    float arrowTipY,
                    int whichSide,
                    float arrowPositionAlongEdgeProportional,
                    float arrowWidth);

    /** Adds another path to this one.

        The new path is added as a new sub-path. (Any currently open paths in this
        path will be left open).

        @param pathToAppend     the path to add
    */
    void addPath (const Path& pathToAppend) throw();

    /** Adds another path to this one, transforming it on the way in.

        The new path is added as a new sub-path, its points being transformed by the given
        matrix before being added.

        @param pathToAppend     the path to add
        @param transformToApply an optional transform to apply to the incoming vertices
    */
    void addPath (const Path& pathToAppend,
                  const AffineTransform& transformToApply) throw();

    /** Swaps the contents of this path with another one.

        The internal data of the two paths is swapped over, so this is much faster than
        copying it to a temp variable and back.
    */
    void swapWithPath (Path& other);

    //==============================================================================
    /** Applies a 2D transform to all the vertices in the path.

        @see AffineTransform, scaleToFit, getTransformToScaleToFit
    */
    void applyTransform (const AffineTransform& transform) throw();

    /** Rescales this path to make it fit neatly into a given space.

        This is effectively a quick way of calling
        applyTransform (getTransformToScaleToFit (x, y, w, h, preserveProportions))

        @param x                    the x position of the rectangle to fit the path inside
        @param y                    the y position of the rectangle to fit the path inside
        @param width                the width of the rectangle to fit the path inside
        @param height               the height of the rectangle to fit the path inside
        @param preserveProportions  if true, it will fit the path into the space without altering its
                                    horizontal/vertical scale ratio; if false, it will distort the
                                    path to fill the specified ratio both horizontally and vertically

        @see applyTransform, getTransformToScaleToFit
    */
    void scaleToFit (const float x, const float y,
                     const float width, const float height,
                     const bool preserveProportions) throw();

    /** Returns a transform that can be used to rescale the path to fit into a given space.

        @param x                    the x position of the rectangle to fit the path inside
        @param y                    the y position of the rectangle to fit the path inside
        @param width                the width of the rectangle to fit the path inside
        @param height               the height of the rectangle to fit the path inside
        @param preserveProportions  if true, it will fit the path into the space without altering its
                                    horizontal/vertical scale ratio; if false, it will distort the
                                    path to fill the specified ratio both horizontally and vertically
        @param justificationType    if the proportions are preseved, the resultant path may be smaller
                                    than the available rectangle, so this describes how it should be
                                    positioned within the space.
        @returns                    an appropriate transformation

        @see applyTransform, scaleToFit

    */
    const AffineTransform getTransformToScaleToFit (const float x, const float y,
                                                    const float width, const float height,
                                                    const bool preserveProportions,
                                                    const Justification& justificationType = Justification::centred) const throw();

    /** Creates a version of this path where all sharp corners have been replaced by curves.

        Wherever two lines meet at an angle, this will replace the corner with a curve
        of the given radius.
    */
    const Path createPathWithRoundedCorners (const float cornerRadius) const throw();

    //==============================================================================
    /** Changes the winding-rule to be used when filling the path.

        If set to true (which is the default), then the path uses a non-zero-winding rule
        to determine which points are inside the path. If set to false, it uses an
        alternate-winding rule.

        The winding-rule comes into play when areas of the shape overlap other
        areas, and determines whether the overlapping regions are considered to be
        inside or outside.

        Changing this value just sets a flag - it doesn't affect the contents of the
        path.

        @see isUsingNonZeroWinding
    */
    void setUsingNonZeroWinding (const bool isNonZeroWinding) throw();

    /** Returns the flag that indicates whether the path should use a non-zero winding rule.

        The default for a new path is true.

        @see setUsingNonZeroWinding
    */
    bool isUsingNonZeroWinding() const                  { return useNonZeroWinding; }


    //==============================================================================
    /** Iterates the lines and curves that a path contains.

        @see Path, PathFlatteningIterator
    */
    class JUCE_API  Iterator
    {
    public:
        //==============================================================================
        Iterator (const Path& path);
        ~Iterator();

        //==============================================================================
        /** Moves onto the next element in the path.

            If this returns false, there are no more elements. If it returns true,
            the elementType variable will be set to the type of the current element,
            and some of the x and y variables will be filled in with values.
        */
        bool next();

        //==============================================================================
        enum PathElementType
        {
            startNewSubPath,    /**< For this type, x1 and y1 will be set to indicate the first point in the subpath.  */
            lineTo,             /**< For this type, x1 and y1 indicate the end point of the line.  */
            quadraticTo,        /**< For this type, x1, y1, x2, y2 indicate the control point and endpoint of a quadratic curve. */
            cubicTo,            /**< For this type, x1, y1, x2, y2, x3, y3 indicate the two control points and the endpoint of a cubic curve. */
            closePath           /**< Indicates that the sub-path is being closed. None of the x or y values are valid in this case. */
        };

        PathElementType elementType;

        float x1, y1, x2, y2, x3, y3;

        //==============================================================================
    private:
        const Path& path;
        int index;

        Iterator (const Iterator&);
        const Iterator& operator= (const Iterator&);
    };

    //==============================================================================
    /** Loads a stored path from a data stream.

        The data in the stream must have been written using writePathToStream().

        Note that this will append the stored path to whatever is currently in
        this path, so you might need to call clear() beforehand.

        @see loadPathFromData, writePathToStream
    */
    void loadPathFromStream (InputStream& source);

    /** Loads a stored path from a block of data.

        This is similar to loadPathFromStream(), but just reads from a block
        of data. Useful if you're including stored shapes in your code as a
        block of static data.

        @see loadPathFromStream, writePathToStream
    */
    void loadPathFromData (const unsigned char* const data,
                           const int numberOfBytes) throw();

    /** Stores the path by writing it out to a stream.

        After writing out a path, you can reload it using loadPathFromStream().

        @see loadPathFromStream, loadPathFromData
    */
    void writePathToStream (OutputStream& destination) const;

    //==============================================================================
    /** Creates a string containing a textual representation of this path.
        @see restoreFromString
    */
    const String toString() const;

    /** Restores this path from a string that was created with the toString() method.
        @see toString()
    */
    void restoreFromString (const String& stringVersion);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class PathFlatteningIterator;
    friend class Path::Iterator;
    ArrayAllocationBase <float> data;
    int numElements;
    float pathXMin, pathXMax, pathYMin, pathYMax;
    bool useNonZeroWinding;

    static const float lineMarker;
    static const float moveMarker;
    static const float quadMarker;
    static const float cubicMarker;
    static const float closeSubPathMarker;
};

#endif   // __JUCE_PATH_JUCEHEADER__
