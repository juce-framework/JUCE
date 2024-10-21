/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

extern ComponentPeer* createNonRepaintingEmbeddedWindowsPeer (Component&, Component* parent);

//==============================================================================
class OpenGLContext::NativeContext  : private ComponentPeer::ScaleFactorListener
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixelFormat,
                   void* contextToShareWithIn,
                   bool /*useMultisampling*/,
                   OpenGLVersion version)
    {
        placeholderComponent.reset (new PlaceholderComponent (*this));
        createNativeWindow (component);

        PIXELFORMATDESCRIPTOR pfd;
        initialisePixelFormatDescriptor (pfd, pixelFormat);

        auto pixFormat = ChoosePixelFormat (dc.get(), &pfd);

        if (pixFormat != 0)
            SetPixelFormat (dc.get(), pixFormat, &pfd);

        initialiseWGLExtensions (dc.get());
        renderContext.reset (createRenderContext (version, dc.get()));

        if (renderContext != nullptr)
        {
            makeActive();

            auto wglFormat = wglChoosePixelFormatExtension (pixelFormat);
            deactivateCurrentContext();

            if (wglFormat != pixFormat && wglFormat != 0)
            {
                // can't change the pixel format of a window, so need to delete the
                // old one and create a new one.
                dc.reset();
                nativeWindow = nullptr;
                createNativeWindow (component);

                if (SetPixelFormat (dc.get(), wglFormat, &pfd))
                {
                    renderContext.reset();
                    renderContext.reset (createRenderContext (version, dc.get()));
                }
            }

            if (contextToShareWithIn != nullptr)
                wglShareLists ((HGLRC) contextToShareWithIn, renderContext.get());

            component.getTopLevelComponent()->repaint();
            component.repaint();
        }
    }

    ~NativeContext() override
    {
        renderContext.reset();
        dc.reset();

        if (safeComponent != nullptr)
            if (auto* peer = safeComponent->getTopLevelComponent()->getPeer())
                peer->removeScaleFactorListener (this);
    }

    InitResult initialiseOnRenderThread (OpenGLContext& c)
    {
        threadAwarenessSetter = std::make_unique<ScopedThreadDPIAwarenessSetter> (nativeWindow->getNativeHandle());
        context = &c;
        return InitResult::success;
    }

    void shutdownOnRenderThread()
    {
        deactivateCurrentContext();
        context = nullptr;
        threadAwarenessSetter = nullptr;
    }

    static void deactivateCurrentContext()  { wglMakeCurrent (nullptr, nullptr); }
    bool makeActive() const noexcept        { return isActive() || wglMakeCurrent (dc.get(), renderContext.get()) != FALSE; }
    bool isActive() const noexcept          { return wglGetCurrentContext() == renderContext.get(); }
    void swapBuffers() const noexcept       { SwapBuffers (dc.get()); }

    bool setSwapInterval (int numFramesPerSwap)
    {
        jassert (isActive()); // this can only be called when the context is active..
        return wglSwapIntervalEXT != nullptr && wglSwapIntervalEXT (numFramesPerSwap) != FALSE;
    }

    int getSwapInterval() const
    {
        jassert (isActive()); // this can only be called when the context is active..
        return wglGetSwapIntervalEXT != nullptr ? wglGetSwapIntervalEXT() : 0;
    }

    void updateWindowPosition (Rectangle<int> bounds)
    {
        if (nativeWindow != nullptr)
        {
            if (! approximatelyEqual (nativeScaleFactor, 1.0))
                bounds = (bounds.toDouble() * nativeScaleFactor).toNearestInt();

            SetWindowPos ((HWND) nativeWindow->getNativeHandle(), nullptr,
                          bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                          SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    }

    bool createdOk() const noexcept                 { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept            { return renderContext.get(); }
    unsigned int getFrameBufferID() const noexcept  { return 0; }

    void triggerRepaint()
    {
        if (context != nullptr)
            context->triggerRepaint();
    }

    struct Locker
    {
        explicit Locker (NativeContext& ctx) : lock (ctx.mutex) {}
        const ScopedLock lock;
    };

    HWND getNativeHandle()
    {
        if (nativeWindow != nullptr)
            return (HWND) nativeWindow->getNativeHandle();

        return nullptr;
    }

private:
    //==============================================================================
    static void initialiseWGLExtensions (HDC dcIn)
    {
        static bool initialised = false;

        if (initialised)
            return;

        initialised = true;

        const auto dummyContext = wglCreateContext (dcIn);
        wglMakeCurrent (dcIn, dummyContext);

        #define JUCE_INIT_WGL_FUNCTION(name)    name = (type_ ## name) OpenGLHelpers::getExtensionFunction (#name);
        JUCE_INIT_WGL_FUNCTION (wglChoosePixelFormatARB)
        JUCE_INIT_WGL_FUNCTION (wglSwapIntervalEXT)
        JUCE_INIT_WGL_FUNCTION (wglGetSwapIntervalEXT)
        JUCE_INIT_WGL_FUNCTION (wglCreateContextAttribsARB)
        #undef JUCE_INIT_WGL_FUNCTION

        wglMakeCurrent (nullptr, nullptr);
        wglDeleteContext (dummyContext);
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

    static HGLRC createRenderContext (OpenGLVersion version, HDC dcIn)
    {
        const auto components = [&]() -> Optional<Version>
        {
            switch (version)
            {
                case OpenGLVersion::openGL3_2: return Version { 3, 2 };
                case OpenGLVersion::openGL4_1: return Version { 4, 1 };
                case OpenGLVersion::openGL4_3: return Version { 4, 3 };

                case OpenGLVersion::defaultGLVersion: break;
            }

            return {};
        }();

        if (components.hasValue() && wglCreateContextAttribsARB != nullptr)
        {
           #if JUCE_DEBUG
            constexpr auto contextFlags = WGL_CONTEXT_DEBUG_BIT_ARB;
            constexpr auto noErrorChecking = GL_FALSE;
           #else
            constexpr auto contextFlags = 0;
            constexpr auto noErrorChecking = GL_TRUE;
           #endif

            const int attribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB,   components->major,
                WGL_CONTEXT_MINOR_VERSION_ARB,   components->minor,
                WGL_CONTEXT_PROFILE_MASK_ARB,    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_FLAGS_ARB,           contextFlags,
                WGL_CONTEXT_OPENGL_NO_ERROR_ARB, noErrorChecking,
                0
            };

            const auto c = wglCreateContextAttribsARB (dcIn, nullptr, attribs);

            if (c != nullptr)
                return c;
        }

        return wglCreateContext (dcIn);
    }

    //==============================================================================
    struct PlaceholderComponent  : public Component
    {
        explicit PlaceholderComponent (NativeContext& c)
            : context (c)
        {
            setOpaque (true);
        }

        // The windowing code will call this when a paint callback happens
        void handleCommandMessage (int) override   { context.triggerRepaint(); }

        NativeContext& context;
    };

    //==============================================================================
    void nativeScaleFactorChanged (double newScaleFactor) override
    {
        if (approximatelyEqual (newScaleFactor, nativeScaleFactor)
            || safeComponent == nullptr)
            return;

        if (auto* peer = safeComponent->getTopLevelComponent()->getPeer())
        {
            nativeScaleFactor = newScaleFactor;
            updateWindowPosition (peer->getAreaCoveredBy (*safeComponent));
        }
    }

    void createNativeWindow (Component& component)
    {
        auto* topComp = component.getTopLevelComponent();

        {
            auto* parentHWND = topComp->getWindowHandle();

            ScopedThreadDPIAwarenessSetter setter { parentHWND };
            nativeWindow.reset (createNonRepaintingEmbeddedWindowsPeer (*placeholderComponent, topComp));
        }

        if (auto* peer = topComp->getPeer())
        {
            safeComponent = Component::SafePointer<Component> (&component);

            nativeScaleFactor = peer->getPlatformScaleFactor();
            updateWindowPosition (peer->getAreaCoveredBy (component));
            peer->addScaleFactorListener (this);
        }

        nativeWindow->setVisible (true);
        dc = std::unique_ptr<std::remove_pointer_t<HDC>, DeviceContextDeleter> { GetDC ((HWND) nativeWindow->getNativeHandle()),
                                                                                 DeviceContextDeleter { (HWND) nativeWindow->getNativeHandle() } };
    }

    int wglChoosePixelFormatExtension (const OpenGLPixelFormat& pixelFormat) const
    {
        int format = 0;

        if (wglChoosePixelFormatARB != nullptr)
        {
            int atts[64];
            int n = 0;

            atts[n++] = WGL_DRAW_TO_WINDOW_ARB;   atts[n++] = GL_TRUE;
            atts[n++] = WGL_SUPPORT_OPENGL_ARB;   atts[n++] = GL_TRUE;
            atts[n++] = WGL_DOUBLE_BUFFER_ARB;    atts[n++] = GL_TRUE;
            atts[n++] = WGL_PIXEL_TYPE_ARB;       atts[n++] = WGL_TYPE_RGBA_ARB;
            atts[n++] = WGL_ACCELERATION_ARB;
            atts[n++] = WGL_FULL_ACCELERATION_ARB;

            atts[n++] = WGL_COLOR_BITS_ARB;  atts[n++] = pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits;
            atts[n++] = WGL_RED_BITS_ARB;    atts[n++] = pixelFormat.redBits;
            atts[n++] = WGL_GREEN_BITS_ARB;  atts[n++] = pixelFormat.greenBits;
            atts[n++] = WGL_BLUE_BITS_ARB;   atts[n++] = pixelFormat.blueBits;
            atts[n++] = WGL_ALPHA_BITS_ARB;  atts[n++] = pixelFormat.alphaBits;
            atts[n++] = WGL_DEPTH_BITS_ARB;  atts[n++] = pixelFormat.depthBufferBits;

            atts[n++] = WGL_STENCIL_BITS_ARB;       atts[n++] = pixelFormat.stencilBufferBits;
            atts[n++] = WGL_ACCUM_RED_BITS_ARB;     atts[n++] = pixelFormat.accumulationBufferRedBits;
            atts[n++] = WGL_ACCUM_GREEN_BITS_ARB;   atts[n++] = pixelFormat.accumulationBufferGreenBits;
            atts[n++] = WGL_ACCUM_BLUE_BITS_ARB;    atts[n++] = pixelFormat.accumulationBufferBlueBits;
            atts[n++] = WGL_ACCUM_ALPHA_BITS_ARB;   atts[n++] = pixelFormat.accumulationBufferAlphaBits;

            if (pixelFormat.multisamplingLevel > 0
                  && OpenGLHelpers::isExtensionSupported ("GL_ARB_multisample"))
            {
                atts[n++] = WGL_SAMPLE_BUFFERS_ARB;
                atts[n++] = 1;
                atts[n++] = WGL_SAMPLES_ARB;
                atts[n++] = pixelFormat.multisamplingLevel;
            }

            atts[n++] = 0;
            jassert (n <= numElementsInArray (atts));

            UINT formatsCount = 0;
            wglChoosePixelFormatARB (dc.get(), atts, nullptr, 1, &format, &formatsCount);
        }

        return format;
    }

    //==============================================================================
    #define JUCE_DECLARE_WGL_EXTENSION_FUNCTION(name, returnType, params) \
        typedef returnType (__stdcall *type_ ## name) params; static type_ ## name name;

    JUCE_DECLARE_WGL_EXTENSION_FUNCTION (wglChoosePixelFormatARB,    BOOL,  (HDC, const int*, const FLOAT*, UINT, int*, UINT*))
    JUCE_DECLARE_WGL_EXTENSION_FUNCTION (wglSwapIntervalEXT,         BOOL,  (int))
    JUCE_DECLARE_WGL_EXTENSION_FUNCTION (wglGetSwapIntervalEXT,      int,   ())
    JUCE_DECLARE_WGL_EXTENSION_FUNCTION (wglCreateContextAttribsARB, HGLRC, (HDC, HGLRC, const int*))
    #undef JUCE_DECLARE_WGL_EXTENSION_FUNCTION

    //==============================================================================
    struct RenderContextDeleter
    {
        void operator() (HGLRC ptr) const { wglDeleteContext (ptr); }
    };

    struct DeviceContextDeleter
    {
        void operator() (HDC ptr) const { ReleaseDC (hwnd, ptr); }
        HWND hwnd;
    };

    CriticalSection mutex;
    std::unique_ptr<PlaceholderComponent> placeholderComponent;
    std::unique_ptr<ComponentPeer> nativeWindow;
    std::unique_ptr<ScopedThreadDPIAwarenessSetter> threadAwarenessSetter;
    Component::SafePointer<Component> safeComponent;
    std::unique_ptr<std::remove_pointer_t<HGLRC>, RenderContextDeleter> renderContext;
    std::unique_ptr<std::remove_pointer_t<HDC>, DeviceContextDeleter> dc;
    OpenGLContext* context = nullptr;
    double nativeScaleFactor = 1.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};


//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return wglGetCurrentContext() != nullptr;
}

} // namespace juce
