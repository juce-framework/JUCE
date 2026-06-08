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
class OpenGLContext::NativeContext  : private AsyncUpdater
{
public:
    NativeContext (Component& component,
                   const OpenGLPixelFormat& pixelFormat,
                   void* contextToShareWithIn,
                   bool /*useMultisampling*/,
                   OpenGLVersion version)
        : safeComponent (&component),
          sharedContext (contextToShareWithIn)
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

            component.getTopLevelComponent()->repaint();
            component.repaint();
        }
    }

    ~NativeContext() override
    {
        cancelPendingUpdate();
        renderContext.reset();
        dc.reset();
    }

    InitResult initialiseOnRenderThread (OpenGLContext& c)
    {
        threadAwarenessSetter.emplace (nativeWindow->getNativeHandle());
        context = &c;

        if (sharedContext != nullptr)
        {
            if (! wglShareLists ((HGLRC) sharedContext, renderContext.get()))
            {
                TCHAR messageBuffer[256] = {};

                FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               nullptr,
                               GetLastError(),
                               MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                               messageBuffer,
                               (DWORD) numElementsInArray (messageBuffer) - 1,
                               nullptr);

                DBG (messageBuffer);
                jassertfalse;
            }
        }

        return InitResult::success;
    }

    void shutdownOnRenderThread()
    {
        deactivateCurrentContext();
        context = nullptr;
        threadAwarenessSetter.reset();
    }

    static void deactivateCurrentContext()  { wglMakeCurrent (nullptr, nullptr); }
    bool makeActive() const noexcept        { return isActive() || wglMakeCurrent (dc.get(), renderContext.get()) != FALSE; }
    bool isActive() const noexcept          { return wglGetCurrentContext() == renderContext.get(); }

    void swapBuffers() noexcept
    {
        SwapBuffers (dc.get());

        if (! std::exchange (haveBuffersBeenSwapped, true))
            triggerAsyncUpdate();
    }

    bool setSwapInterval (int numFramesPerSwap)
    {
        jassert (isActive()); // this can only be called when the context is active
        return wglSwapIntervalEXT != nullptr && wglSwapIntervalEXT (numFramesPerSwap) != FALSE;
    }

    int getSwapInterval() const
    {
        jassert (isActive()); // this can only be called when the context is active
        return wglGetSwapIntervalEXT != nullptr ? wglGetSwapIntervalEXT() : 0;
    }

    void updateWindowPosition()
    {
        if (nativeWindow != nullptr)
        {
            const auto bounds = getPhysicalBounds();

            const ScopedThreadDPIAwarenessSetter scope { nativeWindow->getNativeHandle() };

            SetWindowPos ((HWND) nativeWindow->getNativeHandle(),
                          nullptr,
                          bounds.getX(),
                          bounds.getY(),
                          bounds.getWidth(),
                          bounds.getHeight(),
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

    void addListener (NativeContextListener&) {}
    void removeListener (NativeContextListener&) {}

private:
    //==============================================================================
    Rectangle<int> getPhysicalBounds() const
    {
        if (safeComponent == nullptr)
            return {};

        auto& component = *safeComponent;

        if (auto* peer = component.getPeer())
        {
            const auto peerBounds = peer->getAreaCoveredBy (component);
            const auto physicalBounds = peerBounds.toDouble() * peer->getPlatformScaleFactor();
            return physicalBounds.toNearestInt();
        }

        return component.getBounds();
    }

    void handleAsyncUpdate() override
    {
        nativeWindow->setVisible (true);
    }

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
        const auto components = std::invoke ([&]() -> Optional<Version>
        {
            switch (version)
            {
                case openGL3_2: return Version { 3, 2 };
                case openGL4_1: return Version { 4, 1 };
                case openGL4_3: return Version { 4, 3 };

                case defaultGLVersion: break;
            }

            return {};
        });

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
    void nativeScaleFactorChanged (double newScaleFactor)
    {
        if (approximatelyEqual (newScaleFactor, nativeScaleFactor)
            || safeComponent == nullptr)
            return;

        nativeScaleFactor = newScaleFactor;
        updateWindowPosition();
    }

    void createNativeWindow (Component& component)
    {
        safeComponent = &component;

        auto* topComp = component.getTopLevelComponent();

        {
            auto* parentHWND = topComp->getWindowHandle();

            ScopedThreadDPIAwarenessSetter setter { parentHWND };
            nativeWindow.reset (createNonRepaintingEmbeddedWindowsPeer (*placeholderComponent, topComp));
        }

        if (auto* peer = topComp->getPeer())
        {
            nativeScaleFactor = peer->getPlatformScaleFactor();
            updateWindowPosition();
        }

        dc = { GetDC ((HWND) nativeWindow->getNativeHandle()),
               DeviceContextDeleter { (HWND) nativeWindow->getNativeHandle() } };
    }

    int wglChoosePixelFormatExtension (const OpenGLPixelFormat& pixelFormat) const
    {
        int format = 0;

        if (wglChoosePixelFormatARB != nullptr)
        {
            int atts[64];
            auto* ptr = atts;

            const int common[]
            {
                WGL_DRAW_TO_WINDOW_ARB,   GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB,   GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB,    GL_TRUE,
                WGL_PIXEL_TYPE_ARB,       WGL_TYPE_RGBA_ARB,
                WGL_ACCELERATION_ARB,     WGL_FULL_ACCELERATION_ARB,

                WGL_COLOR_BITS_ARB,       pixelFormat.redBits + pixelFormat.greenBits + pixelFormat.blueBits,
                WGL_RED_BITS_ARB,         pixelFormat.redBits,
                WGL_GREEN_BITS_ARB,       pixelFormat.greenBits,
                WGL_BLUE_BITS_ARB,        pixelFormat.blueBits,
                WGL_ALPHA_BITS_ARB,       pixelFormat.alphaBits,
                WGL_DEPTH_BITS_ARB,       pixelFormat.depthBufferBits,

                WGL_STENCIL_BITS_ARB,     pixelFormat.stencilBufferBits,
                WGL_ACCUM_RED_BITS_ARB,   pixelFormat.accumulationBufferRedBits,
                WGL_ACCUM_GREEN_BITS_ARB, pixelFormat.accumulationBufferGreenBits,
                WGL_ACCUM_BLUE_BITS_ARB,  pixelFormat.accumulationBufferBlueBits,
                WGL_ACCUM_ALPHA_BITS_ARB, pixelFormat.accumulationBufferAlphaBits,
            };

            ptr = std::copy (std::begin (common), std::end (common), ptr);

            if (pixelFormat.multisamplingLevel > 0
                  && OpenGLHelpers::isExtensionSupported ("GL_ARB_multisample"))
            {
                const int multisample[]
                {
                    WGL_SAMPLE_BUFFERS_ARB, 1,
                    WGL_SAMPLES_ARB,        pixelFormat.multisamplingLevel,
                };

                ptr = std::copy (std::begin (multisample), std::end (multisample), ptr);
            }

            *ptr++ = 0;
            jassert (std::distance (atts, ptr) <= numElementsInArray (atts));

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
    std::optional<ScopedThreadDPIAwarenessSetter> threadAwarenessSetter;
    Component::SafePointer<Component> safeComponent;
    std::unique_ptr<std::remove_pointer_t<HGLRC>, RenderContextDeleter> renderContext;
    std::unique_ptr<std::remove_pointer_t<HDC>, DeviceContextDeleter> dc;
    OpenGLContext* context = nullptr;
    void* sharedContext = nullptr;
    double nativeScaleFactor = 1.0;
    bool haveBuffersBeenSwapped = false;
    NativeScaleFactorNotifier scaleFactorNotifier { safeComponent.getComponent(),
                                                    [this] (auto x) { nativeScaleFactorChanged (x); } };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};


//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return wglGetCurrentContext() != nullptr;
}

} // namespace juce
