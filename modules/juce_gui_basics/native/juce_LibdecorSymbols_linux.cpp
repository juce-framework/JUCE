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

//==============================================================================
bool LibdecorSymbols::loadAllSymbols()
{
    usable = libdecorLibrary.loadInto (libdecorNew,                         "libdecor_new")
          && libdecorLibrary.loadInto (libdecorUnref,                       "libdecor_unref")
          && libdecorLibrary.loadInto (libdecorGetFd,                       "libdecor_get_fd")
          && libdecorLibrary.loadInto (libdecorDispatch,                    "libdecor_dispatch")
          && libdecorLibrary.loadInto (libdecorDecorate,                    "libdecor_decorate")
          && libdecorLibrary.loadInto (libdecorFrameUnref,                  "libdecor_frame_unref")
          && libdecorLibrary.loadInto (libdecorFrameSetTitle,               "libdecor_frame_set_title")
          && libdecorLibrary.loadInto (libdecorFrameSetAppId,               "libdecor_frame_set_app_id")
          && libdecorLibrary.loadInto (libdecorFrameSetMinContentSize,      "libdecor_frame_set_min_content_size")
          && libdecorLibrary.loadInto (libdecorFrameSetMaxContentSize,      "libdecor_frame_set_max_content_size")
          && libdecorLibrary.loadInto (libdecorFrameSetMinimized,           "libdecor_frame_set_minimized")
          && libdecorLibrary.loadInto (libdecorFrameSetFullscreen,          "libdecor_frame_set_fullscreen")
          && libdecorLibrary.loadInto (libdecorFrameUnsetFullscreen,        "libdecor_frame_unset_fullscreen")
          && libdecorLibrary.loadInto (libdecorFrameMove,                   "libdecor_frame_move")
          && libdecorLibrary.loadInto (libdecorFrameResize,                 "libdecor_frame_resize")
          && libdecorLibrary.loadInto (libdecorFrameMap,                    "libdecor_frame_map")
          && libdecorLibrary.loadInto (libdecorFrameCommit,                 "libdecor_frame_commit")
          && libdecorLibrary.loadInto (libdecorFrameGetXdgSurface,          "libdecor_frame_get_xdg_surface")
          && libdecorLibrary.loadInto (libdecorFrameGetXdgToplevel,         "libdecor_frame_get_xdg_toplevel")
          && libdecorLibrary.loadInto (libdecorFramePopupGrab,              "libdecor_frame_popup_grab")
          && libdecorLibrary.loadInto (libdecorFramePopupUngrab,            "libdecor_frame_popup_ungrab")
          && libdecorLibrary.loadInto (libdecorFrameTranslateCoordinate,    "libdecor_frame_translate_coordinate")
          && libdecorLibrary.loadInto (libdecorStateNew,                    "libdecor_state_new")
          && libdecorLibrary.loadInto (libdecorStateFree,                   "libdecor_state_free")
          && libdecorLibrary.loadInto (libdecorConfigurationGetContentSize, "libdecor_configuration_get_content_size")
          && libdecorLibrary.loadInto (libdecorConfigurationGetWindowState, "libdecor_configuration_get_window_state");

    return usable;
}

} // namespace juce
