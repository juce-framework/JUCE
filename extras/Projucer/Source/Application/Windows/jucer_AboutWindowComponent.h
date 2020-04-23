/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class AboutWindowComponent    : public Component
{
public:
    AboutWindowComponent()
    {
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
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromBottom (20);

        auto leftSlice   = bounds.removeFromLeft (150);
        auto centreSlice = bounds.withTrimmedRight (150);

        juceLogoBounds = leftSlice.removeFromTop (150).toFloat();
        juceLogoBounds.setWidth (juceLogoBounds.getWidth() + 100);
        juceLogoBounds.setHeight (juceLogoBounds.getHeight() + 100);

        copyrightLabel.setBounds (leftSlice.removeFromBottom (20));

        auto titleHeight = 40;

        centreSlice.removeFromTop ((centreSlice.getHeight() / 2) - (titleHeight / 2));

        titleLabel.setBounds (centreSlice.removeFromTop (titleHeight));

        centreSlice.removeFromTop (10);
        versionLabel.setBounds (centreSlice.removeFromTop (40));

        centreSlice.removeFromTop (10);
        aboutButton.setBounds (centreSlice.removeFromBottom (20));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));

        if (juceLogo != nullptr)
            juceLogo->drawWithin (g, juceLogoBounds.translated (-75, -75), RectanglePlacement::centred, 1.0);
    }

private:
    Label titleLabel { "title", "PROJUCER" },
          versionLabel { "version" },
          copyrightLabel { "copyright", String (CharPointer_UTF8 ("\xc2\xa9")) + String (" 2020 Raw Material Software Limited") };

    HyperlinkButton aboutButton { "About Us", URL ("https://juce.com") };

    Rectangle<float> juceLogoBounds;

    std::unique_ptr<Drawable> juceLogo { Drawable::createFromImageData (BinaryData::juce_icon_png,
                                                                        BinaryData::juce_icon_pngSize) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutWindowComponent)
};
