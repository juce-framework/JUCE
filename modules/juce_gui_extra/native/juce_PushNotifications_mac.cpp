/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

namespace juce
{

namespace PushNotificationsDelegateDetailsOsx
{
    using Action = PushNotifications::Notification::Action;

    //==============================================================================
    static NSUserNotification* juceNotificationToNSUserNotification (const PushNotifications::Notification& n)
    {
        auto notification = [[NSUserNotification alloc] init];

        notification.title           = juceStringToNS (n.title);
        notification.subtitle        = juceStringToNS (n.subtitle);
        notification.informativeText = juceStringToNS (n.body);
        notification.userInfo = varToNSDictionary (n.properties);

        auto triggerTime = Time::getCurrentTime() + RelativeTime (n.triggerIntervalSec);
        notification.deliveryDate = [NSDate dateWithTimeIntervalSince1970: (double) triggerTime.toMilliseconds() / 1000.0];

        if (n.repeat && n.triggerIntervalSec >= 60)
        {
            auto dateComponents = [[NSDateComponents alloc] init];
            auto intervalSec = NSInteger (n.triggerIntervalSec);
            dateComponents.second = intervalSec;
            dateComponents.nanosecond = NSInteger ((n.triggerIntervalSec - (double) intervalSec) * 1000000000);

            notification.deliveryRepeatInterval = dateComponents;

            [dateComponents autorelease];
        }

        auto soundToPlayString = n.soundToPlay.toString (true);

        if (soundToPlayString == "default_os_sound")
        {
            notification.soundName = NSUserNotificationDefaultSoundName;
        }
        else if (soundToPlayString.isNotEmpty())
        {
            auto* soundName = juceStringToNS (soundToPlayString.fromLastOccurrenceOf ("/", false, false)
                                                               .upToLastOccurrenceOf (".", false, false));

            notification.soundName = soundName;
        }

        notification.hasActionButton = n.actions.size() > 0;

        if (n.actions.size() > 0)
            notification.actionButtonTitle = juceStringToNS (n.actions.getReference (0).title);

        if (@available (macOS 10.9, *))
        {
            notification.identifier = juceStringToNS (n.identifier);

            if (n.actions.size() > 0)
            {
                notification.hasReplyButton = n.actions.getReference (0).style == Action::text;
                notification.responsePlaceholder = juceStringToNS (n.actions.getReference (0).textInputPlaceholder);
            }

            auto* imageDirectory = n.icon.contains ("/")
                                 ? juceStringToNS (n.icon.upToLastOccurrenceOf ("/", false, true))
                                 : [NSString string];

            auto* imageName      = juceStringToNS (n.icon.fromLastOccurrenceOf ("/", false, false)
                                                         .upToLastOccurrenceOf (".", false, false));
            auto* imageExtension = juceStringToNS (n.icon.fromLastOccurrenceOf (".", false, false));

            NSString* imagePath = nil;

            if ([imageDirectory length] == NSUInteger (0))
            {
                imagePath = [[NSBundle mainBundle] pathForResource: imageName
                                                            ofType: imageExtension];
            }
            else
            {
                imagePath = [[NSBundle mainBundle] pathForResource: imageName
                                                            ofType: imageExtension
                                                       inDirectory: imageDirectory];
            }

            notification.contentImage = [[NSImage alloc] initWithContentsOfFile: imagePath];

            if (@available (macOS 10.10, *))
            {
                if (n.actions.size() > 1)
                {
                    auto additionalActions = [NSMutableArray arrayWithCapacity: (NSUInteger) n.actions.size() - 1];

                    for (int a = 1; a < n.actions.size(); ++a)
                        [additionalActions addObject: [NSUserNotificationAction actionWithIdentifier: juceStringToNS (n.actions[a].identifier)
                                                                                               title: juceStringToNS (n.actions[a].title)]];

                    notification.additionalActions = additionalActions;
                }
            }
        }

        [notification autorelease];

        return notification;
    }

    //==============================================================================
    static PushNotifications::Notification nsUserNotificationToJuceNotification (NSUserNotification* n)
    {
        PushNotifications::Notification notif;

        notif.title       = nsStringToJuce (n.title);
        notif.subtitle    = nsStringToJuce (n.subtitle);
        notif.body        = nsStringToJuce (n.informativeText);

        notif.repeat = n.deliveryRepeatInterval != nil;

        if (n.deliveryRepeatInterval != nil)
        {
            notif.triggerIntervalSec = (double) n.deliveryRepeatInterval.second + ((double) n.deliveryRepeatInterval.nanosecond / 1000000000.0);
        }
        else
        {
            NSDate* dateNow = [NSDate date];
            NSDate* deliveryDate = n.deliveryDate;

            notif.triggerIntervalSec = [dateNow timeIntervalSinceDate: deliveryDate];
        }

        notif.soundToPlay = URL (nsStringToJuce (n.soundName));
        notif.properties  = nsDictionaryToVar (n.userInfo);

        if (@available (macOS 10.9, *))
        {
            notif.identifier = nsStringToJuce (n.identifier);

            if (n.contentImage != nil)
                notif.icon = nsStringToJuce ([n.contentImage name]);
        }

        Array<Action> actions;

        if (n.actionButtonTitle != nil)
        {
            Action action;
            action.title = nsStringToJuce (n.actionButtonTitle);

            if (@available (macOS 10.9, *))
            {
                if (n.hasReplyButton)
                    action.style = Action::text;

                if (n.responsePlaceholder != nil)
                    action.textInputPlaceholder = nsStringToJuce (n.responsePlaceholder);
            }

            actions.add (action);
        }

        if (@available (macOS 10.10, *))
        {
            if (n.additionalActions != nil)
            {
                for (NSUserNotificationAction* a in n.additionalActions)
                {
                    Action action;
                    action.identifier = nsStringToJuce (a.identifier);
                    action.title      = nsStringToJuce (a.title);

                    actions.add (action);
                }
            }
        }

        return notif;
    }

    //==============================================================================
    static var getNotificationPropertiesFromDictionaryVar (const var& dictionaryVar)
    {
        auto* dictionaryVarObject = dictionaryVar.getDynamicObject();

        if (dictionaryVarObject == nullptr)
            return {};

        const auto& properties = dictionaryVarObject->getProperties();

        DynamicObject::Ptr propsVarObject = new DynamicObject();

        for (int i = 0; i < properties.size(); ++i)
        {
            auto propertyName = properties.getName (i).toString();

            if (propertyName == "aps")
                continue;

            propsVarObject->setProperty (propertyName, properties.getValueAt (i));
        }

        return var (propsVarObject.get());
    }

    static PushNotifications::Notification nsDictionaryToJuceNotification (NSDictionary* dictionary)
    {
        const var dictionaryVar = nsDictionaryToVar (dictionary);

        const var apsVar = dictionaryVar.getProperty ("aps", {});

        if (! apsVar.isObject())
            return {};

        var alertVar = apsVar.getProperty ("alert", {});

        const var titleVar = alertVar.getProperty ("title", {});
        const var bodyVar  = alertVar.isObject() ? alertVar.getProperty ("body", {}) : alertVar;

        const var categoryVar = apsVar.getProperty ("category", {});
        const var soundVar    = apsVar.getProperty ("sound", {});
        const var badgeVar    = apsVar.getProperty ("badge", {});
        const var threadIdVar = apsVar.getProperty ("thread-id", {});

        PushNotifications::Notification notification;

        notification.title       = titleVar   .toString();
        notification.body        = bodyVar    .toString();
        notification.groupId     = threadIdVar.toString();
        notification.category    = categoryVar.toString();
        notification.soundToPlay = URL (soundVar.toString());
        notification.badgeNumber = (int) badgeVar;
        notification.properties  = getNotificationPropertiesFromDictionaryVar (dictionaryVar);

        return notification;
    }
}

//==============================================================================
struct PushNotificationsDelegate
{
    PushNotificationsDelegate() : delegate ([getClass().createInstance() init])
    {
        Class::setThis (delegate.get(), this);

        id<NSApplicationDelegate> appDelegate = [[NSApplication sharedApplication] delegate];

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        if ([appDelegate respondsToSelector: @selector (setPushNotificationsDelegate:)])
            [appDelegate performSelector: @selector (setPushNotificationsDelegate:) withObject: delegate.get()];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        [NSUserNotificationCenter defaultUserNotificationCenter].delegate = delegate.get();
    }

    virtual ~PushNotificationsDelegate()
    {
        [NSUserNotificationCenter defaultUserNotificationCenter].delegate = nil;
    }

    virtual void registeredForRemoteNotifications (NSData* deviceToken) = 0;

    virtual void failedToRegisterForRemoteNotifications (NSError* error) = 0;

    virtual void didReceiveRemoteNotification (NSDictionary* userInfo) = 0;

    virtual void didDeliverNotification (NSUserNotification* notification) = 0;

    virtual void didActivateNotification (NSUserNotification* notification) = 0;

    virtual bool shouldPresentNotification (NSUserNotification* notification) = 0;

protected:
    NSUniquePtr<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>> delegate;

private:
    struct Class final : public ObjCClass<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>>
    {
        Class() : ObjCClass<NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate>> ("JucePushNotificationsDelegate_")
        {
            addIvar<PushNotificationsDelegate*> ("self");

            addMethod (@selector (application:didRegisterForRemoteNotificationsWithDeviceToken:), registeredForRemoteNotifications);
            addMethod (@selector (application:didFailToRegisterForRemoteNotificationsWithError:), failedToRegisterForRemoteNotifications);
            addMethod (@selector (application:didReceiveRemoteNotification:),                     didReceiveRemoteNotification);
            addMethod (@selector (userNotificationCenter:didDeliverNotification:),                didDeliverNotification);
            addMethod (@selector (userNotificationCenter:didActivateNotification:),               didActivateNotification);
            addMethod (@selector (userNotificationCenter:shouldPresentNotification:),             shouldPresentNotification);

            registerClass();
        }

        //==============================================================================
        static PushNotificationsDelegate& getThis (id self)         { return *getIvar<PushNotificationsDelegate*> (self, "self"); }
        static void setThis (id self, PushNotificationsDelegate* d) { object_setInstanceVariable (self, "self", d); }

        //==============================================================================
        static void registeredForRemoteNotifications       (id self, SEL, NSApplication*,
                                                            NSData* deviceToken)                { getThis (self).registeredForRemoteNotifications (deviceToken); }

        static void failedToRegisterForRemoteNotifications (id self, SEL, NSApplication*,
                                                            NSError* error)                     { getThis (self).failedToRegisterForRemoteNotifications (error); }

        static void didReceiveRemoteNotification           (id self, SEL, NSApplication*,
                                                            NSDictionary* userInfo)             { getThis (self).didReceiveRemoteNotification (userInfo); }

        static void didDeliverNotification          (id self, SEL, NSUserNotificationCenter*,
                                                     NSUserNotification* notification)          { getThis (self).didDeliverNotification (notification); }

        static void didActivateNotification         (id self, SEL, NSUserNotificationCenter*,
                                                     NSUserNotification* notification)          { getThis (self).didActivateNotification (notification); }

        static bool shouldPresentNotification       (id self, SEL, NSUserNotificationCenter*,
                                                     NSUserNotification* notification)          { return getThis (self).shouldPresentNotification (notification); }
    };

    //==============================================================================
    static Class& getClass()
    {
        static Class c;
        return c;
    }
};

//==============================================================================
bool PushNotifications::Notification::isValid() const noexcept { return true; }

//==============================================================================
struct PushNotifications::Pimpl : private PushNotificationsDelegate
{
    Pimpl (PushNotifications& p)
        : owner (p)
    {
    }

    void requestPermissionsWithSettings (const PushNotifications::Settings& settingsToUse)
    {
        if (@available (macOS 10.7, *))
        {
            settings = settingsToUse;

            NSRemoteNotificationType types = NSUInteger ((bool) settings.allowBadge);

            if (@available (macOS 10.8, *))
            {
                types |= (NSUInteger) ((settings.allowSound ? NSRemoteNotificationTypeSound : 0)
                                     | (settings.allowAlert ? NSRemoteNotificationTypeAlert : 0));
            }

            [[NSApplication sharedApplication] registerForRemoteNotificationTypes: types];
        }
    }

    void requestSettingsUsed()
    {
        if (@available (macOS 10.7, *))
        {
            settings.allowBadge = [NSApplication sharedApplication].enabledRemoteNotificationTypes & NSRemoteNotificationTypeBadge;

            if (@available (macOS 10.8, *))
            {
                settings.allowSound = [NSApplication sharedApplication].enabledRemoteNotificationTypes & NSRemoteNotificationTypeSound;
                settings.allowAlert = [NSApplication sharedApplication].enabledRemoteNotificationTypes & NSRemoteNotificationTypeAlert;
            }

            owner.listeners.call ([&] (Listener& l) { l.notificationSettingsReceived (settings); });
        }
        else
        {
            // no settings available
            owner.listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
        }
    }

    bool areNotificationsEnabled() const { return true; }

    void sendLocalNotification (const Notification& n)
    {
        auto* notification = PushNotificationsDelegateDetailsOsx::juceNotificationToNSUserNotification (n);

        [[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification: notification];
    }

    void getDeliveredNotifications() const
    {
        Array<PushNotifications::Notification> notifs;

        for (NSUserNotification* n in [NSUserNotificationCenter defaultUserNotificationCenter].deliveredNotifications)
            notifs.add (PushNotificationsDelegateDetailsOsx::nsUserNotificationToJuceNotification (n));

        owner.listeners.call ([&] (Listener& l) { l.deliveredNotificationsListReceived (notifs); });
    }

    void removeAllDeliveredNotifications()
    {
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
    }

    void removeDeliveredNotification (const String& identifier)
    {
        PushNotifications::Notification n;
        n.identifier = identifier;

        auto nsNotification = PushNotificationsDelegateDetailsOsx::juceNotificationToNSUserNotification (n);

        [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification: nsNotification];
    }

    void setupChannels ([[maybe_unused]] const Array<ChannelGroup>& groups, [[maybe_unused]] const Array<Channel>& channels)
    {
    }

    void getPendingLocalNotifications() const
    {
        Array<PushNotifications::Notification> notifs;

        for (NSUserNotification* n in [NSUserNotificationCenter defaultUserNotificationCenter].scheduledNotifications)
            notifs.add (PushNotificationsDelegateDetailsOsx::nsUserNotificationToJuceNotification (n));

        owner.listeners.call ([&] (Listener& l) { l.pendingLocalNotificationsListReceived (notifs); });
    }

    void removePendingLocalNotification (const String& identifier)
    {
        PushNotifications::Notification n;
        n.identifier = identifier;

        auto nsNotification = PushNotificationsDelegateDetailsOsx::juceNotificationToNSUserNotification (n);

        [[NSUserNotificationCenter defaultUserNotificationCenter] removeScheduledNotification: nsNotification];
    }

    void removeAllPendingLocalNotifications()
    {
        for (NSUserNotification* n in [NSUserNotificationCenter defaultUserNotificationCenter].scheduledNotifications)
            [[NSUserNotificationCenter defaultUserNotificationCenter] removeScheduledNotification: n];
    }

    String getDeviceToken()
    {
        // You need to call requestPermissionsWithSettings() first.
        jassert (initialised);

        return deviceToken;
    }

    //==============================================================================
    //PushNotificationsDelegate
    void registeredForRemoteNotifications (NSData* deviceTokenToUse) override
    {
        deviceToken = [deviceTokenToUse]() -> String
        {
            auto length = deviceTokenToUse.length;

            if (auto* buffer = (const unsigned char*) deviceTokenToUse.bytes)
            {
                NSMutableString* hexString = [NSMutableString stringWithCapacity: (length * 2)];

                for (NSUInteger i = 0; i < length; ++i)
                    [hexString appendFormat:@"%02x", buffer[i]];

                return nsStringToJuce ([hexString copy]);
            }

            return {};
        }();

        initialised = true;

        owner.listeners.call ([&] (Listener& l) { l.deviceTokenRefreshed (deviceToken); });
    }

    void failedToRegisterForRemoteNotifications ([[maybe_unused]] NSError* error) override
    {
        deviceToken.clear();
    }

    void didReceiveRemoteNotification (NSDictionary* userInfo) override
    {
        auto n = PushNotificationsDelegateDetailsOsx::nsDictionaryToJuceNotification (userInfo);
        owner.listeners.call ([&] (Listener& l) { l.handleNotification (true, n); });
    }

    void didDeliverNotification ([[maybe_unused]] NSUserNotification* notification) override
    {
    }

    void didActivateNotification (NSUserNotification* notification) override
    {
        auto n = PushNotificationsDelegateDetailsOsx::nsUserNotificationToJuceNotification (notification);

        if (notification.activationType == NSUserNotificationActivationTypeContentsClicked)
        {
            owner.listeners.call ([&] (Listener& l) { l.handleNotification (notification.remote, n); });
        }
        else
        {
            const auto actionIdentifier = nsStringToJuce ([&]
            {
                if (@available (macOS 10.10, *))
                    if (notification.additionalActivationAction != nil)
                        return notification.additionalActivationAction.identifier;

                return notification.actionButtonTitle;
            }());

            auto reply = notification.activationType == NSUserNotificationActivationTypeReplied
                            ? nsStringToJuce ([notification.response string])
                            : String();

            owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (notification.remote, n, actionIdentifier, reply); });
        }
    }

    bool shouldPresentNotification (NSUserNotification*) override { return true; }

    void subscribeToTopic ([[maybe_unused]] const String& topic)     {}
    void unsubscribeFromTopic ([[maybe_unused]] const String& topic) {}

    void sendUpstreamMessage ([[maybe_unused]] const String& serverSenderId,
                              [[maybe_unused]] const String& collapseKey,
                              [[maybe_unused]] const String& messageId,
                              [[maybe_unused]] const String& messageType,
                              [[maybe_unused]] int timeToLive,
                              [[maybe_unused]] const StringPairArray& additionalData)
    {
    }

private:
    PushNotifications& owner;

    bool initialised = false;
    String deviceToken;

    PushNotifications::Settings settings;
};

} // namespace juce

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
