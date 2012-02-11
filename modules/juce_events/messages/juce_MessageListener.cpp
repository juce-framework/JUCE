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


MessageListener::MessageListener() noexcept
{
    // are you trying to create a messagelistener before or after juce has been intialised??
    jassert (MessageManager::instance != nullptr);

    if (MessageManager::instance != nullptr)
        MessageManager::instance->messageListeners.add (this);
}

MessageListener::~MessageListener()
{
    if (MessageManager::instance != nullptr)
        MessageManager::instance->messageListeners.removeValue (this);
}

void MessageListener::postMessage (Message* const message) const
{
    message->messageRecipient = const_cast <MessageListener*> (this);

    if (MessageManager::instance == nullptr)
        MessageManager::getInstance();

    MessageManager::instance->postMessageToQueue (message);
}

bool MessageListener::isValidMessageListener() const noexcept
{
    return MessageManager::instance != nullptr
             && MessageManager::instance->messageListeners.contains (this);
}
