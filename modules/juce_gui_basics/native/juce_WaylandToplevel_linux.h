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

class WaylandFullScreenState final
{
public:
    void setFullScreenRequested (bool shouldBeFullScreen);
    void configureReceived (bool compositorReportsFullScreen);
    void toplevelDestroyed();

    bool isFullScreen() const;
    bool isFullScreenRequested() const;

private:
    bool wantsFullScreen = false;
    bool confirmedFullScreen = false;
    bool reportsRequestedState = false;
    bool ignoresNextDisagreement = false;
};

struct WaylandSizeConstraints
{
    Point<int> minimum;
    Point<int> maximum;

    bool operator== (const WaylandSizeConstraints& other) const
    {
        return minimum == other.minimum && maximum == other.maximum;
    }
};

class WaylandToplevel
{
public:
    enum class NativeTitleBar { no, yes };

    struct ConfigureInfo
    {
        std::optional<Point<int>> contentSize;
        bool fullScreen = false;
        bool activated = false;
        bool suspended = false;
        bool maximisedOrTiled = false;
        // Empty when the backend cannot report whether an interactive resize is active.
        std::optional<bool> resizing;
    };

    struct Delegate
    {
        virtual ~Delegate() = default;

        // Returns the content size that should be used when the configure names no size.
        virtual Point<int> prepareToplevelConfigure (const ConfigureInfo&) = 0;

        // Repainting and notifications happen after the protocol reply. Those callbacks may
        // destroy this object, so the toplevel performs no further work after this call.
        virtual void finishToplevelConfigure() = 0;

        // Commits through the peer so pending subsurface positions are tracked.
        virtual void commitSurface() = 0;

        virtual void toplevelCloseRequested() = 0;
    };

    virtual ~WaylandToplevel() = default;

    virtual void show() = 0;
    virtual void setTitle (const String&) = 0;
    virtual void setSizeConstraints (WaylandSizeConstraints) = 0;
    virtual void commitSizeConstraints (Point<int> contentSize) = 0;
    virtual void requestMinimise() = 0;
    virtual void requestFullScreen (bool shouldBeFullScreen) = 0;

    virtual void requestInteractiveMove (wl_seat&, uint32_t serial) = 0;
    virtual void requestInteractiveResize (wl_seat&, uint32_t serial, uint32_t resizeEdge) = 0;

    virtual void contentResized (Point<int> contentSize) = 0;
    virtual xdg_surface* getXdgSurface() const = 0;
    virtual xdg_toplevel* getXdgToplevel() const = 0;
    virtual Rectangle<int> getPopupParentBounds (Rectangle<int> contentBounds) const = 0;

    // Nested popup grabs are counted. The first starts the backend's grab bookkeeping,
    // and the last end notification releases it. Only libdecor acts on these.
    virtual void popupGrabStarted (Component& componentToDismiss) = 0;
    virtual void popupGrabEnded() = 0;

    // Returns a zero border when the backend provides no frame outside the content.
    // No value means that the compositor / libdecor provides a frame (whose size is not reported).
    virtual ComponentPeer::OptionalBorderSize getFrameSizeIfPresent() const = 0;
};

std::unique_ptr<WaylandToplevel> createWaylandToplevel (WaylandToplevel::Delegate&,
                                                        detail::WaylandPeerDiagnostics&,
                                                        wl_surface&,
                                                        const String& title,
                                                        WaylandToplevel::NativeTitleBar,
                                                        bool fullScreenRequested);

} // namespace juce
