/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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

    @tags{Graphics}
*/
class JUCE_API  Path  final
{
public:
    //==============================================================================
    /** Creates an empty path. */
    Path();

    /** Creates a copy of another path. */
    Path (const Path&);

    /** Destructor. */
    ~Path();

    /** Copies this path from another one. */
    Path& operator= (const Path&);

    /** Move constructor */
    Path (Path&&) noexcept;

    /** Move assignment operator */
    Path& operator= (Path&&) noexcept;

    bool operator== (const Path&) const noexcept;
    bool operator!= (const Path&) const noexcept;

    static const float defaultToleranceForTesting;
    static const float defaultToleranceForMeasurement;

    //==============================================================================
    /** Returns true if the path doesn't contain any lines or curves. */
    bool isEmpty() const noexcept;

    /** Returns the smallest rectangle that contains all points within the path. */
    Rectangle<float> getBounds() const noexcept;

    /** Returns the smallest rectangle that contains all points within the path
        after it's been transformed with the given transform matrix.
    */
    Rectangle<float> getBoundsTransformed (const AffineTransform& transform) const noexcept;

    /** Checks whether a point lies within the path.

        This is only relevant for closed paths (see closeSubPath()), and
        may produce false results if used on a path which has open sub-paths.

        The path's winding rule is taken into account by this method.

        The tolerance parameter is the maximum error allowed when flattening the path,
        so this method could return a false positive when your point is up to this distance
        outside the path's boundary.

        @see closeSubPath, setUsingNonZeroWinding
    */
    bool contains (float x, float y,
                   float tolerance = defaultToleranceForTesting) const;

    /** Checks whether a point lies within the path.

        This is only relevant for closed paths (see closeSubPath()), and
        may produce false results if used on a path which has open sub-paths.

        The path's winding rule is taken into account by this method.

        The tolerance parameter is the maximum error allowed when flattening the path,
        so this method could return a false positive when your point is up to this distance
        outside the path's boundary.

        @see closeSubPath, setUsingNonZeroWinding
    */
    bool contains (Point<float> point,
                   float tolerance = defaultToleranceForTesting) const;

    /** Checks whether a line crosses the path.

        This will return positive if the line crosses any of the paths constituent
        lines or curves. It doesn't take into account whether the line is inside
        or outside the path, or whether the path is open or closed.

        The tolerance parameter is the maximum error allowed when flattening the path,
        so this method could return a false positive when your point is up to this distance
        outside the path's boundary.
    */
    bool intersectsLine (Line<float> line,
                         float tolerance = defaultToleranceForTesting) const;

    /** Cuts off parts of a line to keep the parts that are either inside or
        outside this path.

        Note that this isn't smart enough to cope with situations where the
        line would need to be cut into multiple pieces to correctly clip against
        a re-entrant shape.

        @param line                     the line to clip
        @param keepSectionOutsidePath   if true, it's the section outside the path
                                        that will be kept; if false its the section inside
                                        the path
    */
    Line<float> getClippedLine (Line<float> line, bool keepSectionOutsidePath) const;

    /** Returns the length of the path.
        @see getPointAlongPath
    */
    float getLength (const AffineTransform& transform = AffineTransform(),
                     float tolerance = defaultToleranceForMeasurement) const;

    /** Returns a point that is the specified distance along the path.
        If the distance is greater than the total length of the path, this will return the
        end point.
        @see getLength
    */
    Point<float> getPointAlongPath (float distanceFromStart,
                                    const AffineTransform& transform = AffineTransform(),
                                    float tolerance = defaultToleranceForMeasurement) const;

    /** Finds the point along the path which is nearest to a given position.
        This sets pointOnPath to the nearest point, and returns the distance of this point from the start
        of the path.
    */
    float getNearestPoint (Point<float> targetPoint,
                           Point<float>& pointOnPath,
                           const AffineTransform& transform = AffineTransform(),
                           float tolerance = defaultToleranceForMeasurement) const;

    //==============================================================================
    /** Removes all lines and curves, resetting the path completely. */
    void clear() noexcept;

    /** Begins a new subpath with a given starting position.

        This will move the path's current position to the coordinates passed in and
        make it ready to draw lines or curves starting from this position.

        After adding whatever lines and curves are needed, you can either
        close the current sub-path using closeSubPath() or call startNewSubPath()
        to move to a new sub-path, leaving the old one open-ended.

        @see lineTo, quadraticTo, cubicTo, closeSubPath
    */
    void startNewSubPath (float startX, float startY);

    /** Begins a new subpath with a given starting position.

        This will move the path's current position to the coordinates passed in and
        make it ready to draw lines or curves starting from this position.

        After adding whatever lines and curves are needed, you can either
        close the current sub-path using closeSubPath() or call startNewSubPath()
        to move to a new sub-path, leaving the old one open-ended.

        @see lineTo, quadraticTo, cubicTo, closeSubPath
    */
    void startNewSubPath (Point<float> start);

    /** Closes a the current sub-path with a line back to its start-point.

        When creating a closed shape such as a triangle, don't use 3 lineTo()
        calls - instead use two lineTo() calls, followed by a closeSubPath()
        to join the final point back to the start.

        This ensures that closes shapes are recognised as such, and this is
        important for tasks like drawing strokes, which needs to know whether to
        draw end-caps or not.

        @see startNewSubPath, lineTo, quadraticTo, cubicTo, closeSubPath
    */
    void closeSubPath();

    /** Adds a line from the shape's last position to a new end-point.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a straight line.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, quadraticTo, cubicTo, closeSubPath
    */
    void lineTo (float endX, float endY);

    /** Adds a line from the shape's last position to a new end-point.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a straight line.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, quadraticTo, cubicTo, closeSubPath
    */
    void lineTo (Point<float> end);

    /** Adds a quadratic bezier curve from the shape's last position to a new position.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a quadratic spline with one control-point.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, lineTo, cubicTo, closeSubPath
    */
    void quadraticTo (float controlPointX,
                      float controlPointY,
                      float endPointX,
                      float endPointY);

    /** Adds a quadratic bezier curve from the shape's last position to a new position.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a quadratic spline with one control-point.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, lineTo, cubicTo, closeSubPath
    */
    void quadraticTo (Point<float> controlPoint,
                      Point<float> endPoint);

    /** Adds a cubic bezier curve from the shape's last position to a new position.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a cubic spline with two control-points.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, lineTo, quadraticTo, closeSubPath
    */
    void cubicTo (float controlPoint1X,
                  float controlPoint1Y,
                  float controlPoint2X,
                  float controlPoint2Y,
                  float endPointX,
                  float endPointY);

    /** Adds a cubic bezier curve from the shape's last position to a new position.

        This will connect the end-point of the last line or curve that was added
        to a new point, using a cubic spline with two control-points.

        See the class description for an example of how to add lines and curves to a path.

        @see startNewSubPath, lineTo, quadraticTo, closeSubPath
    */
    void cubicTo (Point<float> controlPoint1,
                  Point<float> controlPoint2,
                  Point<float> endPoint);

    /** Returns the last point that was added to the path by one of the drawing methods.
    */
    Point<float> getCurrentPosition() const;

    //==============================================================================
    /** Adds a rectangle to the path.
        The rectangle is added as a new sub-path. (Any currently open paths will be left open).
        @see addRoundedRectangle, addTriangle
    */
    void addRectangle (float x, float y, float width, float height);

    /** Adds a rectangle to the path.
        The rectangle is added as a new sub-path. (Any currently open paths will be left open).
        @see addRoundedRectangle, addTriangle
    */
    template <typename ValueType>
    void addRectangle (Rectangle<ValueType> rectangle)
    {
        addRectangle (static_cast<float> (rectangle.getX()), static_cast<float> (rectangle.getY()),
                      static_cast<float> (rectangle.getWidth()), static_cast<float> (rectangle.getHeight()));
    }

    /** Adds a rectangle with rounded corners to the path.
        The rectangle is added as a new sub-path. (Any currently open paths will be left open).
        @see addRectangle, addTriangle
    */
    void addRoundedRectangle (float x, float y, float width, float height,
                              float cornerSize);

    /** Adds a rectangle with rounded corners to the path.
        The rectangle is added as a new sub-path. (Any currently open paths will be left open).
        @see addRectangle, addTriangle
    */
    void addRoundedRectangle (float x, float y, float width, float height,
                              float cornerSizeX,
                              float cornerSizeY);

    /** Adds a rectangle with rounded corners to the path.
        The rectangle is added as a new sub-path. (Any currently open paths will be left open).
        @see addRectangle, addTriangle
    */
    void addRoundedRectangle (float x, float y, float width, float height,
                              float cornerSizeX, float cornerSizeY,
                              bool curveTopLeft, bool curveTopRight,
                              bool curveBottomLeft, bool curveBottomRight);

    /** Adds a rectangle with rounded corners to the path.
        The rectangle is added as a new sub-path. (Any currently open paths will be left open).
        @see addRectangle, addTriangle
    */
    template <typename ValueType>
    void addRoundedRectangle (Rectangle<ValueType> rectangle, float cornerSizeX, float cornerSizeY)
    {
        addRoundedRectangle (static_cast<float> (rectangle.getX()), static_cast<float> (rectangle.getY()),
                             static_cast<float> (rectangle.getWidth()), static_cast<float> (rectangle.getHeight()),
                             cornerSizeX, cornerSizeY);
    }

    /** Adds a rectangle with rounded corners to the path.
        The rectangle is added as a new sub-path. (Any currently open paths will be left open).
        @see addRectangle, addTriangle
    */
    template <typename ValueType>
    void addRoundedRectangle (Rectangle<ValueType> rectangle, float cornerSize)
    {
        addRoundedRectangle (rectangle, cornerSize, cornerSize);
    }

    /** Adds a triangle to the path.

        The triangle is added as a new closed sub-path. (Any currently open paths will be left open).

        Note that whether the vertices are specified in clockwise or anticlockwise
        order will affect how the triangle is filled when it overlaps other
        shapes (the winding order setting will affect this of course).
    */
    void addTriangle (float x1, float y1,
                      float x2, float y2,
                      float x3, float y3);

    /** Adds a triangle to the path.

        The triangle is added as a new closed sub-path. (Any currently open paths will be left open).

        Note that whether the vertices are specified in clockwise or anticlockwise
        order will affect how the triangle is filled when it overlaps other
        shapes (the winding order setting will affect this of course).
    */
    void addTriangle (Point<float> point1,
                      Point<float> point2,
                      Point<float> point3);

    /** Adds a quadrilateral to the path.

        The quad is added as a new closed sub-path. (Any currently open paths will be left open).

        Note that whether the vertices are specified in clockwise or anticlockwise
        order will affect how the quad is filled when it overlaps other
        shapes (the winding order setting will affect this of course).
    */
    void addQuadrilateral (float x1, float y1,
                           float x2, float y2,
                           float x3, float y3,
                           float x4, float y4);

    /** Adds an ellipse to the path.
        The shape is added as a new sub-path. (Any currently open paths will be left open).
        @see addArc
    */
    void addEllipse (float x, float y, float width, float height);

    /** Adds an ellipse to the path.
        The shape is added as a new sub-path. (Any currently open paths will be left open).
        @see addArc
    */
    void addEllipse (Rectangle<float> area);

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
                            it will be added to the current sub-path, continuing from the current position

        @see addCentredArc, arcTo, addPieSegment, addEllipse
    */
    void addArc (float x, float y, float width, float height,
                 float fromRadians,
                 float toRadians,
                 bool startAsNewSubPath = false);

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
                            it will be added to the current sub-path, continuing from the current position

        @see addArc, arcTo
    */
    void addCentredArc (float centreX, float centreY,
                        float radiusX, float radiusY,
                        float rotationOfEllipse,
                        float fromRadians,
                        float toRadians,
                        bool startAsNewSubPath = false);

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
    void addPieSegment (float x, float y,
                        float width, float height,
                        float fromRadians,
                        float toRadians,
                        float innerCircleProportionalSize);

    /** Adds a "pie-chart" shape to the path.

        The shape is added as a new sub-path. (Any currently open paths will be left open).

        Note that when specifying the start and end angles, the curve will be drawn either clockwise
        or anti-clockwise according to whether the end angle is greater than the start. This means
        that sometimes you may need to use values greater than 2*Pi for the end angle.

        @param segmentBounds the outer rectangle in which the elliptical outline fits
        @param fromRadians   the angle (clockwise) in radians at which to start the arc segment (where 0 is the
                             top-centre of the ellipse)
        @param toRadians     the angle (clockwise) in radians at which to end the arc segment (where 0 is the
                             top-centre of the ellipse)
        @param innerCircleProportionalSize  if this is > 0, then the pie will be drawn as a curved band around a hollow
                             ellipse at its centre, where this value indicates the inner ellipse's size with
                             respect to the outer one.
        @see addArc
    */
    void addPieSegment (Rectangle<float> segmentBounds,
                        float fromRadians,
                        float toRadians,
                        float innerCircleProportionalSize);

    /** Adds a line with a specified thickness.

        The line is added as a new closed sub-path. (Any currently open paths will be
        left open).

        @see addArrow
    */
    void addLineSegment (Line<float> line, float lineThickness);

    /** Adds a line with an arrowhead on the end.
        The arrow is added as a new closed sub-path. (Any currently open paths will be left open).
        @see PathStrokeType::createStrokeWithArrowheads
    */
    void addArrow (Line<float> line,
                   float lineThickness,
                   float arrowheadWidth,
                   float arrowheadLength);

    /** Adds a polygon shape to the path.
        @see addStar
    */
    void addPolygon (Point<float> centre,
                     int numberOfSides,
                     float radius,
                     float startAngle = 0.0f);

    /** Adds a star shape to the path.
        @see addPolygon
    */
    void addStar (Point<float> centre,
                  int numberOfPoints,
                  float innerRadius,
                  float outerRadius,
                  float startAngle = 0.0f);

    /** Adds a speech-bubble shape to the path.

        @param bodyArea         the area of the body of the bubble shape
        @param maximumArea      an area which encloses the body area and defines the limits within which
                                the arrow tip can be drawn - if the tip lies outside this area, the bubble
                                will be drawn without an arrow
        @param arrowTipPosition the location of the tip of the arrow
        @param cornerSize       the size of the rounded corners
        @param arrowBaseWidth   the width of the base of the arrow where it joins the main rectangle
    */
    void addBubble (Rectangle<float> bodyArea,
                    Rectangle<float> maximumArea,
                    const Point<float> arrowTipPosition,
                    const float cornerSize,
                    const float arrowBaseWidth);

    /** Adds another path to this one.

        The new path is added as a new sub-path. (Any currently open paths in this
        path will be left open).

        @param pathToAppend     the path to add
    */
    void addPath (const Path& pathToAppend);

    /** Adds another path to this one, transforming it on the way in.

        The new path is added as a new sub-path, its points being transformed by the given
        matrix before being added.

        @param pathToAppend     the path to add
        @param transformToApply an optional transform to apply to the incoming vertices
    */
    void addPath (const Path& pathToAppend,
                  const AffineTransform& transformToApply);

    /** Swaps the contents of this path with another one.

        The internal data of the two paths is swapped over, so this is much faster than
        copying it to a temp variable and back.
    */
    void swapWithPath (Path&) noexcept;

    //==============================================================================
    /** Preallocates enough space for adding the given number of coordinates to the path.
        If you're about to add a large number of lines or curves to the path, it can make
        the task much more efficient to call this first and avoid costly reallocations
        as the structure grows.
        The actual value to pass is a bit tricky to calculate because the space required
        depends on what you're adding - e.g. each lineTo() or startNewSubPath() will
        require 3 coords (x, y and a type marker). Each quadraticTo() will need 5, and
        a cubicTo() will require 7. Closing a sub-path will require 1.
    */
    void preallocateSpace (int numExtraCoordsToMakeSpaceFor);

    //==============================================================================
    /** Applies a 2D transform to all the vertices in the path.

        @see AffineTransform, scaleToFit, getTransformToScaleToFit
    */
    void applyTransform (const AffineTransform& transform) noexcept;

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
    void scaleToFit (float x, float y, float width, float height,
                     bool preserveProportions) noexcept;

    /** Returns a transform that can be used to rescale the path to fit into a given space.

        @param x                    the x position of the rectangle to fit the path inside
        @param y                    the y position of the rectangle to fit the path inside
        @param width                the width of the rectangle to fit the path inside
        @param height               the height of the rectangle to fit the path inside
        @param preserveProportions  if true, it will fit the path into the space without altering its
                                    horizontal/vertical scale ratio; if false, it will distort the
                                    path to fill the specified ratio both horizontally and vertically
        @param justificationType    if the proportions are preserved, the resultant path may be smaller
                                    than the available rectangle, so this describes how it should be
                                    positioned within the space.
        @returns                    an appropriate transformation

        @see applyTransform, scaleToFit

    */
    AffineTransform getTransformToScaleToFit (float x, float y, float width, float height,
                                              bool preserveProportions,
                                              Justification justificationType = Justification::centred) const;

    /** Returns a transform that can be used to rescale the path to fit into a given space.

        @param area                 the rectangle to fit the path inside
        @param preserveProportions  if true, it will fit the path into the space without altering its
                                    horizontal/vertical scale ratio; if false, it will distort the
                                    path to fill the specified ratio both horizontally and vertically
        @param justificationType    if the proportions are preserved, the resultant path may be smaller
                                    than the available rectangle, so this describes how it should be
                                    positioned within the space.
        @returns                    an appropriate transformation

        @see applyTransform, scaleToFit

    */
    AffineTransform getTransformToScaleToFit (Rectangle<float> area,
                                              bool preserveProportions,
                                              Justification justificationType = Justification::centred) const;

    /** Creates a version of this path where all sharp corners have been replaced by curves.

        Wherever two lines meet at an angle, this will replace the corner with a curve
        of the given radius.
    */
    Path createPathWithRoundedCorners (float cornerRadius) const;

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
    void setUsingNonZeroWinding (bool isNonZeroWinding) noexcept;

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
        Iterator (const Path& path) noexcept;
        ~Iterator() noexcept;

        //==============================================================================
        /** Moves onto the next element in the path.

            If this returns false, there are no more elements. If it returns true,
            the elementType variable will be set to the type of the current element,
            and some of the x and y variables will be filled in with values.
        */
        bool next() noexcept;

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

        float x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;

        //==============================================================================
    private:
        const Path& path;
        const float* index;

        JUCE_DECLARE_NON_COPYABLE (Iterator)
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
    void loadPathFromData (const void* data, size_t numberOfBytes);

    /** Stores the path by writing it out to a stream.
        After writing out a path, you can reload it using loadPathFromStream().
        @see loadPathFromStream, loadPathFromData
    */
    void writePathToStream (OutputStream& destination) const;

    //==============================================================================
    /** Creates a string containing a textual representation of this path.
        @see restoreFromString
    */
    String toString() const;

    /** Restores this path from a string that was created with the toString() method.
        @see toString()
    */
    void restoreFromString (StringRef stringVersion);

private:
    //==============================================================================
    friend class PathFlatteningIterator;
    friend class Path::Iterator;
    friend class EdgeTable;

    Array<float> data;

    struct PathBounds
    {
        PathBounds() noexcept;
        Rectangle<float> getRectangle() const noexcept;
        void reset() noexcept;
        void reset (float, float) noexcept;
        void extend (float, float) noexcept;

        template <typename... Coords>
        void extend (float x, float y, Coords... coords) noexcept
        {
            extend (x, y);
            extend (coords...);
        }

        float pathXMin = 0, pathXMax = 0, pathYMin = 0, pathYMax = 0;
    };

    PathBounds bounds;
    bool useNonZeroWinding = true;

    static const float lineMarker;
    static const float moveMarker;
    static const float quadMarker;
    static const float cubicMarker;
    static const float closeSubPathMarker;

    JUCE_LEAK_DETECTOR (Path)
};

} // namespace juce
