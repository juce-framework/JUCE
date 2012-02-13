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

#ifndef __JUCE_VECTOR3D_JUCEHEADER__
#define __JUCE_VECTOR3D_JUCEHEADER__

//==============================================================================
/**
    A three-coordinate vector.
*/
template <typename Type>
class Vector3D
{
public:
    Vector3D() noexcept  : x(), y(), z() {}
    Vector3D (const Type& xValue, const Type& yValue, const Type& zValue) noexcept  : x (xValue), y (yValue), z (zValue) {}
    Vector3D (const Vector3D& other) noexcept   : x (other.x), y (other.y), z (other.z) {}
    Vector3D& operator= (const Vector3D& other) noexcept     { x = other.x;  y = other.y;  z = other.z;  return *this; }

    /** Returns a vector that lies along the X axis. */
    static Vector3D xAxis() noexcept        { return Vector3D ((Type) 1, 0, 0); }
    /** Returns a vector that lies along the Y axis. */
    static Vector3D yAxis() noexcept        { return Vector3D (0, (Type) 1, 0); }
    /** Returns a vector that lies along the Z axis. */
    static Vector3D zAxis() noexcept        { return Vector3D (0, 0, (Type) 1); }

    Vector3D& operator+= (const Vector3D& other) noexcept        { x += other.x;  y += other.y;  z += other.z;  return *this; }
    Vector3D& operator-= (const Vector3D& other) noexcept        { x -= other.x;  y -= other.y;  z -= other.z;  return *this; }
    Vector3D& operator*= (const Type& scaleFactor) noexcept      { x *= scaleFactor;  y *= scaleFactor;  z *= scaleFactor;  return *this; }
    Vector3D& operator/= (const Type& scaleFactor) noexcept      { x /= scaleFactor;  y /= scaleFactor;  z /= scaleFactor;  return *this; }

    Vector3D operator+ (const Vector3D& other) const noexcept    { return Vector3D (x + other.x, y + other.y, z + other.z); }
    Vector3D operator- (const Vector3D& other) const noexcept    { return Vector3D (x - other.x, y - other.y, z - other.z); }
    Vector3D operator* (const Type& scaleFactor) const noexcept  { return Vector3D (x * scaleFactor, y * scaleFactor, z * scaleFactor); }
    Vector3D operator/ (const Type& scaleFactor) const noexcept  { return Vector3D (x / scaleFactor, y / scaleFactor, z / scaleFactor); }
    Vector3D operator-() const noexcept                          { return Vector3D (-x, -y, -z); }

    /** Returns the dot-product of these two vectors. */
    Type operator* (const Vector3D& other) const noexcept        { return x * other.x + y * other.y + z * other.z; }

    /** Returns the cross-product of these two vectors. */
    Vector3D operator^ (const Vector3D& other) const noexcept    { return Vector3D (y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }

    Type length() const noexcept            { return std::sqrt (lengthSquared()); }
    Type lengthSquared() const noexcept     { return x * x + y * y + z * z; }

    Vector3D normalised() const noexcept    { return *this / length(); }

    /** Returns true if the vector is practically equal to the origin. */
    bool lengthIsBelowEpsilon() const noexcept
    {
        const Type epsilon (std::numeric_limits<Type>::epsilon());
        return ! (x < -epsilon || x > epsilon || y < -epsilon || y > epsilon || z < -epsilon || z > epsilon);
    }

    Type x, y, z;
};


#endif   // __JUCE_VECTOR3D_JUCEHEADER__
