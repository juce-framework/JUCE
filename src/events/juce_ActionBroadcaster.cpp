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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ActionBroadcaster.h"
#include "juce_MessageManager.h"


//==============================================================================
// special message of our own with a string in it
class ActionMessage  : public Message
{
public:
    ActionMessage (const String& messageText, ActionListener* const listener_) noexcept
        : message (messageText)
    {
        pointerParameter = listener_;
    }

    const String message;

private:
    JUCE_DECLARE_NON_COPYABLE (ActionMessage);
};

ActionBroadcaster::CallbackReceiver::CallbackReceiver() {}

void ActionBroadcaster::CallbackReceiver::handleMessage (const Message& message)
{
    const ActionMessage& am = static_cast <const ActionMessage&> (message);
    ActionListener* const target = static_cast <ActionListener*> (am.pointerParameter);

    if (owner->actionListeners.contains (target))
        target->actionListenerCallback (am.message);
}

//==============================================================================
ActionBroadcaster::ActionBroadcaster()
{
    // are you trying to create this object before or after juce has been intialised??
    jassert (MessageManager::instance != nullptr);

    callback.owner = this;
}

ActionBroadcaster::~ActionBroadcaster()
{
    // all event-based objects must be deleted BEFORE juce is shut down!
    jassert (MessageManager::instance != nullptr);
}

void ActionBroadcaster::addActionListener (ActionListener* const listener)
{
    const ScopedLock sl (actionListenerLock);

    if (listener != nullptr)
        actionListeners.add (listener);
}

void ActionBroadcaster::removeActionListener (ActionListener* const listener)
{
    const ScopedLock sl (actionListenerLock);
    actionListeners.removeValue (listener);
}

void ActionBroadcaster::removeAllActionListeners()
{
    const ScopedLock sl (actionListenerLock);
    actionListeners.clear();
}

void ActionBroadcaster::sendActionMessage (const String& message) const
{
    const ScopedLock sl (actionListenerLock);

    for (int i = actionListeners.size(); --i >= 0;)
        callback.postMessage (new ActionMessage (message, actionListeners.getUnchecked(i)));
}


END_JUCE_NAMESPACE
