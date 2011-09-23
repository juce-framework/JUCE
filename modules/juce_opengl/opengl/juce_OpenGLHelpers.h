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

#ifndef __JUCE_OPENGLHELPERS_JUCEHEADER__
#define __JUCE_OPENGLHELPERS_JUCEHEADER__

/**
    A set of miscellaneous openGL helper functions.
*/
class JUCE_API  OpenGLHelpers
{
public:
    /** Clears the GL error state. */
    static void resetErrorState();

    /** Clears the current context using the given colour. */
    static void clear (const Colour& colour);

    /** Gives the current context an orthoganal rendering mode for 2D drawing into the given size. */
    static void prepareFor2D (int width, int height);

    /** This does the same job as gluPerspective(). */
    static void setPerspective (double fovy, double aspect, double zNear, double zFar);

    /** Draws a 2D quad with the specified corner points. */
    static void drawQuad2D (float x1, float y1,
                            float x2, float y2,
                            float x3, float y3,
                            float x4, float y4,
                            const Colour& colour);

    /** Draws a 3D quad with the specified corner points. */
    static void drawQuad3D (float x1, float y1, float z1,
                            float x2, float y2, float z2,
                            float x3, float y3, float z3,
                            float x4, float y4, float z4,
                            const Colour& colour);
};


#endif   // __JUCE_OPENGLHELPERS_JUCEHEADER__
