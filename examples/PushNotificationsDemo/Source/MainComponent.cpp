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

#include "MainComponent.h"

//==============================================================================
MainContentComponent::MainContentComponent()
{
  #if JUCE_ANDROID || JUCE_IOS
    addAndMakeVisible (headerLabel);
    addAndMakeVisible (mainTabs);
    addAndMakeVisible (sendButton);
  #else
    addAndMakeVisible (notAvailableYetLabel);
  #endif

    headerLabel.setJustificationType (Justification::centred);
    notAvailableYetLabel.setJustificationType (Justification::centred);

    const auto colour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    localNotificationsTabs.addTab ("Req. params", colour, &requiredParamsView, false);
    localNotificationsTabs.addTab ("Opt. params1", colour, &optionalParamsOneView, false);
  #if JUCE_ANDROID
    localNotificationsTabs.addTab ("Opt. params2", colour, &optionalParamsTwoView, false);
    localNotificationsTabs.addTab ("Opt. params3", colour, &optionalParamsThreeView, false);
  #endif
    localNotificationsTabs.addTab ("Aux. actions", colour, &auxActionsView, false);

    mainTabs.addTab ("Local", colour, &localNotificationsTabs, false);
    mainTabs.addTab ("Remote", colour, &remoteView, false);

    const auto userArea = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
  #if JUCE_ANDROID || JUCE_IOS
    setSize (userArea.getWidth(), userArea.getHeight());
  #else
    setSize (userArea.getWidth() / 2, userArea.getHeight() / 2);
  #endif

    sendButton.addListener (this);
    auxActionsView.getDeliveredNotificationsButton .addListener (this);
    auxActionsView.removeDeliveredNotifWithIdButton.addListener (this);
    auxActionsView.removeAllDeliveredNotifsButton  .addListener (this);
  #if JUCE_IOS
    auxActionsView.getPendingNotificationsButton .addListener (this);
    auxActionsView.removePendingNotifWithIdButton.addListener (this);
    auxActionsView.removeAllPendingNotifsButton  .addListener (this);
  #endif

    remoteView.getDeviceTokenButton       .addListener (this);
    remoteView.sendRemoteMessageButton    .addListener (this);
    remoteView.subscribeToSportsButton    .addListener (this);
    remoteView.unsubscribeFromSportsButton.addListener (this);

    optionalParamsThreeView.accentColourButton.addListener (this);
    optionalParamsThreeView.ledColourButton   .addListener (this);

    jassert (PushNotifications::getInstance()->areNotificationsEnabled());

    PushNotifications::getInstance()->addListener (this);

  #if JUCE_IOS
    optionalParamsOneView.fireInComboBox.addListener (this);
    PushNotifications::getInstance()->requestPermissionsWithSettings (getIosSettings());
  #elif JUCE_ANDROID
    PushNotifications::ChannelGroup cg { "demoGroup", "demo group" };
    PushNotifications::getInstance()->setupChannels ({{ cg }}, getAndroidChannels());
  #endif
}

MainContentComponent::~MainContentComponent()
{
    PushNotifications::getInstance()->removeListener (this);
}

void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainContentComponent::resized()
{
    auto bounds = getLocalBounds().reduced (getWidth() / 20, getHeight() / 40);

    headerLabel.setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.1f)));

    mainTabs.setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.8f)));

    sendButton.setBounds (bounds);

    notAvailableYetLabel.setBounds (getLocalBounds());
}

void MainContentComponent::buttonClicked (Button* b)
{
    if (b == &sendButton)
        sendLocalNotification();
    else if (b == &optionalParamsThreeView.accentColourButton)
        setupAccentColour();
    else if (b == &optionalParamsThreeView.ledColourButton)
        setupLedColour();
    else if (b == &auxActionsView.getDeliveredNotificationsButton)
        getDeliveredNotifications();
    else if (b == &auxActionsView.removeDeliveredNotifWithIdButton)
        PushNotifications::getInstance()->removeDeliveredNotification (auxActionsView.deliveredNotifIdentifier.getText());
    else if (b == &auxActionsView.removeAllDeliveredNotifsButton)
        PushNotifications::getInstance()->removeAllDeliveredNotifications();
  #if JUCE_IOS
    else if (b == &auxActionsView.getPendingNotificationsButton)
        PushNotifications::getInstance()->getPendingLocalNotifications();
    else if (b == &auxActionsView.removePendingNotifWithIdButton)
        PushNotifications::getInstance()->removePendingLocalNotification (auxActionsView.pendingNotifIdentifier.getText());
    else if (b == &auxActionsView.removeAllPendingNotifsButton)
        PushNotifications::getInstance()->removeAllPendingLocalNotifications();
  #endif
    else if (b == &remoteView.getDeviceTokenButton)
    {
        String token = PushNotifications::getInstance()->getDeviceToken();

        DBG ("token = " + token);

        NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon, "Device token", token);
    }
  #if JUCE_ANDROID
    else if (b == &remoteView.sendRemoteMessageButton)
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

    }
    else if (b == &remoteView.subscribeToSportsButton)
    {
        PushNotifications::getInstance()->subscribeToTopic ("sports");
    }
    else if (b == &remoteView.unsubscribeFromSportsButton)
    {
        PushNotifications::getInstance()->unsubscribeFromTopic ("sports");
    }
  #endif
}

void MainContentComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
  #if JUCE_IOS
    if (comboBoxThatHasChanged == &optionalParamsOneView.fireInComboBox)
    {
        const bool repeatsAllowed = optionalParamsOneView.fireInComboBox.getSelectedItemIndex() >= 6;

        optionalParamsOneView.repeatButton.setEnabled (repeatsAllowed);

        if (! repeatsAllowed)
            optionalParamsOneView.repeatButton.setToggleState (false, NotificationType::sendNotification);
    }
  #else
    ignoreUnused (comboBoxThatHasChanged);
  #endif
}

void MainContentComponent::sendLocalNotification()
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

void MainContentComponent::fillRequiredParams (PushNotifications::Notification& n)
{
    n.identifier = requiredParamsView.identifierEditor.getText();
    n.title = requiredParamsView.titleEditor.getText();
    n.body = requiredParamsView.bodyEditor.getText();
  #if JUCE_IOS
    n.category = requiredParamsView.categories[requiredParamsView.categoryComboBox.getSelectedItemIndex()];
  #elif JUCE_ANDROID
    if (requiredParamsView.iconComboBox.getSelectedItemIndex() == 0)
        n.icon = "ic_stat_name";
    else if (requiredParamsView.iconComboBox.getSelectedItemIndex() == 1)
        n.icon = "ic_stat_name2";
    else if (requiredParamsView.iconComboBox.getSelectedItemIndex() == 2)
        n.icon = "ic_stat_name3";
    else if (requiredParamsView.iconComboBox.getSelectedItemIndex() == 3)
        n.icon = "ic_stat_name4";
    else
        n.icon = "ic_stat_name5";

    // Note: this is not strictly speaking required param, just doing it here because it is the fastest way!
    n.publicVersion = new PushNotifications::Notification();
    n.publicVersion->identifier = "blahblahblah";
    n.publicVersion->title = "Public title!";
    n.publicVersion->body  = "Public body!";
    n.publicVersion->icon  = n.icon;

   #if __ANDROID_API__ >= 26
    n.channelId = String (requiredParamsView.channelIdComboBox.getSelectedItemIndex() + 1);
   #endif
  #endif
}

void MainContentComponent::fillOptionalParamsOne (PushNotifications::Notification& n)
{
    n.subtitle = optionalParamsOneView.subtitleEditor.getText();
    n.badgeNumber = optionalParamsOneView.badgeNumberComboBox.getSelectedItemIndex();

    if (optionalParamsOneView.soundToPlayComboBox.getSelectedItemIndex() > 0)
        n.soundToPlay = URL (optionalParamsOneView.soundToPlayComboBox.getItemText (optionalParamsOneView.soundToPlayComboBox.getSelectedItemIndex()));

    n.properties = JSON::parse (optionalParamsOneView.propertiesEditor.getText());

  #if JUCE_IOS
    n.triggerIntervalSec = double (optionalParamsOneView.fireInComboBox.getSelectedItemIndex() * 10);
    n.repeat = optionalParamsOneView.repeatButton.getToggleState();
  #elif JUCE_ANDROID
    if (optionalParamsOneView.largeIconComboBox.getSelectedItemIndex() == 1)
        n.largeIcon = ImageFileFormat::loadFrom (BinaryData::ic_stat_name6_png, BinaryData::ic_stat_name6_pngSize);
    else if (optionalParamsOneView.largeIconComboBox.getSelectedItemIndex() == 2)
        n.largeIcon = ImageFileFormat::loadFrom (BinaryData::ic_stat_name7_png, BinaryData::ic_stat_name7_pngSize);
    else if (optionalParamsOneView.largeIconComboBox.getSelectedItemIndex() == 3)
        n.largeIcon = ImageFileFormat::loadFrom (BinaryData::ic_stat_name8_png, BinaryData::ic_stat_name8_pngSize);
    else if (optionalParamsOneView.largeIconComboBox.getSelectedItemIndex() == 4)
        n.largeIcon = ImageFileFormat::loadFrom (BinaryData::ic_stat_name9_png, BinaryData::ic_stat_name9_pngSize);
    else if (optionalParamsOneView.largeIconComboBox.getSelectedItemIndex() == 5)
        n.largeIcon = ImageFileFormat::loadFrom (BinaryData::ic_stat_name10_png, BinaryData::ic_stat_name10_pngSize);

    n.badgeIconType = (PushNotifications::Notification::BadgeIconType) optionalParamsOneView.badgeIconComboBox.getSelectedItemIndex();
    n.tickerText  = optionalParamsOneView.tickerTextEditor.getText();

    n.shouldAutoCancel = optionalParamsOneView.autoCancelButton.getToggleState();
    n.alertOnlyOnce = optionalParamsOneView.alertOnlyOnceButton.getToggleState();

    if (optionalParamsOneView.actionsComboBox.getSelectedItemIndex() == 1)
    {
        PushNotifications::Notification::Action a, a2;
        a .style = PushNotifications::Notification::Action::button;
        a2.style = PushNotifications::Notification::Action::button;
        a .title = "Ok";
        a2.title = "Cancel";
        n.actions.add (a);
        n.actions.add (a2);
    }
    else if (optionalParamsOneView.actionsComboBox.getSelectedItemIndex() == 2)
    {
        PushNotifications::Notification::Action a, a2;
        a .title = "Ok";
        a2.title = "Cancel";
        a .style = PushNotifications::Notification::Action::button;
        a2.style = PushNotifications::Notification::Action::button;
        a .icon = "ic_stat_name4";
        a2.icon = "ic_stat_name5";
        n.actions.add (a);
        n.actions.add (a2);
    }
    else if (optionalParamsOneView.actionsComboBox.getSelectedItemIndex() == 3)
    {
        PushNotifications::Notification::Action a, a2;
        a .title = "Input Text Here";
        a2.title = "No";
        a .style = PushNotifications::Notification::Action::text;
        a2.style = PushNotifications::Notification::Action::button;
        a .icon = "ic_stat_name4";
        a2.icon = "ic_stat_name5";
        a.textInputPlaceholder = "placeholder text ...";
        n.actions.add (a);
        n.actions.add (a2);
    }
    else if (optionalParamsOneView.actionsComboBox.getSelectedItemIndex() == 4)
    {
        PushNotifications::Notification::Action a, a2;
        a .title = "Input Text Here";
        a2.title = "No";
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

void MainContentComponent::fillOptionalParamsTwo (PushNotifications::Notification& n)
{
    using Notification = PushNotifications::Notification;

    Notification::Progress progress;
    progress.max     = optionalParamsTwoView.progressMaxComboBox.getSelectedItemIndex() * 10;
    progress.current = optionalParamsTwoView.progressCurrentComboBox.getSelectedItemIndex() * 10;
    progress.indeterminate = optionalParamsTwoView.progressIndeterminateButton.getToggleState();

    n.progress = progress;
    n.person   = optionalParamsTwoView.personEditor.getText();
    n.type                 = Notification::Type                 (optionalParamsTwoView.categoryComboBox.getSelectedItemIndex());
    n.priority             = Notification::Priority             (optionalParamsTwoView.priorityComboBox.getSelectedItemIndex() - 2);
    n.lockScreenAppearance = Notification::LockScreenAppearance (optionalParamsTwoView.lockScreenVisibilityComboBox.getSelectedItemIndex() - 1);
    n.groupId = optionalParamsTwoView.groupIdEditor.getText();
    n.groupSortKey = optionalParamsTwoView.sortKeyEditor.getText();
    n.groupSummary = optionalParamsTwoView.groupSummaryButton.getToggleState();
    n.groupAlertBehaviour = Notification::GroupAlertBehaviour (optionalParamsTwoView.groupAlertBehaviourComboBox.getSelectedItemIndex());
}

void MainContentComponent::fillOptionalParamsThree (PushNotifications::Notification& n)
{
    n.accentColour = optionalParamsThreeView.accentColourButton.findColour (TextButton::buttonColourId, false);
    n.ledColour    = optionalParamsThreeView.ledColourButton   .findColour (TextButton::buttonColourId, false);

    using Notification = PushNotifications::Notification;
    Notification::LedBlinkPattern ledBlinkPattern;
    ledBlinkPattern.msToBeOn  = optionalParamsThreeView.ledMsToBeOnComboBox .getSelectedItemIndex() * 200;
    ledBlinkPattern.msToBeOff = optionalParamsThreeView.ledMsToBeOffComboBox.getSelectedItemIndex() * 200;
    n.ledBlinkPattern = ledBlinkPattern;

    Array<int> vibrationPattern;

    if (optionalParamsThreeView.vibratorMsToBeOnComboBox .getSelectedItemIndex() > 0 &&
        optionalParamsThreeView.vibratorMsToBeOffComboBox.getSelectedItemIndex() > 0)
    {
        vibrationPattern.add (optionalParamsThreeView.vibratorMsToBeOffComboBox.getSelectedItemIndex() * 500);
        vibrationPattern.add (optionalParamsThreeView.vibratorMsToBeOnComboBox .getSelectedItemIndex() * 500);
        vibrationPattern.add (2 * optionalParamsThreeView.vibratorMsToBeOffComboBox.getSelectedItemIndex() * 500);
        vibrationPattern.add (2 * optionalParamsThreeView.vibratorMsToBeOnComboBox .getSelectedItemIndex() * 500);
    }

    n.vibrationPattern = vibrationPattern;

    n.localOnly = optionalParamsThreeView.localOnlyButton.getToggleState();
    n.ongoing = optionalParamsThreeView.ongoingButton.getToggleState();
    n.timestampVisibility = Notification::TimestampVisibility (optionalParamsThreeView.timestampVisibilityComboBox.getSelectedItemIndex());

    if (optionalParamsThreeView.timeoutAfterComboBox.getSelectedItemIndex() > 0)
    {
        auto index = optionalParamsThreeView.timeoutAfterComboBox.getSelectedItemIndex();
        n.timeoutAfterMs = index * 1000 + 4000;
    }
}

void MainContentComponent::setupAccentColour()
{
    optionalParamsThreeView.accentColourSelector = new ColourSelector();
    optionalParamsThreeView.accentColourSelector->setName ("accent colour");
    optionalParamsThreeView.accentColourSelector->setCurrentColour (optionalParamsThreeView.accentColourButton.findColour (TextButton::buttonColourId));
    optionalParamsThreeView.accentColourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
    optionalParamsThreeView.accentColourSelector->setSize (200, 200);
    optionalParamsThreeView.accentColourSelector->addComponentListener (this);
    optionalParamsThreeView.accentColourSelector->addChangeListener (this);

    CallOutBox::launchAsynchronously (optionalParamsThreeView.accentColourSelector, optionalParamsThreeView.accentColourButton.getScreenBounds(), nullptr);
}

void MainContentComponent::setupLedColour()
{
    optionalParamsThreeView.ledColourSelector = new ColourSelector();
    optionalParamsThreeView.ledColourSelector->setName ("led colour");
    optionalParamsThreeView.ledColourSelector->setCurrentColour (optionalParamsThreeView.ledColourButton.findColour (TextButton::buttonColourId));
    optionalParamsThreeView.ledColourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
    optionalParamsThreeView.ledColourSelector->setSize (200, 200);
    optionalParamsThreeView.ledColourSelector->addComponentListener (this);
    optionalParamsThreeView.ledColourSelector->addChangeListener (this);

    CallOutBox::launchAsynchronously (optionalParamsThreeView.ledColourSelector, optionalParamsThreeView.accentColourButton.getScreenBounds(), nullptr);
}

void MainContentComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == optionalParamsThreeView.accentColourSelector)
    {
        Colour c = optionalParamsThreeView.accentColourSelector->getCurrentColour();
        optionalParamsThreeView.accentColourButton.setColour (TextButton::buttonColourId, c);
    }
    else if (source == optionalParamsThreeView.ledColourSelector)
    {
        Colour c = optionalParamsThreeView.ledColourSelector->getCurrentColour();
        optionalParamsThreeView.ledColourButton.setColour (TextButton::buttonColourId, c);
    }
}

void MainContentComponent::componentBeingDeleted (Component& component)
{
    if (&component == optionalParamsThreeView.accentColourSelector)
        optionalParamsThreeView.accentColourSelector = nullptr;
    else if (&component == optionalParamsThreeView.ledColourSelector)
        optionalParamsThreeView.ledColourSelector = nullptr;
}

void MainContentComponent::handleNotification (bool isLocalNotification, const PushNotifications::Notification& n)
{
    ignoreUnused (isLocalNotification);

    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                           "Received notification",
                                           "ID: " + n.identifier
                                           + ", title: " + n.title
                                           + ", body: " + n.body);
}

void MainContentComponent::handleNotificationAction (bool isLocalNotification,
                                                     const PushNotifications::Notification& n,
                                                     const String& actionIdentifier,
                                                     const String& optionalResponse)
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

void MainContentComponent::localNotificationDismissedByUser (const PushNotifications::Notification& n)
{
    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                           "Notification dismissed by a user",
                                           "ID: " + n.identifier
                                           + ", title: " + n.title
                                           + ", body: " + n.body);
}

void MainContentComponent::getDeliveredNotifications()
{
    PushNotifications::getInstance()->getDeliveredNotifications();
}

void MainContentComponent::deliveredNotificationsListReceived (const Array<PushNotifications::Notification>& notifs)
{
    String text = "Received notifications: ";

    for (const auto& n : notifs)
        text << "(" << n.identifier << ", " << n.title << ", " << n.body << "), ";

    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon, "Received notification list", text);
}

void MainContentComponent::pendingLocalNotificationsListReceived (const Array<PushNotifications::Notification>& notifs)
{
    String text = "Pending notifications: ";

    for (const auto& n : notifs)
        text << "(" << n.identifier << ", " << n.title << ", " << n.body << "), ";

    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon, "Pending notification list", text);
}

void MainContentComponent::deviceTokenRefreshed (const String& token)
{
    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                           "Device token refreshed",
                                           token);
}

#if JUCE_ANDROID
void MainContentComponent::remoteNotificationsDeleted()
{
    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                           "Remote notifications deleted",
                                           "Some of the pending messages were removed!");
}

void MainContentComponent::upstreamMessageSent (const String& messageId)
{
    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                           "Upstream message sent",
                                           "Message id: " + messageId);
}

void MainContentComponent::upstreamMessageSendingError (const String& messageId, const String& error)
{
    NativeMessageBox::showMessageBoxAsync (AlertWindow::InfoIcon,
                                           "Upstream message sending error",
                                           "Message id: " + messageId
                                           + "\nerror: " + error);
}

Array<PushNotifications::Channel> MainContentComponent::getAndroidChannels()
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

#elif JUCE_IOS
PushNotifications::Settings MainContentComponent::getIosSettings()
{
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

    return settings;
}
#endif
