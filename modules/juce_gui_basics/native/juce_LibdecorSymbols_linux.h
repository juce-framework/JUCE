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

#define JUCE_GENERATE_LIBDECOR_FUNCTION(functionName, objectName, args, returnType) \
    using functionName      = returnType (*) args; \
    functionName objectName = nullptr;

class JUCE_API  LibdecorSymbols
{
public:
    bool loadAllSymbols();

    // See LibdecorAPI::markUnusableAfterError. The flag lives here because the unique_ptr
    // deleters must consult it without reaching the window system.
    bool isUsable() const noexcept          { return usable; }
    void markUnusableAfterError() noexcept  { usable = false; }

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorNew, libdecorNew,
                                     (wl_display*, libdecor_interface*),
                                     libdecor*)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorUnref, libdecorUnref,
                                     (libdecor*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorGetFd, libdecorGetFd,
                                     (libdecor*),
                                     int)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorDispatch, libdecorDispatch,
                                     (libdecor*, int),
                                     int)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorDecorate, libdecorDecorate,
                                     (libdecor*, wl_surface*, libdecor_frame_interface*, void*),
                                     libdecor_frame*)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameUnref, libdecorFrameUnref,
                                     (libdecor_frame*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameSetTitle, libdecorFrameSetTitle,
                                     (libdecor_frame*, const char*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameSetAppId, libdecorFrameSetAppId,
                                     (libdecor_frame*, const char*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameSetMinContentSize, libdecorFrameSetMinContentSize,
                                     (libdecor_frame*, int, int),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameSetMaxContentSize, libdecorFrameSetMaxContentSize,
                                     (libdecor_frame*, int, int),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameSetMinimized, libdecorFrameSetMinimized,
                                     (libdecor_frame*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameSetFullscreen, libdecorFrameSetFullscreen,
                                     (libdecor_frame*, wl_output*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameUnsetFullscreen, libdecorFrameUnsetFullscreen,
                                     (libdecor_frame*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameMove, libdecorFrameMove,
                                     (libdecor_frame*, wl_seat*, uint32_t),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameResize, libdecorFrameResize,
                                     (libdecor_frame*, wl_seat*, uint32_t, libdecor_resize_edge),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameMap, libdecorFrameMap,
                                     (libdecor_frame*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameCommit, libdecorFrameCommit,
                                     (libdecor_frame*, libdecor_state*, libdecor_configuration*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameGetXdgSurface, libdecorFrameGetXdgSurface,
                                     (libdecor_frame*),
                                     xdg_surface*)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameGetXdgToplevel, libdecorFrameGetXdgToplevel,
                                     (libdecor_frame*),
                                     xdg_toplevel*)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFramePopupGrab, libdecorFramePopupGrab,
                                     (libdecor_frame*, const char*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFramePopupUngrab, libdecorFramePopupUngrab,
                                     (libdecor_frame*, const char*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorFrameTranslateCoordinate, libdecorFrameTranslateCoordinate,
                                     (libdecor_frame*, int, int, int*, int*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorStateNew, libdecorStateNew,
                                     (int, int),
                                     libdecor_state*)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorStateFree, libdecorStateFree,
                                     (libdecor_state*),
                                     void)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorConfigurationGetContentSize, libdecorConfigurationGetContentSize,
                                     (libdecor_configuration*, libdecor_frame*, int*, int*),
                                     bool)

    JUCE_GENERATE_LIBDECOR_FUNCTION (LibdecorConfigurationGetWindowState, libdecorConfigurationGetWindowState,
                                     (libdecor_configuration*, libdecor_window_state*),
                                     bool)

    JUCE_DECLARE_SINGLETON_INLINE (LibdecorSymbols, false)

private:
    LibdecorSymbols() = default;

    ~LibdecorSymbols()
    {
        clearSingletonInstance();
    }

    DynamicLibrary libdecorLibrary { "libdecor-0.so.0" };
    bool usable = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibdecorSymbols)
};

#undef JUCE_GENERATE_LIBDECOR_FUNCTION

} // namespace juce
