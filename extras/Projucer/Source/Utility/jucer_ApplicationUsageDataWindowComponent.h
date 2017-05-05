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

class ApplicationUsageDataWindowComponent    : public Component,
                                               private Button::Listener
{
public:
    ApplicationUsageDataWindowComponent (bool showCheckbox)
    {
        addAndMakeVisible (headerLabel);
        headerLabel.setText ("Application Usage Analytics", dontSendNotification);
        headerLabel.setFont (Font (20.0f, Font::FontStyleFlags::bold));
        headerLabel.setJustificationType (Justification::centred);

        auto textToShow = String ("We use analytics services to understand how developers use our software in order for JUCE to improve its software and services. ");

        if (! showCheckbox)
            textToShow += String (" Analytics can be disabled with an Indie or Pro license. ");

        textToShow += String ("For more information, please read the JUCE EULA and Privacy policy:");

        addAndMakeVisible (bodyLabel);
        bodyLabel.setText (textToShow, dontSendNotification);
        bodyLabel.setFont (Font (14.0f));
        bodyLabel.setJustificationType (Justification::centredLeft);

        addAndMakeVisible (juceEULALink);
        juceEULALink.setButtonText ("JUCE EULA");
        juceEULALink.setFont (Font (14.0f), false);
        juceEULALink.setURL (URL ("https://juce.com/juce-5-license"));

        addAndMakeVisible (privacyPolicyLink);
        privacyPolicyLink.setButtonText ("Privacy Policy");
        privacyPolicyLink.setFont (Font (14.0f), false);
        privacyPolicyLink.setURL (URL ("https://juce.com/privacy-policy"));

        addAndMakeVisible (okButton);
        okButton.setButtonText ("OK");
        okButton.addListener (this);

        if (showCheckbox)
        {
            addAndMakeVisible (shareApplicationUsageDataToggle = new ToggleButton());

            LicenseController* controller = ProjucerApplication::getApp().licenseController;

            if (controller != nullptr && controller->getState().applicationUsageDataState == LicenseState::ApplicationUsageData::disabled)
                shareApplicationUsageDataToggle->setToggleState (false, dontSendNotification);
            else
                shareApplicationUsageDataToggle->setToggleState (true, dontSendNotification);

            addAndMakeVisible(shareApplicationUsageDataLabel = new Label ({}, "Help JUCE to improve its software and services by sharing my application usage data"));
            shareApplicationUsageDataLabel->setFont (Font (14.0f));
            shareApplicationUsageDataLabel->setMinimumHorizontalScale (1.0f);
        }
        else
        {
            addAndMakeVisible (upgradeLicenseButton = new TextButton ("Upgrade License"));
            upgradeLicenseButton->addListener (this);
            upgradeLicenseButton->setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
        }
    }

    ~ApplicationUsageDataWindowComponent()
    {
        if (LicenseController* controller = ProjucerApplication::getApp().licenseController)
        {
            auto newApplicationUsageDataState = LicenseState::ApplicationUsageData::enabled;

            if (shareApplicationUsageDataToggle != nullptr && ! shareApplicationUsageDataToggle->getToggleState())
                newApplicationUsageDataState = LicenseState::ApplicationUsageData::disabled;

            controller->setApplicationUsageDataState (newApplicationUsageDataState);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (20);
        headerLabel.setBounds (bounds.removeFromTop (40));
        bodyLabel.setBounds (bounds.removeFromTop (75));

        bounds.removeFromTop (10);

        auto linkBounds = bounds.removeFromTop (20);
        juceEULALink.setBounds (linkBounds.removeFromLeft (linkBounds.getWidth() / 2).reduced (2));
        privacyPolicyLink.setBounds (linkBounds.reduced (2));

        if (shareApplicationUsageDataToggle != nullptr)
        {
            bounds.removeFromTop (10);

            auto toggleBounds = bounds.removeFromTop (40);
            shareApplicationUsageDataToggle->setBounds (toggleBounds.removeFromLeft (40).reduced (5));
            shareApplicationUsageDataLabel->setBounds (toggleBounds);
        }

        bounds.removeFromTop (10);

        auto buttonW = 125;
        auto buttonH = 40;

        if (upgradeLicenseButton != nullptr)
        {
            auto left = bounds.removeFromLeft (bounds.getWidth() / 2);

            upgradeLicenseButton->setSize (buttonW, buttonH);
            upgradeLicenseButton->setCentrePosition (left.getCentreX(), left.getCentreY());
        }

        okButton.setSize (buttonW, buttonH);
        okButton.setCentrePosition (bounds.getCentreX(), bounds.getCentreY());
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));
    }

private:
    Label headerLabel, bodyLabel;
    HyperlinkButton juceEULALink, privacyPolicyLink;
    ScopedPointer<Label> shareApplicationUsageDataLabel;
    ScopedPointer<ToggleButton> shareApplicationUsageDataToggle;
    TextButton okButton;
    ScopedPointer<TextButton> upgradeLicenseButton;

    void buttonClicked (Button* b) override
    {
        if (b == &okButton)
        {
            ProjucerApplication::getApp().dismissApplicationUsageDataAgreementPopup();
        }
        else if (b == upgradeLicenseButton)
        {
            if (LicenseController* controller = ProjucerApplication::getApp().licenseController)
                controller->chooseNewLicense();
        }
    }

    void lookAndFeelChanged() override
    {
        if (upgradeLicenseButton != nullptr)
            upgradeLicenseButton->setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ApplicationUsageDataWindowComponent)
};
