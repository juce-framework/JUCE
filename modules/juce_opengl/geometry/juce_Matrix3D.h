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
    A 4x4 3D transformation matrix.

    @see Vector3D, Quaternion, AffineTransform
*/
template <typename Type>
class Matrix3D
{
public:
    /** Creates an identity matrix. */
    Matrix3D() noexcept
    {
        mat[0]  = (Type) 1; mat[1]  = 0;        mat[2]  = 0;         mat[3]  = 0;
        mat[4]  = 0;        mat[5]  = (Type) 1; mat[6]  = 0;         mat[7]  = 0;
        mat[8]  = 0;        mat[9]  = 0;        mat[10] = (Type) 1;  mat[11] = 0;
        mat[12] = 0;        mat[13] = 0;        mat[14] = 0;         mat[15] = (Type) 1;
    }

    /** Creates a copy of another matrix. */
    Matrix3D (const Matrix3D& other) noexcept
    {
        memcpy (mat, other.mat, sizeof (mat));
    }

    /** Copies another matrix. */
    Matrix3D& operator= (const Matrix3D& other) noexcept
    {
        memcpy (mat, other.mat, sizeof (mat));
        return *this;
    }

    /** Creates a matrix from its raw 4x4 values. */
    Matrix3D (const Type& m00, const Type& m10, const Type& m20, const Type& m30,
              const Type& m01, const Type& m11, const Type& m21, const Type& m31,
              const Type& m02, const Type& m12, const Type& m22, const Type& m32,
              const Type& m03, const Type& m13, const Type& m23, const Type& m33) noexcept
    {
        mat[0]  = m00;  mat[1]  = m10;  mat[2]  = m20;  mat[3]  = m30;
        mat[4]  = m01;  mat[5]  = m11;  mat[6]  = m21;  mat[7]  = m31;
        mat[8]  = m02;  mat[9]  = m12;  mat[10] = m22;  mat[11] = m32;
        mat[12] = m03;  mat[13] = m13;  mat[14] = m23;  mat[15] = m33;
    }

    /** Creates a matrix from an array of 16 raw values. */
    Matrix3D (const Type* values) noexcept
    {
        memcpy (mat, values, sizeof (mat));
    }

    /** Creates a matrix from a 2D affine transform. */
    Matrix3D (const AffineTransform& transform) noexcept
    {
        mat[0]  = transform.mat00;  mat[1] =  transform.mat10;  mat[2]  = 0;         mat[3]  = 0;
        mat[4]  = transform.mat01;  mat[5] =  transform.mat11;  mat[6]  = 0;         mat[7]  = 0;
        mat[8]  = 0;                mat[9] =  0;                mat[10] = (Type) 1;  mat[11] = 0;
        mat[12] = transform.mat02;  mat[13] = transform.mat12;  mat[14] = 0;         mat[15] = (Type) 1;
    }

    /** Creates a matrix from a 3D vector translation. */
    Matrix3D (Vector3D<Type> vector) noexcept
    {
        mat[0]  = (Type) 1; mat[1]  = 0;        mat[2]  = 0;         mat[3]  = 0;
        mat[4]  = 0;        mat[5]  = (Type) 1; mat[6]  = 0;         mat[7]  = 0;
        mat[8]  = 0;        mat[9]  = 0;        mat[10] = (Type) 1;  mat[11] = 0;
        mat[12] = vector.x; mat[13] = vector.y; mat[14] = vector.z;  mat[15] = (Type) 1;
    }

    /** Returns a new matrix from the given frustrum values. */
    static Matrix3D fromFrustum (Type left, Type right, Type bottom, Type top, Type nearDistance, Type farDistance) noexcept
    {
        return Matrix3D ((2.0f * nearDistance) / (right - left), 0.0f, 0.0f, 0.0f,
                         0.0f, (2.0f * nearDistance) / (top - bottom), 0.0f, 0.0f,
                         (right + left) / (right - left), (top + bottom) / (top - bottom), -(farDistance + nearDistance) / (farDistance - nearDistance), -1.0f,
                         0.0f, 0.0f, -(2.0f * farDistance * nearDistance) / (farDistance - nearDistance), 0.0f);
    }

    /** Multiplies this matrix by another. */
    Matrix3D& operator*= (const Matrix3D& other) noexcept
    {
        return *this = *this * other;
    }

    /** Multiplies this matrix by another, and returns the result. */
    Matrix3D operator* (const Matrix3D& other) const noexcept
    {
        const Type* const m2 = other.mat;

        return Matrix3D (mat[0]  * m2[0] + mat[1]  * m2[4] + mat[2]  * m2[8]  + mat[3]  * m2[12],
                         mat[0]  * m2[1] + mat[1]  * m2[5] + mat[2]  * m2[9]  + mat[3]  * m2[13],
                         mat[0]  * m2[2] + mat[1]  * m2[6] + mat[2]  * m2[10] + mat[3]  * m2[14],
                         mat[0]  * m2[3] + mat[1]  * m2[7] + mat[2]  * m2[11] + mat[3]  * m2[15],
                         mat[4]  * m2[0] + mat[5]  * m2[4] + mat[6]  * m2[8]  + mat[7]  * m2[12],
                         mat[4]  * m2[1] + mat[5]  * m2[5] + mat[6]  * m2[9]  + mat[7]  * m2[13],
                         mat[4]  * m2[2] + mat[5]  * m2[6] + mat[6]  * m2[10] + mat[7]  * m2[14],
                         mat[4]  * m2[3] + mat[5]  * m2[7] + mat[6]  * m2[11] + mat[7]  * m2[15],
                         mat[8]  * m2[0] + mat[9]  * m2[4] + mat[10] * m2[8]  + mat[11] * m2[12],
                         mat[8]  * m2[1] + mat[9]  * m2[5] + mat[10] * m2[9]  + mat[11] * m2[13],
                         mat[8]  * m2[2] + mat[9]  * m2[6] + mat[10] * m2[10] + mat[11] * m2[14],
                         mat[8]  * m2[3] + mat[9]  * m2[7] + mat[10] * m2[11] + mat[11] * m2[15],
                         mat[12] * m2[0] + mat[13] * m2[4] + mat[14] * m2[8]  + mat[15] * m2[12],
                         mat[12] * m2[1] + mat[13] * m2[5] + mat[14] * m2[9]  + mat[15] * m2[13],
                         mat[12] * m2[2] + mat[13] * m2[6] + mat[14] * m2[10] + mat[15] * m2[14],
                         mat[12] * m2[3] + mat[13] * m2[7] + mat[14] * m2[11] + mat[15] * m2[15]);
    }

    /** Returns a copy of this matrix after rotation through the Y, X and then Z angles
        specified by the vector.
    */
    Matrix3D rotated (Vector3D<Type> eulerAngleRadians) const noexcept
    {
        const Type cx = std::cos (eulerAngleRadians.x),  sx = std::sin (eulerAngleRadians.x),
                   cy = std::cos (eulerAngleRadians.y),  sy = std::sin (eulerAngleRadians.y),
                   cz = std::cos (eulerAngleRadians.z),  sz = std::sin (eulerAngleRadians.z);

        return Matrix3D ((cy * cz) + (sx * sy * sz), cx * sz, (cy * sx * sz) - (cz * sy), 0.0f,
                         (cz * sx * sy) - (cy * sz), cx * cz, (cy * cz * sx) + (sy * sz), 0.0f,
                         cx * sy, -sx, cx * cy, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);
    }

    /** The 4x4 matrix values. These are stored in the standard OpenGL order. */
    Type mat[16];
};

} // namespace juce
