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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"




//==============================================================================
class MainContentComponent   : public Component,
                               private Button::Listener,
                               private ComboBox::Listener,
                               private ChangeListener,
                               private ComponentListener,
                               private PushNotifications::Listener
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;

private:
    void buttonClicked (Button*) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    void sendLocalNotification();
    void fillRequiredParams      (PushNotifications::Notification& n);
    void fillOptionalParamsOne   (PushNotifications::Notification& n);
    void fillOptionalParamsTwo   (PushNotifications::Notification& n);
    void fillOptionalParamsThree (PushNotifications::Notification& n);
    void setupAccentColour();
    void setupLedColour();

    void getDeliveredNotifications();

    void changeListenerCallback (ChangeBroadcaster* source) override;

    void componentBeingDeleted (Component& component) override;

    void handleNotification (bool isLocalNotification, const PushNotifications::Notification& n) override;

    void handleNotificationAction (bool isLocalNotification,
                                   const PushNotifications::Notification& n,
                                   const juce::String& actionIdentifier,
                                   const juce::String& optionalResponse) override;

    void localNotificationDismissedByUser (const PushNotifications::Notification& n) override;

    void deliveredNotificationsListReceived (const Array<PushNotifications::Notification>&) override;

    void pendingLocalNotificationsListReceived (const Array<PushNotifications::Notification>&) override;

    void deviceTokenRefreshed (const String& token) override;

  #if JUCE_ANDROID
    void remoteNotificationsDeleted() override;

    void upstreamMessageSent (const String& messageId) override;

    void upstreamMessageSendingError (const String& messageId, const String& error) override;

    static Array<PushNotifications::Channel> getAndroidChannels();
  #elif JUCE_IOS
    static PushNotifications::Settings getIosSettings();
  #endif

    struct RequiredParamsView   : public Component
    {
        RequiredParamsView()
        {
            addAndMakeVisible (identifierLabel);
            addAndMakeVisible (identifierEditor);
            addAndMakeVisible (titleLabel);
            addAndMakeVisible (titleEditor);
            addAndMakeVisible (bodyLabel);
            addAndMakeVisible (bodyEditor);
          #if JUCE_IOS
            addAndMakeVisible (categoryLabel);
            addAndMakeVisible (categoryComboBox);

            categories.add ("okCategory");
            categories.add ("okCancelCategory");
            categories.add ("textCategory");

            for (const auto& c : categories)
                categoryComboBox.addItem (c, categoryComboBox.getNumItems() + 1);
            categoryComboBox.setSelectedItemIndex (0);

          #elif JUCE_ANDROID
           #if __ANDROID_API__ >= 26
            addAndMakeVisible (channelIdLabel);
            addAndMakeVisible (channelIdComboBox);

            for (int i = 1; i <= 3; ++i)
                channelIdComboBox.addItem (String (i), i);
            channelIdComboBox.setSelectedItemIndex (0);

           #endif
            addAndMakeVisible (iconLabel);
            addAndMakeVisible (iconComboBox);

            for (int i = 0; i < 5; ++i)
                iconComboBox.addItem ("icon" + String (i + 1), i + 1);
            iconComboBox.setSelectedItemIndex (0);
          #endif

            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        void resized() override
        {
            const int labelColumnWidth = getWidth() / 3;
          #if JUCE_ANDROID && __ANDROID_API__ >= 26
            const int rowHeight = getHeight() / 8;
          #else
            const int rowHeight = getHeight() / 7;
          #endif

            auto layoutRow = [labelColumnWidth] (Rectangle<int> rowBounds, Component& label, Component& editor)
            {
                label .setBounds (rowBounds.removeFromLeft (labelColumnWidth));
                editor.setBounds (rowBounds);
            };

            auto bounds = getLocalBounds();

            layoutRow (bounds.removeFromTop (rowHeight), identifierLabel, identifierEditor);
            layoutRow (bounds.removeFromTop (rowHeight), titleLabel, titleEditor);
            layoutRow (bounds.removeFromTop (4 * rowHeight), bodyLabel, bodyEditor);
          #if JUCE_IOS
            layoutRow (bounds.removeFromTop (rowHeight), categoryLabel, categoryComboBox);
          #elif JUCE_ANDROID
           #if __ANDROID_API__ >= 26
            layoutRow (bounds.removeFromTop (rowHeight), channelIdLabel, channelIdComboBox);
           #endif
            layoutRow (bounds.removeFromTop (rowHeight), iconLabel, iconComboBox);
          #endif
        }
        Label      identifierLabel { "identifierLabel", "Identifier" };
        TextEditor identifierEditor;
        Label      titleLabel { "titleLabel", "Title" };
        TextEditor titleEditor;
        Label      bodyLabel { "bodyLabel", "Body" };
        TextEditor bodyEditor;
      #if JUCE_IOS
        StringArray  categories;
        Label        categoryLabel { "categoryLabel", "Category" };
        ComboBox     categoryComboBox;
      #elif JUCE_ANDROID
        Label    channelIdLabel { "channelIdLabel", "Channel ID" };
        ComboBox channelIdComboBox;
        Label    iconLabel { "iconLabel", "Icon" };
        ComboBox iconComboBox;
      #endif
    };

    struct OptionalParamsOneView   : public Component
    {
        OptionalParamsOneView()
        {
            addAndMakeVisible (subtitleLabel);
            addAndMakeVisible (subtitleEditor);
            addAndMakeVisible (badgeNumberLabel);
            addAndMakeVisible (badgeNumberComboBox);
            addAndMakeVisible (soundToPlayLabel);
            addAndMakeVisible (soundToPlayComboBox);
            addAndMakeVisible (propertiesLabel);
            addAndMakeVisible (propertiesEditor);
          #if JUCE_IOS
            addAndMakeVisible (fireInLabel);
            addAndMakeVisible (fireInComboBox);
            addAndMakeVisible (repeatLabel);
            addAndMakeVisible (repeatButton);

            fireInComboBox.addItem ("Now", 1);

            for (int i = 1; i < 11; ++i)
                fireInComboBox.addItem (String (10 * i) + "seconds", i + 1);
            fireInComboBox.setSelectedItemIndex (0);

          #elif JUCE_ANDROID
            addAndMakeVisible (largeIconLabel);
            addAndMakeVisible (largeIconComboBox);
            addAndMakeVisible (badgeIconLabel);
            addAndMakeVisible (badgeIconComboBox);
            addAndMakeVisible (tickerTextLabel);
            addAndMakeVisible (tickerTextEditor);
            addAndMakeVisible (autoCancelLabel);
            addAndMakeVisible (autoCancelButton);
            addAndMakeVisible (alertOnlyOnceLabel);
            addAndMakeVisible (alertOnlyOnceButton);
            addAndMakeVisible (actionsLabel);
            addAndMakeVisible (actionsComboBox);

            largeIconComboBox.addItem ("none", 1);

            for (int i = 1; i < 5; ++i)
                largeIconComboBox.addItem ("icon" + String (i), i + 1);
            largeIconComboBox.setSelectedItemIndex (0);

            badgeIconComboBox.addItem ("none", 1);
            badgeIconComboBox.addItem ("small", 2);
            badgeIconComboBox.addItem ("large", 3);
            badgeIconComboBox.setSelectedItemIndex (2);

            actionsComboBox.addItem ("none", 1);
            actionsComboBox.addItem ("ok-cancel", 2);
            actionsComboBox.addItem ("ok-cancel-icons", 3);
            actionsComboBox.addItem ("text-input", 4);
            actionsComboBox.addItem ("text-input-limited_responses", 5);
            actionsComboBox.setSelectedItemIndex (0);
          #endif

            for (int i = 0; i < 7; ++i)
                badgeNumberComboBox.addItem (String (i), i + 1);
            badgeNumberComboBox.setSelectedItemIndex (0);

          #if JUCE_IOS
            String prefix = "sounds/";
            String extension = ".caf";
          #else
            String prefix;
            String extension;
          #endif

            soundToPlayComboBox.addItem ("none", 1);
            soundToPlayComboBox.addItem ("default_os_sound", 2);
            soundToPlayComboBox.addItem (prefix + "demonstrative" + extension, 3);
            soundToPlayComboBox.addItem (prefix + "isntit" + extension, 4);
            soundToPlayComboBox.addItem (prefix + "jinglebellssms" + extension, 5);
            soundToPlayComboBox.addItem (prefix + "served" + extension, 6);
            soundToPlayComboBox.addItem (prefix + "solemn" + extension, 7);
            soundToPlayComboBox.setSelectedItemIndex (1);

            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        void resized() override
        {
            const int labelColumnWidth = getWidth() / 3;
          #if JUCE_ANDROID
            const int rowHeight = getHeight() / 12;
          #else
            const int rowHeight = getHeight() / 8;
          #endif

            auto layoutRow = [labelColumnWidth] (Rectangle<int> rowBounds, Component& label, Component& editor)
            {
                label .setBounds (rowBounds.removeFromLeft (labelColumnWidth));
                editor.setBounds (rowBounds);
            };

            auto bounds = getLocalBounds();

            layoutRow (bounds.removeFromTop (rowHeight), subtitleLabel, subtitleEditor);
            layoutRow (bounds.removeFromTop (rowHeight), badgeNumberLabel, badgeNumberComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), soundToPlayLabel, soundToPlayComboBox);
            layoutRow (bounds.removeFromTop (3 * rowHeight), propertiesLabel, propertiesEditor);
          #if JUCE_IOS
            layoutRow (bounds.removeFromTop (rowHeight), fireInLabel, fireInComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), repeatLabel, repeatButton);
          #elif JUCE_ANDROID
            layoutRow (bounds.removeFromTop (rowHeight), largeIconLabel, largeIconComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), badgeIconLabel, badgeIconComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), tickerTextLabel, tickerTextEditor);
            layoutRow (bounds.removeFromTop (rowHeight), autoCancelLabel, autoCancelButton);
            layoutRow (bounds.removeFromTop (rowHeight), alertOnlyOnceLabel, alertOnlyOnceButton);
            layoutRow (bounds.removeFromTop (rowHeight), actionsLabel, actionsComboBox);
          #endif
        }
        Label      subtitleLabel { "subtitleLabel", "Subtitle" };
        TextEditor subtitleEditor;
        Label      badgeNumberLabel { "badgeNumberLabel", "BadgeNumber" };
        ComboBox   badgeNumberComboBox;
        Label      soundToPlayLabel { "soundToPlayLabel", "SoundToPlay" };
        ComboBox   soundToPlayComboBox;
        Label      propertiesLabel { "propertiesLabel", "Properties" };
        TextEditor propertiesEditor;
      #if JUCE_IOS
        Label        fireInLabel { "fireInLabel", "Fire in" };
        ComboBox     fireInComboBox;
        Label        repeatLabel { "repeatLabel", "Repeat" };
        ToggleButton repeatButton;
      #elif JUCE_ANDROID
        Label        largeIconLabel { "largeIconLabel", "Large Icon" };
        ComboBox     largeIconComboBox;
        Label        badgeIconLabel { "badgeIconLabel", "Badge Icon" };
        ComboBox     badgeIconComboBox;
        Label        tickerTextLabel { "tickerTextLabel", "Ticker Text" };
        TextEditor   tickerTextEditor;
        Label        autoCancelLabel { "autoCancelLabel", "AutoCancel" };
        ToggleButton autoCancelButton;
        Label        alertOnlyOnceLabel { "alertOnlyOnceLabel", "AlertOnlyOnce" };
        ToggleButton alertOnlyOnceButton;
        Label        actionsLabel { "actionsLabel", "Actions" };
        ComboBox     actionsComboBox;
      #endif
    };

    struct OptionalParamsTwoView   : public Component
    {
        OptionalParamsTwoView()
        {
            addAndMakeVisible (progressMaxLabel);
            addAndMakeVisible (progressMaxComboBox);
            addAndMakeVisible (progressCurrentLabel);
            addAndMakeVisible (progressCurrentComboBox);
            addAndMakeVisible (progressIndeterminateLabel);
            addAndMakeVisible (progressIndeterminateButton);
            addAndMakeVisible (categoryLabel);
            addAndMakeVisible (categoryComboBox);
            addAndMakeVisible (priorityLabel);
            addAndMakeVisible (priorityComboBox);
            addAndMakeVisible (personLabel);
            addAndMakeVisible (personEditor);
            addAndMakeVisible (lockScreenVisibilityLabel);
            addAndMakeVisible (lockScreenVisibilityComboBox);
            addAndMakeVisible (groupIdLabel);
            addAndMakeVisible (groupIdEditor);
            addAndMakeVisible (sortKeyLabel);
            addAndMakeVisible (sortKeyEditor);
            addAndMakeVisible (groupSummaryLabel);
            addAndMakeVisible (groupSummaryButton);
            addAndMakeVisible (groupAlertBehaviourLabel);
            addAndMakeVisible (groupAlertBehaviourComboBox);

            for (int i = 0; i < 11; ++i)
            {
                progressMaxComboBox    .addItem (String (i * 10) + "%", i + 1);
                progressCurrentComboBox.addItem (String (i * 10) + "%", i + 1);
            }

            progressMaxComboBox    .setSelectedItemIndex (0);
            progressCurrentComboBox.setSelectedItemIndex (0);

            categoryComboBox.addItem ("unspecified",     1);
            categoryComboBox.addItem ("alarm",           2);
            categoryComboBox.addItem ("call",            3);
            categoryComboBox.addItem ("email",           4);
            categoryComboBox.addItem ("error",           5);
            categoryComboBox.addItem ("event",           6);
            categoryComboBox.addItem ("message",         7);
            categoryComboBox.addItem ("progress",        8);
            categoryComboBox.addItem ("promo",           9);
            categoryComboBox.addItem ("recommendation", 10);
            categoryComboBox.addItem ("reminder",       11);
            categoryComboBox.addItem ("service",        12);
            categoryComboBox.addItem ("social",         13);
            categoryComboBox.addItem ("status",         14);
            categoryComboBox.addItem ("system",         15);
            categoryComboBox.addItem ("transport",      16);
            categoryComboBox.setSelectedItemIndex (0);

            for (int i = -2; i < 3; ++i)
                priorityComboBox.addItem (String (i), i + 3);
            priorityComboBox.setSelectedItemIndex (2);

            lockScreenVisibilityComboBox.addItem ("don't show", 1);
            lockScreenVisibilityComboBox.addItem ("show partially", 2);
            lockScreenVisibilityComboBox.addItem ("show completely", 3);
            lockScreenVisibilityComboBox.setSelectedItemIndex (1);

            groupAlertBehaviourComboBox.addItem ("alert all", 1);
            groupAlertBehaviourComboBox.addItem ("alert summary", 2);
            groupAlertBehaviourComboBox.addItem ("alert children", 3);
            groupAlertBehaviourComboBox.setSelectedItemIndex (0);

            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        void resized() override
        {
            const int labelColumnWidth = getWidth() / 3;
            const int rowHeight = getHeight() / 11;

            auto layoutRow = [labelColumnWidth] (Rectangle<int> rowBounds, Component& label, Component& editor)
            {
                label .setBounds (rowBounds.removeFromLeft (labelColumnWidth));
                editor.setBounds (rowBounds);
            };

            auto bounds = getLocalBounds();

            layoutRow (bounds.removeFromTop (rowHeight), progressMaxLabel, progressMaxComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), progressCurrentLabel, progressCurrentComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), progressIndeterminateLabel, progressIndeterminateButton);
            layoutRow (bounds.removeFromTop (rowHeight), categoryLabel, categoryComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), priorityLabel, priorityComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), personLabel, personEditor);
            layoutRow (bounds.removeFromTop (rowHeight), lockScreenVisibilityLabel, lockScreenVisibilityComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), groupIdLabel, groupIdEditor);
            layoutRow (bounds.removeFromTop (rowHeight), sortKeyLabel, sortKeyEditor);
            layoutRow (bounds.removeFromTop (rowHeight), groupSummaryLabel, groupSummaryButton);
            layoutRow (bounds.removeFromTop (rowHeight), groupAlertBehaviourLabel, groupAlertBehaviourComboBox);
        }

        Label        progressMaxLabel { "progressMaxLabel", "ProgressMax" };
        ComboBox     progressMaxComboBox;
        Label        progressCurrentLabel { "progressCurrentLabel", "ProgressCurrent" };
        ComboBox     progressCurrentComboBox;
        Label        progressIndeterminateLabel { "progressIndeterminateLabel", "ProgressIndeterminate" };
        ToggleButton progressIndeterminateButton;
        Label        categoryLabel { "categoryLabel", "Category" };
        ComboBox     categoryComboBox;
        Label        priorityLabel { "priorityLabel", "Priority" };
        ComboBox     priorityComboBox;
        Label        personLabel { "personLabel", "Person" };
        TextEditor   personEditor;
        Label        lockScreenVisibilityLabel { "lockScreenVisibilityLabel", "LockScreenVisibility" };
        ComboBox     lockScreenVisibilityComboBox;
        Label        groupIdLabel { "groupIdLabel", "GroupID" };
        TextEditor   groupIdEditor;
        Label        sortKeyLabel { "sortKeyLabel", "SortKey" };
        TextEditor   sortKeyEditor;
        Label        groupSummaryLabel { "groupSummaryLabel", "GroupSummary" };
        ToggleButton groupSummaryButton;
        Label        groupAlertBehaviourLabel { "groupAlertBehaviourLabel", "GroupAlertBehaviour" };
        ComboBox     groupAlertBehaviourComboBox;
    };

    struct OptionalParamsThreeView   : public Component
    {
        OptionalParamsThreeView()
        {
            addAndMakeVisible (accentColourLabel);
            addAndMakeVisible (accentColourButton);
            addAndMakeVisible (ledColourLabel);
            addAndMakeVisible (ledColourButton);
            addAndMakeVisible (ledMsToBeOnLabel);
            addAndMakeVisible (ledMsToBeOnComboBox);
            addAndMakeVisible (ledMsToBeOffLabel);
            addAndMakeVisible (ledMsToBeOffComboBox);
            addAndMakeVisible (vibratorMsToBeOnLabel);
            addAndMakeVisible (vibratorMsToBeOnComboBox);
            addAndMakeVisible (vibratorMsToBeOffLabel);
            addAndMakeVisible (vibratorMsToBeOffComboBox);
            addAndMakeVisible (localOnlyLabel);
            addAndMakeVisible (localOnlyButton);
            addAndMakeVisible (ongoingLabel);
            addAndMakeVisible (ongoingButton);
            addAndMakeVisible (timestampVisibilityLabel);
            addAndMakeVisible (timestampVisibilityComboBox);
            addAndMakeVisible (timeoutAfterLabel);
            addAndMakeVisible (timeoutAfterComboBox);

            timeoutAfterComboBox.addItem ("No timeout", 1);

            for (int i = 0; i < 10; ++i)
            {
                ledMsToBeOnComboBox      .addItem (String (i * 200) + "ms", i + 1);
                ledMsToBeOffComboBox     .addItem (String (i * 200) + "ms", i + 1);
                vibratorMsToBeOnComboBox .addItem (String (i * 500) + "ms", i + 1);
                vibratorMsToBeOffComboBox.addItem (String (i * 500) + "ms", i + 1);
                timeoutAfterComboBox.addItem (String (5000 + 1000 * i) + "ms", i + 2);
            }

            ledMsToBeOnComboBox      .setSelectedItemIndex (5);
            ledMsToBeOffComboBox     .setSelectedItemIndex (5);
            vibratorMsToBeOnComboBox .setSelectedItemIndex (0);
            vibratorMsToBeOffComboBox.setSelectedItemIndex (0);
            timeoutAfterComboBox.setSelectedItemIndex (0);

            timestampVisibilityComboBox.addItem ("off", 1);
            timestampVisibilityComboBox.addItem ("on", 2);
            timestampVisibilityComboBox.addItem ("chronometer", 3);
            timestampVisibilityComboBox.addItem ("count down", 4);
            timestampVisibilityComboBox.setSelectedItemIndex (1);

            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        void resized() override
        {
            const int labelColumnWidth = getWidth() / 3;
            const int rowHeight = getHeight() / 10;

            auto layoutRow = [labelColumnWidth] (Rectangle<int> rowBounds, Component& label, Component& editor)
            {
                label .setBounds (rowBounds.removeFromLeft (labelColumnWidth));
                editor.setBounds (rowBounds);
            };

            auto bounds = getLocalBounds();

            layoutRow (bounds.removeFromTop (rowHeight), accentColourLabel, accentColourButton);
            layoutRow (bounds.removeFromTop (rowHeight), ledColourLabel, ledColourButton);
            layoutRow (bounds.removeFromTop (rowHeight), ledMsToBeOnLabel, ledMsToBeOnComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), ledMsToBeOffLabel, ledMsToBeOffComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), vibratorMsToBeOnLabel, vibratorMsToBeOnComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), vibratorMsToBeOffLabel, vibratorMsToBeOffComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), localOnlyLabel, localOnlyButton);
            layoutRow (bounds.removeFromTop (rowHeight), ongoingLabel, ongoingButton);
            layoutRow (bounds.removeFromTop (rowHeight), timestampVisibilityLabel, timestampVisibilityComboBox);
            layoutRow (bounds.removeFromTop (rowHeight), timeoutAfterLabel, timeoutAfterComboBox);
        }

        Label        accentColourLabel { "accentColourLabel", "AccentColour" };
        TextButton   accentColourButton;
        Label        ledColourLabel { "ledColourLabel", "LedColour" };
        TextButton   ledColourButton;
        Label        ledMsToBeOnLabel { "ledMsToBeOnLabel", "LedMsToBeOn" };
        ComboBox     ledMsToBeOnComboBox;
        Label        ledMsToBeOffLabel { "ledMsToBeOffLabel", "LedMsToBeOff" };
        ComboBox     ledMsToBeOffComboBox;
        Label        vibratorMsToBeOnLabel { "vibratorMsToBeOnLabel", "VibrationMsToBeOn" };
        ComboBox     vibratorMsToBeOnComboBox;
        Label        vibratorMsToBeOffLabel { "vibratorMsToBeOffLabel", "VibrationMsToBeOff" };
        ComboBox     vibratorMsToBeOffComboBox;
        Label        localOnlyLabel { "localOnlyLabel", "LocalOnly" };
        ToggleButton localOnlyButton;
        Label        ongoingLabel { "ongoingLabel", "Ongoing" };
        ToggleButton ongoingButton;
        Label        timestampVisibilityLabel { "timestampVisibilityLabel", "TimestampMode" };
        ComboBox     timestampVisibilityComboBox;
        Label        timeoutAfterLabel { "timeoutAfterLabel", "Timeout After Ms" };
        ComboBox     timeoutAfterComboBox;

        ColourSelector* accentColourSelector = nullptr;
        ColourSelector* ledColourSelector = nullptr;
    };

    struct AuxActionsView   : public Component
    {
        AuxActionsView()
        {
            addAndMakeVisible (getDeliveredNotificationsButton);
            addAndMakeVisible (removeDeliveredNotifWithIdButton);
            addAndMakeVisible (deliveredNotifIdentifier);
            addAndMakeVisible (removeAllDeliveredNotifsButton);
          #if JUCE_IOS
            addAndMakeVisible (getPendingNotificationsButton);
            addAndMakeVisible (removePendingNotifWithIdButton);
            addAndMakeVisible (pendingNotifIdentifier);
            addAndMakeVisible (removeAllPendingNotifsButton);
          #endif

            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        void resized() override
        {
            const int columnWidth = getWidth();

            const int rowHeight = getHeight() / 6;

            auto bounds = getLocalBounds();

            getDeliveredNotificationsButton .setBounds (bounds.removeFromTop (rowHeight));

            auto rowBounds = bounds.removeFromTop (rowHeight);
            removeDeliveredNotifWithIdButton.setBounds (rowBounds.removeFromLeft (columnWidth / 2));
            deliveredNotifIdentifier        .setBounds (rowBounds);

            removeAllDeliveredNotifsButton  .setBounds (bounds.removeFromTop (rowHeight));

          #if JUCE_IOS
            getPendingNotificationsButton .setBounds (bounds.removeFromTop (rowHeight));

            rowBounds = bounds.removeFromTop (rowHeight);
            removePendingNotifWithIdButton.setBounds (rowBounds.removeFromLeft (columnWidth / 2));
            pendingNotifIdentifier        .setBounds (rowBounds);

            removeAllPendingNotifsButton  .setBounds (bounds.removeFromTop (rowHeight));
          #endif
        }

        TextButton getDeliveredNotificationsButton { "Get Delivered Notifications" };
        TextButton removeDeliveredNotifWithIdButton { "Remove Delivered Notif With ID:" };
        TextEditor deliveredNotifIdentifier;
        TextButton removeAllDeliveredNotifsButton { "Remove All Delivered Notifs" };
      #if JUCE_IOS
        TextButton getPendingNotificationsButton { "Get Pending Notifications" };
        TextButton removePendingNotifWithIdButton { "Remove Pending Notif With ID:" };
        TextEditor pendingNotifIdentifier;
        TextButton removeAllPendingNotifsButton { "Remove All Pending Notifs" };
      #endif
    };

    struct RemoteView   : public Component
    {
        RemoteView()
        {
            addAndMakeVisible (getDeviceTokenButton);
          #if JUCE_ANDROID
            addAndMakeVisible (sendRemoteMessageButton);
            addAndMakeVisible (subscribeToSportsButton);
            addAndMakeVisible (unsubscribeFromSportsButton);
          #endif
        }

        void resized()
        {
            const int rowSize = getHeight () / 10;

            auto bounds = getLocalBounds().reduced (getWidth() / 10, getHeight() / 10);

            bounds.removeFromTop (2 * rowSize);

            getDeviceTokenButton       .setBounds (bounds.removeFromTop (rowSize));
            sendRemoteMessageButton    .setBounds (bounds.removeFromTop (rowSize));
            subscribeToSportsButton    .setBounds (bounds.removeFromTop (rowSize));
            unsubscribeFromSportsButton.setBounds (bounds.removeFromTop (rowSize));
        }

        TextButton getDeviceTokenButton { "GetDeviceToken" };
        TextButton sendRemoteMessageButton { "SendRemoteMessage" };
        TextButton subscribeToSportsButton { "SubscribeToSports" };
        TextButton unsubscribeFromSportsButton { "UnsubscribeFromSports" };
    };

    Label                   headerLabel { "headerLabel", "Push Notifications Demo" };
    RequiredParamsView      requiredParamsView;
    OptionalParamsOneView   optionalParamsOneView;
    OptionalParamsTwoView   optionalParamsTwoView;
    OptionalParamsThreeView optionalParamsThreeView;
    AuxActionsView          auxActionsView;
    TabbedComponent         localNotificationsTabs { TabbedButtonBar::TabsAtTop };
    RemoteView              remoteView;
    TabbedComponent         mainTabs { TabbedButtonBar::TabsAtTop };
    TextButton              sendButton { "Send!" };
    Label                   notAvailableYetLabel { "notAvailableYetLabel", "Push Notifications feature is not available on this platform yet!" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
