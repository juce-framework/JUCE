/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_RTAS_DIGICODE_HEADER_JUCEHEADER__
#define __JUCE_RTAS_DIGICODE_HEADER_JUCEHEADER__

#include "../juce_IncludeCharacteristics.h"

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

  #define WINDOWS_VERSION           1
  #define PLUGIN_SDK_BUILD          1
  #define PLUGIN_SDK_DIRECTMIDI     1

  // the Digidesign projects all use a struct alignment of 2..
  #pragma pack (2)
  #pragma warning (disable: 4267 4996 4311 4312 4103)

  #include "ForcedInclude.h"

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

  #include "MacAlwaysInclude.h"

#endif
#endif

#endif   // __JUCE_RTAS_DIGICODE_HEADER_JUCEHEADER__
