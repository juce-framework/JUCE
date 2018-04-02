/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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
class ApplicationUsageDataWindowComponent    : public Component
{
public:
    ApplicationUsageDataWindowComponent (bool showCheckbox)
    {
        addAndMakeVisible (headerLabel);
        headerLabel.setText ("Application Usage Analytics", dontSendNotification);
        headerLabel.setFont (Font (20.0f, Font::FontStyleFlags::bold));
        headerLabel.setJustificationType (Justification::centered);

        auto textToShow = String ("We use analytics services to understand how developers use our software in order for JUCE to improve its software and services. ");

        if (! showCheckbox)
            textToShow += String (" Analytics can be disabled with an Indie or Pro license. ");

        textToShow += String ("For more information, please read the JUCE EULA and Privacy policy:");

        addAndMakeVisible (bodyLabel);
        bodyLabel.setText (textToShow, dontSendNotification);
        bodyLabel.setFont (Font (14.0f));
        bodyLabel.setJustificationType (Justification::centeredLeft);

        addAndMakeVisible (juceEULALink);
        juceEULALink.setButtonText ("JUCE EULA");
        juceEULALink.setFont (Font (14.0f), false);
        juceEULALink.setURL (URL ("https://juce.com/juce-5-license"));

        addAndMakeVisible (privacyPolicyLink);
        privacyPolicyLink.setButtonText ("Privacy Policy");
        privacyPolicyLink.setFont (Font (14.0f), false);
        privacyPolicyLink.setURL (URL ("https://juce.com/juce-5-privacy-policy"));

        addAndMakeVisible (okButton);

        if (showCheckbox)
        {
            addAndMakeVisible (shareApplicationUsageDataToggle = new ToggleButton());

            auto* controller = ProjucerApplication::getApp().licenseController.get();

            if (controller != nullptr && controller->getState().applicationUsageDataState == LicenseState::ApplicationUsageData::disabled)
                shareApplicationUsageDataToggle->setToggleState (false, dontSendNotification);
            else
                shareApplicationUsageDataToggle->setToggleState (true, dontSendNotification);

            addAndMakeVisible (shareApplicationUsageDataLabel);
            shareApplicationUsageDataLabel.setFont (Font (14.0f));
            shareApplicationUsageDataLabel.setMinimumHorizontalScale (1.0f);
        }
        else
        {
            addAndMakeVisible (upgradeLicenseButton);
            upgradeLicenseButton.setColor (TextButton::buttonColorId, findColor (secondaryButtonBackgroundColorId));

            upgradeLicenseButton.onClick = []
            {
                if (auto* controller = ProjucerApplication::getApp().licenseController.get())
                    controller->chooseNewLicense();
            };
        }
    }

    ~ApplicationUsageDataWindowComponent()
    {
        if (auto* controller = ProjucerApplication::getApp().licenseController.get())
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
            shareApplicationUsageDataLabel.setBounds (toggleBounds);
        }

        bounds.removeFromTop (10);

        auto buttonW = 125;
        auto buttonH = 40;

        if (upgradeLicenseButton.isShowing())
        {
            auto left = bounds.removeFromLeft (bounds.getWidth() / 2);

            upgradeLicenseButton.setSize (buttonW, buttonH);
            upgradeLicenseButton.setCenterPosition (left.getCenterX(), left.getCenterY());
        }

        okButton.setSize (buttonW, buttonH);
        okButton.setCenterPosition (bounds.getCenterX(), bounds.getCenterY());
        okButton.onClick = [] { ProjucerApplication::getApp().dismissApplicationUsageDataAgreementPopup(); };
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColor (backgroundColorId));
    }

private:
    Label headerLabel, bodyLabel;
    HyperlinkButton juceEULALink, privacyPolicyLink;
    Label shareApplicationUsageDataLabel { {}, "Help JUCE to improve its software and services by sharing my application usage data" };
    ScopedPointer<ToggleButton> shareApplicationUsageDataToggle;
    TextButton okButton { "OK" }, upgradeLicenseButton { "Upgrade License" };

    void lookAndFeelChanged() override
    {
        upgradeLicenseButton.setColor (TextButton::buttonColorId, findColor (secondaryButtonBackgroundColorId));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ApplicationUsageDataWindowComponent)
};
