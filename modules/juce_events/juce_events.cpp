/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef JUCE_EVENTS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#define JUCE_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW 1

#if JUCE_USE_WINRT_MIDI
 #define JUCE_EVENTS_INCLUDE_WINRT_WRAPPER 1
#endif

#include "juce_events.h"

//==============================================================================
#if JUCE_MAC
 #import <IOKit/IOKitLib.h>
 #import <IOKit/IOCFPlugIn.h>
 #import <IOKit/hid/IOHIDLib.h>
 #import <IOKit/hid/IOHIDKeys.h>
 #import <IOKit/pwr_mgt/IOPMLib.h>

#elif JUCE_LINUX || JUCE_BSD
 #include <unistd.h>
#endif

//==============================================================================
#include "messages/juce_ApplicationBase.cpp"
#include "messages/juce_DeletedAtShutdown.cpp"
#include "messages/juce_MessageListener.cpp"
#include "messages/juce_MessageManager.cpp"
#include "broadcasters/juce_ActionBroadcaster.cpp"
#include "broadcasters/juce_AsyncUpdater.cpp"
#include "broadcasters/juce_LockingAsyncUpdater.cpp"
#include "broadcasters/juce_ChangeBroadcaster.cpp"
#include "timers/juce_MultiTimer.cpp"
#include "timers/juce_Timer.cpp"
#include "interprocess/juce_InterprocessConnection.cpp"
#include "interprocess/juce_InterprocessConnectionServer.cpp"
#include "interprocess/juce_ConnectedChildProcess.cpp"
#include "interprocess/juce_NetworkServiceDiscovery.cpp"
#include "native/juce_ScopedLowPowerModeDisabler.cpp"

//==============================================================================
#if JUCE_MAC || JUCE_IOS

 #include "native/juce_MessageQueue_mac.h"

 #if JUCE_MAC
  #include "native/juce_MessageManager_mac.mm"
 #else
  #include "native/juce_MessageManager_ios.mm"
 #endif

#elif JUCE_WINDOWS
 #include "native/juce_RunningInUnity.h"
 #include "native/juce_Messaging_windows.cpp"
 #if JUCE_EVENTS_INCLUDE_WINRT_WRAPPER
  #include "native/juce_WinRTWrapper_windows.cpp"
 #endif

#elif JUCE_LINUX || JUCE_BSD
 #include "native/juce_EventLoopInternal_linux.h"
 #include "native/juce_Messaging_linux.cpp"

#elif JUCE_ANDROID
 #include "native/juce_Messaging_android.cpp"

#endif
