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

#ifndef __JUCE_EVENTS_JUCEHEADER__
#define __JUCE_EVENTS_JUCEHEADER__

//=============================================================================
#include "../juce_core/juce_core.h"

namespace juce
{

// START_AUTOINCLUDE messages, broadcasters, timers,
// interprocess, native/juce_ScopedXLock*
#ifndef __JUCE_APPLICATIONBASE_JUCEHEADER__
 #include "messages/juce_ApplicationBase.h"
#endif
#ifndef __JUCE_CALLBACKMESSAGE_JUCEHEADER__
 #include "messages/juce_CallbackMessage.h"
#endif
#ifndef __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
 #include "messages/juce_DeletedAtShutdown.h"
#endif
#ifndef __JUCE_MESSAGE_JUCEHEADER__
 #include "messages/juce_Message.h"
#endif
#ifndef __JUCE_MESSAGELISTENER_JUCEHEADER__
 #include "messages/juce_MessageListener.h"
#endif
#ifndef __JUCE_MESSAGEMANAGER_JUCEHEADER__
 #include "messages/juce_MessageManager.h"
#endif
#ifndef __JUCE_NOTIFICATIONTYPE_JUCEHEADER__
 #include "messages/juce_NotificationType.h"
#endif
#ifndef __JUCE_ACTIONBROADCASTER_JUCEHEADER__
 #include "broadcasters/juce_ActionBroadcaster.h"
#endif
#ifndef __JUCE_ACTIONLISTENER_JUCEHEADER__
 #include "broadcasters/juce_ActionListener.h"
#endif
#ifndef __JUCE_ASYNCUPDATER_JUCEHEADER__
 #include "broadcasters/juce_AsyncUpdater.h"
#endif
#ifndef __JUCE_CHANGEBROADCASTER_JUCEHEADER__
 #include "broadcasters/juce_ChangeBroadcaster.h"
#endif
#ifndef __JUCE_CHANGELISTENER_JUCEHEADER__
 #include "broadcasters/juce_ChangeListener.h"
#endif
#ifndef __JUCE_LISTENERLIST_JUCEHEADER__
 #include "broadcasters/juce_ListenerList.h"
#endif
#ifndef __JUCE_MULTITIMER_JUCEHEADER__
 #include "timers/juce_MultiTimer.h"
#endif
#ifndef __JUCE_TIMER_JUCEHEADER__
 #include "timers/juce_Timer.h"
#endif
#ifndef __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__
 #include "interprocess/juce_InterprocessConnection.h"
#endif
#ifndef __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__
 #include "interprocess/juce_InterprocessConnectionServer.h"
#endif
#ifndef __JUCE_SCOPEDXLOCK_JUCEHEADER__
 #include "native/juce_ScopedXLock.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_EVENTS_JUCEHEADER__
