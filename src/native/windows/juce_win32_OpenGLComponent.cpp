/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_OPENGL


//==============================================================================
#define WGL_EXT_FUNCTION_INIT(extType, extFunc) \
    ((extFunc = (extType) wglGetProcAddress (#extFunc)) != 0)

typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int* piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef int (WINAPI * PFNWGLGETSWAPINTERVALEXTPROC) (void);

#define WGL_NUMBER_PIXEL_FORMATS_ARB    0x2000
#define WGL_DRAW_TO_WINDOW_ARB          0x2001
#define WGL_ACCELERATION_ARB            0x2003
#define WGL_SWAP_METHOD_ARB             0x2007
#define WGL_SUPPORT_OPENGL_ARB          0x2010
#define WGL_PIXEL_TYPE_ARB              0x2013
#define WGL_DOUBLE_BUFFER_ARB           0x2011
#define WGL_COLOR_BITS_ARB              0x2014
#define WGL_RED_BITS_ARB                0x2015
#define WGL_GREEN_BITS_ARB              0x2017
#define WGL_BLUE_BITS_ARB               0x2019
#define WGL_ALPHA_BITS_ARB              0x201B
#define WGL_DEPTH_BITS_ARB              0x2022
#define WGL_STENCIL_BITS_ARB            0x2023
#define WGL_FULL_ACCELERATION_ARB       0x2027
#define WGL_ACCUM_RED_BITS_ARB          0x201E
#define WGL_ACCUM_GREEN_BITS_ARB        0x201F
#define WGL_ACCUM_BLUE_BITS_ARB         0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB        0x2021
#define WGL_STEREO_ARB                  0x2012
#define WGL_SAMPLE_BUFFERS_ARB          0x2041
#define WGL_SAMPLES_ARB                 0x2042
#define WGL_TYPE_RGBA_ARB               0x202B

static void getWglExtensions (HDC dc, StringArray& result) throw()
{
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = 0;

    if (WGL_EXT_FUNCTION_INIT (PFNWGLGETEXTENSIONSSTRINGARBPROC, wglGetExtensionsStringARB))
        result.addTokens (String (wglGetExtensionsStringARB (dc)), false);
    else
        jassertfalse // If this fails, it may be because you didn't activate the openGL context
}


//==============================================================================
class WindowedGLContext     : public OpenGLContext
{
public:
    WindowedGLContext (Component* const component_,
                       HGLRC contextToShareWith,
                       const OpenGLPixelFormat& pixelFormat)
        : renderContext (0),
          nativeWindow (0),
          dc (0),
          component (component_)
    {
        jassert (component != 0);

        createNativeWindow();

        // Use a default pixel format that should be supported everywhere
        PIXELFORMATDESCRIPTOR pfd;
        zerostruct (pfd);
        pfd.nSize = sizeof (pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 16;

        const int format = ChoosePixelFormat (dc, &pfd);

        if (format != 0)
            SetPixelFormat (dc, format, &pfd);

        renderContext = wglCreateContext (dc);
        makeActive();

        setPixelFormat (pixelFormat);

        if (contextToShareWith != 0 && renderContext != 0)
            wglShareLists (contextToShareWith, renderContext);
    }

    ~WindowedGLContext()
    {
        makeInactive();

        wglDeleteContext (renderContext);

        ReleaseDC ((HWND) nativeWindow->getNativeHandle(), dc);
        delete nativeWindow;
    }

    bool makeActive() const throw()
    {
        jassert (renderContext != 0);
        return wglMakeCurrent (dc, renderContext) != 0;
    }

    bool makeInactive() const throw()
    {
        return (! isActive()) || (wglMakeCurrent (0, 0) != 0);
    }

    bool isActive() const throw()
    {
        return wglGetCurrentContext() == renderContext;
    }

    const OpenGLPixelFormat getPixelFormat() const
    {
        OpenGLPixelFormat pf;

        makeActive();
        StringArray availableExtensions;
        getWglExtensions (dc, availableExtensions);

        fillInPixelFormatDetails (GetPixelFormat (dc), pf, availableExtensions);
        return pf;
    }

    void* getRawContext() const throw()
    {
        return renderContext;
    }

    bool setPixelFormat (const OpenGLPixelFormat& pixelFormat)
    {
        makeActive();

        PIXELFORMATDESCRIPTOR pfd;
        zerostruct (pfd);
        pfd.nSize = sizeof (pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.iLayerType = PFD_MAIN_PLANE;
        pfd.cColorBits = (BYTE) (pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits);
        pfd.cRedBits = (BYTE) pixelFormat.redBits;
        pfd.cGreenBits = (BYTE) pixelFormat.greenBits;
        pfd.cBlueBits = (BYTE) pixelFormat.blueBits;
        pfd.cAlphaBits = (BYTE) pixelFormat.alphaBits;
        pfd.cDepthBits = (BYTE) pixelFormat.depthBufferBits;
        pfd.cStencilBits = (BYTE) pixelFormat.stencilBufferBits;
        pfd.cAccumBits = (BYTE) (pixelFormat.accumulationBufferRedBits + pixelFormat.accumulationBufferGreenBits
                                    + pixelFormat.accumulationBufferBlueBits + pixelFormat.accumulationBufferAlphaBits);
        pfd.cAccumRedBits = (BYTE) pixelFormat.accumulationBufferRedBits;
        pfd.cAccumGreenBits = (BYTE) pixelFormat.accumulationBufferGreenBits;
        pfd.cAccumBlueBits = (BYTE) pixelFormat.accumulationBufferBlueBits;
        pfd.cAccumAlphaBits = (BYTE) pixelFormat.accumulationBufferAlphaBits;

        int format = 0;

        PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 0;

        StringArray availableExtensions;
        getWglExtensions (dc, availableExtensions);

        if (availableExtensions.contains ("WGL_ARB_pixel_format")
             && WGL_EXT_FUNCTION_INIT (PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB))
        {
            int attributes[64];
            int n = 0;

            attributes[n++] = WGL_DRAW_TO_WINDOW_ARB;
            attributes[n++] = GL_TRUE;
            attributes[n++] = WGL_SUPPORT_OPENGL_ARB;
            attributes[n++] = GL_TRUE;
            attributes[n++] = WGL_ACCELERATION_ARB;
            attributes[n++] = WGL_FULL_ACCELERATION_ARB;
            attributes[n++] = WGL_DOUBLE_BUFFER_ARB;
            attributes[n++] = GL_TRUE;
            attributes[n++] = WGL_PIXEL_TYPE_ARB;
            attributes[n++] = WGL_TYPE_RGBA_ARB;

            attributes[n++] = WGL_COLOR_BITS_ARB;
            attributes[n++] = pfd.cColorBits;
            attributes[n++] = WGL_RED_BITS_ARB;
            attributes[n++] = pixelFormat.redBits;
            attributes[n++] = WGL_GREEN_BITS_ARB;
            attributes[n++] = pixelFormat.greenBits;
            attributes[n++] = WGL_BLUE_BITS_ARB;
            attributes[n++] = pixelFormat.blueBits;
            attributes[n++] = WGL_ALPHA_BITS_ARB;
            attributes[n++] = pixelFormat.alphaBits;
            attributes[n++] = WGL_DEPTH_BITS_ARB;
            attributes[n++] = pixelFormat.depthBufferBits;

            if (pixelFormat.stencilBufferBits > 0)
            {
                attributes[n++] = WGL_STENCIL_BITS_ARB;
                attributes[n++] = pixelFormat.stencilBufferBits;
            }

            attributes[n++] = WGL_ACCUM_RED_BITS_ARB;
            attributes[n++] = pixelFormat.accumulationBufferRedBits;
            attributes[n++] = WGL_ACCUM_GREEN_BITS_ARB;
            attributes[n++] = pixelFormat.accumulationBufferGreenBits;
            attributes[n++] = WGL_ACCUM_BLUE_BITS_ARB;
            attributes[n++] = pixelFormat.accumulationBufferBlueBits;
            attributes[n++] = WGL_ACCUM_ALPHA_BITS_ARB;
            attributes[n++] = pixelFormat.accumulationBufferAlphaBits;

            if (availableExtensions.contains ("WGL_ARB_multisample")
                 && pixelFormat.fullSceneAntiAliasingNumSamples > 0)
            {
                attributes[n++] = WGL_SAMPLE_BUFFERS_ARB;
                attributes[n++] = 1;
                attributes[n++] = WGL_SAMPLES_ARB;
                attributes[n++] = pixelFormat.fullSceneAntiAliasingNumSamples;
            }

            attributes[n++] = 0;

            UINT formatsCount;
            const BOOL ok = wglChoosePixelFormatARB (dc, attributes, 0, 1, &format, &formatsCount);
            (void) ok;
            jassert (ok);
        }
        else
        {
            format = ChoosePixelFormat (dc, &pfd);
        }

        if (format != 0)
        {
            makeInactive();

            // win32 can't change the pixel format of a window, so need to delete the
            // old one and create a new one..
            jassert (nativeWindow != 0);
            ReleaseDC ((HWND) nativeWindow->getNativeHandle(), dc);
            delete nativeWindow;

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

    void updateWindowPosition (int x, int y, int w, int h, int)
    {
        SetWindowPos ((HWND) nativeWindow->getNativeHandle(), 0,
                      x, y, w, h,
                      SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }

    void repaint()
    {
        int x, y, w, h;
        nativeWindow->getBounds (x, y, w, h);
        nativeWindow->repaint (0, 0, w, h);
    }

    void swapBuffers()
    {
        SwapBuffers (dc);
    }

    bool setSwapInterval (const int numFramesPerSwap)
    {
        makeActive();

        StringArray availableExtensions;
        getWglExtensions (dc, availableExtensions);

        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

        return availableExtensions.contains ("WGL_EXT_swap_control")
                && WGL_EXT_FUNCTION_INIT (PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT)
                && wglSwapIntervalEXT (numFramesPerSwap) != FALSE;
    }

    int getSwapInterval() const
    {
        makeActive();

        StringArray availableExtensions;
        getWglExtensions (dc, availableExtensions);

        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = 0;

        if (availableExtensions.contains ("WGL_EXT_swap_control")
             && WGL_EXT_FUNCTION_INIT (PFNWGLGETSWAPINTERVALEXTPROC, wglGetSwapIntervalEXT))
            return wglGetSwapIntervalEXT();

        return 0;
    }

    void findAlternativeOpenGLPixelFormats (OwnedArray <OpenGLPixelFormat>& results)
    {
        jassert (isActive());

        StringArray availableExtensions;
        getWglExtensions (dc, availableExtensions);

        PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = 0;
        int numTypes = 0;

        if (availableExtensions.contains("WGL_ARB_pixel_format")
             && WGL_EXT_FUNCTION_INIT (PFNWGLGETPIXELFORMATATTRIBIVARBPROC, wglGetPixelFormatAttribivARB))
        {
            int attributes = WGL_NUMBER_PIXEL_FORMATS_ARB;

            if (! wglGetPixelFormatAttribivARB (dc, 1, 0, 1, &attributes, &numTypes))
                jassertfalse
        }
        else
        {
            numTypes = DescribePixelFormat (dc, 0, 0, 0);
        }

        OpenGLPixelFormat pf;

        for (int i = 0; i < numTypes; ++i)
        {
            if (fillInPixelFormatDetails (i + 1, pf, availableExtensions))
            {
                bool alreadyListed = false;
                for (int j = results.size(); --j >= 0;)
                    if (pf == *results.getUnchecked(j))
                        alreadyListed = true;

                if (! alreadyListed)
                    results.add (new OpenGLPixelFormat (pf));
            }
        }
    }

    void* getNativeWindowHandle() const
    {
        return nativeWindow != 0 ? nativeWindow->getNativeHandle() : 0;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    HGLRC renderContext;

private:
    Win32ComponentPeer* nativeWindow;
    Component* const component;
    HDC dc;

    //==============================================================================
    void createNativeWindow()
    {
        nativeWindow = new Win32ComponentPeer (component, 0);
        nativeWindow->dontRepaint = true;
        nativeWindow->setVisible (true);

        HWND hwnd = (HWND) nativeWindow->getNativeHandle();

        Win32ComponentPeer* const peer = dynamic_cast <Win32ComponentPeer*> (component->getTopLevelComponent()->getPeer());

        if (peer != 0)
        {
            SetParent (hwnd, (HWND) peer->getNativeHandle());
            juce_setWindowStyleBit (hwnd, GWL_STYLE, WS_CHILD, true);
            juce_setWindowStyleBit (hwnd, GWL_STYLE, WS_POPUP, false);
        }

        dc = GetDC (hwnd);
    }

    bool fillInPixelFormatDetails (const int pixelFormatIndex,
                                   OpenGLPixelFormat& result,
                                   const StringArray& availableExtensions) const throw()
    {
        PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = 0;

        if (availableExtensions.contains ("WGL_ARB_pixel_format")
             && WGL_EXT_FUNCTION_INIT (PFNWGLGETPIXELFORMATATTRIBIVARBPROC, wglGetPixelFormatAttribivARB))
        {
            int attributes[32];
            int numAttributes = 0;

            attributes[numAttributes++] = WGL_DRAW_TO_WINDOW_ARB;
            attributes[numAttributes++] = WGL_SUPPORT_OPENGL_ARB;
            attributes[numAttributes++] = WGL_ACCELERATION_ARB;
            attributes[numAttributes++] = WGL_DOUBLE_BUFFER_ARB;
            attributes[numAttributes++] = WGL_PIXEL_TYPE_ARB;
            attributes[numAttributes++] = WGL_RED_BITS_ARB;
            attributes[numAttributes++] = WGL_GREEN_BITS_ARB;
            attributes[numAttributes++] = WGL_BLUE_BITS_ARB;
            attributes[numAttributes++] = WGL_ALPHA_BITS_ARB;
            attributes[numAttributes++] = WGL_DEPTH_BITS_ARB;
            attributes[numAttributes++] = WGL_STENCIL_BITS_ARB;
            attributes[numAttributes++] = WGL_ACCUM_RED_BITS_ARB;
            attributes[numAttributes++] = WGL_ACCUM_GREEN_BITS_ARB;
            attributes[numAttributes++] = WGL_ACCUM_BLUE_BITS_ARB;
            attributes[numAttributes++] = WGL_ACCUM_ALPHA_BITS_ARB;

            if (availableExtensions.contains ("WGL_ARB_multisample"))
                attributes[numAttributes++] = WGL_SAMPLES_ARB;

            int values[32];
            zeromem (values, sizeof (values));

            if (wglGetPixelFormatAttribivARB (dc, pixelFormatIndex, 0, numAttributes, attributes, values))
            {
                int n = 0;
                bool isValidFormat = (values[n++] == GL_TRUE);      // WGL_DRAW_TO_WINDOW_ARB
                isValidFormat = (values[n++] == GL_TRUE) && isValidFormat;   // WGL_SUPPORT_OPENGL_ARB
                isValidFormat = (values[n++] == WGL_FULL_ACCELERATION_ARB) && isValidFormat; // WGL_ACCELERATION_ARB
                isValidFormat = (values[n++] == GL_TRUE) && isValidFormat;   // WGL_DOUBLE_BUFFER_ARB:
                isValidFormat = (values[n++] == WGL_TYPE_RGBA_ARB) && isValidFormat; // WGL_PIXEL_TYPE_ARB
                result.redBits = values[n++];           // WGL_RED_BITS_ARB
                result.greenBits = values[n++];         // WGL_GREEN_BITS_ARB
                result.blueBits = values[n++];          // WGL_BLUE_BITS_ARB
                result.alphaBits = values[n++];         // WGL_ALPHA_BITS_ARB
                result.depthBufferBits = values[n++];   // WGL_DEPTH_BITS_ARB
                result.stencilBufferBits = values[n++]; // WGL_STENCIL_BITS_ARB
                result.accumulationBufferRedBits = values[n++];      // WGL_ACCUM_RED_BITS_ARB
                result.accumulationBufferGreenBits = values[n++];    // WGL_ACCUM_GREEN_BITS_ARB
                result.accumulationBufferBlueBits = values[n++];     // WGL_ACCUM_BLUE_BITS_ARB
                result.accumulationBufferAlphaBits = values[n++];    // WGL_ACCUM_ALPHA_BITS_ARB
                result.fullSceneAntiAliasingNumSamples = (uint8) values[n++];       // WGL_SAMPLES_ARB

                return isValidFormat;
            }
            else
            {
                jassertfalse
            }
        }
        else
        {
            PIXELFORMATDESCRIPTOR pfd;

            if (DescribePixelFormat (dc, pixelFormatIndex, sizeof (pfd), &pfd))
            {
                result.redBits = pfd.cRedBits;
                result.greenBits = pfd.cGreenBits;
                result.blueBits = pfd.cBlueBits;
                result.alphaBits = pfd.cAlphaBits;
                result.depthBufferBits = pfd.cDepthBits;
                result.stencilBufferBits = pfd.cStencilBits;
                result.accumulationBufferRedBits = pfd.cAccumRedBits;
                result.accumulationBufferGreenBits = pfd.cAccumGreenBits;
                result.accumulationBufferBlueBits = pfd.cAccumBlueBits;
                result.accumulationBufferAlphaBits = pfd.cAccumAlphaBits;
                result.fullSceneAntiAliasingNumSamples = 0;

                return true;
            }
            else
            {
                jassertfalse
            }
        }

        return false;
    }

    WindowedGLContext (const WindowedGLContext&);
    const WindowedGLContext& operator= (const WindowedGLContext&);
};

//==============================================================================
OpenGLContext* OpenGLContext::createContextForWindow (Component* const component,
                                                      const OpenGLPixelFormat& pixelFormat,
                                                      const OpenGLContext* const contextToShareWith)
{
    WindowedGLContext* c = new WindowedGLContext (component,
                                                  contextToShareWith != 0 ? (HGLRC) contextToShareWith->getRawContext() : 0,
                                                  pixelFormat);

    if (c->renderContext == 0)
        deleteAndZero (c);

    return c;
}

void* OpenGLComponent::getNativeWindowHandle() const
{
    return context != 0 ? ((WindowedGLContext*) context)->getNativeWindowHandle() : 0;
}

void juce_glViewport (const int w, const int h)
{
    glViewport (0, 0, w, h);
}

void OpenGLPixelFormat::getAvailablePixelFormats (Component* component,
                                                  OwnedArray <OpenGLPixelFormat>& results)
{
    Component tempComp;

    {
        WindowedGLContext wc (component, 0, OpenGLPixelFormat (8, 8, 16, 0));
        wc.makeActive();
        wc.findAlternativeOpenGLPixelFormats (results);
    }
}


#endif
