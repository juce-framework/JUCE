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

#ifndef __JUCE_OPENGLRENDERINGTARGET_JUCEHEADER__
#define __JUCE_OPENGLRENDERINGTARGET_JUCEHEADER__

//==============================================================================
/**
    Base class for OpenGL objects which can be selected as a rendering target.
*/
class JUCE_API  OpenGLRenderingTarget
{
public:
    OpenGLRenderingTarget();
    virtual ~OpenGLRenderingTarget();

    /** Activates this object as the current OpenGL target. */
    virtual bool makeCurrentRenderingTarget() = 0;

    /** Deactivates this object as the current OpenGL target. */
    virtual void releaseAsRenderingTarget() = 0;

    /** Returns the width in pixels of this target. */
    virtual int getRenderingTargetWidth() const = 0;
    /** Returns the height in pixels of this target. */
    virtual int getRenderingTargetHeight() const = 0;

    /** Sets the current matrix for 2D rendering into this object.
        @see OpenGLHelpers::prepareFor2D
    */
    void prepareFor2D();
};


#endif   // __JUCE_OPENGLRENDERINGTARGET_JUCEHEADER__
