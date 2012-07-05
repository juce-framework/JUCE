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

#else
 #if ! (defined (JUCE_SUPPORT_CARBON) || defined (__LP64__))
  #define JUCE_SUPPORT_CARBON 1
 #endif

 #if JUCE_SUPPORT_CARBON
  #define Point CarbonDummyPointName
  #define Component CarbonDummyCompName
  #include <Cocoa/Cocoa.h>
  #include <Carbon/Carbon.h>
  #undef Point
  #undef Component
 #else
  #include <Cocoa/Cocoa.h>
 #endif
 #include <objc/runtime.h>
 #include <objc/objc.h>
 #include <objc/message.h>
#endif
