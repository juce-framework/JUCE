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

#ifndef __JUCE_JUCEHEADER__
#define __JUCE_JUCEHEADER__

/*
    PLEASE NOTE! This file is just here to help transition old code to the newer
    modularised layout - but it will be removed at some point in the future, so
    you should update your projects to use the newer design as soon as possible.

    Now that the library has been broken up into separate modules, instead of
    including one giant header that includes everything, you should include the
    headers of the modules that you want to use. By far the easiest way to do that
    is with the introjucer, which will sort everything out for you and create a
    single header for your app which correctly includes everything you need.
*/
#ifdef _MSC_VER
 #pragma message ("The juce.h file is deprecated - please include each module's header file directly, or preferably let the introjucer handle the inclusion of source code in your project.")
#else
 #warning "The juce.h file is deprecated - please include each module's header file directly, or preferably let the introjucer handle the inclusion of source code in your project."
#endif

//==============================================================================
#include "../../modules/juce_core/juce_core.h"
#include "../../modules/juce_gui_basics/juce_gui_basics.h"
#include "../../modules/juce_data_structures/juce_data_structures.h"
#include "../../modules/juce_events/juce_events.h"
#include "../../modules/juce_graphics/juce_graphics.h"
#include "../../modules/juce_video/juce_video.h"
#include "../../modules/juce_opengl/juce_opengl.h"
#include "../../modules/juce_audio_basics/juce_audio_basics.h"
#include "../../modules/juce_audio_formats/juce_audio_formats.h"
#include "../../modules/juce_audio_processors/juce_audio_processors.h"
#include "../../modules/juce_audio_devices/juce_audio_devices.h"
#include "../../modules/juce_cryptography/juce_cryptography.h"
#include "../../modules/juce_gui_extra/juce_gui_extra.h"
#include "../../modules/juce_audio_utils/juce_audio_utils.h"


//==============================================================================
#if ! DONT_SET_USING_JUCE_NAMESPACE
  /* If you're not mixing JUCE with other libraries, then this will obviously save
     a lot of typing, but can be disabled by setting DONT_SET_USING_JUCE_NAMESPACE.
  */
  using namespace juce;
#endif

#endif   // __JUCE_JUCEHEADER__
