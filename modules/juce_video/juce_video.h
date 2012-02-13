/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_VIDEO_JUCEHEADER__
#define __JUCE_VIDEO_JUCEHEADER__

//=============================================================================
#include "../juce_gui_extra/juce_gui_extra.h"

//=============================================================================
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

//=============================================================================
namespace juce
{

// START_AUTOINCLUDE playback, capture
#ifndef __JUCE_DIRECTSHOWCOMPONENT_JUCEHEADER__
 #include "playback/juce_DirectShowComponent.h"
#endif
#ifndef __JUCE_QUICKTIMEMOVIECOMPONENT_JUCEHEADER__
 #include "playback/juce_QuickTimeMovieComponent.h"
#endif
#ifndef __JUCE_CAMERADEVICE_JUCEHEADER__
 #include "capture/juce_CameraDevice.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_VIDEO_JUCEHEADER__
