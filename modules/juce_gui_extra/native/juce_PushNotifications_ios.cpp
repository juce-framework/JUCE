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

namespace juce
{

struct PushNotificationsDelegateDetails
{
    //==============================================================================
    using Action   = PushNotifications::Settings::Action;
    using Category = PushNotifications::Settings::Category;

    static void* actionToNSAction (const Action& a)
    {
        if (@available (iOS 10, *))
        {
            if (a.style == Action::text)
            {
                return [UNTextInputNotificationAction actionWithIdentifier: juceStringToNS (a.identifier)
                                                                     title: juceStringToNS (a.title)
                                                                   options: NSUInteger (a.destructive << 1 | (! a.triggerInBackground) << 2)
                                                      textInputButtonTitle: juceStringToNS (a.textInputButtonText)
                                                      textInputPlaceholder: juceStringToNS (a.textInputPlaceholder)];
            }

            return [UNNotificationAction actionWithIdentifier: juceStringToNS (a.identifier)
                                                        title: juceStringToNS (a.title)
                                                      options: NSUInteger (a.destructive << 1 | (! a.triggerInBackground) << 2)];
        }

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
        auto action = [[UIMutableUserNotificationAction alloc] init];

        action.identifier     = juceStringToNS (a.identifier);
        action.title          = juceStringToNS (a.title);
        action.behavior       = a.style == Action::text ? UIUserNotificationActionBehaviorTextInput
                                                        : UIUserNotificationActionBehaviorDefault;
        action.parameters     = varToNSDictionary (a.parameters);
        action.activationMode = a.triggerInBackground ? UIUserNotificationActivationModeBackground
                                                      : UIUserNotificationActivationModeForeground;
        action.destructive    = (bool) a.destructive;

        [action autorelease];

        return action;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    static void* categoryToNSCategory (const Category& c)
    {
        if (@available (iOS 10, *))
        {
            auto actions = [NSMutableArray arrayWithCapacity: (NSUInteger) c.actions.size()];

            for (const auto& a : c.actions)
            {
                auto* action = (UNNotificationAction*) actionToNSAction (a);
                [actions addObject: action];
            }

            return [UNNotificationCategory categoryWithIdentifier: juceStringToNS (c.identifier)
                                                          actions: actions
                                                intentIdentifiers: @[]
                                                          options: c.sendDismissAction ? UNNotificationCategoryOptionCustomDismissAction : 0];
        }

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
        auto category = [[UIMutableUserNotificationCategory alloc] init];
        category.identifier = juceStringToNS (c.identifier);

        auto actions = [NSMutableArray arrayWithCapacity: (NSUInteger) c.actions.size()];

        for (const auto& a : c.actions)
        {
            auto* action = (UIUserNotificationAction*) actionToNSAction (a);
            [actions addObject: action];
        }

        [category setActions: actions forContext: UIUserNotificationActionContextDefault];
        [category setActions: actions forContext: UIUserNotificationActionContextMinimal];

        [category autorelease];

        return category;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    //==============================================================================
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    static UILocalNotification* juceNotificationToUILocalNotification (const PushNotifications::Notification& n)
    {
        auto notification = [[UILocalNotification alloc] init];

        notification.alertTitle = juceStringToNS (n.title);
        notification.alertBody  = juceStringToNS (n.body);
        notification.category   = juceStringToNS (n.category);
        notification.applicationIconBadgeNumber = n.badgeNumber;

        auto triggerTime = Time::getCurrentTime() + RelativeTime (n.triggerIntervalSec);
        notification.fireDate   = [NSDate dateWithTimeIntervalSince1970: (double) triggerTime.toMilliseconds() / 1000.0];
        notification.userInfo   = varToNSDictionary (n.properties);

        auto soundToPlayString = n.soundToPlay.toString (true);

        if (soundToPlayString == "default_os_sound")
            notification.soundName = UILocalNotificationDefaultSoundName;
        else if (soundToPlayString.isNotEmpty())
            notification.soundName = juceStringToNS (soundToPlayString);

        return notification;
    }
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    static UNNotificationRequest* juceNotificationToUNNotificationRequest (const PushNotifications::Notification& n)
    {
        // content
        auto content = [[UNMutableNotificationContent alloc] init];

        content.title              = juceStringToNS (n.title);
        content.subtitle           = juceStringToNS (n.subtitle);
        content.threadIdentifier   = juceStringToNS (n.groupId);
        content.body               = juceStringToNS (n.body);
        content.categoryIdentifier = juceStringToNS (n.category);
        content.badge              = [NSNumber numberWithInt: n.badgeNumber];

        auto soundToPlayString = n.soundToPlay.toString (true);

        if (soundToPlayString == "default_os_sound")
            content.sound = [UNNotificationSound defaultSound];
        else if (soundToPlayString.isNotEmpty())
            content.sound = [UNNotificationSound soundNamed: juceStringToNS (soundToPlayString)];

        auto* propsDict = (NSMutableDictionary*) varToNSDictionary (n.properties);
        [propsDict setObject: juceStringToNS (soundToPlayString) forKey: nsStringLiteral ("com.juce.soundName")];
        content.userInfo = propsDict;

        // trigger
        UNTimeIntervalNotificationTrigger* trigger = nil;

        if (std::abs (n.triggerIntervalSec) >= 0.001)
        {
            BOOL shouldRepeat = n.repeat && n.triggerIntervalSec >= 60;
            trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval: n.triggerIntervalSec repeats: shouldRepeat];
        }

        // request
        // each notification on iOS 10 needs to have an identifier, otherwise it will not show up
        jassert (n.identifier.isNotEmpty());
        UNNotificationRequest* request = [UNNotificationRequest requestWithIdentifier: juceStringToNS (n.identifier)
                                                                              content: content
                                                                              trigger: trigger];

        [content autorelease];

        return request;
    }

    static String getUserResponseFromNSDictionary (NSDictionary* dictionary)
    {
        if (dictionary == nil || dictionary.count == 0)
            return {};

        jassert (dictionary.count == 1);

        for (NSString* key in dictionary)
        {
            const auto keyString = nsStringToJuce (key);

            id value = dictionary[key];

            if ([value isKindOfClass: [NSString class]])
                return nsStringToJuce ((NSString*) value);
        }

        jassertfalse;
        return {};
    }

    //==============================================================================
    static var getNotificationPropertiesFromDictionaryVar (const var& dictionaryVar)
    {
        DynamicObject* dictionaryVarObject = dictionaryVar.getDynamicObject();

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

    //==============================================================================
    static double getIntervalSecFromUNNotificationTrigger (UNNotificationTrigger* t)
    {
        if (t != nil)
        {
            if ([t isKindOfClass: [UNTimeIntervalNotificationTrigger class]])
            {
                auto* trigger = (UNTimeIntervalNotificationTrigger*) t;
                return trigger.timeInterval;
            }
            else if ([t isKindOfClass: [UNCalendarNotificationTrigger class]])
            {
                auto* trigger = (UNCalendarNotificationTrigger*) t;
                NSDate* date    = [trigger.dateComponents date];
                NSDate* dateNow = [NSDate date];
                return [dateNow timeIntervalSinceDate: date];
            }
        }

        return 0.;
    }

    static PushNotifications::Notification unNotificationRequestToJuceNotification (UNNotificationRequest* r)
    {
        PushNotifications::Notification n;

        n.identifier = nsStringToJuce (r.identifier);
        n.title      = nsStringToJuce (r.content.title);
        n.subtitle   = nsStringToJuce (r.content.subtitle);
        n.body       = nsStringToJuce (r.content.body);
        n.groupId    = nsStringToJuce (r.content.threadIdentifier);
        n.category   = nsStringToJuce (r.content.categoryIdentifier);
        n.badgeNumber = r.content.badge.intValue;

        auto userInfoVar = nsDictionaryToVar (r.content.userInfo);

        if (auto* object = userInfoVar.getDynamicObject())
        {
            static const Identifier soundName ("com.juce.soundName");
            n.soundToPlay = URL (object->getProperty (soundName).toString());
            object->removeProperty (soundName);
        }

        n.properties = userInfoVar;

        n.triggerIntervalSec = getIntervalSecFromUNNotificationTrigger (r.trigger);
        n.repeat = r.trigger != nil && r.trigger.repeats;

        return n;
    }

    static PushNotifications::Notification unNotificationToJuceNotification (UNNotification* n)
    {
        return unNotificationRequestToJuceNotification (n.request);
    }

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    static PushNotifications::Notification uiLocalNotificationToJuceNotification (UILocalNotification* n)
    {
        PushNotifications::Notification notif;

        notif.title       = nsStringToJuce (n.alertTitle);
        notif.body        = nsStringToJuce (n.alertBody);

        if (n.fireDate != nil)
        {
            NSDate* dateNow = [NSDate date];
            NSDate* fireDate = n.fireDate;

            notif.triggerIntervalSec = [dateNow timeIntervalSinceDate: fireDate];
        }

        notif.soundToPlay = URL (nsStringToJuce (n.soundName));
        notif.badgeNumber = (int) n.applicationIconBadgeNumber;
        notif.category    = nsStringToJuce (n.category);
        notif.properties  = nsDictionaryToVar (n.userInfo);

        return notif;
    }

    static Action uiUserNotificationActionToAction (UIUserNotificationAction* a)
    {
        Action action;

        action.identifier = nsStringToJuce (a.identifier);
        action.title = nsStringToJuce (a.title);
        action.style = a.behavior == UIUserNotificationActionBehaviorTextInput
                     ? Action::text
                     : Action::button;

        action.triggerInBackground = a.activationMode == UIUserNotificationActivationModeBackground;
        action.destructive = a.destructive;
        action.parameters = nsDictionaryToVar (a.parameters);

        return action;
    }

    static Category uiUserNotificationCategoryToCategory (UIUserNotificationCategory* c)
    {
        Category category;
        category.identifier = nsStringToJuce (c.identifier);

        for (UIUserNotificationAction* a in [c actionsForContext: UIUserNotificationActionContextDefault])
            category.actions.add (uiUserNotificationActionToAction (a));

        return category;
    }
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    static Action unNotificationActionToAction (UNNotificationAction* a)
    {
        Action action;

        action.identifier = nsStringToJuce (a.identifier);
        action.title      = nsStringToJuce (a.title);
        action.triggerInBackground = ! (a.options & UNNotificationActionOptionForeground);
        action.destructive         =    a.options & UNNotificationActionOptionDestructive;

        if ([a isKindOfClass: [UNTextInputNotificationAction class]])
        {
            auto* textAction = (UNTextInputNotificationAction*)a;

            action.style = Action::text;
            action.textInputButtonText  = nsStringToJuce (textAction.textInputButtonTitle);
            action.textInputPlaceholder = nsStringToJuce (textAction.textInputPlaceholder);
        }
        else
        {
            action.style = Action::button;
        }

        return action;
    }

    static Category unNotificationCategoryToCategory (UNNotificationCategory* c)
    {
        Category category;

        category.identifier = nsStringToJuce (c.identifier);
        category.sendDismissAction = c.options & UNNotificationCategoryOptionCustomDismissAction;

        for (UNNotificationAction* a in c.actions)
            category.actions.add (unNotificationActionToAction (a));

        return category;
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

private:
    ~PushNotificationsDelegateDetails() = delete;
};

//==============================================================================
bool PushNotifications::Notification::isValid() const noexcept
{
    if (@available (iOS 10, *))
        return title.isNotEmpty() && body.isNotEmpty() && identifier.isNotEmpty() && category.isNotEmpty();

    return title.isNotEmpty() && body.isNotEmpty() && category.isNotEmpty();
}

//==============================================================================
struct PushNotifications::Pimpl
{
    Pimpl (PushNotifications& p)
        : owner (p)
    {
        Class::setThis (delegate.get(), this);

        auto appDelegate = [[UIApplication sharedApplication] delegate];

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        if ([appDelegate respondsToSelector: @selector (setPushNotificationsDelegateToUse:)])
            [appDelegate performSelector: @selector (setPushNotificationsDelegateToUse:) withObject: delegate.get()];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    void requestPermissionsWithSettings (const PushNotifications::Settings& settingsToUse)
    {
        settings = settingsToUse;

        auto categories = [NSMutableSet setWithCapacity: (NSUInteger) settings.categories.size()];

        if (@available (iOS 10, *))
        {
            for (const auto& c : settings.categories)
            {
                auto* category = (UNNotificationCategory*) PushNotificationsDelegateDetails::categoryToNSCategory (c);
                [categories addObject: category];
            }

            UNAuthorizationOptions authOptions = NSUInteger ((bool)settings.allowBadge << 0
                                                           | (bool)settings.allowSound << 1
                                                           | (bool)settings.allowAlert << 2);

            [[UNUserNotificationCenter currentNotificationCenter] setNotificationCategories: categories];
            [[UNUserNotificationCenter currentNotificationCenter] requestAuthorizationWithOptions: authOptions
                                                                                completionHandler: ^(BOOL /*granted*/, NSError* /*error*/)
                                                                                                   {
                                                                                                       requestSettingsUsed();
                                                                                                   }];
        }
        else
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

            for (const auto& c : settings.categories)
            {
                auto* category = (UIUserNotificationCategory*) PushNotificationsDelegateDetails::categoryToNSCategory (c);
                [categories addObject: category];
            }

            UIUserNotificationType type = NSUInteger ((bool)settings.allowBadge << 0
                                                    | (bool)settings.allowSound << 1
                                                    | (bool)settings.allowAlert << 2);

            UIUserNotificationSettings* s = [UIUserNotificationSettings settingsForTypes: type categories: categories];
            [[UIApplication sharedApplication] registerUserNotificationSettings: s];

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }

        [[UIApplication sharedApplication] registerForRemoteNotifications];
    }

    void requestSettingsUsed()
    {
        if (@available (iOS 10, *))
        {
            [[UNUserNotificationCenter currentNotificationCenter] getNotificationSettingsWithCompletionHandler:
             ^(UNNotificationSettings* s)
             {
                 [[UNUserNotificationCenter currentNotificationCenter] getNotificationCategoriesWithCompletionHandler:
                  ^(NSSet<UNNotificationCategory*>* categories)
                  {
                      settings.allowBadge = s.badgeSetting == UNNotificationSettingEnabled;
                      settings.allowSound = s.soundSetting == UNNotificationSettingEnabled;
                      settings.allowAlert = s.alertSetting == UNNotificationSettingEnabled;

                      for (UNNotificationCategory* c in categories)
                          settings.categories.add (PushNotificationsDelegateDetails::unNotificationCategoryToCategory (c));

                      owner.listeners.call ([&] (Listener& l) { l.notificationSettingsReceived (settings); });
                  }
                 ];

             }];
        }
        else
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

            UIUserNotificationSettings* s = [UIApplication sharedApplication].currentUserNotificationSettings;

            settings.allowBadge = s.types & UIUserNotificationTypeBadge;
            settings.allowSound = s.types & UIUserNotificationTypeSound;
            settings.allowAlert = s.types & UIUserNotificationTypeAlert;

            for (UIUserNotificationCategory *c in s.categories)
                settings.categories.add (PushNotificationsDelegateDetails::uiUserNotificationCategoryToCategory (c));

            owner.listeners.call ([&] (Listener& l) { l.notificationSettingsReceived (settings); });

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
    }

    bool areNotificationsEnabled() const { return true; }

    void sendLocalNotification (const Notification& n)
    {
        if (@available (iOS 10, *))
        {
            UNNotificationRequest* request = PushNotificationsDelegateDetails::juceNotificationToUNNotificationRequest (n);

            [[UNUserNotificationCenter currentNotificationCenter] addNotificationRequest: request
                                                                   withCompletionHandler: ^(NSError* error)
                                                                                          {
                                                                                              jassert (error == nil);

                                                                                              if (error != nil)
                                                                                                  NSLog (nsStringLiteral ("addNotificationRequest error: %@"), error);
                                                                                          }];
        }
        else
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

            auto* notification = PushNotificationsDelegateDetails::juceNotificationToUILocalNotification (n);

            [[UIApplication sharedApplication] scheduleLocalNotification: notification];
            [notification release];

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
    }

    void getDeliveredNotifications() const
    {
        if (@available (iOS 10, *))
        {
            [[UNUserNotificationCenter currentNotificationCenter] getDeliveredNotificationsWithCompletionHandler:
             ^(NSArray<UNNotification*>* notifications)
             {
                Array<PushNotifications::Notification> notifs;

                for (UNNotification* n in notifications)
                    notifs.add (PushNotificationsDelegateDetails::unNotificationToJuceNotification (n));

                owner.listeners.call ([&] (Listener& l) { l.deliveredNotificationsListReceived (notifs); });
             }];
        }
        else
        {
            // Not supported on this platform
            jassertfalse;
            owner.listeners.call ([] (Listener& l) { l.deliveredNotificationsListReceived ({}); });
        }
    }

    void removeAllDeliveredNotifications()
    {
        if (@available (iOS 10, *))
        {
            [[UNUserNotificationCenter currentNotificationCenter] removeAllDeliveredNotifications];
        }
        else
        {
            // Not supported on this platform
            jassertfalse;
        }
    }

    void removeDeliveredNotification ([[maybe_unused]] const String& identifier)
    {
        if (@available (iOS 10, *))
        {

            NSArray<NSString*>* identifiers = [NSArray arrayWithObject: juceStringToNS (identifier)];

            [[UNUserNotificationCenter currentNotificationCenter] removeDeliveredNotificationsWithIdentifiers: identifiers];
        }
        else
        {
            // Not supported on this platform
            jassertfalse;
        }
    }

    void setupChannels ([[maybe_unused]] const Array<ChannelGroup>& groups, [[maybe_unused]] const Array<Channel>& channels)
    {
    }

    void getPendingLocalNotifications() const
    {
        if (@available (iOS 10, *))
        {
            [[UNUserNotificationCenter currentNotificationCenter] getPendingNotificationRequestsWithCompletionHandler:
             ^(NSArray<UNNotificationRequest*>* requests)
             {
                 Array<PushNotifications::Notification> notifs;

                 for (UNNotificationRequest* r : requests)
                     notifs.add (PushNotificationsDelegateDetails::unNotificationRequestToJuceNotification (r));

                 owner.listeners.call ([&] (Listener& l) { l.pendingLocalNotificationsListReceived (notifs); });
             }
            ];
        }
        else
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

            Array<PushNotifications::Notification> notifs;

            for (UILocalNotification* n in [UIApplication sharedApplication].scheduledLocalNotifications)
                notifs.add (PushNotificationsDelegateDetails::uiLocalNotificationToJuceNotification (n));

            owner.listeners.call ([&] (Listener& l) { l.pendingLocalNotificationsListReceived (notifs); });

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
    }

    void removePendingLocalNotification (const String& identifier)
    {
        if (@available (iOS 10, *))
        {
            NSArray<NSString*>* identifiers = [NSArray arrayWithObject: juceStringToNS (identifier)];

            [[UNUserNotificationCenter currentNotificationCenter] removePendingNotificationRequestsWithIdentifiers: identifiers];
        }
        else
        {
            // Not supported on this platform
            jassertfalse;
        }
    }

    void removeAllPendingLocalNotifications()
    {
        if (@available (iOS 10, *))
        {
            [[UNUserNotificationCenter currentNotificationCenter] removeAllPendingNotificationRequests];
        }
        else
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

            [[UIApplication sharedApplication] cancelAllLocalNotifications];

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
    }

    String getDeviceToken()
    {
        // You need to call requestPermissionsWithSettings() first.
        jassert (initialised);

        return deviceToken;
    }

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
    //==============================================================================
    void registeredForRemoteNotifications (NSData* deviceTokenToUse)
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

    void failedToRegisterForRemoteNotifications ([[maybe_unused]] NSError* error)
    {
        deviceToken.clear();
    }

    void didReceiveRemoteNotification (NSDictionary* userInfo)
    {
        auto n = PushNotificationsDelegateDetails::nsDictionaryToJuceNotification (userInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotification (false, n); });
    }

    void didReceiveRemoteNotificationFetchCompletionHandler (NSDictionary* userInfo,
                                                             void (^completionHandler)(UIBackgroundFetchResult result))
    {
        didReceiveRemoteNotification (userInfo);
        completionHandler (UIBackgroundFetchResultNewData);
    }

    void handleActionForRemoteNotificationCompletionHandler (NSString* actionIdentifier,
                                                             NSDictionary* userInfo,
                                                             NSDictionary* responseInfo,
                                                             void (^completionHandler)())
    {
        auto n = PushNotificationsDelegateDetails::nsDictionaryToJuceNotification (userInfo);
        auto actionString = nsStringToJuce (actionIdentifier);
        auto response = PushNotificationsDelegateDetails::getUserResponseFromNSDictionary (responseInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (false, n, actionString, response); });

        completionHandler();
    }

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

    void didRegisterUserNotificationSettings (UIUserNotificationSettings*)
    {
        requestSettingsUsed();
    }

    void didReceiveLocalNotification (UILocalNotification* notification)
    {
        auto n = PushNotificationsDelegateDetails::uiLocalNotificationToJuceNotification (notification);

        owner.listeners.call ([&] (Listener& l) { l.handleNotification (true, n); });
    }

    void handleActionForLocalNotificationCompletionHandler (NSString* actionIdentifier,
                                                            UILocalNotification* notification,
                                                            void (^completionHandler)())
    {
        handleActionForLocalNotificationWithResponseCompletionHandler (actionIdentifier,
                                                                       notification,
                                                                       nil,
                                                                       completionHandler);
    }

    void handleActionForLocalNotificationWithResponseCompletionHandler (NSString* actionIdentifier,
                                                                        UILocalNotification* notification,
                                                                        NSDictionary* responseInfo,
                                                                        void (^completionHandler)())
    {
        auto n = PushNotificationsDelegateDetails::uiLocalNotificationToJuceNotification (notification);
        auto actionString = nsStringToJuce (actionIdentifier);
        auto response = PushNotificationsDelegateDetails::getUserResponseFromNSDictionary (responseInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (true, n, actionString, response); });

        completionHandler();
    }

    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    void willPresentNotificationWithCompletionHandler ([[maybe_unused]] UNNotification* notification,
                                                       void (^completionHandler)(UNNotificationPresentationOptions options))
    {
        NSUInteger options = NSUInteger ((int)settings.allowBadge << 0
                                       | (int)settings.allowSound << 1
                                       | (int)settings.allowAlert << 2);

        completionHandler (options);
    }

    void didReceiveNotificationResponseWithCompletionHandler (UNNotificationResponse* response,
                                                              void (^completionHandler)())
    {
        const bool remote = [response.notification.request.trigger isKindOfClass: [UNPushNotificationTrigger class]];

        auto actionString = nsStringToJuce (response.actionIdentifier);

        if (actionString == "com.apple.UNNotificationDefaultActionIdentifier")
            actionString.clear();
        else if (actionString == "com.apple.UNNotificationDismissActionIdentifier")
            actionString = "com.juce.NotificationDeleted";

        auto n = PushNotificationsDelegateDetails::unNotificationToJuceNotification (response.notification);

        String responseString;

        if ([response isKindOfClass: [UNTextInputNotificationResponse class]])
        {
            UNTextInputNotificationResponse* textResponse = (UNTextInputNotificationResponse*)response;
            responseString = nsStringToJuce (textResponse.userText);
        }

        owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (! remote, n, actionString, responseString); });
        completionHandler();
    }

    //==============================================================================
    struct Class final : public ObjCClass<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>>
    {
        Class()
            : ObjCClass ("JucePushNotificationsDelegate_")
        {
            addIvar<Pimpl*> ("self");

            addMethod (@selector (application:didRegisterForRemoteNotificationsWithDeviceToken:), [] (id self, SEL, UIApplication*, NSData* data)
            {
                getThis (self).registeredForRemoteNotifications (data);
            });

            addMethod (@selector (application:didFailToRegisterForRemoteNotificationsWithError:), [] (id self, SEL, UIApplication*, NSError* error)
            {
                getThis (self).failedToRegisterForRemoteNotifications (error);
            });

            addMethod (@selector (application:didReceiveRemoteNotification:), [] (id self, SEL, UIApplication*, NSDictionary* userInfo)
            {
                getThis (self).didReceiveRemoteNotification (userInfo);
            });

            addMethod (@selector (application:didReceiveRemoteNotification:fetchCompletionHandler:), [] (id self, SEL, UIApplication*, NSDictionary* userInfo, void (^completionHandler)(UIBackgroundFetchResult result))
            {
                getThis (self).didReceiveRemoteNotificationFetchCompletionHandler (userInfo, completionHandler);
            });

            addMethod (@selector (application:handleActionWithIdentifier:forRemoteNotification:withResponseInfo:completionHandler:), [] (id self, SEL, UIApplication*, NSString* actionIdentifier, NSDictionary* userInfo, NSDictionary* responseInfo, void (^completionHandler)())
            {
                getThis (self).handleActionForRemoteNotificationCompletionHandler (actionIdentifier, userInfo, responseInfo, completionHandler);
            });

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

            addMethod (@selector (application:didRegisterUserNotificationSettings:), [] (id self, SEL, UIApplication*, UIUserNotificationSettings* settingsToUse)
            {
                getThis (self).didRegisterUserNotificationSettings (settingsToUse);
            });

            addMethod (@selector (application:didReceiveLocalNotification:), [] (id self, SEL, UIApplication*, UILocalNotification* notification)
            {
                getThis (self).didReceiveLocalNotification (notification);
            });

            addMethod (@selector (application:handleActionWithIdentifier:forLocalNotification:completionHandler:), [] (id self, SEL, UIApplication*, NSString* actionIdentifier, UILocalNotification* notification, void (^completionHandler)())
            {
                getThis (self).handleActionForLocalNotificationCompletionHandler (actionIdentifier, notification, completionHandler);
            });

            addMethod (@selector (application:handleActionWithIdentifier:forLocalNotification:withResponseInfo:completionHandler:), [] (id self, SEL, UIApplication*, NSString* actionIdentifier, UILocalNotification* notification, NSDictionary* responseInfo, void (^completionHandler)())
            {
                getThis (self). handleActionForLocalNotificationWithResponseCompletionHandler (actionIdentifier, notification, responseInfo, completionHandler);
            });

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            addMethod (@selector (userNotificationCenter:willPresentNotification:withCompletionHandler:), [] (id self, SEL, UNUserNotificationCenter*, UNNotification* notification, void (^completionHandler)(UNNotificationPresentationOptions options))
            {
                getThis (self).willPresentNotificationWithCompletionHandler (notification, completionHandler);
            });

            addMethod (@selector (userNotificationCenter:didReceiveNotificationResponse:withCompletionHandler:), [] (id self, SEL, UNUserNotificationCenter*, UNNotificationResponse* response, void (^completionHandler)())
            {
                getThis (self).didReceiveNotificationResponseWithCompletionHandler (response, completionHandler);
            });

            registerClass();
        }

        //==============================================================================
        static Pimpl& getThis (id self)         { return *getIvar<Pimpl*> (self, "self"); }
        static void setThis (id self, Pimpl* d) { object_setInstanceVariable (self, "self", d); }
    };

    //==============================================================================
    static Class& getClass()
    {
        static Class c;
        return c;
    }

    NSUniquePtr<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>> delegate { [getClass().createInstance() init] };

    PushNotifications& owner;

    bool initialised = false;
    String deviceToken;

    PushNotifications::Settings settings;
};

} // namespace juce
