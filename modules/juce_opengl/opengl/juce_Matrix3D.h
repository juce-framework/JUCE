/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_MATRIX3D_JUCEHEADER__
#define __JUCE_MATRIX3D_JUCEHEADER__

//==============================================================================
/**
    A 4x4 transformation matrix.

    @see AffineTransform
*/
template <typename Type>
class Matrix3D
{
public:
    Matrix3D() noexcept
    {
        zeromem (mat, sizeof (mat));
        mat[0] = mat[1 + 1 * 4] = mat[2 + 2 * 4] = mat[3 + 3 * 4] = (Type) 1;
    }

    Matrix3D (const Matrix3D& other) noexcept
    {
        memcpy (mat, other.mat, sizeof (mat));
    }

    Matrix3D (const AffineTransform& transform) noexcept
    {
        mat[0]  = transform.mat00;  mat[1] =  transform.mat10;  mat[2]  = 0;         mat[3]  = 0;
        mat[4]  = transform.mat01;  mat[5] =  transform.mat11;  mat[6]  = 0;         mat[7]  = 0;
        mat[8]  = 0;                mat[9] =  0;                mat[10] = (Type) 1;  mat[11] = 0;
        mat[12] = transform.mat02;  mat[13] = transform.mat12;  mat[14] = 0;         mat[15] = (Type) 1;
    }

    Matrix3D (const Type* values) noexcept
    {
        memcpy (mat, values, sizeof (mat));
    }

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

    Matrix3D& operator= (const Matrix3D& other) noexcept
    {
        memcpy (mat, other.mat, sizeof (mat));
        return *this;
    }

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    /** Multiplies the active OpenGL context's matrix by this one. */
    void applyToOpenGL() const noexcept
    {
        OpenGLHelpers::applyMatrix (mat);
    }
   #endif

    /** The 4x4 matrix values. These are stored in the standard OpenGL order. */
    Type mat[16];
};


#endif   // __JUCE_MATRIX3D_JUCEHEADER__
