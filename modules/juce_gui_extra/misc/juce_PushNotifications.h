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

#pragma once

namespace juce
{

/** Singleton class responsible for push notifications functionality. Both remote and
    local notifications are supported. To get information about notifications,
    register a listener on your application startup. It is best to register the
    listener as soon as possible, because your application can be launched from
    a push notification too.

    To send a local notification create an instance of Notification, fill the necessary
    fields and call PushNotifications::sendLocalNotification(). When receiving local or
    remote notifications, inspect the Notification's fields for notification details.
    Bear in mind that some fields will not be available when receiving a remote
    notification.

    @tags{GUI}
*/
class JUCE_API PushNotifications    : private DeletedAtShutdown
{
public:
   #ifndef DOXYGEN
    JUCE_DECLARE_SINGLETON_INLINE (PushNotifications, false)
   #endif

    //==============================================================================
    /** Represents a notification that can be sent or received. */
    struct Notification
    {
        Notification() = default;
        Notification (const Notification& other);

        /** Checks whether a given notification is correctly configured for a given OS. */
        bool isValid() const noexcept;

        /** Represents an action on a notification that can be presented as a button or a text input.
            On Android, each notification has its action specified explicitly, on iOS you configure an
            allowed set of actions on startup and pack them into categories (see Settings).
        */
        struct Action
        {
            /** Controls the appearance of this action. */
            enum Style
            {
                button,                    /**< Show this action as a button. */
                text                       /**< Show this action as a text input field (on Android API 20 or higher is required). */
            };

            /** @name Common fields */
            /**@{*/
            Style  style = button;
            String title;                 /**< Required. the name of the action displayed to the user. */
            String textInputPlaceholder;  /**< Optional: placeholder text for text input notification.
                                               Note that it will be ignored if button style is used. */
            var    parameters;            /**< Optional: additional parameters that can be passed. */
            /**@}*/

            /** @name iOS only fields */
            /**@{*/
            String  identifier;                    /**< Required: unique identifier. This should be one of the
                                                        identifiers set with requestPermissionsWithSettings(). */
            bool    triggerInBackground = false;   /**< Whether the app can process the action in background. */
            bool    destructive = false;           /**< Whether to display the action as destructive. */
            String  textInputButtonText;           /**< Optional: Text displayed on text input notification
                                                        button.
                                                        Note that it will be ignored if style is set to Style::button. */
            /**@}*/

            /** @name Android only fields */
            /**@{*/
            String       icon;            /**< Optional: name of an icon file (without an extension) to be used for
                                                         this action. This must be the name of one of the image
                                                         files included into resources when exporting an Android project
                                                         (see "Extra Android Raw Resources" setting in Projucer).
                                                         Note that not all Android versions support an icon for an action, though
                                                         it is recommended to provide it nevertheless. */

            StringArray allowedResponses; /**< Optional: a list of possible answers if the answer set is limited.
                                               When left empty, then the user will be able to input any text. */
            /**@}*/
        };

        //==============================================================================
        /** @name Common fields */
        /**@{*/

        String identifier;   /**< Required: unique id that can be used to later dismiss the notification
                                  (on iOS available from version 10). */

        String title;        /**< Required: the title of the notification, usually displayed in the first row. */
        String body;         /**< Required: the content of the notification, usually displayed in the second row. */
        String subtitle;     /**< Optional: additional text, that may be displayed e.g. in the third row or in the header
                                  area. Note that on Android, depending on OS version, this may fight for
                                  space with other components of the notification, so use this field
                                  judiciously. On iOS available from version 10. On Android available from API 16. */

        String groupId;      /**< Optional: allows the OS to visually group, collapse, and expand a set of notifications,
                                  note that OS may automatically group notifications if no groupId is specified. */

        int badgeNumber = 0; /**< Optional: on platforms that support it, can set a number this notification represents. */
        URL soundToPlay;     /**< Optional: empty when the notification should be silent. When the name is set to
                                  "default_os_sound", then a default sound will be used.

                                  For a custom sound on OSX, set the URL to the name of a sound file (preferably without
                                  an extension) and place the sound file directly in bundle's "Resources" directory (you
                                  can use "Xcode Resource" tickbox in Projucer to achieve that), i.e. it cannot be in a
                                  subdirectory of "Resources" like "Resources/sound". Alternatively, if a sound file
                                  cannot be found in bundle's "Resources" directory, the OS may look for the sound in the
                                  following paths: "~/Library/Sounds", "/Library/Sounds", "/Network/Library/Sounds",
                                  "/System/Library/Sounds".

                                  For a custom sound on iOS, set the URL to a relative path within your bundle, including
                                  file extension. For instance, if your bundle contains "sounds" folder with "my_sound.caf"
                                  file, then the URL should be "sounds/my_sound.caf".

                                  For a custom sound on Android, set URL to the name of a raw resource file
                                  (without an extension) that was included when exporting an Android project in
                                  Projucer (see "Extra Android Raw Resources" setting). */

        var properties;      /**< Optional: collection of additional properties that may be passed as a dictionary. */

        /**@}*/

        //==============================================================================
        /** @name iOS only fields */
        /**@{*/

        String category;                /**< Required: determines set of actions that will appear (as per setup done
                                             in requestPermissionsWithSettings()). */
        double triggerIntervalSec = 0.; /**< Optional: specifies number of seconds before the notification should trigger. */
        bool   repeat = false;          /**< Optional: allows the notification to continuously retrigger after
                                             triggerIntervalSec seconds. */

        /**@}*/

        //==============================================================================
        /** @name Android only fields */
        /**@{*/

        String icon;             /**< Required: name of an icon file (without an extension) to be used for
                                                this notification. This must be the name of one of the image
                                                files included into resources when exporting an Android project
                                                (see "Extra Android Raw Resources" setting in Projucer). */

        String channelId;       /**< Required for Android API level 26 or above: specifies notification channel id. Refer to
                                     setupChannels(). Ignored on earlier Android versions. */

        Image largeIcon;        /**< Optional: an additional large icon displayed in the notification content view. */

        String tickerText;      /**< Optional: ticker text used for accessibility services. */

        Array<Action> actions;  /**< Optional: actions associated with the notification. Note that the OS may allow only a limited
                                     number of actions to be presented, so always present most important actions first.
                                     Available from Android API 16 or above. */

        /** Used to represent a progress of some operation. */
        struct Progress
        {
            int  max = 0;               /**< Max possible value of a progress. A typical usecase is to set max to 100 and increment
                                             current's value as percentage complete. */
            int  current = 0;           /**< Current progress value, should be from 0 to max. */
            bool indeterminate = false; /**< If true, then the progress represents a continuing activity indicator with ongoing
                                             animation and no numeric value. */
        };

        Progress progress;       /**< Optional: set to default (0, 0, false), to disable progress display. */

        /** Metadata that can be used by the OS to better handle the notification, depending on its priority. */
        enum Type
        {
            unspecified,       /**< Category not set. */
            alarm,             /**< Alarm or timer. */
            call,              /**< Incoming voice/video call or similar. */
            email,             /**< Async message like email. */
            error,             /**< Error in background operation or authentication status. */
            event,             /**< Calendar event. */
            message,           /**< Incoming message (sms, instant message etc.). */
            taskProgress,      /**< Progress for a long-running background operation. */
            promo,             /**< Promotion or advertisement. */
            recommendation,    /**< Specific, single thing related recommendation. */
            reminder,          /**< User-scheduled reminder. */
            service,           /**< Running background service. */
            social,            /**< Social network or sharing update. */
            status,            /**< Ongoing information about device or contextual status. */
            system,            /**< System or device status update. */
            transport          /**< Media transport control for playback. */
        };

        /** Metadata used as a hint to the OS about the priority of the notification. */
        enum Priority
        {
            veryLow  = -2,
            low      = -1,
            medium   =  0,
            high     =  1,
            veryHigh =  2
        };

        String person;               /**< Optional: additional metadata used as a hint to OS that a notification is
                                          related to a specific person. Can be useful for instance messaging apps.
                                          Available from Android API 21 or above. */

        Type type = unspecified;     /**< Optional. Available from Android API 21 or above. */
        Priority priority = medium;  /**< Optional. Available from Android API 16 or above. */

        /** Describes how to show the notification when the screen is locked. Available from Android API 21 or above. */
        enum LockScreenAppearance
        {
            dontShow       = -1,  /**< The notification is not allowed on the lock screen */
            showPartially  =  0,  /**< Only some information is allowed on the lock screen */
            showCompletely =  1   /**< The entire notification is allowed on the lock screen */
        };

        LockScreenAppearance lockScreenAppearance = showPartially;  /**< Optional. */

        std::unique_ptr<Notification> publicVersion; /**< Optional: if you set lockScreenAppearance to showPartially,
                                                          then you can provide "public version" of your notification
                                                          that will be displayed on the lock screen. This way you can
                                                          control what information is visible when the screen is locked. */

        String groupSortKey;         /**< Optional: Used to order notifications within the same group. Available from Android API 20 or above. */
        bool groupSummary = false;   /**< Optional: if true, then this notification will be a group summary of the group set with groupId.
                                                    Available from Android API 20 or above. */

        Colour accentColour;  /**< Optional: sets accent colour. The default colour will be used if accentColour is not set.
                                             Available from Android API 21 or above. */
        Colour ledColour;     /**< Optional: Sets the led colour. The hardware will do its best to approximate the colour.
                                   The default colour will be used if ledColour is not set. */

        /** Allows to control the time the device's led is on and off. */
        struct LedBlinkPattern
        {
            int msToBeOn  = 0;   /**< The led will be on for the given number of milliseconds, after which it will turn off. */
            int msToBeOff = 0;   /**< The led will be off for the given number of milliseconds, after which it will turn on. */
        };

        LedBlinkPattern ledBlinkPattern; /**< Optional. */

        Array<int> vibrationPattern;  /**< Optional: sets the vibration pattern in milliseconds. The first value indicates how long
                                           to wait until vibration starts. The second value indicates how long to vibrate. The third
                                           value will say how long to not vibrate and so on. For instance, if the pattern is:
                                           1000, 2000, 3000, 4000 - then one second after receiving a notification the device will
                                           vibrate for two seconds, followed by 3 seconds of no vibration and finally, 4 seconds of
                                           vibration. */

        bool shouldAutoCancel = true; /**< Optional: If true, the notification will be automatically cancelled when a user clicks it in the panel. */

        bool localOnly = true;  /**< Optional: whether or not the notification should bridge to other devices.
                                               Available from Android API 20 or above. */

        bool ongoing = false;   /**< Optional: If true, then it cannot be dismissed by the user and it must be dismissed manually.
                                     Typically used for ongoing background tasks that the user is actively engaged with. To
                                     dismiss such notification, you need to call removeDeliveredNotification() or
                                     removeAllDeliveredNotifications(). */

        bool alertOnlyOnce = false; /**< Optional: Set this flag if you would only like the sound, vibrate and ticker to be played if the notification
                                         is not already showing. */

        /** Controls timestamp visibility and format. */
        enum TimestampVisibility
        {
            off,                    /**< Do not show timestamp. */
            normal,                 /**< Show normal timestamp. */
            chronometer,            /**< Show chronometer as a stopwatch. Available from Android API 16 or above. */
            countDownChronometer    /**< Set the chronometer to count down instead of counting up. Available from Android API 24 or above.*/
        };

        TimestampVisibility timestampVisibility = normal;  /**< Optional. */

        /** Controls badge icon type to use if a notification is shown as a badge. Available from Android API 26 or above. */
        enum BadgeIconType
        {
            none,
            small,
            large
        };

        BadgeIconType badgeIconType = large;

        /** Controls sound and vibration behaviour for group notifications. Available from Android API 26 or above. */
        enum GroupAlertBehaviour
        {
            alertAll,           /**< both child notifications and group notifications should produce sound and vibration. */
            AlertSummary,       /**< all child notifications in the group should have no sound nor vibration, even
                                     if corresponding notification channel has sounds and vibrations enabled. */
            AlertChildren       /**< summary notifications in the group should have no sound nor vibration, even if
                                     corresponding notification channel has sounds and vibrations enabled. */
        };

        GroupAlertBehaviour groupAlertBehaviour = alertAll;

        int timeoutAfterMs = 0;    /**< specifies a duration in milliseconds, after which the notification should be
                                        cancelled, if it is not already cancelled. Available from Android API 26 or above. */
        /**@}*/
    };


    //==============================================================================
    /** Describes settings we want to use for current device. Note that at the
        moment this is only used on iOS and partially on OSX.

        On OSX only allow* flags are used and they control remote notifications only.
        To control sound, alert and badge settings for local notifications on OSX,
        use Notifications settings in System Preferences.

        To setup push notifications for current device, provide permissions required,
        as well as register categories of notifications you want to support. Each
        category needs to have a unique identifier and it can optionally have multiple
        actions. Each action also needs to have a unique identifier. The example setup
        may look as follows:

        @code

        using Action   = PushNotifications::Settings::Action;
        using Category = PushNotifications::Settings::Category;

        Action okAction;
        okAction.identifier = "okAction";
        okAction.title = "OK!";
        okAction.style = Action::button;
        okAction.triggerInBackground = true;

        Action cancelAction;
        cancelAction.identifier = "cancelAction";
        cancelAction.title = "Cancel";
        cancelAction.style = Action::button;
        cancelAction.triggerInBackground = true;
        cancelAction.destructive = true;

        Action textAction;
        textAction.identifier = "textAction";
        textAction.title = "Enter text";
        textAction.style = Action::text;
        textAction.triggerInBackground = true;
        textAction.destructive = false;
        textAction.textInputButtonText = "Ok";
        textAction.textInputPlaceholder = "Enter text...";

        Category okCategory;
        okCategory.identifier = "okCategory";
        okCategory.actions = { okAction };

        Category okCancelCategory;
        okCancelCategory.identifier = "okCancelCategory";
        okCancelCategory.actions = { okAction, cancelAction };

        Category textCategory;
        textCategory.identifier = "textCategory";
        textCategory.actions = { textAction };
        textCategory.sendDismissAction = true;

        PushNotifications::Settings settings;
        settings.allowAlert = true;
        settings.allowBadge = true;
        settings.allowSound = true;
        settings.categories = { okCategory, okCancelCategory, textCategory };

        @endcode
    */
    struct Settings
    {
        using Action = Notification::Action;

        /** Describes a category of a notification. Each category has a unique identifier
            and a list of associated actions.
            Note that the OS may allow only a limited number of actions to be presented, so
            always present most important actions first.
        */
        struct Category
        {
            String identifier;               /**< unique identifier */
            Array<Action> actions;           /**< optional list of actions within this category */
            bool sendDismissAction = false;  /**< whether dismiss action will be sent to the app */
        };

        bool allowSound = false;      /**< whether the app should play a sound upon notification */
        bool allowAlert = false;      /**< whether the app should present an alert upon notification */
        bool allowBadge = false;      /**< whether the app may badge its icon upon notification */
        Array<Category> categories;   /**< list of categories the app wants to support */
    };

    /** Initialises push notifications on current device with the settings provided.
        Call this on your application startup and on iOS the first time the application starts,
        a user will be presented with a permission request dialog to give push notifications permission.
        Once a user responds, Listener::notificationSettingsReceived() will be called so that
        you can check what permissions where actually granted. The listener callback will be called
        on each subsequent startup too (provided you called requestPermissionsWithSettings() on previous
        application run). This way you can check what are current push notifications permissions.

        Note that settings are currently only used on iOS. When calling on other platforms, Settings
        with no categories and all allow* flags set to true will be received in
        Listener::notificationSettingsReceived().

        You can also call requestSettingsUsed() to explicitly ask for current settings.
    */
    void requestPermissionsWithSettings (const Settings& settings);

    /** Sends an asynchronous request to retrieve current settings that are currently in use.
        These can be exactly the same as used in requestPermissionsWithSettings(), but depending
        on user's subsequent changes in OS settings, the actual current settings may be
        different (e.g. user might have later decided to disable sounds).

        Note that settings are currently only used on iOS and partially on OSX.

        On OSX, only allow* flags are used and they refer to remote notifications only. For
        local notifications, refer to System Preferences.

        When calling this function on other platforms, Settings with no categories and all allow*
        flags set to true will be received in Listener::notificationSettingsReceived().
    */
    void requestSettingsUsed();

    //==============================================================================
    /** Android API level 26 or higher only: Represents notification channel through which
        notifications will be sent. Starting from Android API level 26, you should call setupChannels()
        at the start of your application, before posting any notifications. Then, when sending notifications,
        assign a channel to each created notification.
    */
    struct Channel
    {
        String identifier;          /**< Required: Unique channel identifier. */
        String name;                /**< Required: User facing name of the channel. */

        /** Controls how interruptive the notification posted on this channel are. */
        enum Importance
        {
            none,
            min,
            low,
            normal,
            high,
            max
        };

        Importance importance = normal;     /**< Required. */
        Notification::LockScreenAppearance lockScreenAppearance = Notification::showPartially;  /**< Optional. */

        String description;                 /**< Optional: user visible description of the channel. */
        String groupId;                     /**< Required: group this channel belongs to (see ChannelGroup). */
        Colour ledColour;                   /**< Optional: sets the led colour for notifications in this channel. */
        bool bypassDoNotDisturb = false;    /**< Optional: true if notifications in this channel can bypass do not disturb setting. */
        bool canShowBadge = false;          /**< Optional: true if notifications in this channel can show badges in a Launcher application. */
        bool enableLights = false;          /**< Optional: true if notifications in this channel should show lights (subject to hardware support). */
        bool enableVibration = false;       /**< Optional: true if notifications in this channel should trigger vibrations. */

        URL soundToPlay;                    /**< Optional: sound to play in this channel. See Notification::soundToPlay for more info. */
        Array<int> vibrationPattern;        /**< Optional: vibration pattern for this channel. See Notification::vibrationPattern for more info. */
    };

    /** Android API level 26 or higher only: represents a channel group. This allows for
        visual grouping of corresponding channels in notification settings presented to the user.
        At least one channel group has to be specified before notifications can be sent.
    */
    struct ChannelGroup
    {
        String identifier;                  /**< Required: Unique channel group identifier. */
        String name;                        /**< Required: User visible name of the channel group. */
    };

    /** Android API level 26 or higher only: configures notification channel groups and channels to be
        used in the app. These have to be setup before notifications can be sent on Android API
        level 26 or higher.
    */
    void setupChannels (const Array<ChannelGroup>& groups, const Array<Channel>& channels);

    //==============================================================================
    /** iOS only: sends an asynchronous request to retrieve a list of notifications that were
        scheduled and not yet delivered.

        When the list is retrieved, Listener::pendingLocalNotificationsListReceived() will be called.
    */
    void getPendingLocalNotifications() const;

    /** Unschedules a pending local notification with a given identifier. */
    void removePendingLocalNotification (const String& identifier);

    /** Unschedules all pending local notifications. iOS only. */
    void removeAllPendingLocalNotifications();

    //==============================================================================
    /** Checks whether notifications are enabled for given application.
        On iOS and OSX this will always return true, use requestSettingsUsed() instead.
    */
    bool areNotificationsEnabled() const;

    /** On iOS as well as on Android, sends a local notification.
        This will refresh an existing notification if the same identifier is used as in
        a notification that was already sent and not yet responded by a user.
    */
    void sendLocalNotification (const Notification& notification);

    /** Sends a request for a list of notifications delivered. Such notifications are visible in the
        notification area on the device and they are still waiting for user action/response.
        When the request is finished Listener::deliveredNotificationsListReceived() will be called.

        On iOS, iOS version 10 or higher is required. On Android, API level 18 or higher is required.
        For unsupported platforms, Listener::deliveredNotificationsListReceived() will return an empty array.
    */
    void getDeliveredNotifications() const;

    /** Removes a previously delivered notification. This can be useful for instance when the
        information in the notification becomes obsolete.
    */
    void removeDeliveredNotification (const String& identifier);

    /** Removes all notifications that were delivered. */
    void removeAllDeliveredNotifications();

    //==============================================================================
    /** Retrieves current device token. Note, it is not a good idea to cache this token
        because it may change in the meantime. Always call this method to get the current
        token value.
    */
    String getDeviceToken() const;

    /** Android only: allows to subscribe to messages from a specific topic.
        So you could for instance subscribe this device to all "sports" topic messages
        to receive any remote notifications that have "sports" topic set.
        Refer to Firebase documentation for how to send topic messages.
    */
    void subscribeToTopic (const String& topic);

    /** Android only: allows to remove a topic subscription that was previously added with
        subscribeToTopic().
    */
    void unsubscribeFromTopic (const String& topic);

    /** Android only: sends an upstream message to your app server. The server must implement
        XMPP Connection Server protocol (refer to Firebase documentation).

        @param serverSenderId       Represents the sender. Consult your Firebase project
                                    settings to retrieve the sender id.

        @param collapseKey          Remote messages with the same collapse key that were not
                                    yet delivered will be collapsed into one, with the
                                    newest message replacing all the previous ones.
                                    Note that there may be a limit of maximum collapse keys
                                    used at the same time and beyond the limit (refer to
                                    Firebase documentation) it is not guaranteed which keys
                                    will be in use by the server.

        @param messageId            A unique message ID. Used in error callbacks and debugging.

        @param messageType          Message type.

        @param timeToLive           TTL in seconds. If 0, the message sending will be attempted
                                    immediately and it will be dropped if the device is not
                                    connected. Otherwise, the message will be queued for the
                                    period specified.

        @param additionalData       Collection of key-value pairs to be used as an additional
                                    data for the message.
    */
    void sendUpstreamMessage (const String& serverSenderId,
                              const String& collapseKey,
                              const String& messageId,
                              const String& messageType,
                              int timeToLive,
                              const StringPairArray& additionalData);

    //==============================================================================
    /** Register a listener (ideally on application startup) to receive information about
        notifications received and any callbacks to async functions called.
    */
    struct Listener
    {
        virtual ~Listener() = default;

        /** This callback will be called after you call requestSettingsUsed() or
            requestPermissionsWithSettings().

            Note that settings are currently only used on iOS. When called on other platforms, Settings
            with no categories and all allow flags set to true will be received in
            Listener::notificationSettingsReceived().
        */
        virtual void notificationSettingsReceived (const Settings& settings);

        /** Called when the list of pending notifications, requested by calling
            getPendingLocalNotifications() is returned.
        */
        virtual void pendingLocalNotificationsListReceived (const Array<Notification>& notifications);

        /** This can be called in multiple different situations, depending on the OS and the situation.

            This will be called when a user presses on a notification

            Note: On Android, if remote notification was received while the app was in the background and
            then user pressed on it, the notification object received in this callback will contain only
            "properties" member set. Hence, if you want to know what was the notification title, content
            etc, you need to set them as additional properties, so that you will be able to restore them
            from "properties" dictionary.

            Note you can receive this callback on startup, if the application was launched from a notification.
        */
        virtual void handleNotification (bool isLocalNotification, const Notification& notification);

        /** This can be called when a user performs some action on the notification such as
            pressing on an action button or responding with a text input.
            Note that pressing on a notification area, i.e. not on an action button is not considered
            to be an action, and hence receivedNotification() will be called in that case.

            Note you can receive this callback on startup, if the application was launched from a notification's action.

            @param isLocalNotification If the notification is local
            @param notification        The notification
            @param actionIdentifier    A String identifying the action
            @param optionalResponse    Text response a user inputs for notifications with a text input.
                                       Empty for notifications without a text input option.

        */
        virtual void handleNotificationAction (bool isLocalNotification,
                                               const Notification& notification,
                                               const String& actionIdentifier,
                                               const String& optionalResponse);

        /** For iOS10 and Android, this can be also called when a user dismissed the notification before
            responding to it.
        */
        virtual void localNotificationDismissedByUser (const Notification& notification);

        /** Called after getDeliveredNotifications() request is fulfilled. Returns notifications
            that are visible in the notification area on the device and that are still waiting
            for a user action/response.

            On iOS, iOS version 10 or higher is required. On Android, API level 18 or higher is required.
            For unsupported platforms, an empty array will be returned.
         */
        virtual void deliveredNotificationsListReceived (const Array<Notification>& notifications);

        /** Called whenever a token gets refreshed. You should monitor any token updates, because
            only the last token that is assigned to device is valid and can be used.
        */
        virtual void deviceTokenRefreshed (const String& token);

        /** Called when Firebase Cloud Messaging server deletes pending messages. This can happen when
            1) too many messages were sent to the server (hint: use collapsible messages).
            2) the devices hasn't been online in a long time (refer to Firebase documentation for
               the maximum time a message can be stored on FCM before expiring).
        */
        virtual void remoteNotificationsDeleted();

        /** Called when an upstream message sent with PushNotifications::sendUpstreamMessage() has been
            sent successfully.
            Bear in mind that in may take several minutes or more to receive this callback.
        */
        virtual void upstreamMessageSent (const String& messageId);

        /** Called when there was an error sending an upstream message with
            PushNotifications::sendUpstreamMessage().
            Bear in mind that in may take several minutes or more to receive this callback.
        */
        virtual void upstreamMessageSendingError (const String& messageId, const String& error);
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    PushNotifications();
    ~PushNotifications() override;

    ListenerList<PushNotifications::Listener> listeners;

   #if JUCE_ANDROID
    friend bool juce_handleNotificationIntent (void*);

    friend struct JuceFirebaseInstanceIdService;
    friend struct JuceFirebaseMessagingService;
   #endif

    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace juce
