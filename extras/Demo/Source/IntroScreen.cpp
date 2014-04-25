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
        }

        void paint (Graphics& g) override
        {
            g.setGradientFill (getGradient());

            g.fillPath (logoPath, RectanglePlacement (RectanglePlacement::stretchToFit)
                                    .getTransformToFit (logoPath.getBounds(), getLocalBounds().toFloat()));
        }

        ColourGradient getGradient() const
        {
            Colour c1 = Colour::fromHSV (hues[0].getValue(), 0.3f, 0.9f, 1.0f);
            Colour c2 = Colour::fromHSV (hues[1].getValue(), 0.3f, 0.9f, 1.0f);
            Colour c3 = Colour::fromHSV (hues[2].getValue(), 0.3f, 0.9f, 1.0f);

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
                                                                        parentArea.reduced (50.0f, 50.0f));

                float scaleFactor = 1.0f + size.getValue() * 0.2f;
                float rotationAngle = (angle.getValue() - 0.5f) * 0.1f;

                transform = transform.scaled (scaleFactor, scaleFactor, parentArea.getCentreX(), parentArea.getCentreY())
                                     .rotated (rotationAngle, parentArea.getCentreX(), parentArea.getCentreY());

                setTransform (transform);
            }

            repaint();
        }

    private:
        void timerCallback() override
        {
            updateTransform();
        }

        Path logoPath;
        BouncingNumber gradientPos[4], hues[3], size, angle;
    };

    LogoDrawComponent logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IntroScreen)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<IntroScreen> demo ("00 Welcome!");
