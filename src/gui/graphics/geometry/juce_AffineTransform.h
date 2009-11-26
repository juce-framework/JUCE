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

#ifndef __JUCE_AFFINETRANSFORM_JUCEHEADER__
#define __JUCE_AFFINETRANSFORM_JUCEHEADER__


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
    AffineTransform() throw();

    /** Creates a copy of another transform. */
    AffineTransform (const AffineTransform& other) throw();

    /** Creates a transform from a set of raw matrix values.

        The resulting matrix is:

            (mat00 mat01 mat02)
            (mat10 mat11 mat12)
            (  0     0     1  )
    */
    AffineTransform (const float mat00, const float mat01, const float mat02,
                     const float mat10, const float mat11, const float mat12) throw();

    /** Copies from another AffineTransform object */
    const AffineTransform& operator= (const AffineTransform& other) throw();

    /** Compares two transforms. */
    bool operator== (const AffineTransform& other) const throw();

    /** Compares two transforms. */
    bool operator!= (const AffineTransform& other) const throw();

    /** A ready-to-use identity transform, which you can use to append other
        transformations to.

        e.g. @code
        AffineTransform myTransform = AffineTransform::identity.rotated (.5f)
                                                               .scaled (2.0f);

        @endcode
    */
    static const AffineTransform identity;

    //==============================================================================
    /** Transforms a 2D co-ordinate using this matrix. */
    void transformPoint (float& x,
                         float& y) const throw();

    /** Transforms a 2D co-ordinate using this matrix. */
    void transformPoint (double& x,
                         double& y) const throw();

    //==============================================================================
    /** Returns a new transform which is the same as this one followed by a translation. */
    const AffineTransform translated (const float deltaX,
                                      const float deltaY) const throw();

    /** Returns a new transform which is a translation. */
    static const AffineTransform translation (const float deltaX,
                                              const float deltaY) throw();

    /** Returns a transform which is the same as this one followed by a rotation.

        The rotation is specified by a number of radians to rotate clockwise, centred around
        the origin (0, 0).
    */
    const AffineTransform rotated (const float angleInRadians) const throw();

    /** Returns a transform which is the same as this one followed by a rotation about a given point.

        The rotation is specified by a number of radians to rotate clockwise, centred around
        the co-ordinates passed in.
    */
    const AffineTransform rotated (const float angleInRadians,
                                   const float pivotX,
                                   const float pivotY) const throw();

    /** Returns a new transform which is a rotation about (0, 0). */
    static const AffineTransform rotation (const float angleInRadians) throw();

    /** Returns a new transform which is a rotation about a given point. */
    static const AffineTransform rotation (const float angleInRadians,
                                           const float pivotX,
                                           const float pivotY) throw();

    /** Returns a transform which is the same as this one followed by a re-scaling.

        The scaling is centred around the origin (0, 0).
    */
    const AffineTransform scaled (const float factorX,
                                  const float factorY) const throw();

    /** Returns a new transform which is a re-scale about the origin. */
    static const AffineTransform scale (const float factorX,
                                        const float factorY) throw();

    /** Returns a transform which is the same as this one followed by a shear.

        The shear is centred around the origin (0, 0).
    */
    const AffineTransform sheared (const float shearX,
                                   const float shearY) const throw();

    /** Returns a matrix which is the inverse operation of this one.

        Some matrices don't have an inverse - in this case, the method will just return
        an identity transform.
    */
    const AffineTransform inverted() const throw();

    //==============================================================================
    /** Returns the result of concatenating another transformation after this one. */
    const AffineTransform followedBy (const AffineTransform& other) const throw();

    /** Returns true if this transform has no effect on points. */
    bool isIdentity() const throw();

    /** Returns true if this transform maps to a singularity - i.e. if it has no inverse. */
    bool isSingularity() const throw();

    /** Returns true if the transform only translates, and doesn't scale or rotate the
        points. */
    bool isOnlyTranslation() const throw();

    /** If this transform is only a translation, this returns the X offset.
        @see isOnlyTranslation
    */
    float getTranslationX() const throw()                   { return mat02; }

    /** If this transform is only a translation, this returns the X offset.
        @see isOnlyTranslation
    */
    float getTranslationY() const throw()                   { return mat12; }

    //==============================================================================
    juce_UseDebuggingNewOperator

    //==============================================================================
    /* The transform matrix is:

        (mat00 mat01 mat02)
        (mat10 mat11 mat12)
        (  0     0     1  )
    */
    float mat00, mat01, mat02;
    float mat10, mat11, mat12;

private:
    //==============================================================================
    const AffineTransform followedBy (const float mat00, const float mat01, const float mat02,
                                      const float mat10, const float mat11, const float mat12) const throw();
};

#endif   // __JUCE_AFFINETRANSFORM_JUCEHEADER__
