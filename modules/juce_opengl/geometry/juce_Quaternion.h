/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Holds a quaternion (a 3D vector and a scalar value).

    @tags{OpenGL}
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

} // namespace juce
