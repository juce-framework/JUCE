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

#ifndef __JUCE_AUDIO_FORMATS_JUCEHEADER__
#define __JUCE_AUDIO_FORMATS_JUCEHEADER__

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

//=============================================================================
BEGIN_JUCE_NAMESPACE

// START_AUTOINCLUDE format, codecs, sampler
#ifndef __JUCE_AUDIOFORMAT_JUCEHEADER__
 #include "format/juce_AudioFormat.h"
#endif
#ifndef __JUCE_AUDIOFORMATMANAGER_JUCEHEADER__
 #include "format/juce_AudioFormatManager.h"
#endif
#ifndef __JUCE_AUDIOFORMATREADER_JUCEHEADER__
 #include "format/juce_AudioFormatReader.h"
#endif
#ifndef __JUCE_AUDIOFORMATREADERSOURCE_JUCEHEADER__
 #include "format/juce_AudioFormatReaderSource.h"
#endif
#ifndef __JUCE_AUDIOFORMATWRITER_JUCEHEADER__
 #include "format/juce_AudioFormatWriter.h"
#endif
#ifndef __JUCE_AUDIOSUBSECTIONREADER_JUCEHEADER__
 #include "format/juce_AudioSubsectionReader.h"
#endif
#ifndef __JUCE_AIFFAUDIOFORMAT_JUCEHEADER__
 #include "codecs/juce_AiffAudioFormat.h"
#endif
#ifndef __JUCE_COREAUDIOFORMAT_JUCEHEADER__
 #include "codecs/juce_CoreAudioFormat.h"
#endif
#ifndef __JUCE_FLACAUDIOFORMAT_JUCEHEADER__
 #include "codecs/juce_FlacAudioFormat.h"
#endif
#ifndef __JUCE_OGGVORBISAUDIOFORMAT_JUCEHEADER__
 #include "codecs/juce_OggVorbisAudioFormat.h"
#endif
#ifndef __JUCE_QUICKTIMEAUDIOFORMAT_JUCEHEADER__
 #include "codecs/juce_QuickTimeAudioFormat.h"
#endif
#ifndef __JUCE_WAVAUDIOFORMAT_JUCEHEADER__
 #include "codecs/juce_WavAudioFormat.h"
#endif
#ifndef __JUCE_SAMPLER_JUCEHEADER__
 #include "sampler/juce_Sampler.h"
#endif
// END_AUTOINCLUDE

END_JUCE_NAMESPACE

#endif   // __JUCE_AUDIO_FORMATS_JUCEHEADER__
