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
  #define _STDINT_H                 1

  // the Digidesign projects all use a struct alignment of 2..
  #pragma pack (2)
  #pragma warning (disable: 4267 4996 4311 4312 4103 4121 4100 4127 4189 4245 4389 4512 4701 4703)

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

  #include <MacAlwaysInclude.h>

#endif
#endif
