/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_RELATIVECOORDINATE_JUCEHEADER__
#define __JUCE_RELATIVECOORDINATE_JUCEHEADER__

#include "juce_Path.h"
#include "juce_Rectangle.h"
#include "../../../maths/juce_Expression.h"
#include "../../../containers/juce_OwnedArray.h"
#include "../../../containers/juce_ValueTree.h"


//==============================================================================
/**
    Expresses a coordinate as a dynamically evaluated expression.

    @see RelativePoint, RelativeRectangle
*/
class JUCE_API  RelativeCoordinate
{
public:
    //==============================================================================
    /** Creates a zero coordinate. */
    RelativeCoordinate();
    RelativeCoordinate (const Expression& expression);
    RelativeCoordinate (const RelativeCoordinate& other);
    RelativeCoordinate& operator= (const RelativeCoordinate& other);

    /** Creates an absolute position from the parent origin on either the X or Y axis.

        @param absoluteDistanceFromOrigin   the distance from the origin
    */
    RelativeCoordinate (double absoluteDistanceFromOrigin);

    /** Recreates a coordinate from a string description.
        The string will be parsed by ExpressionParser::parse().
        @param stringVersion    the expression to use
        @see toString
    */
    RelativeCoordinate (const String& stringVersion);

    /** Destructor. */
    ~RelativeCoordinate();

    bool operator== (const RelativeCoordinate& other) const throw();
    bool operator!= (const RelativeCoordinate& other) const throw();

    //==============================================================================
    /** Calculates the absolute position of this coordinate.

        You'll need to provide a suitable Expression::EvaluationContext for looking up any coordinates that may
        be needed to calculate the result.
    */
    double resolve (const Expression::EvaluationContext* evaluationContext) const;

    /** Returns true if this coordinate uses the specified coord name at any level in its evaluation.
        This will recursively check any coordinates upon which this one depends.
    */
    bool references (const String& coordName, const Expression::EvaluationContext* evaluationContext) const;

    /** Returns true if there's a recursive loop when trying to resolve this coordinate's position. */
    bool isRecursive (const Expression::EvaluationContext* evaluationContext) const;

    /** Returns true if this coordinate depends on any other coordinates for its position. */
    bool isDynamic() const;

    //==============================================================================
    /** Changes the value of this coord to make it resolve to the specified position.

        Calling this will leave the anchor points unchanged, but will set this coordinate's absolute
        or relative position to whatever value is necessary to make its resultant position
        match the position that is provided.
    */
    void moveToAbsolute (double absoluteTargetPosition, const Expression::EvaluationContext* evaluationContext);

    /** Changes the name of a symbol if it is used as part of the coordinate's expression. */
    void renameSymbolIfUsed (const String& oldName, const String& newName);

    /** Returns the expression that defines this coordinate. */
    const Expression& getExpression() const         { return term; }


    //==============================================================================
    /** Returns a string which represents this coordinate.
        For details of the string syntax, see the constructor notes.
    */
    const String toString() const;

    //==============================================================================
    /** A set of static strings that are commonly used by the RelativeCoordinate class.

        As well as avoiding using string literals in your code, using these preset values
        has the advantage that all instances of the same string will share the same, reference-counted
        String object, so if you have thousands of points which all refer to the same
        anchor points, this can save a significant amount of memory allocation.
    */
    struct Strings
    {
        static const String parent;         /**< "parent" */
        static const String left;           /**< "left" */
        static const String right;          /**< "right" */
        static const String top;            /**< "top" */
        static const String bottom;         /**< "bottom" */
        static const String parentLeft;     /**< "parent.left" */
        static const String parentTop;      /**< "parent.top" */
        static const String parentRight;    /**< "parent.right" */
        static const String parentBottom;   /**< "parent.bottom" */
    };

private:
    //==============================================================================
    Expression term;
};


//==============================================================================
/**
    An X-Y position stored as a pair of RelativeCoordinate values.

    @see RelativeCoordinate, RelativeRectangle
*/
class JUCE_API  RelativePoint
{
public:
    /** Creates a point at the origin. */
    RelativePoint();

    /** Creates an absolute point, relative to the origin. */
    RelativePoint (const Point<float>& absolutePoint);

    /** Creates an absolute point, relative to the origin. */
    RelativePoint (float absoluteX, float absoluteY);

    /** Creates an absolute point from two coordinates. */
    RelativePoint (const RelativeCoordinate& x, const RelativeCoordinate& y);

    /** Creates a point from a stringified representation.
        The string must contain a pair of coordinates, separated by space or a comma. The syntax for the coordinate
        strings is explained in the RelativeCoordinate class.
        @see toString
    */
    RelativePoint (const String& stringVersion);

    bool operator== (const RelativePoint& other) const throw();
    bool operator!= (const RelativePoint& other) const throw();

    /** Calculates the absolute position of this point.

        You'll need to provide a suitable Expression::EvaluationContext for looking up any coordinates that may
        be needed to calculate the result.
    */
    const Point<float> resolve (const Expression::EvaluationContext* evaluationContext) const;

    /** Changes the values of this point's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    void moveToAbsolute (const Point<float>& newPos, const Expression::EvaluationContext* evaluationContext);

    /** Returns a string which represents this point.
        This returns a comma-separated pair of coordinates. For details of the string syntax used by the
        coordinates, see the RelativeCoordinate constructor notes.
        The string that is returned can be passed to the RelativePoint constructor to recreate the point.
    */
    const String toString() const;

    /** Renames a symbol if it is used by any of the coordinates.
        This calls RelativeCoordinate::renameAnchorIfUsed() on its X and Y coordinates.
    */
    void renameSymbolIfUsed (const String& oldName, const String& newName);

    /** Returns true if this point depends on any other coordinates for its position. */
    bool isDynamic() const;

    // The actual X and Y coords...
    RelativeCoordinate x, y;
};


//==============================================================================
/**
    An rectangle stored as a set of RelativeCoordinate values.

    The rectangle's top, left, bottom and right edge positions are each stored as a RelativeCoordinate.

    @see RelativeCoordinate, RelativePoint
*/
class JUCE_API  RelativeRectangle
{
public:
    //==============================================================================
    /** Creates a zero-size rectangle at the origin. */
    RelativeRectangle();

    /** Creates an absolute rectangle, relative to the origin. */
    explicit RelativeRectangle (const Rectangle<float>& rect, const String& componentName);

    /** Creates a rectangle from four coordinates. */
    RelativeRectangle (const RelativeCoordinate& left, const RelativeCoordinate& right,
                       const RelativeCoordinate& top, const RelativeCoordinate& bottom);

    /** Creates a rectangle from a stringified representation.
        The string must contain a sequence of 4 coordinates, separated by commas, in the order
        left, top, right, bottom. The syntax for the coordinate strings is explained in the
        RelativeCoordinate class.
        @see toString
    */
    explicit RelativeRectangle (const String& stringVersion);

    bool operator== (const RelativeRectangle& other) const throw();
    bool operator!= (const RelativeRectangle& other) const throw();

    //==============================================================================
    /** Calculates the absolute position of this rectangle.

        You'll need to provide a suitable Expression::EvaluationContext for looking up any coordinates that may
        be needed to calculate the result.
    */
    const Rectangle<float> resolve (const Expression::EvaluationContext* evaluationContext) const;

    /** Changes the values of this rectangle's coordinates to make it resolve to the specified position.

        Calling this will leave any anchor points unchanged, but will set any absolute
        or relative positions to whatever values are necessary to make the resultant position
        match the position that is provided.
    */
    void moveToAbsolute (const Rectangle<float>& newPos, const Expression::EvaluationContext* evaluationContext);

    /** Returns a string which represents this point.
        This returns a comma-separated list of coordinates, in the order left, top, right, bottom. For details of
        the string syntax used by the coordinates, see the RelativeCoordinate constructor notes.
        The string that is returned can be passed to the RelativeRectangle constructor to recreate the rectangle.
    */
    const String toString() const;

    /** Renames a symbol if it is used by any of the coordinates.
        This calls RelativeCoordinate::renameSymbolIfUsed() on the rectangle's coordinates.
    */
    void renameSymbolIfUsed (const String& oldName, const String& newName);

    // The actual rectangle coords...
    RelativeCoordinate left, right, top, bottom;
};


//==============================================================================
/**
    A path object that consists of RelativePoint coordinates rather than the normal fixed ones.

    One of these paths can be converted into a Path object for drawing and manipulation, but
    unlike a Path, its points can be dynamic instead of just fixed.

    @see RelativePoint, RelativeCoordinate
*/
class JUCE_API  RelativePointPath
{
public:
    //==============================================================================
    RelativePointPath();
    RelativePointPath (const RelativePointPath& other);
    RelativePointPath (const ValueTree& drawable);
    RelativePointPath (const Path& path);
    ~RelativePointPath();

    //==============================================================================
    /** Resolves this points in this path and adds them to a normal Path object. */
    void createPath (Path& path, Expression::EvaluationContext* coordFinder);

    /** Returns true if the path contains any non-fixed points. */
    bool containsAnyDynamicPoints() const;

    /** Writes the path to this drawable encoding. */
    void writeTo (ValueTree state, UndoManager* undoManager) const;

    /** Quickly swaps the contents of this path with another. */
    void swapWith (RelativePointPath& other) throw();

    //==============================================================================
    /** The types of element that may be contained in this path.
        @see RelativePointPath::ElementBase
    */
    enum ElementType
    {
        nullElement,
        startSubPathElement,
        closeSubPathElement,
        lineToElement,
        quadraticToElement,
        cubicToElement
    };

    //==============================================================================
    /** Base class for the elements that make up a RelativePointPath.
    */
    class JUCE_API  ElementBase
    {
    public:
        ElementBase (ElementType type);
        virtual ~ElementBase() {}
        virtual const ValueTree createTree() const = 0;
        virtual void addToPath (Path& path, Expression::EvaluationContext* coordFinder) const = 0;
        virtual RelativePoint* getControlPoints (int& numPoints) = 0;

        const ElementType type;

    private:
        JUCE_DECLARE_NON_COPYABLE (ElementBase);
    };

    class JUCE_API  StartSubPath  : public ElementBase
    {
    public:
        StartSubPath (const RelativePoint& pos);
        ~StartSubPath() {}
        const ValueTree createTree() const;
        void addToPath (Path& path, Expression::EvaluationContext* coordFinder) const;
        RelativePoint* getControlPoints (int& numPoints);

        RelativePoint startPos;

    private:
        JUCE_DECLARE_NON_COPYABLE (StartSubPath);
    };

    class JUCE_API  CloseSubPath  : public ElementBase
    {
    public:
        CloseSubPath();
        ~CloseSubPath() {}
        const ValueTree createTree() const;
        void addToPath (Path& path, Expression::EvaluationContext* coordFinder) const;
        RelativePoint* getControlPoints (int& numPoints);

    private:
        JUCE_DECLARE_NON_COPYABLE (CloseSubPath);
    };

    class JUCE_API  LineTo  : public ElementBase
    {
    public:
        LineTo (const RelativePoint& endPoint);
        ~LineTo() {}
        const ValueTree createTree() const;
        void addToPath (Path& path, Expression::EvaluationContext* coordFinder) const;
        RelativePoint* getControlPoints (int& numPoints);

        RelativePoint endPoint;

    private:
        JUCE_DECLARE_NON_COPYABLE (LineTo);
    };

    class JUCE_API  QuadraticTo  : public ElementBase
    {
    public:
        QuadraticTo (const RelativePoint& controlPoint, const RelativePoint& endPoint);
        ~QuadraticTo() {}
        const ValueTree createTree() const;
        void addToPath (Path& path, Expression::EvaluationContext* coordFinder) const;
        RelativePoint* getControlPoints (int& numPoints);

        RelativePoint controlPoints[2];

    private:
        JUCE_DECLARE_NON_COPYABLE (QuadraticTo);
    };

    class JUCE_API  CubicTo  : public ElementBase
    {
    public:
        CubicTo (const RelativePoint& controlPoint1, const RelativePoint& controlPoint2, const RelativePoint& endPoint);
        ~CubicTo() {}
        const ValueTree createTree() const;
        void addToPath (Path& path, Expression::EvaluationContext* coordFinder) const;
        RelativePoint* getControlPoints (int& numPoints);

        RelativePoint controlPoints[3];

    private:
        JUCE_DECLARE_NON_COPYABLE (CubicTo);
    };

    //==============================================================================
    OwnedArray <ElementBase> elements;
    bool usesNonZeroWinding;

private:
    bool containsDynamicPoints;

    void parse (const ValueTree& state);

    RelativePointPath& operator= (const RelativePointPath&);
};

//==============================================================================
/**
    A parallelogram defined by three RelativePoint positions.

    @see RelativePoint, RelativeCoordinate
*/
class JUCE_API  RelativeParallelogram
{
public:
    //==============================================================================
    RelativeParallelogram();
    RelativeParallelogram (const Rectangle<float>& simpleRectangle);
    RelativeParallelogram (const RelativePoint& topLeft, const RelativePoint& topRight, const RelativePoint& bottomLeft);
    RelativeParallelogram (const String& topLeft, const String& topRight, const String& bottomLeft);
    ~RelativeParallelogram();

    //==============================================================================
    void resolveThreePoints (Point<float>* points, Expression::EvaluationContext* coordFinder) const;
    void resolveFourCorners (Point<float>* points, Expression::EvaluationContext* coordFinder) const;
    const Rectangle<float> getBounds (Expression::EvaluationContext* coordFinder) const;
    void getPath (Path& path, Expression::EvaluationContext* coordFinder) const;
    const AffineTransform resetToPerpendicular (Expression::EvaluationContext* coordFinder);

    bool operator== (const RelativeParallelogram& other) const throw();
    bool operator!= (const RelativeParallelogram& other) const throw();

    static const Point<float> getInternalCoordForPoint (const Point<float>* parallelogramCorners, Point<float> point) throw();
    static const Point<float> getPointForInternalCoord (const Point<float>* parallelogramCorners, const Point<float>& internalPoint) throw();

    //==============================================================================
    RelativePoint topLeft, topRight, bottomLeft;
};


#endif   // __JUCE_RELATIVECOORDINATE_JUCEHEADER__
