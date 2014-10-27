/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "JuceDemoHeader.h"


//==============================================================================
/**
*/
class IntroScreen  : public Component
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
        g.fillAll (Colour::greyLevel (0.16f));
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds().reduced (10).removeFromBottom (24));
        linkButton.setBounds (area.removeFromRight (getWidth() / 4));
        versionLabel.setBounds (area);
        logo.updateTransform();
    }

private:
    Label versionLabel;
    HyperlinkButton linkButton;

    //==============================================================================
    struct LogoDrawComponent  : public Component,
                                private Timer
    {
        LogoDrawComponent()   : logoPath (MainAppWindow::getJUCELogoPath())
        {
            setBounds (logoPath.getBounds().withPosition (Point<float>()).getSmallestIntegerContainer());

            startTimer (1000 / 60); // try to repaint at 60 fps
            
            elapsed = 0.0f;
        }

        void paint (Graphics& g) override
        {
            //g.setGradientFill (getGradient());
            g.setColour (Colour::greyLevel (0.3f));
            
            float waveStep = 10.0f;

            for (int i = 0; i < getWidth()/waveStep; ++i)
            {
                float x = waveStep*0.5f + waveStep * i;
                float y1 = getHeight() * 0.5f + getHeight() * 0.05f * sin (i * 0.38f + elapsed);
                float y2 = getHeight() * 0.5 + getHeight() * 0.1f * sin (i * 0.2f + elapsed * 2.0f);
                g.drawLine (x, y1, x, y2, 2.0f);
                g.fillEllipse (x - waveStep * 0.3f, y1 - waveStep * 0.3f, waveStep*0.6f, waveStep*0.6f);
                g.fillEllipse (x - waveStep * 0.3f, y2 - waveStep * 0.3f, waveStep*0.6f, waveStep*0.6f);
            }
            
            g.setColour (Colours::orange);
            g.fillPath (logoPath, RectanglePlacement (RectanglePlacement::stretchToFit)
                                    .getTransformToFit (logoPath.getBounds(), getLocalBounds().toFloat().reduced (30, 30)));
        }

        ColourGradient getGradient() const
        {
            Colour c1 = Colour::fromHSV (hues[0].getValue(), 0.9f, 0.9f, 1.0f);
            Colour c2 = Colour::fromHSV (hues[1].getValue(), 0.9f, 0.9f, 1.0f);
            Colour c3 = Colour::fromHSV (hues[2].getValue(), 0.9f, 0.9f, 1.0f);

            float x1 = getWidth()  * gradientPos[0].getValue();
            float x2 = getWidth()  * gradientPos[1].getValue();
            float y1 = getHeight() * gradientPos[2].getValue();
            float y2 = getHeight() * gradientPos[3].getValue();

            ColourGradient gradient (c1, x1, y1,
                                     c2, x2, y2, false);

            gradient.addColour (0.5, c3);
            return gradient;
        }

        void updateTransform()
        {
            if (Component* parent = getParentComponent())
            {
                const Rectangle<float> parentArea (parent->getLocalBounds().toFloat());

                AffineTransform transform = RectanglePlacement (RectanglePlacement::centred)
                                                    .getTransformToFit (getLocalBounds().toFloat(),
                                                                        parentArea);
                setTransform (transform);
            }

            repaint();
        }

    private:
        void timerCallback() override
        {
            //updateTransform();
            repaint();
            elapsed += 0.01f;
        }

        Path logoPath;
        BouncingNumber gradientPos[4], hues[3], size, angle;
        float elapsed;
    };

    LogoDrawComponent logo;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntroScreen)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<IntroScreen> demo ("00 Welcome!");
