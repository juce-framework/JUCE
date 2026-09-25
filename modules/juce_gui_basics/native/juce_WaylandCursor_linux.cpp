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

// Hand-rolled mirrors of the libwayland-cursor ABI structs
struct wl_cursor_image
{
    uint32_t width;
    uint32_t height;
    uint32_t hotspot_x;
    uint32_t hotspot_y;
    uint32_t delay;
};

struct wl_cursor
{
    unsigned int image_count;
    wl_cursor_image** images;
    char* name;
};

WaylandCursor::WaylandCursor (wl_compositor& compositor, wl_shm& shmIn)
    : shm (shmIn)
{
    symbolsAvailable = library.loadInto (themeLoad, "wl_cursor_theme_load")
                    && library.loadInto (themeDestroy, "wl_cursor_theme_destroy")
                    && library.loadInto (themeGetCursor, "wl_cursor_theme_get_cursor")
                    && library.loadInto (imageGetBuffer, "wl_cursor_image_get_buffer");

    cursorSurface.reset (WaylandProtocol::wlCompositorCreateSurface (&compositor));
}

WaylandCursor::~WaylandCursor() = default;

WaylandCursor::CustomBuffer::CustomBuffer (std::unique_ptr<WaylandShmBuffer> bufferIn,
                                            const detail::CustomMouseCursorInfo* cursorInfoIn,
                                            int scaleIn)
    : shmBuffer (std::move (bufferIn)), cursorInfo (cursorInfoIn), scale (scaleIn)
{
}

void WaylandCursor::ThemeDestructor::operator() (wl_cursor_theme* cursorTheme) const
{
    if (cursorTheme != nullptr && destroy != nullptr)
        destroy (cursorTheme);
}

void WaylandCursor::show (PointerTarget target, MouseCursor::StandardCursorType type)
{
    cursorToRestore = type;

    if (type == MouseCursor::NoCursor)
    {
        hide (target);
        return;
    }

    if (! ensureTheme (target.scale))
        return;

    auto* themeCursor = findThemeCursor (type);

    if (themeCursor == nullptr)
        themeCursor = findThemeCursor (MouseCursor::NormalCursor);

    if (themeCursor != nullptr)
        applyThemeCursor (target, *themeCursor);
}

void WaylandCursor::show (PointerTarget target, const detail::CustomMouseCursorInfo* info)
{
    if (info == nullptr)
        return;

    cursorToRestore = info;

    const auto logicalBounds = info->image.getScaledBounds().toNearestInt();

    if (logicalBounds.isEmpty())
    {
        hide (target);
        return;
    }

    const auto width = jmax (1, logicalBounds.getWidth() * target.scale);
    const auto height = jmax (1, logicalBounds.getHeight() * target.scale);
    auto* buffer = getOrCreateCustomBuffer (info, width, height, target.scale);

    if (buffer == nullptr)
        return;

    buffer->busy = true;

    if (! applyBuffer (target, buffer->shmBuffer->handle.get(), width, height, info->hotspot.x, info->hotspot.y))
    {
        buffer->busy = false;
    }
}

void WaylandCursor::showDrag (PointerTarget target, DragAction action)
{
    if (! ensureTheme (target.scale))
        return;

    auto* themeCursor = findThemeCursor (action);

    if (themeCursor == nullptr)
        themeCursor = findThemeCursor (MouseCursor::NormalCursor);

    if (themeCursor != nullptr)
        applyThemeCursor (target, *themeCursor);
}

void WaylandCursor::restoreCursor (PointerTarget target)
{
    if (const auto* standard = std::get_if<MouseCursor::StandardCursorType> (&cursorToRestore))
        show (target, *standard);
    else if (const auto* custom = std::get_if<const detail::CustomMouseCursorInfo*> (&cursorToRestore))
        show (target, *custom);
}

void WaylandCursor::removeCustomCursorCache (const detail::CustomMouseCursorInfo& info)
{
    if (const auto* current = std::get_if<const detail::CustomMouseCursorInfo*> (&cursorToRestore);
        current != nullptr && *current == &info)
    {
        cursorToRestore = MouseCursor::NormalCursor;
    }

    // Busy buffers remain valid until the compositor releases them.
    for (auto& buffer : customBuffers)
        if (buffer.cursorInfo == &info && buffer.busy)
            buffer.cursorInfo = nullptr;

    const auto canRemove = [&info] (const auto& buffer) { return buffer.cursorInfo == &info; };
    customBuffers.erase (std::remove_if (customBuffers.begin(), customBuffers.end(), canRemove), customBuffers.end());
}

bool WaylandCursor::ensureTheme (int scale)
{
    if (theme != nullptr && themeScale == scale)
        return true;

    theme.reset();
    themeScale = 0;

    if (! symbolsAvailable)
        return false;

    const auto themeName = SystemStats::getEnvironmentVariable ("XCURSOR_THEME", {});
    const auto configuredSize = SystemStats::getEnvironmentVariable ("XCURSOR_SIZE", {}).getIntValue();
    const auto logicalSize = configuredSize > 0 ? configuredSize : 24;
    auto* loaded = themeLoad (themeName.isNotEmpty() ? themeName.toRawUTF8() : nullptr,
                              logicalSize * scale,
                              &shm);

    if (loaded == nullptr)
        return false;

    theme = ThemeHandle { loaded, ThemeDestructor { themeDestroy } };
    themeScale = scale;
    return true;
}

wl_cursor* WaylandCursor::findFirstThemeCursor (std::initializer_list<const char*> names) const
{
    for (const auto* name : names)
        if (auto* cursor = themeGetCursor (theme.get(), name))
            return cursor;

    return nullptr;
}

wl_cursor* WaylandCursor::findThemeCursor (MouseCursor::StandardCursorType type) const
{
    switch (type)
    {
        case MouseCursor::ParentCursor:
        case MouseCursor::NormalCursor:                   return findFirstThemeCursor ({ "default", "left_ptr", "arrow" });
        case MouseCursor::WaitCursor:                     return findFirstThemeCursor ({ "wait", "watch" });
        case MouseCursor::IBeamCursor:                    return findFirstThemeCursor ({ "text", "xterm" });
        case MouseCursor::CrosshairCursor:                return findFirstThemeCursor ({ "crosshair", "cross" });
        case MouseCursor::CopyingCursor:                  return findFirstThemeCursor ({ "copy", "dnd-copy" });
        case MouseCursor::PointingHandCursor:             return findFirstThemeCursor ({ "pointer", "hand2" });
        case MouseCursor::DraggingHandCursor:             return findFirstThemeCursor ({ "grab", "openhand", "hand1" });
        case MouseCursor::LeftRightResizeCursor:          return findFirstThemeCursor ({ "ew-resize", "sb_h_double_arrow" });
        case MouseCursor::UpDownResizeCursor:             return findFirstThemeCursor ({ "ns-resize", "sb_v_double_arrow" });
        case MouseCursor::UpDownLeftRightResizeCursor:    return findFirstThemeCursor ({ "all-scroll", "fleur" });
        case MouseCursor::TopEdgeResizeCursor:            return findFirstThemeCursor ({ "n-resize", "top_side" });
        case MouseCursor::BottomEdgeResizeCursor:         return findFirstThemeCursor ({ "s-resize", "bottom_side" });
        case MouseCursor::LeftEdgeResizeCursor:           return findFirstThemeCursor ({ "w-resize", "left_side" });
        case MouseCursor::RightEdgeResizeCursor:          return findFirstThemeCursor ({ "e-resize", "right_side" });
        case MouseCursor::TopLeftCornerResizeCursor:      return findFirstThemeCursor ({ "nw-resize", "top_left_corner" });
        case MouseCursor::TopRightCornerResizeCursor:     return findFirstThemeCursor ({ "ne-resize", "top_right_corner" });
        case MouseCursor::BottomLeftCornerResizeCursor:   return findFirstThemeCursor ({ "sw-resize", "bottom_left_corner" });
        case MouseCursor::BottomRightCornerResizeCursor:  return findFirstThemeCursor ({ "se-resize", "bottom_right_corner" });
        case MouseCursor::NoCursor:
        case MouseCursor::NumStandardCursorTypes:         break;
    }

    return nullptr;
}

wl_cursor* WaylandCursor::findThemeCursor (DragAction action) const
{
    switch (action)
    {
        case DragAction::none: return findFirstThemeCursor ({ "no-drop", "not-allowed", "dnd-none" });
        case DragAction::copy: return findFirstThemeCursor ({ "copy", "dnd-copy" });
        case DragAction::move: return findFirstThemeCursor ({ "move", "dnd-move" });
    }

    return nullptr;
}

WaylandCursor::CustomBuffer* WaylandCursor::getOrCreateCustomBuffer (const detail::CustomMouseCursorInfo* info,
                                                                      int width, int height, int scale)
{
    // The custom cursor info belongs to a non-movable platform handle, so its address is stable.
    for (auto& buffer : customBuffers)
        if (buffer.cursorInfo == info && buffer.scale == scale && ! buffer.busy)
            return &buffer;

    auto buffer = WaylandShmBuffer::create (shm, width, height);

    if (buffer == nullptr)
        return nullptr;

    const auto image = info->image.getImage().rescaled (width, height).convertedToFormat (Image::ARGB);
    copyImageRegionsToWaylandShmBuffer (image, *buffer,
                                        RectangleList<int> { image.getBounds() }, std::nullopt);

    WaylandProtocol::wlBufferAddListener (buffer->handle.get(), &bufferListener, this);
    customBuffers.emplace_back (std::move (buffer), info, scale);
    return &customBuffers.back();
}

void WaylandCursor::hide (PointerTarget target)
{
    if (target.pointer != nullptr)
        WaylandProtocol::wlPointerSetCursor (target.pointer, target.serial, nullptr, 0, 0);
}

void WaylandCursor::applyThemeCursor (PointerTarget target, const wl_cursor& cursor)
{
    if (cursor.image_count == 0)
        return;

    auto* image = cursor.images[0];

    if (image == nullptr)
        return;

    applyBuffer (target, imageGetBuffer (image), (int) image->width, (int) image->height,
                 (int) image->hotspot_x / target.scale, (int) image->hotspot_y / target.scale);
}

bool WaylandCursor::applyBuffer (PointerTarget target, wl_buffer* buffer, int width, int height,
                                 int hotspotX, int hotspotY)
{
    if (target.pointer == nullptr || cursorSurface == nullptr || buffer == nullptr)
        return false;

    WaylandProtocol::wlSurfaceSetBufferScale (cursorSurface.get(), target.scale);
    WaylandProtocol::wlSurfaceAttach (cursorSurface.get(), buffer, 0, 0);
    WaylandProtocol::wlSurfaceDamageBuffer (cursorSurface.get(), 0, 0, width, height);
    WaylandProtocol::wlPointerSetCursor (target.pointer, target.serial, cursorSurface.get(), hotspotX, hotspotY);
    WaylandProtocol::wlSurfaceCommit (cursorSurface.get());
    return true;
}

void WaylandCursor::handleBufferRelease (wl_buffer* released)
{
    const auto matches = [released] (const auto& buffer) { return buffer.shmBuffer->handle.get() == released; };
    const auto buffer = std::find_if (customBuffers.begin(), customBuffers.end(), matches);

    if (buffer == customBuffers.end())
        return;

    buffer->busy = false;

    if (buffer->cursorInfo == nullptr)
    {
        customBuffers.erase (buffer);
        return;
    }

    const auto cursorInfo = buffer->cursorInfo;
    const auto bufferScale = buffer->scale;
    const auto alreadyCached = std::any_of (customBuffers.begin(), customBuffers.end(), [&] (const auto& candidate)
    {
        return &candidate != &*buffer
            && candidate.cursorInfo == cursorInfo
            && candidate.scale == bufferScale
            && ! candidate.busy;
    });

    if (alreadyCached)
        customBuffers.erase (buffer);
}

const wl_buffer_listener WaylandCursor::bufferListener
{
    [] (void* data, wl_buffer* buffer)
    {
        static_cast<WaylandCursor*> (data)->handleBufferRelease (buffer);
    }
};

} // namespace juce
