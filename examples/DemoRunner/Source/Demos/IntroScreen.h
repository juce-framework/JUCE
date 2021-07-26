/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class IntroScreen   : public Component
{
public:
    IntroScreen()
    {
        setOpaque (true);

        addAndMakeVisible (versionLabel);
        addAndMakeVisible (linkButton);
        addAndMakeVisible (logo);

        versionLabel.setText (String ("{version}  built on {date}")
                                  .replace ("{version}", SystemStats::getJUCEVersion())
                                  .replace ("{date}",    String (__DATE__).replace ("  ", " ")),
                              dontSendNotification);

        linkButton.setColour (HyperlinkButton::textColourId, Colours::lightblue);

        setTitle ("Home");
        setFocusContainerType (FocusContainerType::focusContainer);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);

        auto bottomSlice = area.removeFromBottom (24);
        linkButton.setBounds (bottomSlice.removeFromRight (getWidth() / 4));
        versionLabel.setBounds (bottomSlice);

        logo.setBounds (area);
    }

private:
    Label versionLabel;
    HyperlinkButton linkButton { "www.juce.com", { "http://www.juce.com" } };

    //==============================================================================
    struct LogoDrawComponent  : public Component,
                                private Timer
    {
        LogoDrawComponent()
        {
            setTitle ("JUCE Logo");
            startTimerHz (30); // repaint at 30 fps
        }

        void paint (Graphics& g) override
        {
            Path wavePath;

            auto waveStep = 10.0f;
            auto waveY = (float) getHeight() * 0.5f;
            int i = 0;

            for (auto x = waveStep * 0.5f; x < (float) getWidth(); x += waveStep)
            {
                auto y1 = waveY + (float) getHeight() * 0.05f * std::sin ((float) i * 0.38f + elapsed);
                auto y2 = waveY + (float) getHeight() * 0.10f * std::sin ((float) i * 0.20f + elapsed * 2.0f);

                wavePath.addLineSegment ({ x, y1, x, y2 }, 2.0f);
                wavePath.addEllipse (x - waveStep * 0.3f, y1 - waveStep * 0.3f, waveStep * 0.6f, waveStep * 0.6f);
                wavePath.addEllipse (x - waveStep * 0.3f, y2 - waveStep * 0.3f, waveStep * 0.6f, waveStep * 0.6f);

                ++i;
            }

            g.setColour (Colour::greyLevel (0.4f));
            g.fillPath (wavePath);

            g.setColour (Colour (0xc4f39082));
            g.fillPath (logoPath, RectanglePlacement (RectanglePlacement::centred)
                                    .getTransformToFit (logoPath.getBounds(),
                                                        getLocalBounds().reduced (20, getHeight() / 4).toFloat()));
        }

        void timerCallback() override
        {
            repaint();
            elapsed += 0.02f;
        }

        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::image);
        }

        Path logoPath  { getJUCELogoPath() };
        float elapsed = 0.0f;
    };

    LogoDrawComponent logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntroScreen)
};
