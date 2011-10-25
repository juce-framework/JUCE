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

enum
{
    WGL_NUMBER_PIXEL_FORMATS_ARB    = 0x2000,
    WGL_DRAW_TO_WINDOW_ARB          = 0x2001,
    WGL_ACCELERATION_ARB            = 0x2003,
    WGL_SWAP_METHOD_ARB             = 0x2007,
    WGL_SUPPORT_OPENGL_ARB          = 0x2010,
    WGL_PIXEL_TYPE_ARB              = 0x2013,
    WGL_DOUBLE_BUFFER_ARB           = 0x2011,
    WGL_COLOR_BITS_ARB              = 0x2014,
    WGL_RED_BITS_ARB                = 0x2015,
    WGL_GREEN_BITS_ARB              = 0x2017,
    WGL_BLUE_BITS_ARB               = 0x2019,
    WGL_ALPHA_BITS_ARB              = 0x201B,
    WGL_DEPTH_BITS_ARB              = 0x2022,
    WGL_STENCIL_BITS_ARB            = 0x2023,
    WGL_FULL_ACCELERATION_ARB       = 0x2027,
    WGL_ACCUM_RED_BITS_ARB          = 0x201E,
    WGL_ACCUM_GREEN_BITS_ARB        = 0x201F,
    WGL_ACCUM_BLUE_BITS_ARB         = 0x2020,
    WGL_ACCUM_ALPHA_BITS_ARB        = 0x2021,
    WGL_STEREO_ARB                  = 0x2012,
    WGL_SAMPLE_BUFFERS_ARB          = 0x2041,
    WGL_SAMPLES_ARB                 = 0x2042,
    WGL_TYPE_RGBA_ARB               = 0x202B
};

typedef BOOL (WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC, const int*, const FLOAT*, UINT, int*, UINT*);
typedef BOOL (WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int);
typedef int  (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC)();

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;
static PFNWGLSWAPINTERVALEXTPROC      wglSwapIntervalEXT = 0;
static PFNWGLGETSWAPINTERVALEXTPROC   wglGetSwapIntervalEXT = 0;

static void initialiseGLExtensions()
{
    if (wglChoosePixelFormatARB == 0)
    {
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) OpenGLHelpers::getExtensionFunction ("wglChoosePixelFormatARB");
        wglSwapIntervalEXT      = (PFNWGLSWAPINTERVALEXTPROC)      OpenGLHelpers::getExtensionFunction ("wglSwapIntervalEXT");
        wglGetSwapIntervalEXT   = (PFNWGLGETSWAPINTERVALEXTPROC)   OpenGLHelpers::getExtensionFunction ("wglGetSwapIntervalEXT");
    }
}

extern ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component* component, void* parent);

//==============================================================================
class WindowedGLContext     : public OpenGLContext
{
public:
    WindowedGLContext (Component* const component_,
                       HGLRC contextToShareWith,
                       const OpenGLPixelFormat& pixelFormat)
        : renderContext (0),
          component (component_),
          dc (0)
    {
        initialiseGLExtensions();
        jassert (component != nullptr);

        createNativeWindow();

        PIXELFORMATDESCRIPTOR pfd;
        initialisePixelFormatDescriptor (pfd, pixelFormat);

        const int format = ChoosePixelFormat (dc, &pfd);

        if (format != 0)
            SetPixelFormat (dc, format, &pfd);

        renderContext = wglCreateContext (dc);

        if (renderContext != 0)
        {
            makeActive();
            setPixelFormat (pixelFormat);

            if (contextToShareWith != 0)
                wglShareLists (contextToShareWith, renderContext);
        }
    }

    ~WindowedGLContext()
    {
        deleteContext();
        ReleaseDC ((HWND) nativeWindow->getNativeHandle(), dc);
        nativeWindow = nullptr;
    }

    void deleteContext()
    {
        makeInactive();

        if (renderContext != 0)
        {
            wglDeleteContext (renderContext);
            renderContext = 0;
        }
    }

    bool makeActive() const noexcept
    {
        jassert (renderContext != 0);
        return wglMakeCurrent (dc, renderContext) != 0;
    }

    bool makeInactive() const noexcept
    {
        return (! isActive()) || (wglMakeCurrent (0, 0) != 0);
    }

    bool isActive() const noexcept
    {
        return wglGetCurrentContext() == renderContext;
    }

    void* getRawContext() const noexcept
    {
        return renderContext;
    }

    unsigned int getFrameBufferID() const
    {
        return 0;
    }

    bool setPixelFormat (const OpenGLPixelFormat& pixelFormat)
    {
        makeActive();

        PIXELFORMATDESCRIPTOR pfd;
        initialisePixelFormatDescriptor (pfd, pixelFormat);

        int format = 0;

        if (wglChoosePixelFormatARB != nullptr)
        {
            int atts[64];
            int n = 0;

            atts[n++] = WGL_DRAW_TO_WINDOW_ARB;
            atts[n++] = GL_TRUE;
            atts[n++] = WGL_SUPPORT_OPENGL_ARB;
            atts[n++] = GL_TRUE;
            atts[n++] = WGL_ACCELERATION_ARB;
            atts[n++] = WGL_FULL_ACCELERATION_ARB;
            atts[n++] = WGL_DOUBLE_BUFFER_ARB;
            atts[n++] = GL_TRUE;
            atts[n++] = WGL_PIXEL_TYPE_ARB;
            atts[n++] = WGL_TYPE_RGBA_ARB;

            atts[n++] = WGL_COLOR_BITS_ARB;
            atts[n++] = pfd.cColorBits;
            atts[n++] = WGL_RED_BITS_ARB;
            atts[n++] = pixelFormat.redBits;
            atts[n++] = WGL_GREEN_BITS_ARB;
            atts[n++] = pixelFormat.greenBits;
            atts[n++] = WGL_BLUE_BITS_ARB;
            atts[n++] = pixelFormat.blueBits;
            atts[n++] = WGL_ALPHA_BITS_ARB;
            atts[n++] = pixelFormat.alphaBits;
            atts[n++] = WGL_DEPTH_BITS_ARB;
            atts[n++] = pixelFormat.depthBufferBits;

            atts[n++] = WGL_STENCIL_BITS_ARB;
            atts[n++] = pixelFormat.stencilBufferBits;

            atts[n++] = WGL_ACCUM_RED_BITS_ARB;
            atts[n++] = pixelFormat.accumulationBufferRedBits;
            atts[n++] = WGL_ACCUM_GREEN_BITS_ARB;
            atts[n++] = pixelFormat.accumulationBufferGreenBits;
            atts[n++] = WGL_ACCUM_BLUE_BITS_ARB;
            atts[n++] = pixelFormat.accumulationBufferBlueBits;
            atts[n++] = WGL_ACCUM_ALPHA_BITS_ARB;
            atts[n++] = pixelFormat.accumulationBufferAlphaBits;

            if (pixelFormat.multisamplingLevel > 0
                  && OpenGLHelpers::isExtensionSupported ("WGL_ARB_multisample"))
            {
                atts[n++] = WGL_SAMPLE_BUFFERS_ARB;
                atts[n++] = 1;
                atts[n++] = WGL_SAMPLES_ARB;
                atts[n++] = pixelFormat.multisamplingLevel;
            }

            atts[n++] = 0;
            jassert (n <= numElementsInArray (atts));

            UINT formatsCount;
            wglChoosePixelFormatARB (dc, atts, nullptr, 1, &format, &formatsCount);
        }

        if (format == 0)
            format = ChoosePixelFormat (dc, &pfd);

        if (format != 0)
        {
            makeInactive();

            // win32 can't change the pixel format of a window, so need to delete the
            // old one and create a new one..
            jassert (nativeWindow != 0);
            ReleaseDC ((HWND) nativeWindow->getNativeHandle(), dc);
            nativeWindow = nullptr;

            createNativeWindow();

            if (SetPixelFormat (dc, format, &pfd))
            {
                wglDeleteContext (renderContext);
                renderContext = wglCreateContext (dc);

                jassert (renderContext != 0);
                return renderContext != 0;
            }
        }

        return false;
    }

    void swapBuffers()
    {
        SwapBuffers (dc);
    }

    bool setSwapInterval (int numFramesPerSwap)
    {
        makeActive();
        return wglSwapIntervalEXT != nullptr && wglSwapIntervalEXT (numFramesPerSwap) != FALSE;
    }

    int getSwapInterval() const
    {
        makeActive();
        return wglGetSwapIntervalEXT != nullptr ? wglGetSwapIntervalEXT() : 0;
    }

    void* getNativeWindowHandle() const
    {
        return nativeWindow != nullptr ? nativeWindow->getNativeHandle() : nullptr;
    }

    //==============================================================================
    HGLRC renderContext;
    ScopedPointer<ComponentPeer> nativeWindow;

private:
    Component* const component;
    HDC dc;

    //==============================================================================
    void createNativeWindow()
    {
        nativeWindow = createNonRepaintingEmbeddedWindowsPeer (component, component->getTopLevelComponent()->getWindowHandle());
        nativeWindow->setVisible (true);

        dc = GetDC ((HWND) nativeWindow->getNativeHandle());
    }

    static void initialisePixelFormatDescriptor (PIXELFORMATDESCRIPTOR& pfd, const OpenGLPixelFormat& pixelFormat)
    {
        zerostruct (pfd);
        pfd.nSize           = sizeof (pfd);
        pfd.nVersion        = 1;
        pfd.dwFlags         = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType      = PFD_TYPE_RGBA;
        pfd.iLayerType      = PFD_MAIN_PLANE;
        pfd.cColorBits      = (BYTE) (pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits);
        pfd.cRedBits        = (BYTE) pixelFormat.redBits;
        pfd.cGreenBits      = (BYTE) pixelFormat.greenBits;
        pfd.cBlueBits       = (BYTE) pixelFormat.blueBits;
        pfd.cAlphaBits      = (BYTE) pixelFormat.alphaBits;
        pfd.cDepthBits      = (BYTE) pixelFormat.depthBufferBits;
        pfd.cStencilBits    = (BYTE) pixelFormat.stencilBufferBits;
        pfd.cAccumBits      = (BYTE) (pixelFormat.accumulationBufferRedBits + pixelFormat.accumulationBufferGreenBits
                                        + pixelFormat.accumulationBufferBlueBits + pixelFormat.accumulationBufferAlphaBits);
        pfd.cAccumRedBits   = (BYTE) pixelFormat.accumulationBufferRedBits;
        pfd.cAccumGreenBits = (BYTE) pixelFormat.accumulationBufferGreenBits;
        pfd.cAccumBlueBits  = (BYTE) pixelFormat.accumulationBufferBlueBits;
        pfd.cAccumAlphaBits = (BYTE) pixelFormat.accumulationBufferAlphaBits;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowedGLContext);
};

//==============================================================================
OpenGLContext* OpenGLComponent::createContext()
{
    ScopedPointer<WindowedGLContext> c (new WindowedGLContext (this,
                                                               contextToShareListsWith != nullptr ? (HGLRC) contextToShareListsWith->getRawContext() : 0,
                                                               preferredPixelFormat));

    return (c->renderContext != 0) ? c.release() : nullptr;
}

void* OpenGLComponent::getNativeWindowHandle() const
{
    return context != nullptr ? static_cast<WindowedGLContext*> (context.get())->getNativeWindowHandle() : nullptr;
}

void OpenGLComponent::internalRepaint (int x, int y, int w, int h)
{
    Component::internalRepaint (x, y, w, h);

    if (context != nullptr)
    {
        ComponentPeer* peer = static_cast<WindowedGLContext*> (context.get())->nativeWindow;
        peer->repaint (peer->getBounds().withPosition (Point<int>()));
    }
}

void OpenGLComponent::updateEmbeddedPosition (const Rectangle<int>& bounds)
{
    if (context != nullptr)
    {
        ComponentPeer* peer = static_cast<WindowedGLContext*> (context.get())->nativeWindow;

        SetWindowPos ((HWND) peer->getNativeHandle(), 0,
                      bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                      SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }
}

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return wglGetCurrentContext() != 0;
}
