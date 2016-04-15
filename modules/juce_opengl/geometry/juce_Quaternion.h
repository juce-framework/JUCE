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

#ifndef JUCE_QUATERNION_H_INCLUDED
#define JUCE_QUATERNION_H_INCLUDED

#include "juce_Vector3D.h"
#include "juce_Matrix3D.h"

//==============================================================================
/**
    Holds a quaternion (a 3D vector and a scalar value).
*/
template <typename Type>
class Quaternion
{
public:
    Quaternion() noexcept                                               : scalar() {}
    Quaternion (const Quaternion& other) noexcept                       : vector (other.vector), scalar (other.scalar) {}
    Quaternion (Vector3D<Type> vectorPart, Type scalarPart) noexcept    : vector (vectorPart), scalar (scalarPart) {}
    Quaternion (Type x, Type y, Type z, Type w) noexcept                : vector (x, y, z), scalar (w) {}

    /** Creates a quaternion from an angle and an axis. */
    static Quaternion fromAngle (Type angle, Vector3D<Type> axis) noexcept
    {
        return Quaternion (axis.normalised() * std::sin (angle / (Type) 2), std::cos (angle / (Type) 2));
    }

    Quaternion& operator= (Quaternion other) noexcept
    {
        vector = other.vector;
        scalar = other.scalar;
        return *this;
    }

    Quaternion& operator*= (Quaternion other) noexcept
    {
        const Type oldScalar (scalar);
        scalar = (scalar * other.scalar) - (vector * other.vector);
        vector = (other.vector * oldScalar) + (vector * other.scalar) + (vector ^ other.vector);
        return *this;
    }

    Type length() const noexcept        { return std::sqrt (normal()); }
    Type normal() const noexcept        { return scalar * scalar + vector.lengthSquared(); }

    Quaternion normalised() const noexcept
    {
        const Type len (length());
        jassert (len > 0);
        return Quaternion (vector / len, scalar / len);
    }

    /** Returns the matrix that will perform the rotation specified by this quaternion. */
    Matrix3D<Type> getRotationMatrix() const noexcept
    {
        const Type norm (normal());
        const Type s (norm > 0 ? ((Type) 2) / norm : 0);
        const Type xs (s * vector.x),  ys (s * vector.y),  zs (s * vector.z);
        const Type wx (xs * scalar),   wy (ys * scalar),   wz (zs * scalar);
        const Type xx (xs * vector.x), xy (ys * vector.x), xz (zs * vector.x);
        const Type yy (ys * vector.y), yz (zs * vector.y), zz (zs * vector.z);

        return Matrix3D<Type> (((Type) 1) - (yy + zz), xy - wz, xz + wy, 0,
                               xy + wz, ((Type) 1) - (xx+ zz),  yz - wx, 0,
                               xz - wy, yz + wx, ((Type) 1) - (xx + yy), 0,
                               0, 0, 0, (Type) 1);
    }

    /** The vector part of the quaternion. */
    Vector3D<Type> vector;

    /** The scalar part of the quaternion. */
    Type scalar;
};


#endif   // JUCE_QUATERNION_H_INCLUDED
