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

class AboutWindowComponent    : public Component,
                                private Button::Listener
{
public:
    AboutWindowComponent()
        : titleLabel ("title", "PROJUCER"),
          versionLabel ("version"),
          copyrightLabel ("copyright", String (CharPointer_UTF8 ("\xc2\xa9")) + String (" 2017 ROLI Ltd.")),
          aboutButton ("About Us", URL ("https://juce.com"))
    {
        bool showPurchaseButton = false;

       #if ! JUCER_ENABLE_GPL_MODE
        if (LicenseController* controller = ProjucerApplication::getApp().licenseController)
            showPurchaseButton = (controller->getState().type != LicenseState::Type::indie
                               && controller->getState().type != LicenseState::Type::pro);
       #endif

        addAndMakeVisible (titleLabel);
        titleLabel.setJustificationType (Justification::centred);
        titleLabel.setFont (Font (35.0f, Font::FontStyleFlags::bold));

        auto buildDate = Time::getCompilationDate();
        addAndMakeVisible (versionLabel);
        versionLabel.setText ("JUCE v" + ProjucerApplication::getApp().getApplicationVersion()
                              + "\nBuild date: " + String (buildDate.getDayOfMonth())
                                                 + " " + Time::getMonthName (buildDate.getMonth(), true)
                                                 + " " + String (buildDate.getYear()),
                              dontSendNotification);

        versionLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (copyrightLabel);
        copyrightLabel.setJustificationType (Justification::centred);

        addAndMakeVisible (aboutButton);
        aboutButton.setTooltip ( {} );

        if (showPurchaseButton)
        {
            addAndMakeVisible (licenseButton = new TextButton ("Purchase License"));
            licenseButton->addListener (this);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromBottom (20);

        auto rightSlice  = bounds.removeFromRight (150);
        auto leftSlice   = bounds.removeFromLeft (150);
        auto centreSlice = bounds;

        //======================================================================
        rightSlice.removeFromRight (20);
        auto iconSlice = rightSlice.removeFromRight (100);
        huckleberryLogoBounds = iconSlice.removeFromBottom (100).toFloat();

        //======================================================================
        juceLogoBounds = leftSlice.removeFromTop (150).toFloat();
        juceLogoBounds.setWidth (juceLogoBounds.getWidth() + 100);
        juceLogoBounds.setHeight (juceLogoBounds.getHeight() + 100);

        copyrightLabel.setBounds (leftSlice.removeFromBottom (20));

        //======================================================================
        auto titleHeight = 40;

        centreSlice.removeFromTop ((centreSlice.getHeight() / 2) - (titleHeight / 2));

        titleLabel.setBounds (centreSlice.removeFromTop (titleHeight));

        centreSlice.removeFromTop (10);
        versionLabel.setBounds (centreSlice.removeFromTop (40));

        centreSlice.removeFromTop (10);

        if (licenseButton != nullptr)
            licenseButton->setBounds (centreSlice.removeFromTop (25).reduced (25, 0));

        aboutButton.setBounds (centreSlice.removeFromBottom (20));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));

        if (juceLogo != nullptr)
            juceLogo->drawWithin (g, juceLogoBounds.translated (-75, -75), RectanglePlacement::centred, 1.0);

        if (huckleberryLogo != nullptr)
            huckleberryLogo->drawWithin (g, huckleberryLogoBounds, RectanglePlacement::centred, 1.0);
    }

private:
    Label titleLabel, versionLabel, copyrightLabel;
    HyperlinkButton aboutButton;
    ScopedPointer<TextButton> licenseButton;

    Rectangle<float> huckleberryLogoBounds;
    Rectangle<float> juceLogoBounds;

    ScopedPointer<Drawable> juceLogo
        = Drawable::createFromImageData (BinaryData::juce_icon_png,
                                         BinaryData::juce_icon_pngSize);

    ScopedPointer<Drawable> huckleberryLogo
        = Drawable::createFromImageData (BinaryData::huckleberry_icon_svg,
                                         BinaryData::huckleberry_icon_svgSize);

    void buttonClicked (Button* b) override
    {
        if (b == licenseButton)
        {
            if (LicenseController* controller = ProjucerApplication::getApp().licenseController)
                controller->chooseNewLicense();
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutWindowComponent)
};
