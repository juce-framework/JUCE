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

#include "JuceDemoHeader.h"


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
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        logo.setBounds (area);
        area = area.removeFromBottom (24);
        linkButton.setBounds (area.removeFromRight (getWidth() / 4));
        versionLabel.setBounds (area);
    }

private:
    Label versionLabel;
    HyperlinkButton linkButton { "www.juce.com", URL ("http://www.juce.com") };

    //==============================================================================
    struct LogoDrawComponent  : public Component,
                                private Timer
    {
        LogoDrawComponent()
        {
            startTimerHz (30); // repaint at 30 fps
        }

        void paint (Graphics& g) override
        {
            Path wavePath;

            const float waveStep = 10.0f;
            const float waveY = getHeight() * 0.44f;
            int i = 0;

            for (float x = waveStep * 0.5f; x < getWidth(); x += waveStep)
            {
                const float y1 = waveY + getHeight() * 0.05f * std::sin (i * 0.38f + elapsed);
                const float y2 = waveY + getHeight() * 0.10f * std::sin (i * 0.20f + elapsed * 2.0f);

                wavePath.addLineSegment (Line<float> (x, y1, x, y2), 2.0f);
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

        Path logoPath { MainAppWindow::getJUCELogoPath() };
        float elapsed = 0;
    };

    LogoDrawComponent logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntroScreen)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<IntroScreen> demo ("00 Welcome!");
