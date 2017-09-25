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
  version:          5.1.2
  name:             JUCE video playback and capture classes
  description:      Classes for playing video and capturing camera input.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_data_structures juce_cryptography
  OSXFrameworks:    AVKit AVFoundation CoreMedia
  iOSFrameworks:    AVKit AVFoundation CoreMedia

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_VIDEO_H_INCLUDED

//==============================================================================
#include <juce_gui_extra/juce_gui_extra.h>

//=============================================================================
/** Config: JUCE_USE_CAMERA
    Enables web-cam support using the CameraDevice class (Mac and Windows).
*/
#ifndef JUCE_USE_CAMERA
 #define JUCE_USE_CAMERA 0
#endif

#if ! (JUCE_MAC || JUCE_WINDOWS)
 #undef JUCE_USE_CAMERA
#endif

//=============================================================================
#include "playback/juce_VideoComponent.h"
#include "capture/juce_CameraDevice.h"
