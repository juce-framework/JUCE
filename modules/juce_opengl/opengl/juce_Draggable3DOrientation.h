/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_DRAGGABLE3DORIENTATION_H_INCLUDED
#define JUCE_DRAGGABLE3DORIENTATION_H_INCLUDED


//==============================================================================
/**
    Stores a 3D orientation, which can be rotated by dragging with the mouse.
*/
class Draggable3DOrientation
{
public:
    typedef Vector3D<GLfloat> VectorType;
    typedef Quaternion<GLfloat> QuaternionType;

    /** Creates a Draggable3DOrientation, initially set up to be aligned along the X axis. */
    Draggable3DOrientation (float objectRadius = 0.5f) noexcept
        : radius (jmax (0.1f, objectRadius)),
          quaternion (VectorType::xAxis(), 0)
    {
    }

    /** Creates a Draggable3DOrientation from a user-supplied quaternion. */
    Draggable3DOrientation (const Quaternion<GLfloat>& quaternionToUse,
                            float objectRadius = 0.5f) noexcept
        : radius (jmax (0.1f, objectRadius)),
          quaternion (quaternionToUse)
    {
    }

    /** Resets the orientation, specifying the axis to align it along. */
    void reset (const VectorType& axis) noexcept
    {
        quaternion = QuaternionType (axis, 0);
    }

    /** Sets the viewport area within which mouse-drag positions will occur.
        You'll need to set this rectangle before calling mouseDown. The centre of the
        rectangle is assumed to be the centre of the object that will be rotated, and
        the size of the rectangle will be used to scale the object radius - see setRadius().
    */
    void setViewport (const Rectangle<int>& newArea) noexcept
    {
        area = newArea;
    }

    /** Sets the size of the rotated object, as a proportion of the viewport's size.
        @see setViewport
    */
    void setRadius (float newRadius) noexcept
    {
        radius = jmax (0.1f, newRadius);
    }

    /** Begins a mouse-drag operation.
        You must call this before any calls to mouseDrag(). The position that is supplied
        will be treated as being relative to the centre of the rectangle passed to setViewport().
    */
    template <typename Type>
    void mouseDown (Point<Type> mousePos) noexcept
    {
        lastMouse = mousePosToProportion (mousePos.toFloat());
    }

    /** Continues a mouse-drag operation.
        After calling mouseDown() to begin a drag sequence, you can call this method
        to continue it.
    */
    template <typename Type>
    void mouseDrag (Point<Type> mousePos) noexcept
    {
        const VectorType oldPos (projectOnSphere (lastMouse));
        lastMouse = mousePosToProportion (mousePos.toFloat());
        const VectorType newPos (projectOnSphere (lastMouse));

        quaternion *= rotationFromMove (oldPos, newPos);
    }

    /** Returns the matrix that should be used to apply the current orientation.
        @see applyToOpenGLMatrix
    */
    Matrix3D<GLfloat> getRotationMatrix() const noexcept
    {
        return quaternion.getRotationMatrix();
    }

    /** Provides direct access to the quaternion. */
    QuaternionType& getQuaternion() noexcept
    {
        return quaternion;
    }

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    /** Applies this rotation to the active OpenGL context's matrix. */
    void applyToOpenGLMatrix() const noexcept
    {
        getRotationMatrix().applyToOpenGL();
    }
   #endif

private:
    Rectangle<int> area;
    float radius;
    QuaternionType quaternion;
    Point<float> lastMouse;

    Point<float> mousePosToProportion (const Point<float> mousePos) const noexcept
    {
        const int scale = (jmin (area.getWidth(), area.getHeight()) / 2);

        // You must call setViewport() to give this object a valid window size before
        // calling any of the mouse input methods!
        jassert (scale > 0);

        return Point<float> ((mousePos.x - area.getCentreX()) / scale,
                             (area.getCentreY() - mousePos.y) / scale);
    }

    VectorType projectOnSphere (const Point<float> pos) const noexcept
    {
        const GLfloat radiusSquared = radius * radius;
        const GLfloat xySquared = pos.x * pos.x + pos.y * pos.y;

        return VectorType (pos.x, pos.y,
                           xySquared < radiusSquared * 0.5f ? std::sqrt (radiusSquared - xySquared)
                                                            : (radiusSquared / (2.0f * std::sqrt (xySquared))));
    }

    QuaternionType rotationFromMove (const VectorType& from, const VectorType& to) const noexcept
    {
        VectorType rotationAxis (to ^ from);

        if (rotationAxis.lengthIsBelowEpsilon())
            rotationAxis = VectorType::xAxis();

        const GLfloat d = jlimit (-1.0f, 1.0f, (from - to).length() / (2.0f * radius));

        return QuaternionType::fromAngle (2.0f * std::asin (d), rotationAxis);
    }
};

#endif   // JUCE_DRAGGABLE3DORIENTATION_H_INCLUDED
