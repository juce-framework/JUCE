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

// These libwayland-cursor types stay incomplete so building JUCE does not require its headers.
struct wl_cursor_theme;
struct wl_cursor_image;
struct wl_cursor;

class WaylandCursor final
{
public:
    enum class DragAction
    {
        none,
        copy,
        move
    };

    struct PointerTarget
    {
        PointerTarget (wl_pointer* pointerIn, uint32_t serialIn, int scaleIn)
            : pointer (pointerIn), serial (serialIn), scale (jlimit (1, 16, scaleIn))
        {
        }

        wl_pointer* pointer;
        uint32_t serial;
        int scale;
    };

    WaylandCursor (wl_compositor&, wl_shm&);
    ~WaylandCursor();

    void show (PointerTarget, MouseCursor::StandardCursorType);
    void show (PointerTarget, const detail::CustomMouseCursorInfo*);
    void showDrag (PointerTarget, DragAction);
    void restoreCursor (PointerTarget);
    void removeCustomCursorCache (const detail::CustomMouseCursorInfo&);

private:
    struct CustomBuffer
    {
        CustomBuffer (std::unique_ptr<WaylandShmBuffer>, const detail::CustomMouseCursorInfo*, int scale);

        std::unique_ptr<WaylandShmBuffer> shmBuffer;
        const detail::CustomMouseCursorInfo* cursorInfo = nullptr;
        int scale = 1;
        bool busy = false;
    };

    using ThemeLoadFn = wl_cursor_theme* (*) (const char*, int, wl_shm*);
    using ThemeDestroyFn = void (*) (wl_cursor_theme*);
    using ThemeGetCursorFn = wl_cursor* (*) (wl_cursor_theme*, const char*);
    using ImageGetBufferFn = wl_buffer* (*) (wl_cursor_image*);

    struct ThemeDestructor
    {
        // GCC 7 cannot instantiate ThemeHandle while a nested deleter still has an unparsed
        // default member initialiser, so the function is set by constructors instead.
        ThemeDestructor() noexcept : ThemeDestructor (nullptr) {}
        explicit ThemeDestructor (ThemeDestroyFn destroyIn) noexcept : destroy (destroyIn) {}

        void operator() (wl_cursor_theme*) const;
        ThemeDestroyFn destroy;
    };

    using ThemeHandle = std::unique_ptr<wl_cursor_theme, ThemeDestructor>;

    bool ensureTheme (int scale);
    wl_cursor* findFirstThemeCursor (std::initializer_list<const char*> names) const;
    wl_cursor* findThemeCursor (MouseCursor::StandardCursorType) const;
    wl_cursor* findThemeCursor (DragAction) const;
    CustomBuffer* getOrCreateCustomBuffer (const detail::CustomMouseCursorInfo*, int width, int height, int scale);
    void hide (PointerTarget);
    void applyThemeCursor (PointerTarget, const wl_cursor&);
    bool applyBuffer (PointerTarget, wl_buffer*, int width, int height, int hotspotX, int hotspotY);
    void handleBufferRelease (wl_buffer*);

    static const wl_buffer_listener bufferListener;

    DynamicLibrary library { "libwayland-cursor.so.0" };
    ThemeLoadFn themeLoad = nullptr;
    ThemeDestroyFn themeDestroy = nullptr;
    ThemeGetCursorFn themeGetCursor = nullptr;
    ImageGetBufferFn imageGetBuffer = nullptr;
    ThemeHandle theme { nullptr, ThemeDestructor{} };
    std::vector<CustomBuffer> customBuffers;
    std::variant<MouseCursor::StandardCursorType, const detail::CustomMouseCursorInfo*> cursorToRestore { MouseCursor::NormalCursor };
    std::unique_ptr<wl_surface, FunctionPointerDestructor<WaylandProtocol::wlSurfaceDestroy>> cursorSurface;
    wl_shm& shm;
    int themeScale = 0;
    bool symbolsAvailable = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandCursor)
};

} // namespace juce
