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


//==============================================================================
class UserSettingsPopup    : public Component
                            #if ! JUCER_ENABLE_GPL_MODE
                          , private Button::Listener,
                            private LicenseController::StateChangedCallback
                           #endif
{
public:
    UserSettingsPopup (bool isShownInsideWebview)
       #if ! JUCER_ENABLE_GPL_MODE
        : isInsideWebview (isShownInsideWebview)
       #endif
    {
       #if JUCER_ENABLE_GPL_MODE
        ignoreUnused (isShownInsideWebview);
       #endif

        auto standardFont = Font (12.0f);

        addAndMakeVisible (loggedInUsernameLabel = new Label ("Username Label"));

        loggedInUsernameLabel->setFont (standardFont);
        loggedInUsernameLabel->setJustificationType (Justification::centred);
        loggedInUsernameLabel->setMinimumHorizontalScale (0.75f);

       #if JUCER_ENABLE_GPL_MODE
        loggedInUsernameLabel->setText ("GPL Mode: Re-compile with JUCER_ENABLE_GPL_MODE=0 to enable login!",
                                        NotificationType::dontSendNotification);
       #else
        addAndMakeVisible (licenseTypeLabel = new Label ("License Type Label"));

        licenseTypeLabel->setFont (standardFont);
        licenseTypeLabel->setJustificationType (Justification::centred);
        licenseTypeLabel->setMinimumHorizontalScale (1.0f);

        addAndMakeVisible (logoutButton = new TextButton (isInsideWebview ? "Select different account..." : "Logout"));
        logoutButton->addListener (this);
        logoutButton->setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));

        if (! isInsideWebview)
        {
            addAndMakeVisible (switchLicenseButton = new TextButton ("Switch License"));
            switchLicenseButton->addListener (this);
        }

        if (LicenseController* controller = ProjucerApplication::getApp().licenseController)
            licenseStateChanged (controller->getState());
       #endif
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (10, 20);

       #if JUCER_ENABLE_GPL_MODE
        loggedInUsernameLabel->setBounds (bounds);
       #else
        loggedInUsernameLabel->setBounds (bounds.removeFromTop (25));

        if (hasLicenseType)
        {
            bounds.removeFromTop (10);
            licenseTypeLabel->setBounds (bounds.removeFromTop (25));
        }

        bounds.removeFromBottom (5);
        auto buttonArea = bounds.removeFromBottom (30);

        if (! isInsideWebview)
            switchLicenseButton->setBounds (buttonArea.removeFromRight (buttonArea.getWidth() / 2).reduced (2));

        logoutButton->setBounds (buttonArea.reduced (2));
       #endif
    }

private:
    //==============================================================================
   #if ! JUCER_ENABLE_GPL_MODE
    void buttonClicked (Button* b) override
    {
        if (b == logoutButton)
        {
            dismissCalloutBox();
            ProjucerApplication::getApp().doLogout();
        }
        else if (b == switchLicenseButton)
        {
            dismissCalloutBox();
            if (LicenseController* controller = ProjucerApplication::getApp().licenseController)
                controller->chooseNewLicense();
        }
    }


    void licenseStateChanged (const LicenseState& state) override
    {
        hasLicenseType = (state.type != LicenseState::Type::noLicenseChosenYet);
        licenseTypeLabel->setVisible (hasLicenseType);
        loggedInUsernameLabel->setText (state.username, NotificationType::dontSendNotification);
        licenseTypeLabel->setText (LicenseState::licenseTypeToString (state.type), NotificationType::dontSendNotification);
    }

    void dismissCalloutBox()
    {
        if (auto* parent = findParentComponentOfClass<CallOutBox>())
            parent->dismiss();
    }

    void lookAndFeelChanged() override
    {
        if (logoutButton != nullptr)
            logoutButton->setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
    }
   #endif

    //==============================================================================
    ScopedPointer<Label> loggedInUsernameLabel;

   #if ! JUCER_ENABLE_GPL_MODE
    ScopedPointer<Label> licenseTypeLabel;
    ScopedPointer<TextButton> logoutButton, switchLicenseButton;
    bool hasLicenseType = false;
    bool isInsideWebview;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UserSettingsPopup)
};
