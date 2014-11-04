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

#if defined (JUCE_AUDIO_FORMATS_H_INCLUDED) && ! JUCE_AMALGAMATED_INCLUDE
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "../juce_core/native/juce_BasicNativeHeaders.h"
#include "juce_audio_formats.h"

//==============================================================================
#if JUCE_MAC
 #define Point CarbonDummyPointName
 #define Component CarbonDummyCompName
 #if JUCE_QUICKTIME
  #import <QTKit/QTKit.h>
 #endif
 #include <AudioToolbox/AudioToolbox.h>
 #undef Component
 #undef Point

#elif JUCE_IOS
 #import <AudioToolbox/AudioToolbox.h>
 #import <AVFoundation/AVFoundation.h>

//==============================================================================
#elif JUCE_WINDOWS
 #if JUCE_QUICKTIME
  /* If you've got an include error here, you probably need to install the QuickTime SDK and
     add its header directory to your include path.

     Alternatively, if you don't need any QuickTime services, just set the JUCE_QUICKTIME flag to 0.
  */
  #include <Movies.h>
  #include <QTML.h>
  #include <QuickTimeComponents.h>
  #include <MediaHandlers.h>
  #include <ImageCodec.h>

  /* If you've got QuickTime 7 installed, then these COM objects should be found in
     the "\Program Files\Quicktime" directory. You'll need to add this directory to
     your include search path to make these import statements work.
  */
  #import <QTOLibrary.dll>
  #import <QTOControl.dll>

  #if JUCE_MSVC && ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
   #pragma comment (lib, "QTMLClient.lib")
  #endif
 #endif

 #if JUCE_USE_WINDOWS_MEDIA_FORMAT
  #include <wmsdk.h>
 #endif
#endif

//==============================================================================
namespace juce
{

#if JUCE_ANDROID
 #include "../juce_core/native/juce_android_JNIHelpers.h"
 #undef JUCE_QUICKTIME
#endif

#if JUCE_WINDOWS
 #include "../juce_core/native/juce_win32_ComSmartPtr.h"
#endif

#include "format/juce_AudioFormat.cpp"
#include "format/juce_AudioFormatManager.cpp"
#include "format/juce_AudioFormatReader.cpp"
#include "format/juce_AudioFormatReaderSource.cpp"
#include "format/juce_AudioFormatWriter.cpp"
#include "format/juce_AudioSubsectionReader.cpp"
#include "format/juce_BufferingAudioFormatReader.cpp"
#include "sampler/juce_Sampler.cpp"
#include "codecs/juce_AiffAudioFormat.cpp"
#include "codecs/juce_CoreAudioFormat.cpp"
#include "codecs/juce_FlacAudioFormat.cpp"
#include "codecs/juce_MP3AudioFormat.cpp"
#include "codecs/juce_OggVorbisAudioFormat.cpp"
#include "codecs/juce_QuickTimeAudioFormat.cpp"
#include "codecs/juce_WavAudioFormat.cpp"
#include "codecs/juce_LAMEEncoderAudioFormat.cpp"

#if JUCE_WINDOWS && JUCE_USE_WINDOWS_MEDIA_FORMAT
 #include "codecs/juce_WindowsMediaAudioFormat.cpp"
#endif

}
