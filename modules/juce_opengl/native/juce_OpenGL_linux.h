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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

//==============================================================================
// Defined in juce_Windowing_linux.cpp
void juce_LinuxAddRepaintListener (ComponentPeer*, Component* dummy);
void juce_LinuxRemoveRepaintListener (ComponentPeer*, Component* dummy);

bool OpenGLHelpers::isOpenGLES()
{
    return eglQueryAPI() == EGL_OPENGL_ES_API;
}

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")
static constexpr EGLContext nullContext = EGL_NO_CONTEXT;
static constexpr EGLDisplay nullDisplay = EGL_NO_DISPLAY;
static constexpr EGLSurface nullSurface = EGL_NO_SURFACE;
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
class OpenGLContext::NativeContext
{
private:
    struct DummyComponent  : public Component
    {
        explicit DummyComponent (NativeContext& nativeParentContext)
            : native (nativeParentContext)
        {
        }

        void handleCommandMessage (int commandId) override
        {
            if (commandId == 0)
                native.triggerRepaint();
        }

        NativeContext& native;
    };

    using PtrEGLContext = EGLHelpers::PtrEGLContext;
    using PtrEGLSurface = EGLHelpers::PtrEGLSurface;

public:
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& cPixelFormat,
                   void* shareContext,
                   bool useMultisamplingIn,
                   API apiIn,
                   Version versionIn,
                   Profile profileIn)
        : component (comp),
          contextToShareWith (shareContext),
          dummy (*this),
          api (apiIn),
          version (versionIn),
          profile (profileIn)
    {
        auto* peer = component.getPeer();

        if (peer == nullptr)
        {
            // Attach an OpenGL context only after the component has a native peer.
            jassertfalse;
            return;
        }

        const auto usesWayland = isWaylandComponentPeer (peer);
        const auto* ext = eglQueryString (nullDisplay, EGL_EXTENSIONS);

        if (ext == nullptr)
        {
            // The EGL implementation must report its client extensions before JUCE can
            // select a native display type.
            jassertfalse;
            return;
        }

        const auto platformDisplayToken = std::invoke ([&]() -> std::optional<EGLenum>
        {
            if (usesWayland)
            {
                if (strstr (ext, "EGL_KHR_platform_wayland") != nullptr)
                    return EGL_PLATFORM_WAYLAND_KHR;

                if (strstr (ext, "EGL_EXT_platform_wayland") != nullptr)
                    return EGL_PLATFORM_WAYLAND_EXT;

                return {};
            }

            if (strstr (ext, "EGL_KHR_platform_x11") != nullptr)
                return EGL_PLATFORM_X11_KHR;

            if (strstr (ext, "EGL_EXT_platform_x11") != nullptr)
                return EGL_PLATFORM_X11_EXT;

            return {};
        });

        if (! platformDisplayToken.has_value())
        {
            // The EGL implementation must support the platform used by the component peer.
            jassertfalse;
            return;
        }

        std::unique_ptr<WaylandOpenGLSurface> waylandSurface;
        std::optional<XWindowSystemUtilities::ScopedXLock> xLock;
        void* nativeDisplay = nullptr;
        ::Display* xDisplay = nullptr;

        if (usesWayland)
        {
            waylandSurface = createWaylandOpenGLSurface (component);

            if (waylandSurface == nullptr)
                return;

            nativeDisplay = waylandSurface->getDisplay();
        }
        else
        {
            xDisplay = XWindowSystem::getInstance()->getDisplay();

            if (xDisplay == nullptr)
                return;

            xLock.emplace();
            X11Symbols::getInstance()->xSync (xDisplay, False);
            nativeDisplay = xDisplay;
        }

        eglDisplay = eglGetPlatformDisplay (*platformDisplayToken, nativeDisplay, nullptr);

        if (eglDisplay == nullDisplay)
            return;

        {
            EGLint major = 0, minor = 0;

            if (! eglInitialize (eglDisplay, &major, &minor))
                return;
        }

        const EGLint optionalAttribs[]
        {
            EGL_SAMPLE_BUFFERS, useMultisamplingIn ? 1 : 0,
            EGL_SAMPLES,        cPixelFormat.multisamplingLevel
        };

        if (! tryChooseConfig (cPixelFormat, optionalAttribs) && ! tryChooseConfig (cPixelFormat, {}))
            return;

        xLock.reset();

        if (usesWayland)
        {
            auto& window = nativeWindow.emplace<WaylandOpenGLWindow> (std::move (waylandSurface));

            if (! window.isValid())
                return;
        }
        else
        {
            auto& window = nativeWindow.emplace<X11OpenGLWindow> (component, *peer, xDisplay,
                                                                  eglDisplay, eglConfig);

            if (! window.isValid())
                return;
        }

        juce_LinuxAddRepaintListener (peer, &dummy);

        constructorDidComplete = true;
    }

    ~NativeContext()
    {
        eglSurface.reset();
        renderContext.reset();

        if (eglDisplay != nullDisplay)
            eglTerminate (eglDisplay);

        if (auto* peer = component.getPeer())
            juce_LinuxRemoveRepaintListener (peer, &dummy);
    }

    InitResult initialiseOnRenderThread (OpenGLContext& c)
    {
        renderContext = EGLHelpers::initEGLContext (api, version, profile, eglDisplay, eglConfig, contextToShareWith);

        if (renderContext == nullptr)
            return InitResult::fatal;

        eglSurface = PtrEGLSurface { eglCreatePlatformWindowSurface (eglDisplay,
                                                                     eglConfig,
                                                                     getNativeWindow(),
                                                                     nullptr),
                                     eglDisplay };

        if (eglSurface == nullptr)
            return InitResult::fatal;

        c.makeActive();
        context = &c;
        return InitResult::success;
    }

    void shutdownOnRenderThread()
    {
        context = nullptr;
        deactivateCurrentContext();
        renderContext.reset();
        eglSurface.reset();
    }

    bool makeActive() const noexcept
    {
        return renderContext != nullptr
                 && eglSurface != nullptr
                 && eglMakeCurrent (eglDisplay, eglSurface.get(), eglSurface.get(), renderContext.get());
    }

    bool isActive() const noexcept
    {
        return eglGetCurrentContext() == renderContext.get() && renderContext != nullptr;
    }

    static void deactivateCurrentContext()
    {
        const auto currentDisplay = eglGetCurrentDisplay();

        if (currentDisplay != nullDisplay)
            eglMakeCurrent (currentDisplay, nullSurface, nullSurface, nullContext);
    }

    void swapBuffers()
    {
        auto* wayland = std::get_if<WaylandOpenGLWindow> (&nativeWindow);

        if (wayland == nullptr)
        {
            eglSwapBuffers (eglDisplay, eglSurface.get());
            return;
        }

        const std::scoped_lock lock { waylandWindowMutex };

        if (! wayland->prepareForSwap (swapFrames > 0))
            return;

        eglSwapBuffers (eglDisplay, eglSurface.get());

        // Request a frame for the new size on the message thread.
        if (wayland->finishSwap())
            dummy.postCommandMessage (0);
    }

    void updateWindowPosition()
    {
        if (auto* x11 = std::get_if<X11OpenGLWindow> (&nativeWindow))
        {
            x11->updateBounds();
        }
        else if (auto* wayland = std::get_if<WaylandOpenGLWindow> (&nativeWindow))
        {
            const auto becameVisible = std::invoke ([&]
            {
                const std::scoped_lock lock { waylandWindowMutex };
                return wayland->updateBounds();
            });

            if (becameVisible)
                triggerRepaint();
        }
    }

    bool setSwapInterval (int numFramesPerSwap)
    {
        if (numFramesPerSwap == swapFrames)
            return true;

        if (std::holds_alternative<WaylandOpenGLWindow> (nativeWindow) && numFramesPerSwap > 1)
            return false;

        swapFrames = numFramesPerSwap;

        if (std::holds_alternative<WaylandOpenGLWindow> (nativeWindow))
        {
            // JUCE throttles Wayland rendering with wl_surface.frame callbacks, so disable
            // EGL throttling to prevent hidden surfaces from blocking the render thread.
            eglSwapInterval (eglDisplay, 0);
        }
        else
        {
            eglSwapInterval (eglDisplay, numFramesPerSwap);
        }

        return true;
    }

    bool isReadyForRender()
    {
        if (auto* wayland = std::get_if<WaylandOpenGLWindow> (&nativeWindow))
        {
            const std::scoped_lock lock { waylandWindowMutex };
            return wayland->isReadyForRender (swapFrames > 0);
        }

        return true;
    }

    void setFrameReadyCallback (std::function<void()> callback)
    {
        if (auto* wayland = std::get_if<WaylandOpenGLWindow> (&nativeWindow))
            wayland->setFrameReadyCallback (std::move (callback));
    }

    int getSwapInterval() const                 { return swapFrames; }
    bool createdOk() const noexcept             { return constructorDidComplete; }
    void* getRawContext() const noexcept        { return renderContext.get(); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

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

private:
    void* getNativeWindow()
    {
        if (auto* x11 = std::get_if<X11OpenGLWindow> (&nativeWindow))
            return x11->getNativeWindow();

        if (auto* wayland = std::get_if<WaylandOpenGLWindow> (&nativeWindow))
            return wayland->getNativeWindow();

        return nullptr;
    }

    bool tryChooseConfig (const OpenGLPixelFormat& format, Span<const EGLint> optionalAttribs)
    {
        std::vector<EGLint> allAttribs
        {
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, api == OpenGLAPI::openGLES ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_BIT,
            EGL_RED_SIZE,        format.redBits,
            EGL_GREEN_SIZE,      format.greenBits,
            EGL_BLUE_SIZE,       format.blueBits,
            EGL_ALPHA_SIZE,      format.alphaBits,
            EGL_DEPTH_SIZE,      format.depthBufferBits,
            EGL_STENCIL_SIZE,    format.stencilBufferBits,
        };

        allAttribs.insert (allAttribs.end(), optionalAttribs.begin(), optionalAttribs.end());

        allAttribs.push_back (EGL_NONE);

        EGLint numConfigs = 0;
        return eglChooseConfig (eglDisplay, allAttribs.data(), &eglConfig, 1, &numConfigs) && numConfigs > 0;
    }

    CriticalSection mutex;
    std::mutex waylandWindowMutex;
    Component& component;

    EGLDisplay eglDisplay = nullDisplay;
    PtrEGLContext renderContext;
    PtrEGLSurface eglSurface;

    std::variant<std::monostate, X11OpenGLWindow, WaylandOpenGLWindow> nativeWindow;

    int swapFrames = 0;
    EGLConfig eglConfig = nullptr;
    void* contextToShareWith;

    OpenGLContext* context = nullptr;
    DummyComponent dummy;

    API api{};
    Version version{};
    Profile profile{};

    bool constructorDidComplete = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return eglGetCurrentContext() != nullContext;
}

} // namespace juce
