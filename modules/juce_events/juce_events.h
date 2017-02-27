/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_events
  vendor:           juce
  version:          4.3.1
  name:             JUCE message and event handling classes
  description:      Classes for running an application's main event loop and sending/receiving messages, timers, etc.
  website:          http://www.juce.com/juce
  license:          ISC

  dependencies:     juce_core

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_EVENTS_H_INCLUDED

#include <juce_core/juce_core.h>

//==============================================================================
/** Config: JUCE_EXECUTE_APP_SUSPEND_ON_IOS_BACKGROUND_TASK
    Will execute your application's suspend method on an iOS background task, giving
    you extra time to save your applications state.
*/
#ifndef JUCE_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK
 #define JUCE_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK 0
#endif

#if JUCE_EVENTS_INCLUDE_WINRT_WRAPPER && JUCE_WINDOWS
 #include <hstring.h>
#endif

namespace juce
{

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
#include "broadcasters/juce_ChangeListener.h"
#include "broadcasters/juce_ChangeBroadcaster.h"
#include "timers/juce_Timer.h"
#include "timers/juce_MultiTimer.h"
#include "interprocess/juce_InterprocessConnection.h"
#include "interprocess/juce_InterprocessConnectionServer.h"
#include "interprocess/juce_ConnectedChildProcess.h"

#if JUCE_LINUX
 #include "native/juce_linux_EventLoop.h"
#endif


#if JUCE_WINDOWS
 #if JUCE_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW
  #include "native/juce_win32_HiddenMessageWindow.h"
 #endif
 #if JUCE_EVENTS_INCLUDE_WINRT_WRAPPER
  #include "native/juce_win32_WinRTWrapper.h"
 #endif
#endif

}
