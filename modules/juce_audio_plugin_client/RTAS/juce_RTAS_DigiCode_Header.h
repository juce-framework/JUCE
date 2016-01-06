/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_RTAS_DIGICODE_HEADER_H_INCLUDED
#define JUCE_RTAS_DIGICODE_HEADER_H_INCLUDED

#if JucePlugin_Build_RTAS

//==============================================================================
#if JucePlugin_Build_RTAS
#ifdef _MSC_VER

  #define kCompileAsCodeResource    0
  #define kBuildStandAlone          0
  #define kNoDSP                    0
  #define kNoDAE                    0
  #define kNoSDS                    0
  #define kNoViews                  0
  #define kUseDSPCodeDecode         0

  #define WIN32                     1
  #define WINDOWS_VERSION           1
  #define PLUGIN_SDK_BUILD          1
  #define PLUGIN_SDK_DIRECTMIDI     1

  // the Digidesign projects all use a struct alignment of 2..
  #pragma pack (2)
  #pragma warning (disable: 4267 4996 4311 4312 4103 4121 4100 4127 4189 4245 4389 4512 4701)

  #include <ForcedInclude.h>

#else

  #define kCompileAsCodeResource    0
  #define kNoDSP                    1
  #define kNoDAE                    0
  #define kNoSDS                    0
  #define kNoViews                  0
  #define kUseDSPCodeDecode         0

  #define MAC_VERSION               1
  #define PLUGIN_SDK_BUILD          1
  #define PLUGIN_SDK_DIRECTMIDI     1
  #define DIGI_PASCAL

  #define Point CarbonDummyPointName
  #define Component CarbonDummyCompName
  #include <MacAlwaysInclude.h>

#endif
#endif
#endif

#endif   // JUCE_RTAS_DIGICODE_HEADER_H_INCLUDED
