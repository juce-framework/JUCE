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

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_events
  vendor:           juce
  version:          4.3.0
  name:             JUCE message and event handling classes
  description:      Classes for running an application's main event loop and sending/receiving messages, timers, etc.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_core
  linuxPackages:    x11

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_EVENTS_H_INCLUDED
#define JUCE_EVENTS_H_INCLUDED

//==============================================================================
#include <juce_core/juce_core.h>

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
#include "native/juce_ScopedXLock.h"

#if JUCE_EVENTS_INCLUDE_WIN32_MESSAGE_WINDOW && JUCE_WINDOWS
 #include "native/juce_win32_HiddenMessageWindow.h"
#endif

}

#endif   // JUCE_EVENTS_H_INCLUDED
