/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ActionBroadcaster.h"
#include "juce_MessageManager.h"


//==============================================================================
ActionBroadcaster::ActionBroadcaster() throw()
{
    // are you trying to create this object before or after juce has been intialised??
    jassert (MessageManager::instance != 0);
}

ActionBroadcaster::~ActionBroadcaster()
{
    // all event-based objects must be deleted BEFORE juce is shut down!
    jassert (MessageManager::instance != 0);
}

void ActionBroadcaster::addActionListener (ActionListener* const listener)
{
    actionListenerList.addActionListener (listener);
}

void ActionBroadcaster::removeActionListener (ActionListener* const listener)
{
    jassert (actionListenerList.isValidMessageListener());

    if (actionListenerList.isValidMessageListener())
        actionListenerList.removeActionListener (listener);
}

void ActionBroadcaster::removeAllActionListeners()
{
    actionListenerList.removeAllActionListeners();
}

void ActionBroadcaster::sendActionMessage (const String& message) const
{
    actionListenerList.sendActionMessage (message);
}

END_JUCE_NAMESPACE
