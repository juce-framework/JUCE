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

#if JUCE_WINDOWS
 #undef _WIN32_WINNT
 #define _WIN32_WINNT 0x500
 #undef STRICT
 #define STRICT 1
 #include <windows.h>
 #include <float.h>
 #pragma warning (disable : 4312 4355)
 #ifdef __INTEL_COMPILER
  #pragma warning (disable : 1899)
 #endif

#elif JUCE_LINUX
 #include <float.h>
 #include <sys/time.h>
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 #include <X11/Xatom.h>
 #undef Font
 #undef KeyPress
 #undef Drawable
 #undef Time
#elif JUCE_ANDROID
#else
 #if ! (defined (JUCE_SUPPORT_CARBON) || defined (__LP64__))
  #define JUCE_SUPPORT_CARBON 1
 #endif

 #ifdef __OBJC__
  #if JUCE_MAC
   #include <Cocoa/Cocoa.h>
  #elif JUCE_IOS
   #include <UIKit/UIKit.h>
  #else
   #error
  #endif
 #endif

 #if JUCE_SUPPORT_CARBON && (! JUCE_IOS)
  #include <Carbon/Carbon.h>
 #endif

 #include <objc/runtime.h>
 #include <objc/objc.h>
 #include <objc/message.h>
#endif
