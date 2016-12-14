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

class ActionBroadcaster::ActionMessage  : public MessageManager::MessageBase
{
public:
    ActionMessage (const ActionBroadcaster* ab,
                   const String& messageText, ActionListener* l) noexcept
        : broadcaster (const_cast<ActionBroadcaster*> (ab)),
          message (messageText),
          listener (l)
    {}

    void messageCallback() override
    {
        if (const ActionBroadcaster* const b = broadcaster)
            if (b->actionListeners.contains (listener))
                listener->actionListenerCallback (message);
    }

private:
    WeakReference<ActionBroadcaster> broadcaster;
    const String message;
    ActionListener* const listener;

    JUCE_DECLARE_NON_COPYABLE (ActionMessage)
};

//==============================================================================
ActionBroadcaster::ActionBroadcaster()
{
    // are you trying to create this object before or after juce has been intialised??
    jassert (MessageManager::getInstanceWithoutCreating() != nullptr);
}

ActionBroadcaster::~ActionBroadcaster()
{
    // all event-based objects must be deleted BEFORE juce is shut down!
    jassert (MessageManager::getInstanceWithoutCreating() != nullptr);

    masterReference.clear();
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
        (new ActionMessage (this, message, actionListeners.getUnchecked(i)))->post();
}
