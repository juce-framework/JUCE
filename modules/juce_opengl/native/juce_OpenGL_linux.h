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

struct XFreeDeleter
{
    void operator() (void* ptr) const
    {
        if (ptr != nullptr)
            X11Symbols::getInstance()->xFree (ptr);
    }
};

template <typename Data>
std::unique_ptr<Data, XFreeDeleter> makeXFreePtr (Data* raw) { return std::unique_ptr<Data, XFreeDeleter> (raw); }

//==============================================================================
// Defined in juce_Windowing_linux.cpp
void juce_LinuxAddRepaintListener (ComponentPeer*, Component* dummy);
void juce_LinuxRemoveRepaintListener (ComponentPeer*, Component* dummy);

bool OpenGLHelpers::isOpenGLES()
{
    return eglQueryAPI() == EGL_OPENGL_ES_API;
}

class PeerListener : private ComponentMovementWatcher
{
public:
    PeerListener (Component& comp, Window embeddedWindow)
        : ComponentMovementWatcher (&comp),
          window (embeddedWindow),
          association (comp.getPeer(), window) {}

private:
    using ComponentMovementWatcher::componentMovedOrResized,
          ComponentMovementWatcher::componentVisibilityChanged;

    void componentMovedOrResized (bool, bool) override {}
    void componentVisibilityChanged() override {}

    void componentPeerChanged() override
    {
        // This should not be rewritten as a ternary expression or similar.
        // The old association must be destroyed before the new one is created.
        association = {};

        if (auto* comp = getComponent())
            association = ScopedWindowAssociation (comp->getPeer(), window);
    }

    Window window{};
    ScopedWindowAssociation association;
};

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
        const auto* ext = eglQueryString (nullDisplay, EGL_EXTENSIONS);

        if (ext == nullptr)
        {
            // JUCE needs the EGL implementation to support at least the X11
            // platform extension.
            jassertfalse;
            return;
        }

        const auto platformDisplayToken = std::invoke ([&]() -> std::optional<EGLenum>
        {
            if (strstr (ext, "EGL_KHR_platform_x11") != nullptr)
                return EGL_PLATFORM_X11_KHR;

            if (strstr (ext, "EGL_EXT_platform_x11") != nullptr)
                return EGL_PLATFORM_X11_EXT;

            return {};
        });

        if (! platformDisplayToken.has_value())
        {
            // At the moment JUCE can only create a GL context under X11.
            // If this EGL implementation doesn't support X11, things would break
            // when we tried to pass an X11 display/window/etc. into EGL functions.
            jassertfalse;
            return;
        }

        display = XWindowSystem::getInstance()->getDisplay();

        XWindowSystemUtilities::ScopedXLock xLock;

        X11Symbols::getInstance()->xSync (display, False);

        eglDisplay = eglGetPlatformDisplay (*platformDisplayToken, display, nullptr);

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

        EGLint nativeVisualId = 0;
        eglGetConfigAttrib (eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVisualId);

        auto* peer = component.getPeer();
        jassert (peer != nullptr);

        auto windowH = (Window) peer->getNativeHandle();

        auto [visual, depth] = std::invoke ([this, nativeVisualId]() -> std::tuple<Visual*, int>
        {
            XVisualInfo visualInfo{};
            visualInfo.visualid = (VisualID) nativeVisualId;
            int numVisuals = 0;
            auto xVisualInfo = makeXFreePtr (X11Symbols::getInstance()->xGetVisualInfo (display,
                                                                                        VisualIDMask,
                                                                                        &visualInfo,
                                                                                        &numVisuals));

            if (xVisualInfo != nullptr && numVisuals > 0)
                return { xVisualInfo->visual,
                         xVisualInfo->depth };

            return { DefaultVisual (display, DefaultScreen (display)),
                     DefaultDepth  (display, DefaultScreen (display)) };
        });

        auto colourMap = X11Symbols::getInstance()->xCreateColormap (display, windowH, visual, AllocNone);

        XSetWindowAttributes swa;
        swa.colormap = colourMap;
        swa.border_pixel = 0;
        swa.event_mask = embeddedWindowEventMask;

        const auto physicalBounds = getPhysicalBounds();

        embeddedWindow = X11Symbols::getInstance()->xCreateWindow (display,
                                                                   windowH,
                                                                   physicalBounds.getX(),
                                                                   physicalBounds.getY(),
                                                                   (unsigned int) jmax (1, physicalBounds.getWidth()),
                                                                   (unsigned int) jmax (1, physicalBounds.getHeight()),
                                                                   0,
                                                                   depth,
                                                                   InputOutput,
                                                                   visual,
                                                                   CWBorderPixel | CWColormap | CWEventMask,
                                                                   &swa);

        peerListener.emplace (component, embeddedWindow);

        X11Symbols::getInstance()->xMapWindow (display, embeddedWindow);
        X11Symbols::getInstance()->xFreeColormap (display, colourMap);

        X11Symbols::getInstance()->xSync (display, False);

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
        {
            juce_LinuxRemoveRepaintListener (peer, &dummy);

            if (embeddedWindow != 0)
            {
                XWindowSystemUtilities::ScopedXLock xLock;

                X11Symbols::getInstance()->xUnmapWindow (display, embeddedWindow);
                X11Symbols::getInstance()->xDestroyWindow (display, embeddedWindow);
                X11Symbols::getInstance()->xSync (display, False);

                XEvent event;
                while (X11Symbols::getInstance()->xCheckWindowEvent (display,
                                                                     embeddedWindow,
                                                                     embeddedWindowEventMask,
                                                                     &event) == True)
                {
                }
            }
        }
    }

    InitResult initialiseOnRenderThread (OpenGLContext& c)
    {
        renderContext = EGLHelpers::initEGLContext (api, version, profile, eglDisplay, eglConfig, contextToShareWith);

        if (renderContext == nullptr)
            return InitResult::fatal;

        eglSurface = PtrEGLSurface { eglCreatePlatformWindowSurface (eglDisplay,
                                                                     eglConfig,
                                                                     &embeddedWindow,
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
        eglSwapBuffers (eglDisplay, eglSurface.get());
    }

    Rectangle<int> getPhysicalBounds() const
    {
        if (auto* peer = component.getPeer())
        {
            const auto peerBounds = peer->getAreaCoveredBy (component);
            const auto physicalBounds = peerBounds.toDouble() * peer->getPlatformScaleFactor();
            return physicalBounds.toNearestInt();
        }

        return component.getBounds();
    }

    void updateWindowPosition()
    {
        const auto physicalBounds = getPhysicalBounds();

        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xMoveResizeWindow (display,
                                                      embeddedWindow,
                                                      physicalBounds.getX(),
                                                      physicalBounds.getY(),
                                                      (unsigned int) jmax (1, physicalBounds.getWidth()),
                                                      (unsigned int) jmax (1, physicalBounds.getHeight()));
    }

    bool setSwapInterval (int numFramesPerSwap)
    {
        if (numFramesPerSwap == swapFrames)
            return true;

        swapFrames = numFramesPerSwap;
        eglSwapInterval (eglDisplay, numFramesPerSwap);
        return true;
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

    void addListener (NativeContextListener&) {}
    void removeListener (NativeContextListener&) {}

private:
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

    static constexpr int embeddedWindowEventMask = ExposureMask | StructureNotifyMask;

    CriticalSection mutex;
    Component& component;

    EGLDisplay eglDisplay = nullDisplay;
    PtrEGLContext renderContext;
    PtrEGLSurface eglSurface;

    Window embeddedWindow = {};

    std::optional<PeerListener> peerListener;

    int swapFrames = 0;
    EGLConfig eglConfig = nullptr;
    void* contextToShareWith;

    OpenGLContext* context = nullptr;
    DummyComponent dummy;

    ::Display* display = nullptr;

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
