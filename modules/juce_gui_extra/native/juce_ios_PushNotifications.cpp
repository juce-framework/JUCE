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

#if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
template <> struct ContainerDeletePolicy<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>> { static void destroy (NSObject* o) { [o release]; } };
#endif

namespace PushNotificationsDelegateDetails
{
    //==============================================================================
    using Action   = PushNotifications::Settings::Action;
    using Category = PushNotifications::Settings::Category;

    void* actionToNSAction (const Action& a, bool iOSEarlierThan10)
    {
        if (iOSEarlierThan10)
        {
            auto* action = [[UIMutableUserNotificationAction alloc] init];

            action.identifier     = juceStringToNS (a.identifier);
            action.title          = juceStringToNS (a.title);
            action.behavior       = a.style == Action::text ? UIUserNotificationActionBehaviorTextInput
                                                            : UIUserNotificationActionBehaviorDefault;
            action.parameters     = varObjectToNSDictionary (a.parameters);
            action.activationMode = a.triggerInBackground ? UIUserNotificationActivationModeBackground
                                                          : UIUserNotificationActivationModeForeground;
            action.destructive    = (bool) a.destructive;

            [action autorelease];

            return action;
        }
        else
        {
          #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
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
          #else
            return nullptr;
          #endif
        }
    }

    void* categoryToNSCategory (const Category& c, bool iOSEarlierThan10)
    {
        if (iOSEarlierThan10)
        {
            auto* category = [[UIMutableUserNotificationCategory alloc] init];
            category.identifier = juceStringToNS (c.identifier);

            auto* actions = [NSMutableArray arrayWithCapacity: (NSUInteger) c.actions.size()];

            for (const auto& a : c.actions)
            {
                auto* action = (UIUserNotificationAction*) actionToNSAction (a, iOSEarlierThan10);
                [actions addObject: action];
            }

            [category setActions: actions forContext: UIUserNotificationActionContextDefault];
            [category setActions: actions forContext: UIUserNotificationActionContextMinimal];

            [category autorelease];

            return category;
        }
        else
        {
          #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
            auto* actions = [NSMutableArray arrayWithCapacity: (NSUInteger) c.actions.size()];

            for (const auto& a : c.actions)
            {
                auto* action = (UNNotificationAction*) actionToNSAction (a, iOSEarlierThan10);
                [actions addObject: action];
            }

            return [UNNotificationCategory categoryWithIdentifier: juceStringToNS (c.identifier)
                                                          actions: actions
                                                intentIdentifiers: @[]
                                                          options: c.sendDismissAction ? UNNotificationCategoryOptionCustomDismissAction : 0];
          #else
            return nullptr;
          #endif
        }
    }

    //==============================================================================
    UILocalNotification* juceNotificationToUILocalNotification (const PushNotifications::Notification& n)
    {
        auto* notification = [[UILocalNotification alloc] init];

        notification.alertTitle = juceStringToNS (n.title);
        notification.alertBody  = juceStringToNS (n.body);
        notification.category   = juceStringToNS (n.category);
        notification.applicationIconBadgeNumber = n.badgeNumber;

        auto triggerTime = Time::getCurrentTime() + RelativeTime (n.triggerIntervalSec);
        notification.fireDate   = [NSDate dateWithTimeIntervalSince1970: triggerTime.toMilliseconds() / 1000.];
        notification.userInfo   = varObjectToNSDictionary (n.properties);

        auto soundToPlayString = n.soundToPlay.toString (true);

        if (soundToPlayString == "default_os_sound")
            notification.soundName = UILocalNotificationDefaultSoundName;
        else if (soundToPlayString.isNotEmpty())
            notification.soundName = juceStringToNS (soundToPlayString);

        return notification;
    }

  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
    UNNotificationRequest* juceNotificationToUNNotificationRequest (const PushNotifications::Notification& n)
    {
        // content
        auto* content = [[UNMutableNotificationContent alloc] init];

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

        auto* propsDict = (NSMutableDictionary*) varObjectToNSDictionary (n.properties);
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
        // each notification on iOS 10 needs to have an identifer, otherwise it will not show up
        jassert (n.identifier.isNotEmpty());
        UNNotificationRequest* request = [UNNotificationRequest requestWithIdentifier: juceStringToNS (n.identifier)
                                                                              content: content
                                                                              trigger: trigger];

        [content autorelease];

        return request;
    }
  #endif

    String getUserResponseFromNSDictionary (NSDictionary* dictionary)
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
    var getNotificationPropertiesFromDictionaryVar (const var& dictionaryVar)
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

        return var (propsVarObject);
    }

    //==============================================================================
  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
    double getIntervalSecFromUNNotificationTrigger (UNNotificationTrigger* t)
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

    PushNotifications::Notification unNotificationRequestToJuceNotification (UNNotificationRequest* r)
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

    PushNotifications::Notification unNotificationToJuceNotification (UNNotification* n)
    {
        return unNotificationRequestToJuceNotification (n.request);
    }
  #endif

    PushNotifications::Notification uiLocalNotificationToJuceNotification (UILocalNotification* n)
    {
        PushNotifications::Notification notif;

        notif.title       = nsStringToJuce (n.alertTitle);
        notif.body        = nsStringToJuce (n.alertBody);

        if (n.fireDate != nil)
        {
            NSDate* dateNow = [NSDate date];
            notif.triggerIntervalSec = [dateNow timeIntervalSinceDate: n.fireDate];
        }

        notif.soundToPlay = URL (nsStringToJuce (n.soundName));
        notif.badgeNumber = (int) n.applicationIconBadgeNumber;
        notif.category    = nsStringToJuce (n.category);
        notif.properties  = nsDictionaryToVar (n.userInfo);

        return notif;
    }

    Action uiUserNotificationActionToAction (UIUserNotificationAction* a)
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

    Category uiUserNotificationCategoryToCategory (UIUserNotificationCategory* c)
    {
        Category category;
        category.identifier = nsStringToJuce (c.identifier);

        for (UIUserNotificationAction* a in [c actionsForContext: UIUserNotificationActionContextDefault])
            category.actions.add (uiUserNotificationActionToAction (a));

        return category;
    }

  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
    Action unNotificationActionToAction (UNNotificationAction* a)
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

    Category unNotificationCategoryToCategory (UNNotificationCategory* c)
    {
        Category category;

        category.identifier = nsStringToJuce (c.identifier);
        category.sendDismissAction = c.options & UNNotificationCategoryOptionCustomDismissAction;

        for (UNNotificationAction* a in c.actions)
            category.actions.add (unNotificationActionToAction (a));

        return category;
    }
  #endif

    PushNotifications::Notification nsDictionaryToJuceNotification (NSDictionary* dictionary)
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
        Class::setThis (delegate, this);

        id<UIApplicationDelegate> appDelegate = [[UIApplication sharedApplication] delegate];

        SEL selector = NSSelectorFromString (@"setPushNotificationsDelegateToUse:");

        if ([appDelegate respondsToSelector: selector])
            [appDelegate performSelector: selector withObject: delegate];
    }

    virtual ~PushNotificationsDelegate() {}

    virtual void didRegisterUserNotificationSettings (UIUserNotificationSettings* notificationSettings) = 0;

    virtual void registeredForRemoteNotifications (NSData* deviceToken) = 0;

    virtual void failedToRegisterForRemoteNotifications (NSError* error) = 0;

    virtual void didReceiveRemoteNotification (NSDictionary* userInfo) = 0;

    virtual void didReceiveRemoteNotificationFetchCompletionHandler (NSDictionary* userInfo,
                                                                     void (^completionHandler)(UIBackgroundFetchResult result)) = 0;

    virtual void handleActionForRemoteNotificationCompletionHandler (NSString* actionIdentifier,
                                                                     NSDictionary* userInfo,
                                                                     NSDictionary* responseInfo,
                                                                     void (^completionHandler)()) = 0;

    virtual void didReceiveLocalNotification (UILocalNotification* notification) = 0;

    virtual void handleActionForLocalNotificationCompletionHandler (NSString* actionIdentifier,
                                                                    UILocalNotification* notification,
                                                                    void (^completionHandler)()) = 0;

    virtual void handleActionForLocalNotificationWithResponseCompletionHandler (NSString* actionIdentifier,
                                                                                UILocalNotification* notification,
                                                                                NSDictionary* responseInfo,
                                                                                void (^completionHandler)()) = 0;

  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
    virtual void willPresentNotificationWithCompletionHandler (UNNotification* notification,
                                                               void (^completionHandler)(UNNotificationPresentationOptions options)) = 0;

    virtual void didReceiveNotificationResponseWithCompletionHandler (UNNotificationResponse* response,
                                                                      void (^completionHandler)()) = 0;
  #endif

protected:
  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
    ScopedPointer<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>> delegate;
  #else
    ScopedPointer<NSObject<UIApplicationDelegate>> delegate;
  #endif

private:
    //==============================================================================
  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
    struct Class    : public ObjCClass<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>>
    {
        Class() : ObjCClass<NSObject<UIApplicationDelegate, UNUserNotificationCenterDelegate>> ("JucePushNotificationsDelegate_")
  #else
    struct Class    : public ObjCClass<NSObject<UIApplicationDelegate>>
    {
        Class() : ObjCClass<NSObject<UIApplicationDelegate>> ("JucePushNotificationsDelegate_")
  #endif
        {
            addIvar<PushNotificationsDelegate*> ("self");

            addMethod (@selector (application:didRegisterUserNotificationSettings:),                                                 didRegisterUserNotificationSettings,                            "v@:@@");
            addMethod (@selector (application:didRegisterForRemoteNotificationsWithDeviceToken:),                                    registeredForRemoteNotifications,                               "v@:@@");
            addMethod (@selector (application:didFailToRegisterForRemoteNotificationsWithError:),                                    failedToRegisterForRemoteNotifications,                         "v@:@@");
            addMethod (@selector (application:didReceiveRemoteNotification:),                                                        didReceiveRemoteNotification,                                   "v@:@@");
            addMethod (@selector (application:didReceiveRemoteNotification:fetchCompletionHandler:),                                 didReceiveRemoteNotificationFetchCompletionHandler,             "v@:@@@");
            addMethod (@selector (application:handleActionWithIdentifier:forRemoteNotification:withResponseInfo:completionHandler:), handleActionForRemoteNotificationCompletionHandler,             "v@:@@@@@");
            addMethod (@selector (application:didReceiveLocalNotification:),                                                         didReceiveLocalNotification,                                    "v@:@@");
            addMethod (@selector (application:handleActionWithIdentifier:forLocalNotification:completionHandler:),                   handleActionForLocalNotificationCompletionHandler,              "v@:@@@@");
            addMethod (@selector (application:handleActionWithIdentifier:forLocalNotification:withResponseInfo:completionHandler:),  handleActionForLocalNotificationWithResponseCompletionHandler,  "v@:@@@@@");

          #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
            addMethod (@selector (userNotificationCenter:willPresentNotification:withCompletionHandler:),                            willPresentNotificationWithCompletionHandler,                   "v@:@@@");
            addMethod (@selector (userNotificationCenter:didReceiveNotificationResponse:withCompletionHandler:),                     didReceiveNotificationResponseWithCompletionHandler,            "v@:@@@");
          #endif

            registerClass();
        }

        //==============================================================================
        static PushNotificationsDelegate& getThis (id self)         { return *getIvar<PushNotificationsDelegate*> (self, "self"); }
        static void setThis (id self, PushNotificationsDelegate* d) { object_setInstanceVariable (self, "self", d); }

        //==============================================================================
        static void didRegisterUserNotificationSettings                           (id self, SEL, UIApplication*,
                                                                                   UIUserNotificationSettings* settings)                        { getThis (self).didRegisterUserNotificationSettings (settings); }
        static void registeredForRemoteNotifications                              (id self, SEL, UIApplication*,
                                                                                   NSData* deviceToken)                                         { getThis (self).registeredForRemoteNotifications (deviceToken); }

        static void failedToRegisterForRemoteNotifications                        (id self, SEL, UIApplication*,
                                                                                   NSError* error)                                              { getThis (self).failedToRegisterForRemoteNotifications (error); }

        static void didReceiveRemoteNotification                                  (id self, SEL, UIApplication*,
                                                                                   NSDictionary* userInfo)                                      { getThis (self).didReceiveRemoteNotification (userInfo); }

        static void didReceiveRemoteNotificationFetchCompletionHandler            (id self, SEL, UIApplication*,
                                                                                   NSDictionary* userInfo,
                                                                                   void (^completionHandler)(UIBackgroundFetchResult result))   { getThis (self).didReceiveRemoteNotificationFetchCompletionHandler (userInfo, completionHandler); }

        static void handleActionForRemoteNotificationCompletionHandler            (id self, SEL, UIApplication*,
                                                                                   NSString* actionIdentifier,
                                                                                   NSDictionary* userInfo,
                                                                                   NSDictionary* responseInfo,
                                                                                   void (^completionHandler)())                                 { getThis (self).handleActionForRemoteNotificationCompletionHandler (actionIdentifier, userInfo, responseInfo, completionHandler); }

        static void didReceiveLocalNotification                                   (id self, SEL, UIApplication*,
                                                                                   UILocalNotification* notification)                           { getThis (self).didReceiveLocalNotification (notification); }

        static void handleActionForLocalNotificationCompletionHandler             (id self, SEL, UIApplication*,
                                                                                   NSString* actionIdentifier,
                                                                                   UILocalNotification* notification,
                                                                                   void (^completionHandler)())                                 { getThis (self).handleActionForLocalNotificationCompletionHandler (actionIdentifier, notification, completionHandler); }

        static void handleActionForLocalNotificationWithResponseCompletionHandler (id self, SEL, UIApplication*,
                                                                                   NSString* actionIdentifier,
                                                                                   UILocalNotification* notification,
                                                                                   NSDictionary* responseInfo,
                                                                                   void (^completionHandler)())                                 { getThis (self). handleActionForLocalNotificationWithResponseCompletionHandler (actionIdentifier, notification, responseInfo, completionHandler); }

      #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        static void willPresentNotificationWithCompletionHandler        (id self, SEL, UNUserNotificationCenter*,
                                                                         UNNotification* notification,
                                                                         void (^completionHandler)(UNNotificationPresentationOptions options))  { getThis (self).willPresentNotificationWithCompletionHandler (notification, completionHandler); }

        static void didReceiveNotificationResponseWithCompletionHandler (id self, SEL, UNUserNotificationCenter*,
                                                                         UNNotificationResponse* response,
                                                                         void (^completionHandler)())                                           { getThis (self).didReceiveNotificationResponseWithCompletionHandler (response, completionHandler); }
      #endif
    };

    //==============================================================================
    static Class& getClass()
    {
        static Class c;
        return c;
    }
};

//==============================================================================
bool PushNotifications::Notification::isValid() const noexcept
{
    const bool iOSEarlierThan10 = std::floor (NSFoundationVersionNumber) <= NSFoundationVersionNumber_iOS_9_x_Max;

    if (iOSEarlierThan10)
        return title.isNotEmpty() && body.isNotEmpty() && category.isNotEmpty();

    return title.isNotEmpty() && body.isNotEmpty() && identifier.isNotEmpty() && category.isNotEmpty();
}

//==============================================================================
struct PushNotifications::Pimpl : private PushNotificationsDelegate
{
    Pimpl (PushNotifications& p)
        : owner (p)
    {
    }

    void requestPermissionsWithSettings (const PushNotifications::Settings& settingsToUse)
    {
        settings = settingsToUse;

        auto* categories = [NSMutableSet setWithCapacity: (NSUInteger) settings.categories.size()];

        if (iOSEarlierThan10)
        {
            for (const auto& c : settings.categories)
            {
                auto* category = (UIUserNotificationCategory*) PushNotificationsDelegateDetails::categoryToNSCategory (c, iOSEarlierThan10);
                [categories addObject: category];
            }

            UIUserNotificationType type = NSUInteger ((bool)settings.allowBadge << 0
                                                    | (bool)settings.allowSound << 1
                                                    | (bool)settings.allowAlert << 2);

            UIUserNotificationSettings* s = [UIUserNotificationSettings settingsForTypes: type categories: categories];
            [[UIApplication sharedApplication] registerUserNotificationSettings: s];
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
        {
            for (const auto& c : settings.categories)
            {
                auto* category = (UNNotificationCategory*) PushNotificationsDelegateDetails::categoryToNSCategory (c, iOSEarlierThan10);
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
       #endif

        [[UIApplication sharedApplication] registerForRemoteNotifications];
    }

    void requestSettingsUsed()
    {
        if (iOSEarlierThan10)
        {
            UIUserNotificationSettings* s = [UIApplication sharedApplication].currentUserNotificationSettings;

            settings.allowBadge = s.types & UIUserNotificationTypeBadge;
            settings.allowSound = s.types & UIUserNotificationTypeSound;
            settings.allowAlert = s.types & UIUserNotificationTypeAlert;

            for (UIUserNotificationCategory *c in s.categories)
                settings.categories.add (PushNotificationsDelegateDetails::uiUserNotificationCategoryToCategory (c));

            owner.listeners.call ([&] (Listener& l) { l.notificationSettingsReceived (settings); });
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
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
       #endif
    }

    bool areNotificationsEnabled() const { return true; }

    void sendLocalNotification (const Notification& n)
    {
        if (iOSEarlierThan10)
        {
            auto* notification = PushNotificationsDelegateDetails::juceNotificationToUILocalNotification (n);

            [[UIApplication sharedApplication] scheduleLocalNotification: notification];
            [notification release];
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
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
       #endif
    }

    void getDeliveredNotifications() const
    {
        if (iOSEarlierThan10)
        {
            // Not supported on this platform
            jassertfalse;
            owner.listeners.call ([] (Listener& l) { l.deliveredNotificationsListReceived ({}); });
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
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
       #endif
    }

    void removeAllDeliveredNotifications()
    {
        if (iOSEarlierThan10)
        {
            // Not supported on this platform
            jassertfalse;
        }
        else
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        {

            [[UNUserNotificationCenter currentNotificationCenter] removeAllDeliveredNotifications];
        }
       #endif
    }

    void removeDeliveredNotification (const String& identifier)
    {
        if (iOSEarlierThan10)
        {
            ignoreUnused (identifier);
            // Not supported on this platform
            jassertfalse;
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
        {

            NSArray<NSString*>* identifiers = [NSArray arrayWithObject: juceStringToNS (identifier)];

            [[UNUserNotificationCenter currentNotificationCenter] removeDeliveredNotificationsWithIdentifiers: identifiers];
        }
       #endif
    }

    void setupChannels (const Array<ChannelGroup>& groups, const Array<Channel>& channels)
    {
        ignoreUnused (groups, channels);
    }

    void getPendingLocalNotifications() const
    {
        if (iOSEarlierThan10)
        {
            Array<PushNotifications::Notification> notifs;

            for (UILocalNotification* n in [UIApplication sharedApplication].scheduledLocalNotifications)
                notifs.add (PushNotificationsDelegateDetails::uiLocalNotificationToJuceNotification (n));

            owner.listeners.call ([&] (Listener& l) { l.pendingLocalNotificationsListReceived (notifs); });
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
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
       #endif
    }

    void removePendingLocalNotification (const String& identifier)
    {
        if (iOSEarlierThan10)
        {
            // Not supported on this platform
            jassertfalse;
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
        {

            NSArray<NSString*>* identifiers = [NSArray arrayWithObject: juceStringToNS (identifier)];

            [[UNUserNotificationCenter currentNotificationCenter] removePendingNotificationRequestsWithIdentifiers: identifiers];
        }
       #endif
    }

    void removeAllPendingLocalNotifications()
    {
        if (iOSEarlierThan10)
        {
            [[UIApplication sharedApplication] cancelAllLocalNotifications];
        }
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        else
        {
            [[UNUserNotificationCenter currentNotificationCenter] removeAllPendingNotificationRequests];
        }
       #endif
    }

    String getDeviceToken()
    {
        // You need to call requestPermissionsWithSettings() first.
        jassert (initialised);

        return deviceToken;
    }

    //==============================================================================
    //PushNotificationsDelegate
    void didRegisterUserNotificationSettings (UIUserNotificationSettings*) override
    {
        requestSettingsUsed();
    }

    void registeredForRemoteNotifications (NSData* deviceTokenToUse) override
    {
        NSString* deviceTokenString = [[[[deviceTokenToUse description]
                                          stringByReplacingOccurrencesOfString: nsStringLiteral ("<") withString: nsStringLiteral ("")]
                                          stringByReplacingOccurrencesOfString: nsStringLiteral (">") withString: nsStringLiteral ("")]
                                          stringByReplacingOccurrencesOfString: nsStringLiteral (" ") withString: nsStringLiteral ("")];

        deviceToken = nsStringToJuce (deviceTokenString);

        initialised = true;

        owner.listeners.call ([&] (Listener& l) { l.deviceTokenRefreshed (deviceToken); });
    }

    void failedToRegisterForRemoteNotifications (NSError* error) override
    {
        ignoreUnused (error);

        deviceToken.clear();
    }

    void didReceiveRemoteNotification (NSDictionary* userInfo) override
    {
        auto n = PushNotificationsDelegateDetails::nsDictionaryToJuceNotification (userInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotification (false, n); });
    }

    void didReceiveRemoteNotificationFetchCompletionHandler (NSDictionary* userInfo,
                                                             void (^completionHandler)(UIBackgroundFetchResult result)) override
    {
        didReceiveRemoteNotification (userInfo);
        completionHandler (UIBackgroundFetchResultNewData);
    }

    void handleActionForRemoteNotificationCompletionHandler (NSString* actionIdentifier,
                                                             NSDictionary* userInfo,
                                                             NSDictionary* responseInfo,
                                                             void (^completionHandler)()) override
    {
        auto n = PushNotificationsDelegateDetails::nsDictionaryToJuceNotification (userInfo);
        auto actionString = nsStringToJuce (actionIdentifier);
        auto response = PushNotificationsDelegateDetails::getUserResponseFromNSDictionary (responseInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (false, n, actionString, response); });

        completionHandler();
    }

    void didReceiveLocalNotification (UILocalNotification* notification) override
    {
        auto n = PushNotificationsDelegateDetails::uiLocalNotificationToJuceNotification (notification);

        owner.listeners.call ([&] (Listener& l) { l.handleNotification (true, n); });
    }

    void handleActionForLocalNotificationCompletionHandler (NSString* actionIdentifier,
                                                            UILocalNotification* notification,
                                                            void (^completionHandler)()) override
    {
        handleActionForLocalNotificationWithResponseCompletionHandler (actionIdentifier,
                                                                       notification,
                                                                       nil,
                                                                       completionHandler);
    }

    void handleActionForLocalNotificationWithResponseCompletionHandler (NSString* actionIdentifier,
                                                                        UILocalNotification* notification,
                                                                        NSDictionary* responseInfo,
                                                                        void (^completionHandler)()) override
    {
        auto n = PushNotificationsDelegateDetails::uiLocalNotificationToJuceNotification (notification);
        auto actionString = nsStringToJuce (actionIdentifier);
        auto response = PushNotificationsDelegateDetails::getUserResponseFromNSDictionary (responseInfo);

        owner.listeners.call ([&] (Listener& l) { l.handleNotificationAction (true, n, actionString, response); });

        completionHandler();
    }

  #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
    void willPresentNotificationWithCompletionHandler (UNNotification* notification,
                                                       void (^completionHandler)(UNNotificationPresentationOptions options)) override
    {
        NSUInteger options = NSUInteger ((int)settings.allowBadge << 0
                                       | (int)settings.allowSound << 1
                                       | (int)settings.allowAlert << 2);

        ignoreUnused (notification);

        completionHandler (options);
    }

    void didReceiveNotificationResponseWithCompletionHandler (UNNotificationResponse* response,
                                                              void (^completionHandler)()) override
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
  #endif

    void subscribeToTopic (const String& topic)     { ignoreUnused (topic); }
    void unsubscribeFromTopic (const String& topic) { ignoreUnused (topic); }

    void sendUpstreamMessage (const String& serverSenderId,
                              const String& collapseKey,
                              const String& messageId,
                              const String& messageType,
                              int timeToLive,
                              const StringPairArray& additionalData)
    {
        ignoreUnused (serverSenderId, collapseKey, messageId, messageType);
        ignoreUnused (timeToLive, additionalData);
    }

private:
    PushNotifications& owner;

    const bool iOSEarlierThan10 = std::floor (NSFoundationVersionNumber) <= NSFoundationVersionNumber_iOS_9_x_Max;

    bool initialised = false;
    String deviceToken;

    PushNotifications::Settings settings;
};

} // namespace juce
