/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_EVENTS_H_INCLUDED
#define JUCE_EVENTS_H_INCLUDED

//=============================================================================
#include "../juce_core/juce_core.h"

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
#include "broadcasters/juce_ListenerList.h"
#include "broadcasters/juce_ActionBroadcaster.h"
#include "broadcasters/juce_ActionListener.h"
#include "broadcasters/juce_AsyncUpdater.h"
#include "broadcasters/juce_ChangeListener.h"
#include "broadcasters/juce_ChangeBroadcaster.h"
#include "timers/juce_Timer.h"
#include "timers/juce_MultiTimer.h"
#include "interprocess/juce_InterprocessConnection.h"
#include "interprocess/juce_InterprocessConnectionServer.h"
#include "native/juce_ScopedXLock.h"

}

#endif   // JUCE_EVENTS_H_INCLUDED
