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

struct wl_egl_window;

namespace juce
{

class WaylandEGLSymbols final
{
public:
    using WlEglWindowCreate = wl_egl_window* (*) (wl_surface*, int, int);
    using WlEglWindowDestroy = void (*) (wl_egl_window*);
    using WlEglWindowResize = void (*) (wl_egl_window*, int, int, int, int);

    WaylandEGLSymbols()
        : symbolsLoaded (loadAllSymbols())
    {
    }

    bool isLoaded() const noexcept { return symbolsLoaded; }

    WlEglWindowCreate wlEglWindowCreate = nullptr;
    WlEglWindowDestroy wlEglWindowDestroy = nullptr;
    WlEglWindowResize wlEglWindowResize = nullptr;

private:
    bool loadAllSymbols()
    {
        return waylandEGL.loadInto (wlEglWindowCreate,  "wl_egl_window_create")
            && waylandEGL.loadInto (wlEglWindowDestroy, "wl_egl_window_destroy")
            && waylandEGL.loadInto (wlEglWindowResize,  "wl_egl_window_resize");
    }

    DynamicLibrary waylandEGL { "libwayland-egl.so.1" };
    bool symbolsLoaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandEGLSymbols)
};

class WaylandOpenGLWindow final
{
public:
    explicit WaylandOpenGLWindow (std::unique_ptr<WaylandOpenGLSurface> surfaceIn)
        : surface (std::move (surfaceIn))
    {
        if (! symbols->isLoaded())
            return;

        const auto bounds = surface->updateBounds ({});
        windowSize = bounds.bufferSize;
        pendingSize = windowSize;
        visible = bounds.visible;

        if (windowSize.x <= 0 || windowSize.y <= 0)
            return;

        window.reset (symbols->wlEglWindowCreate (surface->getSurface(), windowSize.x, windowSize.y));
    }

    bool isValid() const noexcept { return window != nullptr; }
    void* getNativeWindow() noexcept { return window.get(); }

    bool updateBounds()
    {
        const auto wasVisible = visible;
        const auto bounds = surface->updateBounds (attachedBufferSize);
        visible = bounds.visible;

        if (bounds.bufferSize.x > 0 && bounds.bufferSize.y > 0)
            pendingSize = bounds.bufferSize;

        if (! visible)
        {
            // The child surface is unmapped while hidden, so nothing is attached.
            attachedBufferSize = {};
            return false;
        }

        if (! wasVisible)
        {
            // A frame callback requested before the child was hidden may never arrive.
            frameReady = true;
            return true;
        }

        return false;
    }

    void setFrameReadyCallback (std::function<void()> callback)
    {
        frameReadyCallback = std::move (callback);
    }

    bool isReadyForRender (bool useFrameCallbacks) const noexcept
    {
        return visible && (! useFrameCallbacks || frameReady.load());
    }

    bool prepareForSwap (bool useFrameCallbacks)
    {
        if (! visible)
            return false;

        surface->applyViewport (windowSize);

        if (useFrameCallbacks)
        {
            frameReady = false;

            if (! surface->requestFrameCallback ([this] { handleFrameCallback(); }))
                frameReady = true;
        }

        return true;
    }

    bool finishSwap()
    {
        attachedBufferSize = windowSize;

        if (pendingSize == windowSize)
            return false;

        // Resize between frames so the next buffer and its crop use the new size.
        windowSize = pendingSize;
        symbols->wlEglWindowResize (window.get(), windowSize.x, windowSize.y, 0, 0);
        return true;
    }

private:
    using WindowHandle = std::unique_ptr<wl_egl_window, WaylandEGLSymbols::WlEglWindowDestroy>;

    void handleFrameCallback()
    {
        frameReady = true;
        NullCheckedInvocation::invoke (frameReadyCallback);
    }

    std::function<void()> frameReadyCallback;
    std::atomic<bool> frameReady { true };
    std::unique_ptr<WaylandOpenGLSurface> surface;
    SharedResourcePointer<WaylandEGLSymbols> symbols;
    WindowHandle window { nullptr, symbols->wlEglWindowDestroy };
    Point<int> pendingSize;
    Point<int> windowSize;
    Point<int> attachedBufferSize;
    bool visible = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandOpenGLWindow)
};

} // namespace juce
