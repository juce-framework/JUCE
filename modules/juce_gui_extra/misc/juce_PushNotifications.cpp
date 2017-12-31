/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
#if ! JUCE_ANDROID && ! JUCE_IOS && ! JUCE_MAC
bool PushNotifications::Notification::isValid() const noexcept { return true; }
#endif

//==============================================================================
JUCE_IMPLEMENT_SINGLETON (PushNotifications)

PushNotifications::PushNotifications()
  #if JUCE_PUSH_NOTIFICATIONS
    : pimpl (new Pimpl (*this))
  #endif
{
}

PushNotifications::~PushNotifications() { clearSingletonInstance(); }

void PushNotifications::addListener (Listener* l)      { listeners.add (l); }
void PushNotifications::removeListener (Listener* l)   { listeners.remove (l); }

void PushNotifications::requestPermissionsWithSettings (const PushNotifications::Settings& settings)
{
  #if JUCE_PUSH_NOTIFICATIONS && (JUCE_IOS || JUCE_MAC)
    pimpl->requestPermissionsWithSettings (settings);
  #else
    ignoreUnused (settings);
    listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
  #endif
}

void PushNotifications::requestSettingsUsed()
{
  #if JUCE_PUSH_NOTIFICATIONS && (JUCE_IOS || JUCE_MAC)
    pimpl->requestSettingsUsed();
  #else
    listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
  #endif
}

bool PushNotifications::areNotificationsEnabled() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    return pimpl->areNotificationsEnabled();
  #else
    return false;
  #endif
}

void PushNotifications::getDeliveredNotifications() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->getDeliveredNotifications();
  #endif
}

void PushNotifications::removeAllDeliveredNotifications()
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removeAllDeliveredNotifications();
  #endif
}

String PushNotifications::getDeviceToken() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    return pimpl->getDeviceToken();
  #else
    return {};
  #endif
}

void PushNotifications::setupChannels (const Array<ChannelGroup>& groups, const Array<Channel>& channels)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->setupChannels (groups, channels);
  #else
    ignoreUnused (groups, channels);
  #endif
}

void PushNotifications::getPendingLocalNotifications() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->getPendingLocalNotifications();
  #endif
}

void PushNotifications::removeAllPendingLocalNotifications()
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removeAllPendingLocalNotifications();
  #endif
}

void PushNotifications::subscribeToTopic (const String& topic)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->subscribeToTopic (topic);
  #else
    ignoreUnused (topic);
  #endif
}

void PushNotifications::unsubscribeFromTopic (const String& topic)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->unsubscribeFromTopic (topic);
  #else
    ignoreUnused (topic);
  #endif
}


void PushNotifications::sendLocalNotification (const Notification& n)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->sendLocalNotification (n);
  #else
    ignoreUnused (n);
  #endif
}

void PushNotifications::removeDeliveredNotification (const String& identifier)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removeDeliveredNotification (identifier);
  #else
    ignoreUnused (identifier);
  #endif
}

void PushNotifications::removePendingLocalNotification (const String& identifier)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removePendingLocalNotification (identifier);
  #else
    ignoreUnused (identifier);
  #endif
}

void PushNotifications::sendUpstreamMessage (const String& serverSenderId,
                                             const String& collapseKey,
                                             const String& messageId,
                                             const String& messageType,
                                             int timeToLive,
                                             const StringPairArray& additionalData)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->sendUpstreamMessage (serverSenderId,
                                collapseKey,
                                messageId,
                                messageType,
                                timeToLive,
                                additionalData);
  #else
    ignoreUnused (serverSenderId, collapseKey, messageId, messageType);
    ignoreUnused (timeToLive, additionalData);
  #endif
}

} // namespace juce
