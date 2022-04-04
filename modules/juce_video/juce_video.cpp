/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef JUCE_VIDEO_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1

#include "juce_video.h"

#if JUCE_MAC || JUCE_IOS
 #import <AVFoundation/AVFoundation.h>
 #import <AVKit/AVKit.h>

//==============================================================================
#elif JUCE_WINDOWS
 #include "wmsdkidl.h"
 #include "native/juce_win32_ComTypes.h"

 #if ! JUCE_MINGW && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment (lib, "strmiids.lib")

  #if JUCE_USE_CAMERA
   #pragma comment (lib, "Strmiids.lib")
   #pragma comment (lib, "wmvcore.lib")
  #endif

  #if JUCE_MEDIAFOUNDATION
   #pragma comment (lib, "mfuuid.lib")
  #endif
 #endif
#endif

//==============================================================================
#include "playback/juce_VideoComponent.cpp"

#if JUCE_USE_CAMERA
 #include "capture/juce_CameraDevice.cpp"
#endif
