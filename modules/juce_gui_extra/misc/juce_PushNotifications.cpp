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

namespace juce
{

//==============================================================================
#if ! JUCE_PUSH_NOTIFICATIONS_IMPL

struct PushNotifications::Impl
{
    explicit Impl (PushNotifications& o) : owner (o) {}

    void requestPermissionsWithSettings (const Settings&) const
    {
        owner.listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
    }

    void requestSettingsUsed() const
    {
        owner.listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
    }

    bool areNotificationsEnabled() const { return false; }
    void getDeliveredNotifications() const {}
    void removeAllDeliveredNotifications() const {}
    String getDeviceToken() const { return {}; }
    void setupChannels (const Array<ChannelGroup>&, const Array<Channel>&) const {}
    void getPendingLocalNotifications() const {}
    void removeAllPendingLocalNotifications() const {}
    void subscribeToTopic (const String&) const {}
    void unsubscribeFromTopic (const String&) const {}
    void sendLocalNotification (const Notification&) const {}
    void removeDeliveredNotification (const String&) const {}
    void removePendingLocalNotification (const String&) const {}
    void sendUpstreamMessage (const String&,
                              const String&,
                              const String&,
                              const String&,
                              int,
                              const StringPairArray&) const {}

private:
    PushNotifications& owner;
};

bool PushNotifications::Notification::isValid() const noexcept { return true; }

#endif

//==============================================================================
PushNotifications::Notification::Notification (const Notification& other)
    : identifier (other.identifier),
      title (other.title),
      body (other.body),
      subtitle (other.subtitle),
      groupId (other.groupId),
      badgeNumber (other.badgeNumber),
      soundToPlay (other.soundToPlay),
      properties (other.properties),
      category (other.category),
      triggerIntervalSec (other.triggerIntervalSec),
      repeat (other.repeat),
      icon (other.icon),
      channelId (other.channelId),
      largeIcon (other.largeIcon),
      tickerText (other.tickerText),
      actions (other.actions),
      progress (other.progress),
      person (other.person),
      type (other.type),
      priority (other.priority),
      lockScreenAppearance (other.lockScreenAppearance),
      publicVersion (other.publicVersion.get() != nullptr ? new Notification (*other.publicVersion) : nullptr),
      groupSortKey (other.groupSortKey),
      groupSummary (other.groupSummary),
      accentColour (other.accentColour),
      ledColour (other.ledColour),
      ledBlinkPattern (other.ledBlinkPattern),
      vibrationPattern (other.vibrationPattern),
      shouldAutoCancel (other.shouldAutoCancel),
      localOnly (other.localOnly),
      ongoing (other.ongoing),
      alertOnlyOnce (other.alertOnlyOnce),
      timestampVisibility (other.timestampVisibility),
      badgeIconType (other.badgeIconType),
      groupAlertBehaviour (other.groupAlertBehaviour),
      timeoutAfterMs (other.timeoutAfterMs)
{
}

//==============================================================================
PushNotifications::PushNotifications()
    : pimpl (new Impl (*this))
{
}

PushNotifications::~PushNotifications() { clearSingletonInstance(); }

void PushNotifications::addListener (Listener* l)      { listeners.add (l); }
void PushNotifications::removeListener (Listener* l)   { listeners.remove (l); }

void PushNotifications::requestPermissionsWithSettings (const Settings& settings)
{
    pimpl->requestPermissionsWithSettings (settings);
}

void PushNotifications::requestSettingsUsed()
{
    pimpl->requestSettingsUsed();
}

bool PushNotifications::areNotificationsEnabled() const
{
    return pimpl->areNotificationsEnabled();
}

void PushNotifications::getDeliveredNotifications() const
{
    pimpl->getDeliveredNotifications();
}

void PushNotifications::removeAllDeliveredNotifications()
{
    pimpl->removeAllDeliveredNotifications();
}

String PushNotifications::getDeviceToken() const
{
    return pimpl->getDeviceToken();
}

void PushNotifications::setupChannels (const Array<ChannelGroup>& groups,
                                       const Array<Channel>& channels)
{
    pimpl->setupChannels (groups, channels);
}

void PushNotifications::getPendingLocalNotifications() const
{
    pimpl->getPendingLocalNotifications();
}

void PushNotifications::removeAllPendingLocalNotifications()
{
    pimpl->removeAllPendingLocalNotifications();
}

void PushNotifications::subscribeToTopic (const String& topic)
{
    pimpl->subscribeToTopic (topic);
}

void PushNotifications::unsubscribeFromTopic (const String& topic)
{
    pimpl->unsubscribeFromTopic (topic);
}

void PushNotifications::sendLocalNotification (const Notification& n)
{
    pimpl->sendLocalNotification (n);
}

void PushNotifications::removeDeliveredNotification (const String& identifier)
{
    pimpl->removeDeliveredNotification (identifier);
}

void PushNotifications::removePendingLocalNotification (const String& identifier)
{
    pimpl->removePendingLocalNotification (identifier);
}

void PushNotifications::sendUpstreamMessage (const String& serverSenderId,
                                             const String& collapseKey,
                                             const String& messageId,
                                             const String& messageType,
                                             int timeToLive,
                                             const StringPairArray& additionalData)
{
    pimpl->sendUpstreamMessage (serverSenderId,
                                collapseKey,
                                messageId,
                                messageType,
                                timeToLive,
                                additionalData);
}

//==============================================================================
void PushNotifications::Listener::notificationSettingsReceived ([[maybe_unused]] const Settings& settings) {}
void PushNotifications::Listener::pendingLocalNotificationsListReceived ([[maybe_unused]] const Array<Notification>& notifications) {}
void PushNotifications::Listener::handleNotification ([[maybe_unused]] bool isLocalNotification,
                                                      [[maybe_unused]] const Notification& notification) {}
void PushNotifications::Listener::handleNotificationAction ([[maybe_unused]] bool isLocalNotification,
                                                            [[maybe_unused]] const Notification& notification,
                                                            [[maybe_unused]] const String& actionIdentifier,
                                                            [[maybe_unused]] const String& optionalResponse) {}
void PushNotifications::Listener::localNotificationDismissedByUser ([[maybe_unused]] const Notification& notification) {}
void PushNotifications::Listener::deliveredNotificationsListReceived ([[maybe_unused]] const Array<Notification>& notifications) {}
void PushNotifications::Listener::deviceTokenRefreshed ([[maybe_unused]] const String& token) {}
void PushNotifications::Listener::remoteNotificationsDeleted() {}
void PushNotifications::Listener::upstreamMessageSent ([[maybe_unused]] const String& messageId) {}
void PushNotifications::Listener::upstreamMessageSendingError ([[maybe_unused]] const String& messageId,
                                                               [[maybe_unused]] const String& error) {}

//==============================================================================
void privatePostSystemNotification (const String& notificationTitle, const String& notificationBody);
void privatePostSystemNotification ([[maybe_unused]] const String& notificationTitle,
                                    [[maybe_unused]] const String& notificationBody)
{
  #if JUCE_PUSH_NOTIFICATIONS
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    auto* notificationsInstance = PushNotifications::getInstance();

    if (notificationsInstance == nullptr)
        return;

   #if JUCE_ANDROID
    notificationsInstance->requestPermissionsWithSettings ({});

    static auto channels = std::invoke ([]() -> Array<PushNotifications::Channel>
    {
        PushNotifications::Channel chan;

        chan.identifier = "1";
        chan.name = "Notifications";
        chan.description = "Accessibility notifications";
        chan.groupId = "accessibility";
        chan.ledColour = Colours::yellow;
        chan.canShowBadge = true;
        chan.enableLights = true;
        chan.enableVibration = true;
        chan.soundToPlay = URL ("default_os_sound");
        chan.vibrationPattern = { 1000, 1000 };

        return { chan };
    });

    notificationsInstance->setupChannels ({ PushNotifications::ChannelGroup { "accessibility", "accessibility" } },
                                          channels);
   #else
    static auto settings = std::invoke ([]
    {
        PushNotifications::Settings s;
        s.allowAlert = true;
        s.allowBadge = true;
        s.allowSound = true;

       #if JUCE_IOS
        PushNotifications::Settings::Category c;
        c.identifier = "Accessibility";

        s.categories = { c };
       #endif

        return s;
    });

    notificationsInstance->requestPermissionsWithSettings (settings);
   #endif

    const auto notification = std::invoke ([&notificationTitle, &notificationBody]
    {
        PushNotifications::Notification n;

        n.identifier = String (Random::getSystemRandom().nextInt());
        n.title = notificationTitle;
        n.body = notificationBody;

       #if JUCE_IOS
        n.category = "Accessibility";
       #elif JUCE_ANDROID
        n.channelId = "1";
        n.icon = "accessibilitynotificationicon";
       #endif

        return n;
    });

    if (notification.isValid())
        notificationsInstance->sendLocalNotification (notification);

   #else
    SystemTrayIconComponent systemTrayIcon;

    Image im (Image::ARGB, 128, 128, true);
    systemTrayIcon.setIconImage (im, im);

    systemTrayIcon.showInfoBubble (notificationTitle, notificationBody);
   #endif
  #endif
}

} // namespace juce
