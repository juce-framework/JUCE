/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "JuceDemoHeader.h"


//==============================================================================
class IntroScreen   : public Component
{
public:
    IntroScreen()
        : linkButton ("www.juce.com", URL ("http://www.juce.com"))
    {
        setOpaque (true);

        addAndMakeVisible (versionLabel);
        addAndMakeVisible (linkButton);
        addAndMakeVisible (logo);

        versionLabel.setColour (Label::textColourId, Colours::white);
        versionLabel.setText (String ("{version}  built on {date}")
                                  .replace ("{version}", SystemStats::getJUCEVersion())
                                  .replace ("{date}",    String (__DATE__).replace ("  ", " ")),
                              dontSendNotification);

        linkButton.setColour (HyperlinkButton::textColourId, Colours::lightblue);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colour (0xff4d4d4d));
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds().reduced (10));
        logo.setBounds (area);
        area = area.removeFromBottom (24);
        linkButton.setBounds (area.removeFromRight (getWidth() / 4));
        versionLabel.setBounds (area);
    }

private:
    Label versionLabel;
    HyperlinkButton linkButton;

    //==============================================================================
    struct LogoDrawComponent  : public Component,
                                private Timer
    {
        LogoDrawComponent()   : logoPath (MainAppWindow::getJUCELogoPath()), elapsed (0.0f)
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

    private:
        void timerCallback() override
        {
            repaint();
            elapsed += 0.02f;
        }

        Path logoPath;
        float elapsed;
    };

    LogoDrawComponent logo;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntroScreen)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<IntroScreen> demo ("00 Welcome!");
