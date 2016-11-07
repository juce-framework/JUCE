/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_AFFINETRANSFORM_H_INCLUDED
#define JUCE_AFFINETRANSFORM_H_INCLUDED


//==============================================================================
/**
    Represents a 2D affine-transformation matrix.

    An affine transformation is a transformation such as a rotation, scale, shear,
    resize or translation.

    These are used for various 2D transformation tasks, e.g. with Path objects.

    @see Path, Point, Line
*/
class JUCE_API  AffineTransform
{
public:
    //==============================================================================
    /** Creates an identity transform. */
    AffineTransform() noexcept;

    /** Creates a copy of another transform. */
    AffineTransform (const AffineTransform& other) noexcept;

    /** Creates a transform from a set of raw matrix values.

        The resulting matrix is:

            (mat00 mat01 mat02)
            (mat10 mat11 mat12)
            (  0     0     1  )
    */
    AffineTransform (float mat00, float mat01, float mat02,
                     float mat10, float mat11, float mat12) noexcept;

    /** Copies from another AffineTransform object */
    AffineTransform& operator= (const AffineTransform& other) noexcept;

    /** Compares two transforms. */
    bool operator== (const AffineTransform& other) const noexcept;

    /** Compares two transforms. */
    bool operator!= (const AffineTransform& other) const noexcept;

   #if JUCE_ALLOW_STATIC_NULL_VARIABLES
    /** A ready-to-use identity transform.
        Note that you should always avoid using a static variable like this, and
        prefer AffineTransform() or {} if you need a default-constructed instance.
    */
    static const AffineTransform identity;
   #endif

    //==============================================================================
    /** Transforms a 2D coordinate using this matrix. */
    template <typename ValueType>
    void transformPoint (ValueType& x, ValueType& y) const noexcept
    {
        const ValueType oldX = x;
        x = static_cast<ValueType> (mat00 * oldX + mat01 * y + mat02);
        y = static_cast<ValueType> (mat10 * oldX + mat11 * y + mat12);
    }

    /** Transforms two 2D coordinates using this matrix.
        This is just a shortcut for calling transformPoint() on each of these pairs of
        coordinates in turn. (And putting all the calculations into one function hopefully
        also gives the compiler a bit more scope for pipelining it).
    */
    template <typename ValueType>
    void transformPoints (ValueType& x1, ValueType& y1,
                          ValueType& x2, ValueType& y2) const noexcept
    {
        const ValueType oldX1 = x1, oldX2 = x2;
        x1 = static_cast<ValueType> (mat00 * oldX1 + mat01 * y1 + mat02);
        y1 = static_cast<ValueType> (mat10 * oldX1 + mat11 * y1 + mat12);
        x2 = static_cast<ValueType> (mat00 * oldX2 + mat01 * y2 + mat02);
        y2 = static_cast<ValueType> (mat10 * oldX2 + mat11 * y2 + mat12);
    }

    /** Transforms three 2D coordinates using this matrix.
        This is just a shortcut for calling transformPoint() on each of these pairs of
        coordinates in turn. (And putting all the calculations into one function hopefully
        also gives the compiler a bit more scope for pipelining it).
    */
    template <typename ValueType>
    void transformPoints (ValueType& x1, ValueType& y1,
                          ValueType& x2, ValueType& y2,
                          ValueType& x3, ValueType& y3) const noexcept
    {
        const ValueType oldX1 = x1, oldX2 = x2, oldX3 = x3;
        x1 = static_cast<ValueType> (mat00 * oldX1 + mat01 * y1 + mat02);
        y1 = static_cast<ValueType> (mat10 * oldX1 + mat11 * y1 + mat12);
        x2 = static_cast<ValueType> (mat00 * oldX2 + mat01 * y2 + mat02);
        y2 = static_cast<ValueType> (mat10 * oldX2 + mat11 * y2 + mat12);
        x3 = static_cast<ValueType> (mat00 * oldX3 + mat01 * y3 + mat02);
        y3 = static_cast<ValueType> (mat10 * oldX3 + mat11 * y3 + mat12);
    }

    //==============================================================================
    /** Returns a new transform which is the same as this one followed by a translation. */
    AffineTransform translated (float deltaX,
                                float deltaY) const noexcept;

    /** Returns a new transform which is the same as this one followed by a translation. */
    template <typename PointType>
    AffineTransform translated (PointType delta) const noexcept
    {
        return translated ((float) delta.x, (float) delta.y);
    }

    /** Returns a new transform which is a translation. */
    static AffineTransform translation (float deltaX,
                                        float deltaY) noexcept;

    /** Returns a new transform which is a translation. */
    template <typename PointType>
    static AffineTransform translation (PointType delta) noexcept
    {
        return translation ((float) delta.x, (float) delta.y);
    }

    /** Returns a copy of this transform with the specified translation matrix values. */
    AffineTransform withAbsoluteTranslation (float translationX,
                                             float translationY) const noexcept;

    /** Returns a transform which is the same as this one followed by a rotation.

        The rotation is specified by a number of radians to rotate clockwise, centred around
        the origin (0, 0).
    */
    AffineTransform rotated (float angleInRadians) const noexcept;

    /** Returns a transform which is the same as this one followed by a rotation about a given point.

        The rotation is specified by a number of radians to rotate clockwise, centred around
        the coordinates passed in.
    */
    AffineTransform rotated (float angleInRadians,
                             float pivotX,
                             float pivotY) const noexcept;

    /** Returns a new transform which is a rotation about (0, 0). */
    static AffineTransform rotation (float angleInRadians) noexcept;

    /** Returns a new transform which is a rotation about a given point. */
    static AffineTransform rotation (float angleInRadians,
                                     float pivotX,
                                     float pivotY) noexcept;

    /** Returns a transform which is the same as this one followed by a re-scaling.
        The scaling is centred around the origin (0, 0).
    */
    AffineTransform scaled (float factorX,
                            float factorY) const noexcept;

    /** Returns a transform which is the same as this one followed by a re-scaling.
        The scaling is centred around the origin (0, 0).
    */
    AffineTransform scaled (float factor) const noexcept;

    /** Returns a transform which is the same as this one followed by a re-scaling.
        The scaling is centred around the origin provided.
    */
    AffineTransform scaled (float factorX, float factorY,
                            float pivotX, float pivotY) const noexcept;

    /** Returns a new transform which is a re-scale about the origin. */
    static AffineTransform scale (float factorX,
                                  float factorY) noexcept;

    /** Returns a new transform which is a re-scale about the origin. */
    static AffineTransform scale (float factor) noexcept;

    /** Returns a new transform which is a re-scale centred around the point provided. */
    static AffineTransform scale (float factorX, float factorY,
                                  float pivotX, float pivotY) noexcept;

    /** Returns a transform which is the same as this one followed by a shear.
        The shear is centred around the origin (0, 0).
    */
    AffineTransform sheared (float shearX, float shearY) const noexcept;

    /** Returns a shear transform, centred around the origin (0, 0). */
    static AffineTransform shear (float shearX, float shearY) noexcept;

    /** Returns a transform that will flip coordinates vertically within a window of the given height.
        This is handy for converting between upside-down coordinate systems such as OpenGL or CoreGraphics.
    */
    static AffineTransform verticalFlip (float height) noexcept;

    /** Returns a matrix which is the inverse operation of this one.

        Some matrices don't have an inverse - in this case, the method will just return
        an identity transform.
    */
    AffineTransform inverted() const noexcept;

    /** Returns the transform that will map three known points onto three coordinates
        that are supplied.

        This returns the transform that will transform (0, 0) into (x00, y00),
        (1, 0) to (x10, y10), and (0, 1) to (x01, y01).
    */
    static AffineTransform fromTargetPoints (float x00, float y00,
                                             float x10, float y10,
                                             float x01, float y01) noexcept;

    /** Returns the transform that will map three specified points onto three target points. */
    static AffineTransform fromTargetPoints (float sourceX1, float sourceY1, float targetX1, float targetY1,
                                             float sourceX2, float sourceY2, float targetX2, float targetY2,
                                             float sourceX3, float sourceY3, float targetX3, float targetY3) noexcept;

    //==============================================================================
    /** Returns the result of concatenating another transformation after this one. */
    AffineTransform followedBy (const AffineTransform& other) const noexcept;

    /** Returns true if this transform has no effect on points. */
    bool isIdentity() const noexcept;

    /** Returns true if this transform maps to a singularity - i.e. if it has no inverse. */
    bool isSingularity() const noexcept;

    /** Returns true if the transform only translates, and doesn't scale or rotate the
        points. */
    bool isOnlyTranslation() const noexcept;

    /** If this transform is only a translation, this returns the X offset.
        @see isOnlyTranslation
    */
    float getTranslationX() const noexcept                  { return mat02; }

    /** If this transform is only a translation, this returns the X offset.
        @see isOnlyTranslation
    */
    float getTranslationY() const noexcept                  { return mat12; }

    /** Returns the approximate scale factor by which lengths will be transformed.
        Obviously a length may be scaled by entirely different amounts depending on its
        direction, so this is only appropriate as a rough guide.
    */
    float getScaleFactor() const noexcept;

    //==============================================================================
    /* The transform matrix is:

        (mat00 mat01 mat02)
        (mat10 mat11 mat12)
        (  0     0     1  )
    */
    float mat00, mat01, mat02;
    float mat10, mat11, mat12;
};

#endif   // JUCE_AFFINETRANSFORM_H_INCLUDED
