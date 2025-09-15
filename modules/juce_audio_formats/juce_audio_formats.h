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

  ID:                 juce_audio_formats
  vendor:             juce
  version:            8.0.10
  name:               JUCE audio file format codecs
  description:        Classes for reading and writing various audio file formats.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_audio_basics
  OSXFrameworks:      CoreAudio CoreMIDI QuartzCore AudioToolbox
  iOSFrameworks:      AudioToolbox QuartzCore

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_AUDIO_FORMATS_H_INCLUDED

#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
/** Config: JUCE_USE_FLAC
    Enables the FLAC audio codec classes (available on all platforms).
    If your app doesn't need to read FLAC files, you might want to disable this to
    reduce the size of your codebase and build time.
*/
#ifndef JUCE_USE_FLAC
 #define JUCE_USE_FLAC 1
#endif

/** Config: JUCE_USE_OGGVORBIS
    Enables the Ogg-Vorbis audio codec classes (available on all platforms).
    If your app doesn't need to read Ogg-Vorbis files, you might want to disable this to
    reduce the size of your codebase and build time.
*/
#ifndef JUCE_USE_OGGVORBIS
 #define JUCE_USE_OGGVORBIS 1
#endif

/** Config: JUCE_USE_MP3AUDIOFORMAT
    Enables the software-based MP3AudioFormat class.
    IMPORTANT DISCLAIMER: By choosing to enable the JUCE_USE_MP3AUDIOFORMAT flag and to compile
    this MP3 code into your software, you do so AT YOUR OWN RISK! By doing so, you are agreeing
    that Raw Material Software Limited is in no way responsible for any patent, copyright, or other
    legal issues that you may suffer as a result.

    The code in juce_MP3AudioFormat.cpp is NOT guaranteed to be free from infringements of 3rd-party
    intellectual property. If you wish to use it, please seek your own independent advice about the
    legality of doing so. If you are not willing to accept full responsibility for the consequences
    of using this code, then do not enable this setting.
*/
#ifndef JUCE_USE_MP3AUDIOFORMAT
 #define JUCE_USE_MP3AUDIOFORMAT 0
#endif

/** Config: JUCE_USE_LAME_AUDIO_FORMAT
    Enables the LameEncoderAudioFormat class.
*/
#ifndef JUCE_USE_LAME_AUDIO_FORMAT
 #define JUCE_USE_LAME_AUDIO_FORMAT 0
#endif

/** Config: JUCE_USE_WINDOWS_MEDIA_FORMAT
    Enables the Windows Media SDK codecs.
*/
#ifndef JUCE_USE_WINDOWS_MEDIA_FORMAT
 #define JUCE_USE_WINDOWS_MEDIA_FORMAT 1
#endif

#if ! JUCE_WINDOWS
 #undef JUCE_USE_WINDOWS_MEDIA_FORMAT
 #define JUCE_USE_WINDOWS_MEDIA_FORMAT 0
#endif

//==============================================================================
#include "format/juce_AudioFormatReader.h"
#include "format/juce_AudioFormatWriterOptions.h"
#include "format/juce_AudioFormatWriter.h"
#include "format/juce_MemoryMappedAudioFormatReader.h"
#include "format/juce_AudioFormat.h"
#include "format/juce_AudioFormatManager.h"
#include "format/juce_AudioFormatReaderSource.h"
#include "format/juce_AudioSubsectionReader.h"
#include "format/juce_BufferingAudioFormatReader.h"
#include "codecs/juce_AiffAudioFormat.h"
#include "codecs/juce_CoreAudioFormat.h"
#include "codecs/juce_FlacAudioFormat.h"
#include "codecs/juce_LAMEEncoderAudioFormat.h"
#include "codecs/juce_MP3AudioFormat.h"
#include "codecs/juce_OggVorbisAudioFormat.h"
#include "codecs/juce_WavAudioFormat.h"
#include "codecs/juce_WindowsMediaAudioFormat.h"
#include "sampler/juce_Sampler.h"

#if JucePlugin_Enable_ARA
 #include <juce_audio_processors/juce_audio_processors.h>

 #include "format/juce_ARAAudioReaders.h"
#endif
