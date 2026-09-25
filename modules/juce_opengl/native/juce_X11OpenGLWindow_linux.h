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

static Rectangle<int> getOpenGLPhysicalBounds (Component& component)
{
    if (auto* peer = component.getPeer())
    {
        const auto peerBounds = peer->getAreaCoveredBy (component);
        return (peerBounds.toDouble() * peer->getPlatformScaleFactor()).toNearestInt();
    }

    return component.getBounds();
}

class X11OpenGLWindow final
{
public:
    X11OpenGLWindow (Component& componentIn, ComponentPeer& peer, ::Display* displayIn,
                     EGLDisplay eglDisplay, EGLConfig eglConfig)
        : component (componentIn),
          display (displayIn)
    {
        if (display == nullptr)
            return;

        XWindowSystemUtilities::ScopedXLock xLock;

        EGLint nativeVisualId = 0;
        eglGetConfigAttrib (eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVisualId);

        const auto parentWindow = (Window) peer.getNativeHandle();

        const auto [visual, depth] = std::invoke ([&]() -> std::tuple<Visual*, int>
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

        const auto colourMap = X11Symbols::getInstance()->xCreateColormap (display, parentWindow, visual, AllocNone);

        XSetWindowAttributes swa;
        swa.colormap = colourMap;
        swa.border_pixel = 0;
        swa.event_mask = embeddedWindowEventMask;

        const auto physicalBounds = getOpenGLPhysicalBounds (component);

        embeddedWindow = X11Symbols::getInstance()->xCreateWindow (display,
                                                                   parentWindow,
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

        if (embeddedWindow != 0)
        {
            peerListener.emplace (component, embeddedWindow);
            X11Symbols::getInstance()->xMapWindow (display, embeddedWindow);
        }

        X11Symbols::getInstance()->xFreeColormap (display, colourMap);
        X11Symbols::getInstance()->xSync (display, False);
    }

    ~X11OpenGLWindow()
    {
        peerListener.reset();

        if (display == nullptr || embeddedWindow == 0)
            return;

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

    bool isValid() const noexcept { return embeddedWindow != 0; }
    void* getNativeWindow() noexcept { return &embeddedWindow; }

    void updateBounds()
    {
        const auto physicalBounds = getOpenGLPhysicalBounds (component);

        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xMoveResizeWindow (display,
                                                      embeddedWindow,
                                                      physicalBounds.getX(),
                                                      physicalBounds.getY(),
                                                      (unsigned int) jmax (1, physicalBounds.getWidth()),
                                                      (unsigned int) jmax (1, physicalBounds.getHeight()));
    }

private:
    static constexpr int embeddedWindowEventMask = ExposureMask | StructureNotifyMask;

    Component& component;
    ::Display* display = nullptr;
    Window embeddedWindow = {};
    std::optional<PeerListener> peerListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (X11OpenGLWindow)
};

} // namespace juce
