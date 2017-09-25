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

} // namespace juce
