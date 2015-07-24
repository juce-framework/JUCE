/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

extern ::Display* display;
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
        ScopedXLock xlock;
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
            ScopedXLock xlock;
            XUnmapWindow (display, embeddedWindow);
            XDestroyWindow (display, embeddedWindow);
        }

        if (bestVisual != nullptr)
            XFree (bestVisual);
    }

    void initialiseOnRenderThread (OpenGLContext& c)
    {
        ScopedXLock xlock;
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

        ScopedXLock xlock;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    ScopedXLock xlock;
    return glXGetCurrentContext() != 0;
}
