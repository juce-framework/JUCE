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

struct libdecor;
struct libdecor_frame;
struct libdecor_configuration;
struct libdecor_state;

namespace juce
{

// These definitions mirror the libdecor ABI.

// The fixed underlying types keep these enums 32 bits wide and make every value libdecor can
// write, including bits newer than this copy, a valid value of the enum.

// Mirrors enum libdecor_error.
enum libdecor_error : uint32_t
{
    libdecorErrorCompositorIncompatible,
    libdecorErrorInvalidFrameConfiguration
};

// Mirrors enum libdecor_resize_edge.
enum libdecor_resize_edge : uint32_t
{
    libdecorResizeEdgeNone,
    libdecorResizeEdgeTop,
    libdecorResizeEdgeBottom,
    libdecorResizeEdgeLeft,
    libdecorResizeEdgeTopLeft,
    libdecorResizeEdgeBottomLeft,
    libdecorResizeEdgeRight,
    libdecorResizeEdgeTopRight,
    libdecorResizeEdgeBottomRight
};

// Mirrors enum libdecor_window_state. libdecor 0.2.4 defines nothing above suspended.
enum libdecor_window_state : uint32_t
{
    libdecorWindowStateNone        = 0,
    libdecorWindowStateActive      = 1 << 0,
    libdecorWindowStateMaximized   = 1 << 1,
    libdecorWindowStateFullscreen  = 1 << 2,
    libdecorWindowStateTiledLeft   = 1 << 3,
    libdecorWindowStateTiledRight  = 1 << 4,
    libdecorWindowStateTiledTop    = 1 << 5,
    libdecorWindowStateTiledBottom = 1 << 6,
    libdecorWindowStateSuspended   = 1 << 7
};

struct libdecor_interface
{
    void (* error) (libdecor*, libdecor_error, const char*);
    void (* reserved0) (void);
    void (* reserved1) (void);
    void (* reserved2) (void);
    void (* reserved3) (void);
    void (* reserved4) (void);
    void (* reserved5) (void);
    void (* reserved6) (void);
    void (* reserved7) (void);
    void (* reserved8) (void);
    void (* reserved9) (void);
};

struct libdecor_frame_interface
{
    void (* configure) (libdecor_frame*, libdecor_configuration*, void*);
    void (* close) (libdecor_frame*, void*);
    void (* commit) (libdecor_frame*, void*);
    void (* dismiss_popup) (libdecor_frame*, const char*, void*);
    void (* reserved0) (void);
    void (* reserved1) (void);
    void (* reserved2) (void);
    void (* reserved3) (void);
    void (* reserved4) (void);
    void (* reserved5) (void);
    void (* reserved6) (void);
    void (* reserved7) (void);
    void (* reserved8) (void);
    void (* reserved9) (void);
};

struct LibdecorContextDeleter
{
    void operator() (libdecor*) const;
};

struct LibdecorFrameDeleter
{
    void operator() (libdecor_frame*) const;
};

struct LibdecorStateDeleter
{
    void operator() (libdecor_state*) const;
};

using LibdecorContextHandle = std::unique_ptr<libdecor, LibdecorContextDeleter>;
using LibdecorFrameHandle = std::unique_ptr<libdecor_frame, LibdecorFrameDeleter>;
using LibdecorStateHandle = std::unique_ptr<libdecor_state, LibdecorStateDeleter>;

} // namespace juce

namespace juce::LibdecorAPI
{
// Calling libdecor after it reports an error may use plugin state it already destroyed.
void markUnusableAfterError();

// Returns false once libdecor has failed, for callers that would otherwise read a no-op as an error.
bool isUsable();

LibdecorContextHandle createContext (wl_display*, libdecor_interface*);
int getFd (libdecor*);
int dispatch (libdecor*, int timeoutMs);

LibdecorFrameHandle decorate (libdecor*, wl_surface*, libdecor_frame_interface*, void* userData);
void frameSetTitle (libdecor_frame*, StringRef title);
void frameSetAppId (libdecor_frame*, StringRef appId);
void frameSetMinContentSize (libdecor_frame*, Point<int> size);
void frameSetMaxContentSize (libdecor_frame*, Point<int> size);
void frameSetMinimized (libdecor_frame*);
void frameSetFullscreen (libdecor_frame*, wl_output*);
void frameUnsetFullscreen (libdecor_frame*);
void frameMove (libdecor_frame*, wl_seat*, uint32_t serial);
void frameResize (libdecor_frame*, wl_seat*, uint32_t serial, libdecor_resize_edge);
void frameMap (libdecor_frame*);
void frameCommit (libdecor_frame*, libdecor_state*, libdecor_configuration*);
xdg_surface* frameGetXdgSurface (libdecor_frame*);
xdg_toplevel* frameGetXdgToplevel (libdecor_frame*);
void framePopupGrab (libdecor_frame*, StringRef seatName);
void framePopupUngrab (libdecor_frame*, StringRef seatName);
Point<int> frameTranslateCoordinate (libdecor_frame*, Point<int> contentPoint);

LibdecorStateHandle createState (Point<int> contentSize);

std::optional<Point<int>> getConfigurationContentSize (libdecor_configuration*, libdecor_frame*);
std::optional<libdecor_window_state> getConfigurationWindowState (libdecor_configuration*);
} // namespace juce::LibdecorAPI
