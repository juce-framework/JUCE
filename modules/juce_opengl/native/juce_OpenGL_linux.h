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

extern Display* display;
extern XContext windowHandleXContext;

//==============================================================================
class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixelFormat,
                   void* shareContext,
                   bool /*useMultisampling*/,
                   OpenGLVersion)
        : renderContext (0), embeddedWindow (0), swapFrames (0), bestVisual (0),
          contextToShareWith (shareContext)
    {
        ScopedXLock xlock;
        XSync (display, False);

        GLint attribs[] =
        {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_RED_SIZE,         pixelFormat.redBits,
            GLX_GREEN_SIZE,       pixelFormat.greenBits,
            GLX_BLUE_SIZE,        pixelFormat.blueBits,
            GLX_ALPHA_SIZE,       pixelFormat.alphaBits,
            GLX_DEPTH_SIZE,       pixelFormat.depthBufferBits,
            GLX_STENCIL_SIZE,     pixelFormat.stencilBufferBits,
            GLX_ACCUM_RED_SIZE,   pixelFormat.accumulationBufferRedBits,
            GLX_ACCUM_GREEN_SIZE, pixelFormat.accumulationBufferGreenBits,
            GLX_ACCUM_BLUE_SIZE,  pixelFormat.accumulationBufferBlueBits,
            GLX_ACCUM_ALPHA_SIZE, pixelFormat.accumulationBufferAlphaBits,
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

        const Rectangle<int> bounds (component.getTopLevelComponent()
                                        ->getLocalArea (&component, component.getLocalBounds()));

        embeddedWindow = XCreateWindow (display, windowH,
                                        bounds.getX(), bounds.getY(),
                                        jmax (1, bounds.getWidth()),
                                        jmax (1, bounds.getHeight()),
                                        0, bestVisual->depth,
                                        InputOutput,
                                        bestVisual->visual,
                                        CWBorderPixel | CWColormap | CWEventMask,
                                        &swa);

        XSaveContext (display, (XID) embeddedWindow, windowHandleXContext, (XPointer) peer);

        XMapWindow (display, embeddedWindow);
        XFreeColormap (display, colourMap);

        XSync (display, False);
    }

    ~NativeContext()
    {
        if (embeddedWindow != 0)
        {
            ScopedXLock xlock;
            XUnmapWindow (display, embeddedWindow);
            XDestroyWindow (display, embeddedWindow);
        }

        if (bestVisual != nullptr)
            XFree (bestVisual);
    }

    void initialiseOnRenderThread (OpenGLContext& context)
    {
        ScopedXLock xlock;
        renderContext = glXCreateContext (display, bestVisual, (GLXContext) contextToShareWith, GL_TRUE);
        context.makeActive();
    }

    void shutdownOnRenderThread()
    {
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

        ScopedXLock xlock;
        XMoveResizeWindow (display, embeddedWindow,
                           bounds.getX(), bounds.getY(),
                           jmax (1, bounds.getWidth()),
                           jmax (1, bounds.getHeight()));
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

    struct Locker { Locker (NativeContext&) {} };

private:
    GLXContext renderContext;
    Window embeddedWindow;

    int swapFrames;
    Rectangle<int> bounds;
    XVisualInfo* bestVisual;
    void* contextToShareWith;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    ScopedXLock xlock;
    return glXGetCurrentContext() != 0;
}
