/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
#include "interprocess/juce_ChildProcessManager.cpp"
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
