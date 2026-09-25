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

// Hand-rolled mirror of the libwayland ABI structs
struct wl_message
{
    const char* name;
    const char* signature;
    const wl_interface** types;
};

struct wl_interface
{
    const char* name;
    int version;
    int methodCount;
    const wl_message* methods;
    int eventCount;
    const wl_message* events;
};

struct wl_array
{
    std::size_t size;
    std::size_t alloc;
    void* data;
};

struct wl_registry_listener
{
    void (*global) (void*, wl_registry*, uint32_t, const char*, uint32_t);
    void (*global_remove) (void*, wl_registry*, uint32_t);
};

// The final two events are available from version 6.
struct wl_surface_listener
{
    void (*enter) (void*, wl_surface*, wl_output*);
    void (*leave) (void*, wl_surface*, wl_output*);
    void (*preferred_buffer_scale) (void*, wl_surface*, int32_t);
    void (*preferred_buffer_transform) (void*, wl_surface*, uint32_t);
};

// The final two events are available from version 4.
struct wl_output_listener
{
    void (*geometry) (void*, wl_output*, int32_t, int32_t, int32_t, int32_t, int32_t, const char*, const char*, int32_t);
    void (*mode) (void*, wl_output*, uint32_t, int32_t, int32_t, int32_t);
    void (*done) (void*, wl_output*);
    void (*scale) (void*, wl_output*, int32_t);
    void (*name) (void*, wl_output*, const char*);
    void (*description) (void*, wl_output*, const char*);
};

struct zxdg_output_v1_listener
{
    void (*logical_position) (void*, zxdg_output_v1*, int32_t, int32_t);
    void (*logical_size) (void*, zxdg_output_v1*, int32_t, int32_t);
    void (*done) (void*, zxdg_output_v1*);
    void (*name) (void*, zxdg_output_v1*, const char*);
    void (*description) (void*, zxdg_output_v1*, const char*);
};

struct wl_callback_listener
{
    void (*done) (void*, wl_callback*, uint32_t);
};

struct wl_buffer_listener
{
    void (*release) (void*, wl_buffer*);
};

struct wl_data_offer_listener
{
    void (*offer) (void*, wl_data_offer*, const char*);
    void (*source_actions) (void*, wl_data_offer*, uint32_t);
    void (*action) (void*, wl_data_offer*, uint32_t);
};

// The final three events are available from version 3.
struct wl_data_source_listener
{
    void (*target) (void*, wl_data_source*, const char*);
    void (*send) (void*, wl_data_source*, const char*, int32_t);
    void (*cancelled) (void*, wl_data_source*);
    void (*dnd_drop_performed) (void*, wl_data_source*);
    void (*dnd_finished) (void*, wl_data_source*);
    void (*action) (void*, wl_data_source*, uint32_t);
};

struct wl_data_device_listener
{
    void (*data_offer) (void*, wl_data_device*, wl_data_offer*);
    void (*enter) (void*, wl_data_device*, uint32_t, wl_surface*, int32_t, int32_t, wl_data_offer*);
    void (*leave) (void*, wl_data_device*);
    void (*motion) (void*, wl_data_device*, uint32_t, int32_t, int32_t);
    void (*drop) (void*, wl_data_device*);
    void (*selection) (void*, wl_data_device*, wl_data_offer*);
};

struct xdg_wm_base_listener
{
    void (*ping) (void*, xdg_wm_base*, uint32_t);
};

struct xdg_surface_listener
{
    void (*configure) (void*, xdg_surface*, uint32_t);
};

struct xdg_popup_listener
{
    void (*configure) (void*, xdg_popup*, int32_t, int32_t, int32_t, int32_t);
    void (*popup_done) (void*, xdg_popup*);
    void (*repositioned) (void*, xdg_popup*, uint32_t);
};

struct xdg_toplevel_listener
{
    void (*configure) (void*, xdg_toplevel*, int32_t, int32_t, wl_array*);
    void (*close) (void*, xdg_toplevel*);
    void (*configure_bounds) (void*, xdg_toplevel*, int32_t, int32_t);
    void (*wm_capabilities) (void*, xdg_toplevel*, wl_array*);
};

struct xdg_activation_token_v1_listener
{
    void (*done) (void*, xdg_activation_token_v1*, const char*);
};

struct zxdg_toplevel_decoration_v1_listener
{
    void (*configure) (void*, zxdg_toplevel_decoration_v1*, uint32_t);
};

struct zxdg_exported_v2_listener
{
    void (*handle) (void*, zxdg_exported_v2*, const char*);
};

struct wp_fractional_scale_v1_listener
{
    void (*preferred_scale) (void*, wp_fractional_scale_v1*, uint32_t);
};

struct zwp_text_input_v3_listener
{
    void (*enter) (void*, zwp_text_input_v3*, wl_surface*);
    void (*leave) (void*, zwp_text_input_v3*, wl_surface*);
    void (*preedit_string) (void*, zwp_text_input_v3*, const char*, int32_t, int32_t);
    void (*commit_string) (void*, zwp_text_input_v3*, const char*);
    void (*delete_surrounding_text) (void*, zwp_text_input_v3*, uint32_t, uint32_t);
    void (*done) (void*, zwp_text_input_v3*, uint32_t);
};

struct wl_seat_listener
{
    void (*capabilities) (void*, wl_seat*, uint32_t);
    void (*name) (void*, wl_seat*, const char*);
};

// wl_fixed_t is int32_t. fd (h) arrives as int32_t.
// the v11 warp event is the next thing to be added
struct wl_pointer_listener
{
    void (*enter) (void*, wl_pointer*, uint32_t, wl_surface*, int32_t, int32_t);
    void (*leave) (void*, wl_pointer*, uint32_t, wl_surface*);
    void (*motion) (void*, wl_pointer*, uint32_t, int32_t, int32_t);
    void (*button) (void*, wl_pointer*, uint32_t, uint32_t, uint32_t, uint32_t);
    void (*axis) (void*, wl_pointer*, uint32_t, uint32_t, int32_t);
    void (*frame) (void*, wl_pointer*);
    void (*axis_source) (void*, wl_pointer*, uint32_t);
    void (*axis_stop) (void*, wl_pointer*, uint32_t, uint32_t);
    void (*axis_discrete) (void*, wl_pointer*, uint32_t, int32_t);
    void (*axis_value120) (void*, wl_pointer*, uint32_t, int32_t);
    void (*axis_relative_direction) (void*, wl_pointer*, uint32_t, uint32_t);
};

// The keymap fd (h) arrives as int32_t.
struct wl_keyboard_listener
{
    void (*keymap) (void*, wl_keyboard*, uint32_t, int32_t, uint32_t);
    void (*enter) (void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*);
    void (*leave) (void*, wl_keyboard*, uint32_t, wl_surface*);
    void (*key) (void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t);
    void (*modifiers) (void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void (*repeat_info) (void*, wl_keyboard*, int32_t, int32_t);
};

struct wl_touch_listener
{
    void (*down) (void*, wl_touch*, uint32_t, uint32_t, wl_surface*, int32_t, int32_t, int32_t);
    void (*up) (void*, wl_touch*, uint32_t, uint32_t, int32_t);
    void (*motion) (void*, wl_touch*, uint32_t, int32_t, int32_t, int32_t);
    void (*frame) (void*, wl_touch*);
    void (*cancel) (void*, wl_touch*);
    void (*shape) (void*, wl_touch*, int32_t, int32_t, int32_t);
    void (*orientation) (void*, wl_touch*, int32_t, int32_t);
};

enum : uint32_t
{
    WL_MARSHAL_FLAG_DESTROY = 1
};
