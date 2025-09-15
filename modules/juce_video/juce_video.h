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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_video
  vendor:             juce
  version:            8.0.10
  name:               JUCE video playback and capture classes
  description:        Classes for playing video and capturing camera input.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_gui_extra
  OSXFrameworks:      AVKit AVFoundation CoreMedia
  iOSFrameworks:      AVKit AVFoundation CoreMedia

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_VIDEO_H_INCLUDED

//==============================================================================
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
/** Config: JUCE_USE_CAMERA
    Enables camera support using the CameraDevice class (Mac, Windows, iOS, Android).
*/
#ifndef JUCE_USE_CAMERA
 #define JUCE_USE_CAMERA 0
#endif

#ifndef JUCE_CAMERA_LOG_ENABLED
 #define JUCE_CAMERA_LOG_ENABLED 0
#endif

#if JUCE_CAMERA_LOG_ENABLED
 #define JUCE_CAMERA_LOG(x) DBG(x)
#else
 #define JUCE_CAMERA_LOG(x) {}
#endif

#if ! (JUCE_MAC || JUCE_WINDOWS || JUCE_IOS || JUCE_ANDROID)
 #undef JUCE_USE_CAMERA
#endif

//==============================================================================
/** Config: JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
    Enables synchronisation between video playback volume and OS media volume.
    Currently supported on Android only.
 */
#ifndef JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
 #define JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME 1
#endif

#ifndef JUCE_VIDEO_LOG_ENABLED
 #define JUCE_VIDEO_LOG_ENABLED 1
#endif

#if JUCE_VIDEO_LOG_ENABLED
 #define JUCE_VIDEO_LOG(x) DBG(x)
#else
 #define JUCE_VIDEO_LOG(x) {}
#endif

//==============================================================================
#include "playback/juce_VideoComponent.h"
#include "capture/juce_CameraDevice.h"
