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

#ifndef JUCE_AUDIO_FORMATS_H_INCLUDED
#define JUCE_AUDIO_FORMATS_H_INCLUDED

#include "../juce_audio_basics/juce_audio_basics.h"

//=============================================================================
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
    that Raw Material Software is in no way responsible for any patent, copyright, or other
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

#if ! JUCE_MSVC
 #undef JUCE_USE_WINDOWS_MEDIA_FORMAT
 #define JUCE_USE_WINDOWS_MEDIA_FORMAT 0
#endif

//=============================================================================
namespace juce
{

class AudioFormat;
#include "format/juce_AudioFormatReader.h"
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
#include "codecs/juce_QuickTimeAudioFormat.h"
#include "codecs/juce_WavAudioFormat.h"
#include "codecs/juce_WindowsMediaAudioFormat.h"
#include "sampler/juce_Sampler.h"

}

#endif   // JUCE_AUDIO_FORMATS_H_INCLUDED
