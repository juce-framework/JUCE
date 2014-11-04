/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_VIDEO_H_INCLUDED
#define JUCE_VIDEO_H_INCLUDED

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

#include "playback/juce_DirectShowComponent.h"
#include "playback/juce_QuickTimeMovieComponent.h"
#include "capture/juce_CameraDevice.h"

}

#endif   // JUCE_VIDEO_H_INCLUDED
