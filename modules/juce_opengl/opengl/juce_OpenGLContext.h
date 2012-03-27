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
*/
class JUCE_API  OpenGLRenderer
{
public:
    OpenGLRenderer() {}
    virtual ~OpenGLRenderer() {}

    /**
    */
    virtual void newOpenGLContextCreated() = 0;

    /**
    */
    virtual void renderOpenGL() = 0;

    /**
    */
    virtual void openGLContextClosing() = 0;
};

//==============================================================================
/**
    A base class for types of OpenGL context.

    An OpenGLComponent will supply its own context for drawing in its window.
*/
class JUCE_API  OpenGLContext
{
public:
    OpenGLContext();

    /** Destructor. */
    ~OpenGLContext();

    //==============================================================================
    /** */
    void setRenderer (OpenGLRenderer* rendererToUse,
                      bool shouldAlsoPaintComponent) noexcept;

    /** */
    void attachTo (Component& component);

    /** */
    void attachTo (Component& component,
                   const OpenGLPixelFormat& preferredPixelFormat,
                   const OpenGLContext* contextToShareWith);
    /** */
    void detach();

    /** */
    bool isAttached() const noexcept;

    //==============================================================================
    /** Makes this context the currently active one. */
    bool makeActive() const noexcept;
    /** If this context is currently active, it is disactivated. */
    bool makeInactive() const noexcept;
    /** Returns true if this context is currently active. */
    bool isActive() const noexcept;

    /** Returns the component to which this context is currently attached, or nullptr. */
    Component* getTargetComponent() const noexcept;

    /** Returns the context that's currently in active use by the calling thread, or
        nullptr if no context is active.
    */
    static OpenGLContext* getCurrentContext();

    //==============================================================================
    /** Swaps the buffers (if the context can do this).
        There's normally no need to call this directly - the buffers will be swapped
        automatically after your OpenGLRenderer::renderOpenGL() method has been called.
    */
    void swapBuffers();

    /** Sets whether the context checks the vertical sync before swapping.

        The value is the number of frames to allow between buffer-swapping. This is
        fairly system-dependent, but 0 turns off syncing, 1 makes it swap on frame-boundaries,
        and greater numbers indicate that it should swap less often.

        Returns true if it sets the value successfully - some platforms won't support
        this setting.
    */
    bool setSwapInterval (int numFramesPerSwap);

    /** Returns the current swap-sync interval.
        See setSwapInterval() for info about the value returned.
    */
    int getSwapInterval() const;

    /** */
    void triggerRepaint();

    //==============================================================================
    /** Returns the width of this context */
    inline int getWidth() const noexcept                    { return width; }

    /** Returns the height of this context */
    inline int getHeight() const noexcept                   { return height; }

    /** If this context is backed by a frame buffer, this returns its ID number,
        or 0 if the context has no accessible framebuffer.
    */
    unsigned int getFrameBufferID() const noexcept;

    /** Returns true if shaders can be used in this context. */
    bool areShadersAvailable() const;

    /** This structure holds a set of dynamically loaded GL functions for use on this context. */
    OpenGLExtensionFunctions extensions;

    /** This retrieves an object that was previously stored with setAssociatedObject().
        If no object is found with the given name, this will return nullptr.
        This method must only be called from within the GL rendering methods.
        @see setAssociatedObject
    */
    ReferenceCountedObjectPtr<ReferenceCountedObject> getAssociatedObject (const char* name) const;

    /** Attaches a named object to the context, which will be deleted when the context is
        destroyed.

        This allows you to store an object which will be released before the context is
        deleted. The main purpose is for caching GL objects such as shader programs, which
        will become invalid when the context is deleted.

        This method must only be called from within the GL rendering methods.
    */
    void setAssociatedObject (const char* name, ReferenceCountedObject* newObject);

    //==============================================================================
    /** Returns an OS-dependent handle to some kind of underlting OS-provided GL context.

        The exact type of the value returned will depend on the OS and may change
        if the implementation changes. If you want to use this, digging around in the
        native code is probably the best way to find out what it is.
    */
    void* getRawContext() const noexcept;


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

   #ifndef DOXYGEN
    class NativeContext;
   #endif

private:
    class CachedImage;
    class Attachment;
    NativeContext* nativeContext;
    OpenGLRenderer* renderer;
    ScopedPointer<Attachment> attachment;
    int width, height;
    bool renderComponents;

    CachedImage* getCachedImage() const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLContext);
};



#endif   // __JUCE_OPENGLCONTEXT_JUCEHEADER__
