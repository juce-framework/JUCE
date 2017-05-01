/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_video
  vendor:           juce
  version:          5.0.0
  name:             JUCE video playback and capture classes
  description:      Classes for playing video and capturing camera input.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_data_structures juce_cryptography
  OSXFrameworks:    AVFoundation CoreMedia

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_VIDEO_H_INCLUDED

//==============================================================================
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
/** Config: JUCE_DIRECTSHOW
    Enables DirectShow media-streaming architecture (MS Windows only).
*/
#ifndef JUCE_DIRECTSHOW
 #define JUCE_DIRECTSHOW 0
#endif

/** Config: JUCE_MEDIAFOUNDATION
    Enables Media Foundation multimedia platform (Windows Vista and above).
*/
#ifndef JUCE_MEDIAFOUNDATION
 #define JUCE_MEDIAFOUNDATION 0
#endif

#if ! JUCE_WINDOWS
 #undef JUCE_DIRECTSHOW
 #undef JUCE_MEDIAFOUNDATION
#endif

/** Config: JUCE_QUICKTIME
    Enables the QuickTimeMovieComponent class (Mac and Windows).
    If you're building on Windows, you'll need to have the Apple QuickTime SDK
    installed, and its header files will need to be on your include path.
*/
#if ! (defined (JUCE_QUICKTIME) || JUCE_LINUX || JUCE_IOS || JUCE_ANDROID || (JUCE_WINDOWS && ! JUCE_MSVC))
 #define JUCE_QUICKTIME 0
#endif

/** Config: JUCE_USE_CAMERA
    Enables web-cam support using the CameraDevice class (Mac and Windows).
*/
#if (JUCE_QUICKTIME || JUCE_WINDOWS) && ! defined (JUCE_USE_CAMERA)
 #define JUCE_USE_CAMERA 0
#endif

#if ! (JUCE_MAC || JUCE_WINDOWS)
 #undef JUCE_QUICKTIME
 #undef JUCE_USE_CAMERA
#endif

//==============================================================================
namespace juce
{

#if JUCE_DIRECTSHOW || DOXYGEN
 #include "playback/juce_DirectShowComponent.h"
#endif

#if JUCE_MAC || DOXYGEN
 #include "playback/juce_MovieComponent.h"
#endif

#include "capture/juce_CameraDevice.h"

}
