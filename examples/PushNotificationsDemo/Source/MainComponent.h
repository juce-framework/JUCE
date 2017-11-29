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
  #elif JUCE_IOS || JUCE_MAC
    static PushNotifications::Settings getNotificationSettings();
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
        Label       identifierLabel { "identifierLabel", "Identifier" };
        TextEditor  identifierEditor;
        Label       titleLabel { "titleLabel", "Title" };
        TextEditor  titleEditor;
        Label       bodyLabel { "bodyLabel", "Body" };
        TextEditor  bodyEditor;

        Label       categoryLabel { "categoryLabel", "Category" };
        ComboBox    categoryComboBox;
        Label       channelIdLabel { "channelIdLabel", "Channel ID" };
        ComboBox    channelIdComboBox;
        Label       iconLabel { "iconLabel", "Icon" };
        ComboBox    iconComboBox;

        Label        subtitleLabel { "subtitleLabel", "Subtitle" };
        TextEditor   subtitleEditor;
        Label        badgeNumberLabel { "badgeNumberLabel", "BadgeNumber" };
        ComboBox     badgeNumberComboBox;
        Label        soundToPlayLabel { "soundToPlayLabel", "SoundToPlay" };
        ComboBox     soundToPlayComboBox;
        Label        propertiesLabel { "propertiesLabel", "Properties" };
        TextEditor   propertiesEditor;
        Label        fireInLabel { "fireInLabel", "Fire in" };
        ComboBox     fireInComboBox;
        Label        repeatLabel { "repeatLabel", "Repeat" };
        ToggleButton repeatButton;
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

        Label        progressMaxLabel { "progressMaxLabel", "ProgressMax" };
        ComboBox     progressMaxComboBox;
        Label        progressCurrentLabel { "progressCurrentLabel", "ProgressCurrent" };
        ComboBox     progressCurrentComboBox;
        Label        progressIndeterminateLabel { "progressIndeterminateLabel", "ProgressIndeterminate" };
        ToggleButton progressIndeterminateButton;
        Label        notifCategoryLabel { "notifCategoryLabel", "Category" };
        ComboBox     notifCategoryComboBox;
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

    void setupControls();
    void distributeControls();

    struct ParamsView   : public Component
    {
        ParamsView()
        {
            // For now, to be able to dismiss mobile keyboard.
            setWantsKeyboardFocus (true);
        }

        void addRowComponent (RowComponent *rc)
        {
            rowComponents.add (rc);
            addAndMakeVisible (rc);
        }

        void resized() override
        {
            int totalRowUnits = 0;

            for (const auto &rc : rowComponents)
                totalRowUnits += rc->rowUnits;

            const int rowHeight = getHeight() / totalRowUnits;

            auto bounds = getLocalBounds();

            for (auto &rc : rowComponents)
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
            const int columnWidth = getWidth();

            const int rowHeight = getHeight() / 6;

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

        TextButton getDeliveredNotificationsButton { "Get Delivered Notifications" };
        TextButton removeDeliveredNotifWithIdButton { "Remove Delivered Notif With ID:" };
        TextEditor deliveredNotifIdentifier;
        TextButton removeAllDeliveredNotifsButton { "Remove All Delivered Notifs" };
        TextButton getPendingNotificationsButton { "Get Pending Notifications" };
        TextButton removePendingNotifWithIdButton { "Remove Pending Notif With ID:" };
        TextEditor pendingNotifIdentifier;
        TextButton removeAllPendingNotifsButton { "Remove All Pending Notifs" };
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
                MainContentComponent::showRemoteInstructions();

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
    ParamsView paramsOneView;
    ParamsView paramsTwoView;
    ParamsView paramsThreeView;
    ParamsView paramsFourView;
    AuxActionsView auxActionsView;
    TabbedComponent localNotificationsTabs { TabbedButtonBar::TabsAtTop };
    RemoteView remoteView;
    DemoTabbedComponent mainTabs { TabbedButtonBar::TabsAtTop };
    TextButton sendButton { "Send!" };
    Label notAvailableYetLabel { "notAvailableYetLabel", "Push Notifications feature is not available on this platform yet!" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
