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

 #define Point CarbonDummyPointName

 #if JUCE_SUPPORT_CARBON
  #define Component CarbonDummyCompName
 #endif

 #ifdef __OBJC__
  #include <Cocoa/Cocoa.h>
 #endif

 #if JUCE_SUPPORT_CARBON
  #include <Carbon/Carbon.h>
  #undef Component
 #endif

 #undef Point

 #include <objc/runtime.h>
 #include <objc/objc.h>
 #include <objc/message.h>
#endif
