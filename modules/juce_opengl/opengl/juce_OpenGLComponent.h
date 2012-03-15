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

class OpenGLGraphicsContext;
#if JUCE_ANDROID
class AndroidGLContext;
#endif

//==============================================================================
/**
    A component that contains an OpenGL canvas.

    Override this, add it to whatever component you want to, and use the renderOpenGL()
    method to draw its contents.

    Important note! When using a GL component with a background thread, your sub-class
    must call stopRenderThread() in its destructor. See stopRenderThread() for
    more details.
*/
class JUCE_API  OpenGLComponent  : public OpenGLBaseType
{
public:
    //==============================================================================
    /** These flags can be combined and passed to the OpenGLComponent constructor to
        specify various options.
    */
    enum OpenGLFlags
    {
        /** This value can be used if you want your OpenGLComponent to use the
            default settings.
        */
        openGLDefault = 8,

       #if JUCE_IOS || JUCE_ANDROID
        openGLES1 = 1,  /**< This selects openGL ES 1.0 */
        openGLES2 = 2,  /**< This selects openGL ES 2.0 */
       #endif

        /** If this flag is enabled, the component will launch a background thread to
            perform the rendering. If this flag is not enabled, then renderOpenGL()
            will be invoked on the main event thread when the component has been told to
            repaint, or after triggerRepaint() has been called.

            Important note! When using a background thread, your sub-class MUST call
            stopRenderThread() in its destructor. See stopRenderThread() for
            more details.
        */
        useBackgroundThread = 4,

        /** If this flag is enabled, then any sub-components of the OpenGLComponent
            will be correctly overlaid on top of the GL content, and its paint() method will
            be able to render over it. If you're not using sub-components, you can disable
            this flag, which will eliminate some overhead.
        */
        allowSubComponents = 8
    };

    //==============================================================================
    /** Creates an OpenGLComponent.
        The flags parameter should be a combination of the values in the
        OpenGLFlags enum.
    */
    OpenGLComponent (int flags = openGLDefault);

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
    inline bool isUsingDedicatedThread() const noexcept        { return (flags & useBackgroundThread) != 0; }

    /** Shuts down the rendering thread.
        This must be called by your sub-class's destructor, to make sure that all rendering
        callbacks have stopped before your class starts to be destroyed.
    */
    void stopRenderThread();

    //==============================================================================
    /** This callback is where your subclass should draw its openGL content.

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
    virtual void newOpenGLContextCreated();

    /** This method is called when the component shuts down its OpenGL context.

        You can use this callback to delete textures and any other OpenGL objects you
        created in the component's context. Be aware: if you are using a render
        thread, this may be called on the thread.

        When this callback happens, the context will have been made current
        using the makeCurrentContextActive() method, so there's no need to call it
        again in your code.
     */
    virtual void releaseOpenGLContext();

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

    /** Causes a repaint to be invoked asynchronously.
        This has a similar effect to calling repaint(), and triggers a callback to
        renderOpenGL(), but unlike repaint(), it does not mark any of the component's
        children as needing a redraw, which means that their cached state can be re-used
        if possible.
    */
    void triggerRepaint();

    //==============================================================================
    /** Delete the context.
        You should only need to call this if you've written a custom thread - if so, make
        sure that your thread calls this before it terminates.
    */
    void deleteContext();

    /** Tries to synchronously delete and re-create the context.
        If the context doesn't exist already, this will try to create one.
        If it exists, it'll first delete the existing one, and create a new one.
        You may need to call this if you require a temporary context for some reason
        before the normal call to newOpenGLContextCreated() is made.

        @returns true if a new context has been successfully created - this may not be
        possible on all platforms.
    */
    bool rebuildContext();

    //==============================================================================
    /** Returns the native handle of an embedded heavyweight window, if there is one.

        E.g. On windows, this will return the HWND of the sub-window containing
        the opengl context, on the mac it'll be the NSOpenGLView.
    */
    void* getNativeWindowHandle() const;

    /** @internal */
    void paint (Graphics&);

private:
    const int flags;

    class OpenGLComponentWatcher;
    friend class OpenGLComponentWatcher;
    friend class ScopedPointer <OpenGLComponentWatcher>;
    ScopedPointer <OpenGLComponentWatcher> componentWatcher;
    ScopedPointer <OpenGLContext> context;
    OpenGLContext* contextToShareListsWith;

    CriticalSection contextLock;
    OpenGLPixelFormat preferredPixelFormat;
    bool needToDeleteContext;

    class OpenGLCachedComponentImage;
    friend class OpenGLCachedComponentImage;
    OpenGLCachedComponentImage* cachedImage;
    OpenGLCachedComponentImage* getGLCachedImage();

    OpenGLContext* createContext();
    void updateContext();
    void recreateContextAsync();
    void startRenderThread();

    int renderAndSwapBuffers();  // (This method has been deprecated)

   #if JUCE_ANDROID
    friend class AndroidGLContext;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent);
};

#endif   // __JUCE_OPENGLCOMPONENT_JUCEHEADER__
