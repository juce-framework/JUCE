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

#ifndef __JUCE_OPENGLCONTEXT_JUCEHEADER__
#define __JUCE_OPENGLCONTEXT_JUCEHEADER__

#include "juce_OpenGLPixelFormat.h"
#include "../native/juce_OpenGLExtensions.h"


//==============================================================================
/**
    A base class for types of OpenGL context.

    An OpenGLComponent will supply its own context for drawing in its window.
*/
class JUCE_API  OpenGLContext
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~OpenGLContext();

    //==============================================================================
    /** Makes this context the currently active one. */
    virtual bool makeActive() const noexcept = 0;
    /** If this context is currently active, it is disactivated. */
    virtual bool makeInactive() const noexcept = 0;
    /** Returns true if this context is currently active. */
    virtual bool isActive() const noexcept = 0;

    /** Swaps the buffers (if the context can do this). */
    virtual void swapBuffers() = 0;

    /** Sets whether the context checks the vertical sync before swapping.

        The value is the number of frames to allow between buffer-swapping. This is
        fairly system-dependent, but 0 turns off syncing, 1 makes it swap on frame-boundaries,
        and greater numbers indicate that it should swap less often.

        Returns true if it sets the value successfully.
    */
    virtual bool setSwapInterval (int numFramesPerSwap) = 0;

    /** Returns the current swap-sync interval.
        See setSwapInterval() for info about the value returned.
    */
    virtual int getSwapInterval() const = 0;

    //==============================================================================
    /** Returns an OS-dependent handle to the raw GL context.

        On win32, this will be a HGLRC; on the Mac, an NSOpenGLContext; on Linux,
        a GLXContext.
    */
    virtual void* getRawContext() const noexcept = 0;

    /** Returns the width of this context */
    virtual int getWidth() const = 0;
    /** Returns the height of this context */
    virtual int getHeight() const = 0;

    /** Updates the native context's area within its parent. */
    virtual void updateWindowPosition (const Rectangle<int>&) = 0;

    /** If this context is backed by a frame buffer, this returns its ID number, or
        0 if the context has no accessible framebuffer.
    */
    virtual unsigned int getFrameBufferID() const = 0;

    /** Checks whether the current context supports the specified extension. */
    bool isExtensionSupported (const char* const extensionName);

    /** Returns true if shaders can be used in this context. */
    virtual bool areShadersAvailable() const;

    //==============================================================================
    /** Returns the context that's currently in active use by the calling thread.

        Returns 0 if there isn't an active context.
    */
    static OpenGLContext* getCurrentContext();

    /** This property set allows you to attach persisent values to the context. */
    NamedValueSet properties;

    /** This structure holds a set of dynamically loaded GL functions for use on this context. */
    OpenGLExtensionFunctions extensions;

    //==============================================================================
    /** Draws the currently selected texture into this context at its original size.
        @param targetClipArea  the target area to draw into (in top-left origin coords)
        @param anchorPosAndTextureSize  the position of this rectangle is the texture's top-left
                                        anchor position in the target space, and the size must be
                                        the total size of the texture.
        @param contextWidth     the width of the context or framebuffer that is being drawn into,
                                used for scaling of the coordinates.
        @param contextHeight    the height of the context or framebuffer that is being drawn into,
                                used for vertical flipping of the y coordinates.
    */
    void copyTexture (const Rectangle<int>& targetClipArea,
                      const Rectangle<int>& anchorPosAndTextureSize,
                      int contextWidth, int contextHeight);

protected:
    //==============================================================================
    OpenGLContext() noexcept;

private:
    mutable int shaderLanguageAvailable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLContext);
};


#endif   // __JUCE_OPENGLCONTEXT_JUCEHEADER__
