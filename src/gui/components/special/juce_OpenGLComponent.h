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

#include "../juce_Component.h"

// this is used to disable OpenGL, and is defined in juce_Config.h
#if JUCE_OPENGL || DOXYGEN


//==============================================================================
/**
    Represents the various properties of an OpenGL bitmap format.

    @see OpenGLComponent::setPixelFormat
*/
class JUCE_API  OpenGLPixelFormat
{
public:
    //==============================================================================
    /** Creates an OpenGLPixelFormat.

        The default constructor just initialises the object as a simple 8-bit
        RGBA format.
    */
    OpenGLPixelFormat (int bitsPerRGBComponent = 8,
                       int alphaBits = 8,
                       int depthBufferBits = 16,
                       int stencilBufferBits = 0);

    OpenGLPixelFormat (const OpenGLPixelFormat&);
    OpenGLPixelFormat& operator= (const OpenGLPixelFormat&);
    bool operator== (const OpenGLPixelFormat&) const;

    //==============================================================================
    int redBits;          /**< The number of bits per pixel to use for the red channel. */
    int greenBits;        /**< The number of bits per pixel to use for the green channel. */
    int blueBits;         /**< The number of bits per pixel to use for the blue channel. */
    int alphaBits;        /**< The number of bits per pixel to use for the alpha channel. */

    int depthBufferBits;      /**< The number of bits per pixel to use for a depth buffer. */
    int stencilBufferBits;    /**< The number of bits per pixel to use for a stencil buffer. */

    int accumulationBufferRedBits;    /**< The number of bits per pixel to use for an accumulation buffer's red channel. */
    int accumulationBufferGreenBits;  /**< The number of bits per pixel to use for an accumulation buffer's green channel. */
    int accumulationBufferBlueBits;   /**< The number of bits per pixel to use for an accumulation buffer's blue channel. */
    int accumulationBufferAlphaBits;  /**< The number of bits per pixel to use for an accumulation buffer's alpha channel. */

    uint8 fullSceneAntiAliasingNumSamples;      /**< The number of samples to use in full-scene anti-aliasing (if available). */

    //==============================================================================
    /** Returns a list of all the pixel formats that can be used in this system.

        A reference component is needed in case there are multiple screens with different
        capabilities - in which case, the one that the component is on will be used.
    */
    static void getAvailablePixelFormats (Component* component,
                                          OwnedArray <OpenGLPixelFormat>& results);

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR (OpenGLPixelFormat);
};

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
    /** Returns the pixel format being used by this context. */
    virtual const OpenGLPixelFormat getPixelFormat() const = 0;

    /** For windowed contexts, this moves the context within the bounds of
        its parent window.
    */
    virtual void updateWindowPosition (const Rectangle<int>& bounds) = 0;

    /** For windowed contexts, this triggers a repaint of the window.

        (Not relevent on all platforms).
    */
    virtual void repaint() = 0;

    /** Returns an OS-dependent handle to the raw GL context.

        On win32, this will be a HGLRC; on the Mac, an AGLContext; on Linux,
        a GLXContext.
    */
    virtual void* getRawContext() const noexcept = 0;

    /** Deletes the context.

        This must only be called on the message thread, or will deadlock.
        On background threads, call getCurrentContext()->deleteContext(), but be careful not
        to call any other OpenGL function afterwards.
        This doesn't touch other resources, such as window handles, etc.
        You'll probably never have to call this method directly.
    */
    virtual void deleteContext() = 0;

    //==============================================================================
    /** Returns the context that's currently in active use by the calling thread.

        Returns 0 if there isn't an active context.
    */
    static OpenGLContext* getCurrentContext();

protected:
    //==============================================================================
    OpenGLContext() noexcept;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLContext);
};


//==============================================================================
/**
    A component that contains an OpenGL canvas.

    Override this, add it to whatever component you want to, and use the renderOpenGL()
    method to draw its contents.

*/
class JUCE_API  OpenGLComponent  : public Component
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
    /** Changes the pixel format used by this component.

        @see OpenGLPixelFormat::getAvailablePixelFormats()
    */
    void setPixelFormat (const OpenGLPixelFormat& formatToUse);

    /** Returns the pixel format that this component is currently using. */
    const OpenGLPixelFormat getPixelFormat() const;

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

    /** Makes this component the current openGL context.

        You might want to use this in things like your resize() method, before calling
        GL commands.

        If this returns false, then the context isn't active, so you should avoid
        making any calls.

        This call may actually create a context if one isn't currently initialised. If
        it does this, it will also synchronously call the newOpenGLContextCreated()
        method to let you initialise it as necessary.

        @see OpenGLContext::makeActive
    */
    bool makeCurrentContextActive();

    /** Stops the current component being the active OpenGL context.

        This is the opposite of makeCurrentContextActive()

        @see OpenGLContext::makeInactive
    */
    void makeCurrentContextInactive();

    /** Returns true if this component's context is the active openGL context for the
        current thread.

        @see OpenGLContext::isActive
    */
    bool isActiveContext() const noexcept;


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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent);
};


#endif
#endif   // __JUCE_OPENGLCOMPONENT_JUCEHEADER__
