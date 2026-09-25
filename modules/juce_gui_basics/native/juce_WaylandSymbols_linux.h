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

class JUCE_API WaylandClientSymbols
{
public:
#define CLIENT_METHODS \
    X (wl_display_connect, (const char*), wl_display*) \
    X (wl_display_disconnect, (wl_display*), void) \
    X (wl_display_get_fd, (wl_display*), int) \
    X (wl_display_dispatch_pending, (wl_display*), int) \
    X (wl_display_prepare_read, (wl_display*), int) \
    X (wl_display_read_events, (wl_display*), int) \
    X (wl_display_cancel_read, (wl_display*), void) \
    X (wl_display_flush, (wl_display*), int) \
    X (wl_display_roundtrip, (wl_display*), int) \
    X (wl_display_get_error, (wl_display*), int) \
    X (wl_display_get_protocol_error, (wl_display*, const wl_interface**, uint32_t*), uint32_t) \
    X (wl_proxy_marshal_flags, (wl_proxy*, uint32_t, const wl_interface*, uint32_t, uint32_t, ...), wl_proxy*) \
    X (wl_proxy_get_version, (wl_proxy*), uint32_t) \
    X (wl_proxy_destroy, (wl_proxy*), void) \
    X (wl_proxy_add_listener, (wl_proxy*, void (**)(void), void*), int) \

#define CLIENT_INTERFACES \
    X (wl_buffer_interface) \
    X (wl_callback_interface) \
    X (wl_compositor_interface) \
    X (wl_data_device_interface) \
    X (wl_data_device_manager_interface) \
    X (wl_data_offer_interface) \
    X (wl_data_source_interface) \
    X (wl_keyboard_interface) \
    X (wl_output_interface) \
    X (wl_pointer_interface) \
    X (wl_region_interface) \
    X (wl_registry_interface) \
    X (wl_seat_interface) \
    X (wl_shm_interface) \
    X (wl_shm_pool_interface) \
    X (wl_subcompositor_interface) \
    X (wl_subsurface_interface) \
    X (wl_surface_interface) \
    X (wl_touch_interface) \

#define X(name, args, result) result (*name) args = nullptr;
    CLIENT_METHODS
#undef X

#define X(name) const wl_interface* name = nullptr;
    CLIENT_INTERFACES
#undef X

    bool loadAllSymbols()
    {
        return
#define X(name, args, result) waylandClient.loadInto (name, #name) &&
        CLIENT_METHODS
#undef X
#define X(name) waylandClient.loadInto (name, #name) &&
        CLIENT_INTERFACES
#undef X
        true;
    }

#undef CLIENT_METHODS

    JUCE_DECLARE_SINGLETON_INLINE (WaylandClientSymbols, false)

private:
    WaylandClientSymbols() = default;

    ~WaylandClientSymbols()
    {
        clearSingletonInstance();
    }

    DynamicLibrary waylandClient { "libwayland-client.so.0" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandClientSymbols)
};

#undef JUCE_GENERATE_WAYLAND_FUNCTION

} // namespace juce
