/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             PushNotificationsDemo
 version:          2.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases push notifications features. To run this demo you must enable the
                   "Push Notifications Capability" option in the Projucer exporter.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, xcode_iphone, androidstudio

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        PushNotificationsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class PushNotificationsDemo   : public Component,
                                private ChangeListener,
                                private ComponentListener,
                                private PushNotifications::Listener
{
public:
    //==============================================================================
    PushNotificationsDemo()
    {
        setupControls();
        distributeControls();

      #if JUCE_PUSH_NOTIFICATIONS
        addAndMakeVisible (headerLabel);
        addAndMakeVisible (mainTabs);
        addAndMakeVisible (sendButton);
      #else
        addAndMakeVisible (notAvailableYetLabel);
      #endif

        headerLabel         .setJustificationType (Justification::centred);
        notAvailableYetLabel.setJustificationType (Justification::centred);

      #if JUCE_MAC
        StringArray tabNames { "Params1", "Params2", "Params3", "Params4" };
      #else
        StringArray tabNames { "Req. params", "Opt. params1", "Opt. params2", "Opt. params3" };
      #endif

        auto colour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
        localNotificationsTabs.addTab (tabNames[0], colour, &paramsOneView, false);
        localNotificationsTabs.addTab (tabNames[1], colour, &paramsTwoView, false);
      #if JUCE_ANDROID
        localNotificationsTabs.addTab (tabNames[2], colour, &paramsThreeView, false);
        localNotificationsTabs.addTab (tabNames[3], colour, &paramsFourView,  false);
      #endif
        localNotificationsTabs.addTab ("Aux. actions", colour, &auxActionsView, false);

        mainTabs.addTab ("Local",  colour, &localNotificationsTabs, false);
        mainTabs.addTab ("Remote", colour, &remoteView,             false);

        auto userArea = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
      #if JUCE_ANDROID || JUCE_IOS
        setSize (userArea.getWidth(), userArea.getHeight());
      #else
        setSize (userArea.getWidth() / 2, userArea.getHeight() / 2);
      #endif

        sendButton.onClick = [this] { sendLocalNotification(); };
        auxActionsView.getDeliveredNotificationsButton .onClick = [this]
            { PushNotifications::getInstance()->getDeliveredNotifications(); };
        auxActionsView.removeDeliveredNotifWithIdButton.onClick = [this]
            { PushNotifications::getInstance()->removeDeliveredNotification (auxActionsView.deliveredNotifIdentifier.getText()); };
        auxActionsView.removeAllDeliveredNotifsButton  .onClick = [this]
            { PushNotifications::getInstance()->removeAllDeliveredNotifications(); };
      #if JUCE_IOS || JUCE_MAC
        auxActionsView.getPendingNotificationsButton .onClick = [this]
            { PushNotifications::getInstance()->getPendingLocalNotifications(); };
        auxActionsView.removePendingNotifWithIdButton.onClick = [this]
            { PushNotifications::getInstance()->removePendingLocalNotification (auxActionsView.pendingNotifIdentifier.getText()); };
        auxActionsView.removeAllPendingNotifsButton  .onClick = [this]
            { PushNotifications::getInstance()->removeAllPendingLocalNotifications(); };
      #endif

        remoteView.getDeviceTokenButton.onClick = [this]
        {
            String token = PushNotifications::getInstance()->getDeviceToken();

            DBG ("token = " + token);

            if (token.isEmpty())
                showRemoteInstructions();
            else
                NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon, "Device token", token);
        };

      #if JUCE_ANDROID
        remoteView.sendRemoteMessageButton.onClick = [this]
        {
            StringPairArray data;
            data.set ("key1", "value1");
            data.set ("key2", "value2");

            static int id = 100;
            PushNotifications::getInstance()->sendUpstreamMessage ("872047750958",
                                                                   "com.juce.pushnotificationsdemo",
                                                                   String (id++),
                                                                   "standardType",
                                                                   3600,
                                                                   data);
        };

        remoteView.subscribeToSportsButton    .onClick = [this]
            { PushNotifications::getInstance()->subscribeToTopic ("sports"); };
        remoteView.unsubscribeFromSportsButton.onClick = [this]
            { PushNotifications::getInstance()->unsubscribeFromTopic ("sports"); };
      #endif

        paramControls.accentColourButton.onClick = [this] { setupAccentColour(); };
        paramControls.ledColourButton   .onClick = [this] { setupLedColour(); };

        jassert (PushNotifications::getInstance()->areNotificationsEnabled());

        PushNotifications::getInstance()->addListener (this);

      #if JUCE_IOS || JUCE_MAC
        paramControls.fireInComboBox.onChange = [this] { delayNotification(); };
        PushNotifications::getInstance()->requestPermissionsWithSettings (getNotificationSettings());
      #elif JUCE_ANDROID
        PushNotifications::ChannelGroup cg { "demoGroup", "demo group" };
        PushNotifications::getInstance()->setupChannels ({ { cg } }, getAndroidChannels());
      #endif
    }

    ~PushNotificationsDemo()
    {
        PushNotifications::getInstance()->removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (getWidth() / 20, getHeight() / 40);

        headerLabel.setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.1f)));
        mainTabs   .setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.8f)));
        sendButton .setBounds (bounds);

        notAvailableYetLabel.setBounds (getLocalBounds());
    }

private:
    void delayNotification()
    {
        auto repeatsAllowed = paramControls.fireInComboBox.getSelectedItemIndex() >= 6;

        paramControls.repeatButton.setEnabled (repeatsAllowed);

        if (! repeatsAllowed)
            paramControls.repeatButton.setToggleState (false, NotificationType::sendNotification);
    }

    void sendLocalNotification()
    {
        PushNotifications::Notification n;

        fillRequiredParams (n);
        fillOptionalParamsOne (n);
      #if JUCE_ANDROID
        fillOptionalParamsTwo (n);
        fillOptionalParamsThree (n);
      #endif

        if (! n.isValid())
        {
          #if JUCE_IOS
            String requiredFields = "identifier (from iOS 10), title, body and category";
          #elif JUCE_ANDROID
            String requiredFields = "channel ID (from Android O), title, body and icon";
          #else
            String requiredFields = "all required fields";
          #endif

            NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                   "Incorrect notifications setup",
                                                   "Please make sure that "
                                                   + requiredFields + " are set.");


            return;
        }

        PushNotifications::getInstance()->sendLocalNotification (n);
    }

    void fillRequiredParams      (PushNotifications::Notification& n)
    {
        n.identifier = paramControls.identifierEditor.getText();
        n.title      = paramControls.titleEditor     .getText();
        n.body       = paramControls.bodyEditor      .getText();
      #if JUCE_IOS
        n.category = paramControls.categoryComboBox.getText();
      #elif JUCE_ANDROID || JUCE_MAC
       #if JUCE_MAC
        String prefix = "Notifications/images/";
        String extension = ".png";
       #else
        String prefix;
        String extension;
       #endif
        if (paramControls.iconComboBox.getSelectedItemIndex() == 0)
            n.icon = prefix + "ic_stat_name" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 1)
            n.icon = prefix + "ic_stat_name2" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 2)
            n.icon = prefix + "ic_stat_name3" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 3)
            n.icon = prefix + "ic_stat_name4" + extension;
        else if (paramControls.iconComboBox.getSelectedItemIndex() == 4)
            n.icon = prefix + "ic_stat_name5" + extension;
      #endif

      #if JUCE_ANDROID
        // Note: this is not strictly speaking required param, just doing it here because it is the fastest way!
        n.publicVersion = new PushNotifications::Notification();
        n.publicVersion->identifier = "blahblahblah";
        n.publicVersion->title      = "Public title!";
        n.publicVersion->body       = "Public body!";
        n.publicVersion->icon       = n.icon;

        n.channelId = String (paramControls.channelIdComboBox.getSelectedItemIndex() + 1);
      #endif
    }

    void fillOptionalParamsOne   (PushNotifications::Notification& n)
    {
        n.subtitle = paramControls.subtitleEditor.getText();
        n.badgeNumber = paramControls.badgeNumberComboBox.getSelectedItemIndex();

        if (paramControls.soundToPlayComboBox.getSelectedItemIndex() > 0)
            n.soundToPlay = URL (paramControls.soundToPlayComboBox.getItemText (paramControls.soundToPlayComboBox.getSelectedItemIndex()));

        n.properties = JSON::parse (paramControls.propertiesEditor.getText());

      #if JUCE_IOS || JUCE_MAC
        n.triggerIntervalSec = double (paramControls.fireInComboBox.getSelectedItemIndex() * 10);
        n.repeat = paramControls.repeatButton.getToggleState();
      #elif JUCE_ANDROID
        if (paramControls.largeIconComboBox.getSelectedItemIndex() == 1)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name6.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 2)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name7.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 3)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name8.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 4)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name9.png");
        else if (paramControls.largeIconComboBox.getSelectedItemIndex() == 5)
            n.largeIcon = getImageFromAssets ("Notifications/images/ic_stat_name10.png");

        n.badgeIconType = (PushNotifications::Notification::BadgeIconType) paramControls.badgeIconComboBox.getSelectedItemIndex();
        n.tickerText  = paramControls.tickerTextEditor.getText();

        n.shouldAutoCancel = paramControls.autoCancelButton   .getToggleState();
        n.alertOnlyOnce    = paramControls.alertOnlyOnceButton.getToggleState();
      #endif

      #if JUCE_ANDROID || JUCE_MAC
        if (paramControls.actionsComboBox.getSelectedItemIndex() == 1)
        {
            PushNotifications::Notification::Action a, a2;
            a .style = PushNotifications::Notification::Action::button;
            a2.style = PushNotifications::Notification::Action::button;
            a .title = a .identifier = "Ok";
            a2.title = a2.identifier = "Cancel";
            n.actions.add (a);
            n.actions.add (a2);
        }
        else if (paramControls.actionsComboBox.getSelectedItemIndex() == 2)
        {
            PushNotifications::Notification::Action a, a2;
            a .title = a .identifier = "Input Text Here";
            a2.title = a2.identifier = "No";
            a .style = PushNotifications::Notification::Action::text;
            a2.style = PushNotifications::Notification::Action::button;
            a .icon = "ic_stat_name4";
            a2.icon = "ic_stat_name5";
            a.textInputPlaceholder = "placeholder text ...";
            n.actions.add (a);
            n.actions.add (a2);
        }
        else if (paramControls.actionsComboBox.getSelectedItemIndex() == 3)
        {
            PushNotifications::Notification::Action a, a2;
            a .title = a .identifier = "Ok";
            a2.title = a2.identifier = "Cancel";
            a .style = PushNotifications::Notification::Action::button;
            a2.style = PushNotifications::Notification::Action::button;
            a .icon = "ic_stat_name4";
            a2.icon = "ic_stat_name5";
            n.actions.add (a);
            n.actions.add (a2);
        }
        else if (paramControls.actionsComboBox.getSelectedItemIndex() == 4)
        {
            PushNotifications::Notification::Action a, a2;
            a .title = a .identifier = "Input Text Here";
            a2.title = a2.identifier = "No";
            a .style = PushNotifications::Notification::Action::text;
            a2.style = PushNotifications::Notification::Action::button;
            a .icon = "ic_stat_name4";
            a2.icon = "ic_stat_name5";
            a.textInputPlaceholder = "placeholder text ...";
            a.allowedResponses.add ("Response 1");
            a.allowedResponses.add ("Response 2");
            a.allowedResponses.add ("Response 3");
            n.actions.add (a);
            n.actions.add (a2);
        }
      #endif
    }

    void fillOptionalParamsTwo   (PushNotifications::Notification& n)
    {
        using Notification = PushNotifications::Notification;

        Notification::Progress progress;
        progress.max           = paramControls.progressMaxComboBox    .getSelectedItemIndex() * 10;
        progress.current       = paramControls.progressCurrentComboBox.getSelectedItemIndex() * 10;
        progress.indeterminate = paramControls.progressIndeterminateButton.getToggleState();

        n.progress = progress;
        n.person   = paramControls.personEditor.getText();
        n.type                 = Notification::Type                 (paramControls.categoryComboBox            .getSelectedItemIndex());
        n.priority             = Notification::Priority             (paramControls.priorityComboBox            .getSelectedItemIndex() - 2);
        n.lockScreenAppearance = Notification::LockScreenAppearance (paramControls.lockScreenVisibilityComboBox.getSelectedItemIndex() - 1);
        n.groupId      = paramControls.groupIdEditor.getText();
        n.groupSortKey = paramControls.sortKeyEditor.getText();
        n.groupSummary = paramControls.groupSummaryButton.getToggleState();
        n.groupAlertBehaviour = Notification::GroupAlertBehaviour (paramControls.groupAlertBehaviourComboBox.getSelectedItemIndex());
    }

    void fillOptionalParamsThree (PushNotifications::Notification& n)
    {
        n.accentColour = paramControls.accentColourButton.findColour (TextButton::buttonColourId, false);
        n.ledColour    = paramControls.ledColourButton   .findColour (TextButton::buttonColourId, false);

        using Notification = PushNotifications::Notification;
        Notification::LedBlinkPattern ledBlinkPattern;
        ledBlinkPattern.msToBeOn  = paramControls.ledMsToBeOnComboBox .getSelectedItemIndex() * 200;
        ledBlinkPattern.msToBeOff = paramControls.ledMsToBeOffComboBox.getSelectedItemIndex() * 200;
        n.ledBlinkPattern = ledBlinkPattern;

        Array<int> vibrationPattern;

        if (paramControls.vibratorMsToBeOnComboBox .getSelectedItemIndex() > 0 &&
            paramControls.vibratorMsToBeOffComboBox.getSelectedItemIndex() > 0)
        {
            vibrationPattern.add     (paramControls.vibratorMsToBeOffComboBox.getSelectedItemIndex() * 500);
            vibrationPattern.add     (paramControls.vibratorMsToBeOnComboBox .getSelectedItemIndex() * 500);
            vibrationPattern.add (2 * paramControls.vibratorMsToBeOffComboBox.getSelectedItemIndex() * 500);
            vibrationPattern.add (2 * paramControls.vibratorMsToBeOnComboBox .getSelectedItemIndex() * 500);
        }

        n.vibrationPattern = vibrationPattern;

        n.localOnly = paramControls.localOnlyButton.getToggleState();
        n.ongoing = paramControls.ongoingButton.getToggleState();
        n.timestampVisibility = Notification::TimestampVisibility (paramControls.timestampVisibilityComboBox.getSelectedItemIndex());

        if (paramControls.timeoutAfterComboBox.getSelectedItemIndex() > 0)
        {
            auto index = paramControls.timeoutAfterComboBox.getSelectedItemIndex();
            n.timeoutAfterMs = index * 1000 + 4000;
        }
    }

    void setupAccentColour()
    {
        paramControls.accentColourSelector = new ColourSelector();
        paramControls.accentColourSelector->setName ("accent colour");
        paramControls.accentColourSelector->setCurrentColour (paramControls.accentColourButton.findColour (TextButton::buttonColourId));
        paramControls.accentColourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
        paramControls.accentColourSelector->setSize (200, 200);
        paramControls.accentColourSelector->addComponentListener (this);
        paramControls.accentColourSelector->addChangeListener (this);

        CallOutBox::launchAsynchronously (paramControls.accentColourSelector, paramControls.accentColourButton.getScreenBounds(), nullptr);
    }

    void setupLedColour()
    {
        paramControls.ledColourSelector = new ColourSelector();
        paramControls.ledColourSelector->setName ("led colour");
        paramControls.ledColourSelector->setCurrentColour (paramControls.ledColourButton.findColour (TextButton::buttonColourId));
        paramControls.ledColourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
        paramControls.ledColourSelector->setSize (200, 200);
        paramControls.ledColourSelector->addComponentListener (this);
        paramControls.ledColourSelector->addChangeListener (this);

        CallOutBox::launchAsynchronously (paramControls.ledColourSelector, paramControls.accentColourButton.getScreenBounds(), nullptr);
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == paramControls.accentColourSelector)
        {
            auto c = paramControls.accentColourSelector->getCurrentColour();
            paramControls.accentColourButton.setColour (TextButton::buttonColourId, c);
        }
        else if (source == paramControls.ledColourSelector)
        {
            auto c = paramControls.ledColourSelector->getCurrentColour();
            paramControls.ledColourButton.setColour (TextButton::buttonColourId, c);
        }
    }

    void componentBeingDeleted (Component& component) override
    {
        if (&component == paramControls.accentColourSelector)
            paramControls.accentColourSelector = nullptr;
        else if (&component == paramControls.ledColourSelector)
            paramControls.ledColourSelector = nullptr;
    }

    void handleNotification (bool isLocalNotification, const PushNotifications::Notification& n) override
    {
        ignoreUnused (isLocalNotification);

        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Received notification",
                                               "ID: " + n.identifier
                                               + ", title: " + n.title
                                               + ", body: " + n.body);
    }

    void handleNotificationAction (bool isLocalNotification,
                                   const PushNotifications::Notification& n,
                                   const juce::String& actionIdentifier,
                                   const juce::String& optionalResponse) override
    {
        ignoreUnused (isLocalNotification);

        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Received notification action",
                                               "ID: " + n.identifier
                                               + ", title: " + n.title
                                               + ", body: " + n.body
                                               + ", action: " + actionIdentifier
                                               + ", optionalResponse: " + optionalResponse);

        PushNotifications::getInstance()->removeDeliveredNotification (n.identifier);
    }

    void localNotificationDismissedByUser (const PushNotifications::Notification& n) override
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Notification dismissed by a user",
                                               "ID: " + n.identifier
                                               + ", title: " + n.title
                                               + ", body: " + n.body);
    }

    void deliveredNotificationsListReceived (const Array<PushNotifications::Notification>& notifs) override
    {
        String text = "Received notifications: ";

        for (auto& n : notifs)
            text << "(" << n.identifier << ", " << n.title << ", " << n.body << "), ";

        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon, "Received notification list", text);
    }

    void pendingLocalNotificationsListReceived (const Array<PushNotifications::Notification>& notifs) override
    {
        String text = "Pending notifications: ";

        for (auto& n : notifs)
            text << "(" << n.identifier << ", " << n.title << ", " << n.body << "), ";

        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon, "Pending notification list", text);
    }

    void deviceTokenRefreshed (const String& token) override
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Device token refreshed",
                                               token);
    }

  #if JUCE_ANDROID
    void remoteNotificationsDeleted() override
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Remote notifications deleted",
                                               "Some of the pending messages were removed!");
    }

    void upstreamMessageSent (const String& messageId) override
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Upstream message sent",
                                               "Message id: " + messageId);
    }

    void upstreamMessageSendingError (const String& messageId, const String& error) override
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Upstream message sending error",
                                               "Message id: " + messageId
                                               + "\nerror: " + error);
    }

    static Array<PushNotifications::Channel> getAndroidChannels()
    {
        using Channel = PushNotifications::Channel;

        Channel ch1, ch2, ch3;

        ch1.identifier = "1";
        ch1.name = "HighImportance";
        ch1.importance = PushNotifications::Channel::max;
        ch1.lockScreenAppearance = PushNotifications::Notification::showCompletely;
        ch1.description = "High Priority Channel for important stuff";
        ch1.groupId = "demoGroup";
        ch1.ledColour = Colours::red;
        ch1.bypassDoNotDisturb = true;
        ch1.canShowBadge = true;
        ch1.enableLights = true;
        ch1.enableVibration = true;
        ch1.soundToPlay = URL ("demonstrative");
        ch1.vibrationPattern = { 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 };

        ch2.identifier = "2";
        ch2.name = "MediumImportance";
        ch2.importance = PushNotifications::Channel::normal;
        ch2.lockScreenAppearance = PushNotifications::Notification::showPartially;
        ch2.description = "Medium Priority Channel for standard stuff";
        ch2.groupId = "demoGroup";
        ch2.ledColour = Colours::yellow;
        ch2.canShowBadge = true;
        ch2.enableLights = true;
        ch2.enableVibration = true;
        ch2.soundToPlay = URL ("default_os_sound");
        ch2.vibrationPattern = { 1000, 1000 };

        ch3.identifier = "3";
        ch3.name = "LowImportance";
        ch3.importance = PushNotifications::Channel::min;
        ch3.lockScreenAppearance = PushNotifications::Notification::dontShow;
        ch3.description = "Low Priority Channel for silly stuff";
        ch3.groupId = "demoGroup";

        return { ch1, ch2, ch3 };
    }

  #elif JUCE_IOS || JUCE_MAC
    static PushNotifications::Settings getNotificationSettings()
    {
        PushNotifications::Settings settings;
        settings.allowAlert = true;
        settings.allowBadge = true;
        settings.allowSound = true;

      #if JUCE_IOS
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

        settings.categories = { okCategory, okCancelCategory, textCategory };
      #endif

        return settings;
    }
  #endif

    struct RowComponent : public Component
    {
        RowComponent (Label& l, Component& c, int u = 1)
            : label (l),
              editor (c),
              rowUnits (u)
        {
            addAndMakeVisible (label);
            addAndMakeVisible (editor);
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            label .setBounds (bounds.removeFromLeft (getWidth() / 3));
            editor.setBounds (bounds);
        }

        Label& label;
        Component& editor;
        int rowUnits;
    };

    struct ParamControls
    {
        Label        identifierLabel            { "identifierLabel",            "Identifier" };
        TextEditor   identifierEditor;
        Label        titleLabel                 { "titleLabel",                 "Title" };
        TextEditor   titleEditor;
        Label        bodyLabel                  { "bodyLabel",                  "Body" };
        TextEditor   bodyEditor;

        Label        categoryLabel              { "categoryLabel",              "Category" };
        ComboBox     categoryComboBox;
        Label        channelIdLabel             { "channelIdLabel",             "Channel ID" };
        ComboBox     channelIdComboBox;
        Label        iconLabel                  { "iconLabel",                  "Icon" };
        ComboBox     iconComboBox;

        Label        subtitleLabel              { "subtitleLabel",              "Subtitle" };
        TextEditor   subtitleEditor;
        Label        badgeNumberLabel           { "badgeNumberLabel",           "BadgeNumber" };
        ComboBox     badgeNumberComboBox;
        Label        soundToPlayLabel           { "soundToPlayLabel",           "SoundToPlay" };
        ComboBox     soundToPlayComboBox;
        Label        propertiesLabel            { "propertiesLabel",            "Properties" };
        TextEditor   propertiesEditor;
        Label        fireInLabel                { "fireInLabel",                "Fire in" };
        ComboBox     fireInComboBox;
        Label        repeatLabel                { "repeatLabel",                "Repeat" };
        ToggleButton repeatButton;
        Label        largeIconLabel             { "largeIconLabel",             "Large Icon" };
        ComboBox     largeIconComboBox;
        Label        badgeIconLabel             { "badgeIconLabel",             "Badge Icon" };
        ComboBox     badgeIconComboBox;
        Label        tickerTextLabel            { "tickerTextLabel",            "Ticker Text" };
        TextEditor   tickerTextEditor;
        Label        autoCancelLabel            { "autoCancelLabel",            "AutoCancel" };
        ToggleButton autoCancelButton;
        Label        alertOnlyOnceLabel         { "alertOnlyOnceLabel",         "AlertOnlyOnce" };
        ToggleButton alertOnlyOnceButton;
        Label        actionsLabel               { "actionsLabel",               "Actions" };
        ComboBox     actionsComboBox;

        Label        progressMaxLabel           { "progressMaxLabel",           "ProgressMax" };
        ComboBox     progressMaxComboBox;
        Label        progressCurrentLabel       { "progressCurrentLabel",       "ProgressCurrent" };
        ComboBox     progressCurrentComboBox;
        Label        progressIndeterminateLabel { "progressIndeterminateLabel", "ProgressIndeterminate" };
        ToggleButton progressIndeterminateButton;
        Label        notifCategoryLabel         { "notifCategoryLabel",         "Category" };
        ComboBox     notifCategoryComboBox;
        Label        priorityLabel              { "priorityLabel",              "Priority" };
        ComboBox     priorityComboBox;
        Label        personLabel                { "personLabel",                "Person" };
        TextEditor   personEditor;
        Label        lockScreenVisibilityLabel  { "lockScreenVisibilityLabel",  "LockScreenVisibility" };
        ComboBox     lockScreenVisibilityComboBox;
        Label        groupIdLabel               { "groupIdLabel",               "GroupID" };
        TextEditor   groupIdEditor;
        Label        sortKeyLabel               { "sortKeyLabel",               "SortKey" };
        TextEditor   sortKeyEditor;
        Label        groupSummaryLabel          { "groupSummaryLabel",          "GroupSummary" };
        ToggleButton groupSummaryButton;
        Label        groupAlertBehaviourLabel   { "groupAlertBehaviourLabel",   "GroupAlertBehaviour" };
        ComboBox     groupAlertBehaviourComboBox;

        Label        accentColourLabel          { "accentColourLabel",          "AccentColour" };
        TextButton   accentColourButton;
        Label        ledColourLabel             { "ledColourLabel",             "LedColour" };
        TextButton   ledColourButton;
        Label        ledMsToBeOnLabel           { "ledMsToBeOnLabel",           "LedMsToBeOn" };
        ComboBox     ledMsToBeOnComboBox;
        Label        ledMsToBeOffLabel          { "ledMsToBeOffLabel",          "LedMsToBeOff" };
        ComboBox     ledMsToBeOffComboBox;
        Label        vibratorMsToBeOnLabel      { "vibratorMsToBeOnLabel",      "VibrationMsToBeOn" };
        ComboBox     vibratorMsToBeOnComboBox;
        Label        vibratorMsToBeOffLabel     { "vibratorMsToBeOffLabel",     "VibrationMsToBeOff" };
        ComboBox     vibratorMsToBeOffComboBox;
        Label        localOnlyLabel             { "localOnlyLabel",             "LocalOnly" };
        ToggleButton localOnlyButton;
        Label        ongoingLabel               { "ongoingLabel",               "Ongoing" };
        ToggleButton ongoingButton;
        Label        timestampVisibilityLabel   { "timestampVisibilityLabel",   "TimestampMode" };
        ComboBox     timestampVisibilityComboBox;
        Label        timeoutAfterLabel          { "timeoutAfterLabel",          "Timeout After Ms" };
        ComboBox     timeoutAfterComboBox;

        ColourSelector* accentColourSelector = nullptr;
        ColourSelector* ledColourSelector    = nullptr;
    };

    void setupControls()
    {
        auto& pc = paramControls;

        StringArray categories { "okCategory", "okCancelCategory", "textCategory" };

        for (auto& c : categories)
            pc.categoryComboBox.addItem (c, pc.categoryComboBox.getNumItems() + 1);
        pc.categoryComboBox.setSelectedItemIndex (0);

        for (auto i = 1; i <= 3; ++i)
            pc.channelIdComboBox.addItem (String (i), i);
        pc.channelIdComboBox.setSelectedItemIndex (0);

        for (auto i = 0; i < 5; ++i)
            pc.iconComboBox.addItem ("icon" + String (i + 1), i + 1);
        pc.iconComboBox.setSelectedItemIndex (0);

      #if JUCE_MAC
        pc.iconComboBox.addItem ("none", 100);
      #endif

        pc.fireInComboBox.addItem ("Now", 1);

        for (auto i = 1; i < 11; ++i)
            pc.fireInComboBox.addItem (String (10 * i) + "seconds", i + 1);
        pc.fireInComboBox.setSelectedItemIndex (0);

        pc.largeIconComboBox.addItem ("none", 1);

        for (auto i = 1; i < 5; ++i)
            pc.largeIconComboBox.addItem ("icon" + String (i), i + 1);
        pc.largeIconComboBox.setSelectedItemIndex (0);

        pc.badgeIconComboBox.addItem ("none",  1);
        pc.badgeIconComboBox.addItem ("small", 2);
        pc.badgeIconComboBox.addItem ("large", 3);
        pc.badgeIconComboBox.setSelectedItemIndex (2);

        pc.actionsComboBox.addItem ("none",       1);
        pc.actionsComboBox.addItem ("ok-cancel",  2);
        pc.actionsComboBox.addItem ("text-input", 3);
      #if JUCE_ANDROID
        pc.actionsComboBox.addItem ("ok-cancel-icons", 4);
        pc.actionsComboBox.addItem ("text-input-limited_responses", 5);
      #endif
        pc.actionsComboBox.setSelectedItemIndex (0);

        for (auto i = 0; i < 7; ++i)
            pc.badgeNumberComboBox.addItem (String (i), i + 1);
        pc.badgeNumberComboBox.setSelectedItemIndex (0);

      #if JUCE_IOS
        String prefix = "Notifications/sounds/";
        String extension = ".caf";
      #else
        String prefix;
        String extension;
      #endif

        pc.soundToPlayComboBox.addItem ("none", 1);
        pc.soundToPlayComboBox.addItem ("default_os_sound", 2);
        pc.soundToPlayComboBox.addItem (prefix + "demonstrative"  + extension, 3);
        pc.soundToPlayComboBox.addItem (prefix + "isntit"         + extension, 4);
        pc.soundToPlayComboBox.addItem (prefix + "jinglebellssms" + extension, 5);
        pc.soundToPlayComboBox.addItem (prefix + "served"         + extension, 6);
        pc.soundToPlayComboBox.addItem (prefix + "solemn"         + extension, 7);
        pc.soundToPlayComboBox.setSelectedItemIndex (1);

        for (auto i = 0; i < 11; ++i)
        {
            pc.progressMaxComboBox    .addItem (String (i * 10) + "%", i + 1);
            pc.progressCurrentComboBox.addItem (String (i * 10) + "%", i + 1);
        }

        pc.progressMaxComboBox    .setSelectedItemIndex (0);
        pc.progressCurrentComboBox.setSelectedItemIndex (0);

        pc.notifCategoryComboBox.addItem ("unspecified",     1);
        pc.notifCategoryComboBox.addItem ("alarm",           2);
        pc.notifCategoryComboBox.addItem ("call",            3);
        pc.notifCategoryComboBox.addItem ("email",           4);
        pc.notifCategoryComboBox.addItem ("error",           5);
        pc.notifCategoryComboBox.addItem ("event",           6);
        pc.notifCategoryComboBox.addItem ("message",         7);
        pc.notifCategoryComboBox.addItem ("progress",        8);
        pc.notifCategoryComboBox.addItem ("promo",           9);
        pc.notifCategoryComboBox.addItem ("recommendation", 10);
        pc.notifCategoryComboBox.addItem ("reminder",       11);
        pc.notifCategoryComboBox.addItem ("service",        12);
        pc.notifCategoryComboBox.addItem ("social",         13);
        pc.notifCategoryComboBox.addItem ("status",         14);
        pc.notifCategoryComboBox.addItem ("system",         15);
        pc.notifCategoryComboBox.addItem ("transport",      16);
        pc.notifCategoryComboBox.setSelectedItemIndex (0);

        for (auto i = -2; i < 3; ++i)
            pc.priorityComboBox.addItem (String (i), i + 3);
        pc.priorityComboBox.setSelectedItemIndex (2);

        pc.lockScreenVisibilityComboBox.addItem ("don't show",      1);
        pc.lockScreenVisibilityComboBox.addItem ("show partially",  2);
        pc.lockScreenVisibilityComboBox.addItem ("show completely", 3);
        pc.lockScreenVisibilityComboBox.setSelectedItemIndex (1);

        pc.groupAlertBehaviourComboBox.addItem ("alert all",      1);
        pc.groupAlertBehaviourComboBox.addItem ("alert summary",  2);
        pc.groupAlertBehaviourComboBox.addItem ("alert children", 3);
        pc.groupAlertBehaviourComboBox.setSelectedItemIndex (0);

        pc.timeoutAfterComboBox.addItem ("No timeout", 1);

        for (auto i = 0; i < 10; ++i)
        {
            pc.ledMsToBeOnComboBox      .addItem (String (i * 200) + "ms", i + 1);
            pc.ledMsToBeOffComboBox     .addItem (String (i * 200) + "ms", i + 1);
            pc.vibratorMsToBeOnComboBox .addItem (String (i * 500) + "ms", i + 1);
            pc.vibratorMsToBeOffComboBox.addItem (String (i * 500) + "ms", i + 1);
            pc.timeoutAfterComboBox     .addItem (String (5000 + 1000 * i) + "ms", i + 2);
        }

        pc.ledMsToBeOnComboBox      .setSelectedItemIndex (5);
        pc.ledMsToBeOffComboBox     .setSelectedItemIndex (5);
        pc.vibratorMsToBeOnComboBox .setSelectedItemIndex (0);
        pc.vibratorMsToBeOffComboBox.setSelectedItemIndex (0);
        pc.timeoutAfterComboBox     .setSelectedItemIndex (0);

        pc.timestampVisibilityComboBox.addItem ("off",         1);
        pc.timestampVisibilityComboBox.addItem ("on",          2);
        pc.timestampVisibilityComboBox.addItem ("chronometer", 3);
        pc.timestampVisibilityComboBox.addItem ("count down",  4);
        pc.timestampVisibilityComboBox.setSelectedItemIndex (1);
    }

    void distributeControls()
    {
        auto& pc = paramControls;

        paramsOneView  .addRowComponent (new RowComponent (pc.identifierLabel,            pc.identifierEditor));
        paramsOneView  .addRowComponent (new RowComponent (pc.titleLabel,                 pc.titleEditor));
        paramsOneView  .addRowComponent (new RowComponent (pc.bodyLabel,                  pc.bodyEditor, 4));
      #if JUCE_IOS
        paramsOneView  .addRowComponent (new RowComponent (pc.categoryLabel,              pc.categoryComboBox));
      #elif JUCE_ANDROID
        paramsOneView  .addRowComponent (new RowComponent (pc.channelIdLabel,             pc.channelIdComboBox));
      #endif
      #if JUCE_ANDROID || JUCE_MAC
        paramsOneView  .addRowComponent (new RowComponent (pc.iconLabel,                  pc.iconComboBox));
      #endif

        paramsTwoView  .addRowComponent (new RowComponent (pc.subtitleLabel,              pc.subtitleEditor));
      #if ! JUCE_MAC
        paramsTwoView  .addRowComponent (new RowComponent (pc.badgeNumberLabel,           pc.badgeNumberComboBox));
      #endif
        paramsTwoView  .addRowComponent (new RowComponent (pc.soundToPlayLabel,           pc.soundToPlayComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.propertiesLabel,            pc.propertiesEditor, 3));
      #if JUCE_IOS || JUCE_MAC
        paramsTwoView  .addRowComponent (new RowComponent (pc.fireInLabel,                pc.fireInComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.repeatLabel,                pc.repeatButton));
      #elif JUCE_ANDROID
        paramsTwoView  .addRowComponent (new RowComponent (pc.largeIconLabel,             pc.largeIconComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.badgeIconLabel,             pc.badgeIconComboBox));
        paramsTwoView  .addRowComponent (new RowComponent (pc.tickerTextLabel,            pc.tickerTextEditor));
        paramsTwoView  .addRowComponent (new RowComponent (pc.autoCancelLabel,            pc.autoCancelButton));
        paramsTwoView  .addRowComponent (new RowComponent (pc.alertOnlyOnceLabel,         pc.alertOnlyOnceButton));
      #endif
      #if JUCE_ANDROID || JUCE_MAC
        paramsTwoView  .addRowComponent (new RowComponent (pc.actionsLabel,               pc.actionsComboBox));
      #endif
      #if JUCE_ANDROID
        paramsThreeView.addRowComponent (new RowComponent (pc.progressMaxLabel,           pc.progressMaxComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.progressCurrentLabel,       pc.progressCurrentComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.progressIndeterminateLabel, pc.progressIndeterminateButton));
        paramsThreeView.addRowComponent (new RowComponent (pc.categoryLabel,              pc.categoryComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.priorityLabel,              pc.priorityComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.personLabel,                pc.personEditor));
        paramsThreeView.addRowComponent (new RowComponent (pc.lockScreenVisibilityLabel,  pc.lockScreenVisibilityComboBox));
        paramsThreeView.addRowComponent (new RowComponent (pc.groupIdLabel,               pc.groupIdEditor));
        paramsThreeView.addRowComponent (new RowComponent (pc.sortKeyLabel,               pc.sortKeyEditor));
        paramsThreeView.addRowComponent (new RowComponent (pc.groupSummaryLabel,          pc.groupSummaryButton));
        paramsThreeView.addRowComponent (new RowComponent (pc.groupAlertBehaviourLabel,   pc.groupAlertBehaviourComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.accentColourLabel,          pc.accentColourButton));
        paramsFourView .addRowComponent (new RowComponent (pc.ledColourLabel,             pc.ledColourButton));
        paramsFourView .addRowComponent (new RowComponent (pc.ledMsToBeOffLabel,          pc.ledMsToBeOffComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.ledMsToBeOnLabel,           pc.ledMsToBeOnComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.vibratorMsToBeOffLabel,     pc.vibratorMsToBeOffComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.vibratorMsToBeOnLabel,      pc.vibratorMsToBeOnComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.localOnlyLabel,             pc.localOnlyButton));
        paramsFourView .addRowComponent (new RowComponent (pc.ongoingLabel,               pc.ongoingButton));
        paramsFourView .addRowComponent (new RowComponent (pc.timestampVisibilityLabel,   pc.timestampVisibilityComboBox));
        paramsFourView .addRowComponent (new RowComponent (pc.timeoutAfterLabel,          pc.timeoutAfterComboBox));
      #endif
    }

    struct ParamsView   : public Component
    {
        ParamsView()
        {
            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        void addRowComponent (RowComponent* rc)
        {
            rowComponents.add (rc);
            addAndMakeVisible (rc);
        }

        void resized() override
        {
            auto totalRowUnits = 0;

            for (auto* rc : rowComponents)
                totalRowUnits += rc->rowUnits;

            auto rowHeight = getHeight() / totalRowUnits;

            auto bounds = getLocalBounds();

            for (auto* rc : rowComponents)
                rc->setBounds (bounds.removeFromTop (rc->rowUnits * rowHeight));

            auto* last = rowComponents[rowComponents.size() - 1];
            last->setBounds (last->getBounds().withHeight (getHeight() - last->getY()));
        }

    private:
        OwnedArray<RowComponent> rowComponents;
    };

    struct AuxActionsView   : public Component
    {
        AuxActionsView()
        {
            addAndMakeVisible (getDeliveredNotificationsButton);
            addAndMakeVisible (removeDeliveredNotifWithIdButton);
            addAndMakeVisible (deliveredNotifIdentifier);
            addAndMakeVisible (removeAllDeliveredNotifsButton);
          #if JUCE_IOS || JUCE_MAC
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
            auto columnWidth = getWidth();
            auto rowHeight = getHeight() / 6;
            auto bounds = getLocalBounds();

            getDeliveredNotificationsButton .setBounds (bounds.removeFromTop (rowHeight));

            auto rowBounds = bounds.removeFromTop (rowHeight);
            removeDeliveredNotifWithIdButton.setBounds (rowBounds.removeFromLeft (columnWidth / 2));
            deliveredNotifIdentifier        .setBounds (rowBounds);

            removeAllDeliveredNotifsButton  .setBounds (bounds.removeFromTop (rowHeight));

           #if JUCE_IOS || JUCE_MAC
            getPendingNotificationsButton .setBounds (bounds.removeFromTop (rowHeight));

            rowBounds = bounds.removeFromTop (rowHeight);
            removePendingNotifWithIdButton.setBounds (rowBounds.removeFromLeft (columnWidth / 2));
            pendingNotifIdentifier        .setBounds (rowBounds);

            removeAllPendingNotifsButton  .setBounds (bounds.removeFromTop (rowHeight));
           #endif
        }

        TextButton getDeliveredNotificationsButton  { "Get Delivered Notifications" };
        TextButton removeDeliveredNotifWithIdButton { "Remove Delivered Notif With ID:" };
        TextEditor deliveredNotifIdentifier;
        TextButton removeAllDeliveredNotifsButton   { "Remove All Delivered Notifs" };
        TextButton getPendingNotificationsButton    { "Get Pending Notifications" };
        TextButton removePendingNotifWithIdButton   { "Remove Pending Notif With ID:" };
        TextEditor pendingNotifIdentifier;
        TextButton removeAllPendingNotifsButton     { "Remove All Pending Notifs" };
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
            auto rowSize = getHeight () / 10;

            auto bounds = getLocalBounds().reduced (getWidth() / 10, getHeight() / 10);

            bounds.removeFromTop (2 * rowSize);

            getDeviceTokenButton       .setBounds (bounds.removeFromTop (rowSize));
            sendRemoteMessageButton    .setBounds (bounds.removeFromTop (rowSize));
            subscribeToSportsButton    .setBounds (bounds.removeFromTop (rowSize));
            unsubscribeFromSportsButton.setBounds (bounds.removeFromTop (rowSize));
        }

        TextButton getDeviceTokenButton        { "GetDeviceToken" };
        TextButton sendRemoteMessageButton     { "SendRemoteMessage" };
        TextButton subscribeToSportsButton     { "SubscribeToSports" };
        TextButton unsubscribeFromSportsButton { "UnsubscribeFromSports" };
    };

    struct DemoTabbedComponent  : public TabbedComponent
    {
        explicit DemoTabbedComponent (TabbedButtonBar::Orientation orientation)
            : TabbedComponent (orientation)
        {
        }

        void currentTabChanged (int, const String& newCurrentTabName) override
        {
            if (! showedRemoteInstructions && newCurrentTabName == "Remote")
            {
                PushNotificationsDemo::showRemoteInstructions();
                showedRemoteInstructions = true;
            }
        }

    private:
        bool showedRemoteInstructions = false;
    };

    static void showRemoteInstructions()
    {
       #if JUCE_IOS || JUCE_MAC
        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                               "Remote Notifications instructions",
                                               "In order to be able to test remote notifications "
                                               "ensure that the app is signed and that you register "
                                               "the bundle ID for remote notifications in "
                                               "Apple Developer Center.");
       #endif
    }

    Label headerLabel { "headerLabel", "Push Notifications Demo" };
    ParamControls paramControls;
    ParamsView paramsOneView, paramsTwoView, paramsThreeView, paramsFourView;
    AuxActionsView auxActionsView;
    TabbedComponent localNotificationsTabs { TabbedButtonBar::TabsAtTop };
    RemoteView remoteView;
    DemoTabbedComponent mainTabs { TabbedButtonBar::TabsAtTop };
    TextButton sendButton      { "Send!" };
    Label notAvailableYetLabel { "notAvailableYetLabel",
                                 "Push Notifications feature is not available on this platform yet!" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PushNotificationsDemo)
};
