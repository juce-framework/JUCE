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

extern XContext windowHandleXContext;

//==============================================================================
// Defined juce_linux_Windowing.cpp
Rectangle<int> juce_LinuxScaledToPhysicalBounds (ComponentPeer* peer, const Rectangle<int>& bounds);
void juce_LinuxAddRepaintListener (ComponentPeer* peer, Component* dummy);
void juce_LinuxRemoveRepaintListener (ComponentPeer* peer, Component* dummy);

//==============================================================================
class OpenGLContext::NativeContext
{
private:
    class DummyComponent : public Component
    {
    public:
        DummyComponent (OpenGLContext::NativeContext& nativeParentContext)
            : native (nativeParentContext)
        {
        }

        void handleCommandMessage (int commandId) override
        {
            if (commandId == 0)
                native.triggerRepaint();
        }
    private:
        OpenGLContext::NativeContext& native;
    };

public:
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& cPixelFormat,
                   void* shareContext,
                   bool /*useMultisampling*/,
                   OpenGLVersion)
        : component (comp), renderContext (0), embeddedWindow (0), swapFrames (0), bestVisual (0),
          contextToShareWith (shareContext), context (nullptr), dummy (*this)
    {
        display = XWindowSystem::getInstance()->displayRef();

        ScopedXLock xlock (display);
        XSync (display, False);

        GLint attribs[] =
        {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_RED_SIZE,         cPixelFormat.redBits,
            GLX_GREEN_SIZE,       cPixelFormat.greenBits,
            GLX_BLUE_SIZE,        cPixelFormat.blueBits,
            GLX_ALPHA_SIZE,       cPixelFormat.alphaBits,
            GLX_DEPTH_SIZE,       cPixelFormat.depthBufferBits,
            GLX_STENCIL_SIZE,     cPixelFormat.stencilBufferBits,
            GLX_ACCUM_RED_SIZE,   cPixelFormat.accumulationBufferRedBits,
            GLX_ACCUM_GREEN_SIZE, cPixelFormat.accumulationBufferGreenBits,
            GLX_ACCUM_BLUE_SIZE,  cPixelFormat.accumulationBufferBlueBits,
            GLX_ACCUM_ALPHA_SIZE, cPixelFormat.accumulationBufferAlphaBits,
            None
        };

        bestVisual = glXChooseVisual (display, DefaultScreen (display), attribs);
        if (bestVisual == nullptr)
            return;

        ComponentPeer* const peer = component.getPeer();
        Window windowH = (Window) peer->getNativeHandle();

        Colormap colourMap = XCreateColormap (display, windowH, bestVisual->visual, AllocNone);
        XSetWindowAttributes swa;
        swa.colormap = colourMap;
        swa.border_pixel = 0;
        swa.event_mask = ExposureMask | StructureNotifyMask;

        Rectangle<int> glBounds (component.getTopLevelComponent()
                               ->getLocalArea (&component, component.getLocalBounds()));

        glBounds = juce_LinuxScaledToPhysicalBounds (peer, glBounds);

        embeddedWindow = XCreateWindow (display, windowH,
                                        glBounds.getX(), glBounds.getY(),
                                        (unsigned int) jmax (1, glBounds.getWidth()),
                                        (unsigned int) jmax (1, glBounds.getHeight()),
                                        0, bestVisual->depth,
                                        InputOutput,
                                        bestVisual->visual,
                                        CWBorderPixel | CWColormap | CWEventMask,
                                        &swa);

        XSaveContext (display, (XID) embeddedWindow, windowHandleXContext, (XPointer) peer);

        XMapWindow (display, embeddedWindow);
        XFreeColormap (display, colourMap);

        XSync (display, False);

        juce_LinuxAddRepaintListener (peer, &dummy);
    }

    ~NativeContext()
    {
        juce_LinuxRemoveRepaintListener (component.getPeer(), &dummy);

        if (embeddedWindow != 0)
        {
            ScopedXLock xlock (display);
            XUnmapWindow (display, embeddedWindow);
            XDestroyWindow (display, embeddedWindow);
        }

        if (bestVisual != nullptr)
            XFree (bestVisual);

        XWindowSystem::getInstance()->displayUnref();
    }

    void initialiseOnRenderThread (OpenGLContext& c)
    {
        ScopedXLock xlock (display);
        renderContext = glXCreateContext (display, bestVisual, (GLXContext) contextToShareWith, GL_TRUE);
        c.makeActive();
        context = &c;
    }

    void shutdownOnRenderThread()
    {
        context = nullptr;
        deactivateCurrentContext();
        glXDestroyContext (display, renderContext);
        renderContext = nullptr;
    }

    bool makeActive() const noexcept
    {
        return renderContext != 0
                 && glXMakeCurrent (display, embeddedWindow, renderContext);
    }

    bool isActive() const noexcept
    {
        return glXGetCurrentContext() == renderContext && renderContext != 0;
    }

    static void deactivateCurrentContext()
    {
        ScopedXDisplay xDisplay;
        ::Display* display = xDisplay.get();
        glXMakeCurrent (display, None, 0);
    }

    void swapBuffers()
    {
        glXSwapBuffers (display, embeddedWindow);
    }

    void updateWindowPosition (const Rectangle<int>& newBounds)
    {
        bounds = newBounds;

        const Rectangle<int> physicalBounds =
            juce_LinuxScaledToPhysicalBounds (component.getPeer(), bounds);

        ScopedXLock xlock (display);
        XMoveResizeWindow (display, embeddedWindow,
                           physicalBounds.getX(), physicalBounds.getY(),
                           (unsigned int) jmax (1, physicalBounds.getWidth()),
                           (unsigned int) jmax (1, physicalBounds.getHeight()));
    }

    bool setSwapInterval (int numFramesPerSwap)
    {
        if (numFramesPerSwap == swapFrames)
            return true;

        PFNGLXSWAPINTERVALSGIPROC GLXSwapIntervalSGI
            = (PFNGLXSWAPINTERVALSGIPROC) OpenGLHelpers::getExtensionFunction ("glXSwapIntervalSGI");

        if (GLXSwapIntervalSGI != nullptr)
        {
            swapFrames = numFramesPerSwap;
            GLXSwapIntervalSGI (numFramesPerSwap);
            return true;
        }

        return false;
    }

    int getSwapInterval() const                 { return swapFrames; }
    bool createdOk() const noexcept             { return true; }
    void* getRawContext() const noexcept        { return renderContext; }
    GLuint getFrameBufferID() const noexcept    { return 0; }

    void triggerRepaint()
    {
        if (context != nullptr)
            context->triggerRepaint();
    }

    struct Locker { Locker (NativeContext&) {} };

private:
    Component& component;
    GLXContext renderContext;
    Window embeddedWindow;

    int swapFrames;
    Rectangle<int> bounds;
    XVisualInfo* bestVisual;
    void* contextToShareWith;

    OpenGLContext* context;
    DummyComponent dummy;

    ::Display* display;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    ScopedXDisplay xDisplay;
    ::Display* display = xDisplay.get();

    ScopedXLock xlock (display);
    return glXGetCurrentContext() != 0;
}
