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


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_events
  vendor:             juce
  version:            8.0.2
  name:               JUCE message and event handling classes
  description:        Classes for running an application's main event loop and sending/receiving messages, timers, etc.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_core

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_EVENTS_H_INCLUDED

#include <juce_core/juce_core.h>

//==============================================================================
/** Config: JUCE_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK
    Will execute your application's suspend method on an iOS background task, giving
    you extra time to save your applications state.
*/
#ifndef JUCE_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK
 #define JUCE_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK 0
#endif

#if JUCE_WINDOWS && JUCE_EVENTS_INCLUDE_WINRT_WRAPPER
 // If this header file is missing then you are probably attempting to use WinRT
 // functionality without the WinRT libraries installed on your system. Try installing
 // the latest Windows Standalone SDK and maybe also adding the path to the WinRT
 // headers to your build system. This path should have the form
 // "C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\winrt".
 #include <inspectable.h>
 #include <hstring.h>
#endif

#include "messages/juce_MessageManager.h"
#include "messages/juce_Message.h"
#include "messages/juce_MessageListener.h"
#include "messages/juce_CallbackMessage.h"
#include "messages/juce_DeletedAtShutdown.h"
#include "messages/juce_NotificationType.h"
#include "messages/juce_ApplicationBase.h"
#include "messages/juce_Initialisation.h"
#include "messages/juce_MountedVolumeListChangeDetector.h"
#include "broadcasters/juce_ActionBroadcaster.h"
#include "broadcasters/juce_ActionListener.h"
#include "broadcasters/juce_AsyncUpdater.h"
#include "broadcasters/juce_LockingAsyncUpdater.h"
#include "broadcasters/juce_ChangeListener.h"
#include "broadcasters/juce_ChangeBroadcaster.h"
#include "timers/juce_Timer.h"
#include "timers/juce_TimedCallback.h"
#include "timers/juce_MultiTimer.h"
#include "interprocess/juce_ChildProcessManager.h"
#include "interprocess/juce_InterprocessConnection.h"
#include "interprocess/juce_InterprocessConnectionServer.h"
#include "interprocess/juce_ConnectedChildProcess.h"
#include "interprocess/juce_NetworkServiceDiscovery.h"
#include "native/juce_ScopedLowPowerModeDisabler.h"

#if JUCE_LINUX || JUCE_BSD
 #include "native/juce_EventLoop_linux.h"
#endif

#if JUCE_WINDOWS
 #if JUCE_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW
  #include "native/juce_HiddenMessageWindow_windows.h"
 #endif
 #if JUCE_EVENTS_INCLUDE_WINRT_WRAPPER
  #include "native/juce_WinRTWrapper_windows.h"
 #endif
#endif
