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

#ifndef __JUCE_OPENGLCOMPONENT_JUCEHEADER__
#define __JUCE_OPENGLCOMPONENT_JUCEHEADER__

#include "juce_OpenGLContext.h"

#if JUCE_MAC && ! defined (DOXYGEN)
 typedef NSViewComponent OpenGLBaseType;
#else
 typedef Component OpenGLBaseType;
#endif

//==============================================================================
/**
    A component that contains an OpenGL canvas.

    Override this, add it to whatever component you want to, and use the renderOpenGL()
    method to draw its contents.

*/
class JUCE_API  OpenGLComponent  : public OpenGLBaseType,
                                   public OpenGLRenderingTarget
{
public:
    //==============================================================================
    /** Used to select the type of openGL API to use, if more than one choice is available
        on a particular platform.
    */
    enum OpenGLType
    {
        openGLDefault = 0,

       #if JUCE_IOS
        openGLES1,  /**< On the iPhone, this selects openGL ES 1.0 */
        openGLES2   /**< On the iPhone, this selects openGL ES 2.0 */
       #endif
    };

    /** Creates an OpenGLComponent.
        If useBackgroundThread is true, the component will launch a background thread
        to do the rendering. If false, then renderOpenGL() will be called as part of the
        normal paint() method.
    */
    OpenGLComponent (OpenGLType type = openGLDefault,
                     bool useBackgroundThread = false);

    /** Destructor. */
    ~OpenGLComponent();

    //==============================================================================
    /** Changes the pixel format used by this component. */
    void setPixelFormat (const OpenGLPixelFormat& formatToUse);

    /** Specifies an OpenGL context which should be shared with the one that this
        component is using.

        This is an OpenGL feature that lets two contexts share their texture data.

        Note that this pointer is stored by the component, and when the component
        needs to recreate its internal context for some reason, the same context
        will be used again to share lists. So if you pass a context in here,
        don't delete the context while this component is still using it! You can
        call shareWith (nullptr) to stop this component from sharing with it.
    */
    void shareWith (OpenGLContext* contextToShareListsWith);

    /** Returns the context that this component is sharing with.
        @see shareWith
    */
    OpenGLContext* getShareContext() const noexcept     { return contextToShareListsWith; }


    //==============================================================================
    /** Flips the openGL buffers over. */
    void swapBuffers();

    /** Returns true if the component is performing the rendering on a background thread.
        This property is specified in the constructor.
    */
    bool isUsingDedicatedThread() const noexcept        { return useThread; }

    /** This replaces the normal paint() callback - use it to draw your openGL stuff.

        When this is called, makeCurrentContextActive() will already have been called
        for you, so you just need to draw.
    */
    virtual void renderOpenGL() = 0;

    /** This method is called when the component creates a new OpenGL context.

        A new context may be created when the component is first used, or when it
        is moved to a different window, or when the window is hidden and re-shown,
        etc.

        You can use this callback as an opportunity to set up things like textures
        that your context needs.

        New contexts are created on-demand by the makeCurrentContextActive() method - so
        if the context is deleted, e.g. by changing the pixel format or window, no context
        will be created until the next call to makeCurrentContextActive(), which will
        synchronously create one and call this method. This means that if you're using
        a non-GUI thread for rendering, you can make sure this method is be called by
        your renderer thread.

        When this callback happens, the context will already have been made current
        using the makeCurrentContextActive() method, so there's no need to call it
        again in your code.
    */
    virtual void newOpenGLContextCreated() = 0;

    /** This method is called when the component shuts down its OpenGL context.

        You can use this callback to delete textures and any other OpenGL objects you
        created in the component's context. Be aware: if you are using a render
        thread, this may be called on the thread.

        When this callback happens, the context will have been made current
        using the makeCurrentContextActive() method, so there's no need to call it
        again in your code.
     */
    virtual void releaseOpenGLContext()                         {}

    //==============================================================================
    /** Returns the context that will draw into this component.

        This may return 0 if the component is currently invisible or hasn't currently
        got a context. The context object can be deleted and a new one created during
        the lifetime of this component, and there may be times when it doesn't have one.

        @see newOpenGLContextCreated()
    */
    OpenGLContext* getCurrentContext() const noexcept           { return context; }

    /** Makes this component the currently active openGL context.

        If this returns false, then the context isn't active, so you should avoid
        making any calls.

        This call may actually create a context if one isn't currently initialised. If
        it does this, it will also synchronously call the newOpenGLContextCreated()
        method to let you initialise it as necessary.

        @see releaseAsRenderingTarget
    */
    bool makeCurrentRenderingTarget();

    /** Stops the current component being the active OpenGL context.
        This is the opposite of makeCurrentRenderingTarget()
        @see makeCurrentRenderingTarget
    */
    void releaseAsRenderingTarget();

    int getRenderingTargetWidth() const         { return getWidth(); }
    int getRenderingTargetHeight() const        { return getHeight(); }

    //==============================================================================
    /** Calls the rendering callback, and swaps the buffers afterwards.
        This is called automatically by paint() when the component needs to be rendered.
        Returns true if the operation succeeded.
    */
    virtual bool renderAndSwapBuffers();

    /** This returns a critical section that can be used to lock the current context.

        Because the context that is used by this component can change, e.g. when the
        component is shown or hidden, then if you're rendering to it on a background
        thread, this allows you to lock the context for the duration of your rendering
        routine.
    */
    CriticalSection& getContextLock() noexcept      { return contextLock; }

    /** Delete the context.
        You should only need to call this if you've written a custom thread - if so, make
        sure that your thread calls this before it terminates.
    */
    void deleteContext();

    /** If this component is backed by a frame buffer, this returns its ID number, or
        0 if the component has no accessible framebuffer.
    */
    unsigned int getFrameBufferID() const;

    //==============================================================================
    /** Returns the native handle of an embedded heavyweight window, if there is one.

        E.g. On windows, this will return the HWND of the sub-window containing
        the opengl context, on the mac it'll be the NSOpenGLView.
    */
    void* getNativeWindowHandle() const;

protected:
    /** Kicks off a thread to start rendering.
        The default implementation creates and manages an internal thread that tries
        to render at around 50fps, but this can be overloaded to create a custom thread.
    */
    virtual void startRenderThread();

    /** Cleans up the rendering thread.
        Used to shut down the thread that was started by startRenderThread(). If you've
        created a custom thread, then you should overload this to clean it up and delete it.
    */
    virtual void stopRenderThread();

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);

private:
    const OpenGLType type;

    class OpenGLComponentRenderThread;
    friend class OpenGLComponentRenderThread;
    friend class ScopedPointer <OpenGLComponentRenderThread>;
    ScopedPointer <OpenGLComponentRenderThread> renderThread;

    class OpenGLComponentWatcher;
    friend class OpenGLComponentWatcher;
    friend class ScopedPointer <OpenGLComponentWatcher>;
    ScopedPointer <OpenGLComponentWatcher> componentWatcher;
    ScopedPointer <OpenGLContext> context;
    OpenGLContext* contextToShareListsWith;

    CriticalSection contextLock;
    OpenGLPixelFormat preferredPixelFormat;
    bool needToUpdateViewport, needToDeleteContext, threadStarted;
    const bool useThread;

    OpenGLContext* createContext();
    void updateContext();
    void updateContextPosition();
    void stopBackgroundThread();
    void recreateContextAsync();
    void internalRepaint (int x, int y, int w, int h);
    void updateEmbeddedPosition (const Rectangle<int>&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent);
};

#endif   // __JUCE_OPENGLCOMPONENT_JUCEHEADER__
