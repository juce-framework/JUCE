/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             LineSpacingDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Demonstrates the line spacing options of GlyphArrangement.

 dependencies:     juce_core, juce_events, juce_data_structures, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        LineSpacingDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

//==============================================================================
struct LineSpacingDemo final : public Component
{
    LineSpacingDemo()
    {
        lineSpacingSlider.setRange (0.0, 40.0, 0.1);
        lineHeightMultipleSlider.setRange (1.0, 3.0, 0.1);

        Slider* sliders[] { &lineSpacingSlider, &lineHeightMultipleSlider };

        for (auto* s : sliders)
            s->onValueChange = [this] { update(); };

        lineSpacingLabel.attachToComponent (&lineSpacingSlider, false);
        lineHeightMultipleLabel.attachToComponent (&lineHeightMultipleSlider, false);

        demoDescription.setJustificationType (Justification::centredLeft);

        Component* components[] { &lineSpacingLabel,
                                  &lineHeightMultipleLabel,
                                  &lineSpacingSlider,
                                  &lineHeightMultipleSlider,
                                  &demoDescription };

        for (auto* c : components)
            addAndMakeVisible (c);

        setSize (700, 500);
    }

    String justified = "addJustifiedText() places the baseline at the y argument. It will wrap lines "
                       "to enforce the maximum width, but it cannot be "
                       "vertically constrained. The specified Font options will always be respected. "
                       "Alignment and line spacing can be adjusted.";

    String fitted = "addFittedText() places the top of the first line at the y argument. It can be "
                    "vertically constrained. It uses the specified Font as a default, but it will "
                    "reduce the font size and squash the text if necessary to fit it in the available "
                    "space.";

    static constexpr int demoAreaPadding = 10;

    Rectangle<int> getDemoBounds() const
    {
        return getLocalBounds().withTrimmedTop (220).reduced (demoAreaPadding).withTrimmedBottom (40);
    }

    Rectangle<int> getJustifiedBounds() const
    {
        auto bounds = getDemoBounds();
        auto half = bounds.removeFromLeft (bounds.getWidth() / 2);
        half.removeFromRight (25);
        return half;
    }

    Rectangle<int> getFittedBounds() const
    {
        auto bounds = getDemoBounds();
        auto half = bounds.removeFromRight (bounds.getWidth() / 2);
        half.removeFromLeft (25);
        return half;
    }

    void paintGuideLines (Graphics& g)
    {
        const auto textColour = getLookAndFeel().findColour (Label::textColourId);
        const auto lineColour = textColour.withSaturation (0.4f).withRotatedHue (0.1f);

        g.setColour (lineColour);
        const auto demoBounds = getDemoBounds().toFloat();
        g.drawLine (demoBounds.getCentreX() - 90.0f,
                    demoBounds.getY(),
                    demoBounds.getCentreX() + 90.0f,
                    demoBounds.getY(),
                    1.5f);

        const auto jb = getJustifiedBounds().toFloat();
        const auto jbMin = jb.getY() - font.getAscent();
        const auto jbMax = (float) jb.getBottom();
        g.drawLine (jb.getX(), jbMin, jb.getX(), jbMax, 1.5f);
        g.drawLine (jb.getRight(), jbMin, jb.getRight(), jbMax, 1.5f);

        const auto fb = getFittedBounds().toFloat();
        g.drawLine (fb.getX(), fb.getY(), fb.getX(), fb.getBottom(), 1.5f);
        g.drawLine (fb.getRight(), fb.getY(), fb.getRight(), fb.getBottom(), 1.5f);
        g.drawLine (fb.getX(), fb.getBottom(), fb.getX() + 10.0f, fb.getBottom(), 1.5f);
        g.drawLine (fb.getRight(), fb.getBottom(), fb.getRight() - 10.0f, fb.getBottom(), 1.5f);

        g.setColour (textColour);
        g.drawText ("y",
                    Rectangle { 40.0f, 20.0f }.withCentre ({ demoBounds.getCentreX(), demoBounds.getY() - 6.0f }),
                    Justification::centredTop);
    }

    void paint (Graphics& g) override
    {
        paintGuideLines (g);
        paintGlyphArrangement (g);
    }

    void paintGlyphArrangement (Graphics& g)
    {
        g.setColour (getLookAndFeel().findColour (Label::textColourId));
        ga.draw (g);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (demoAreaPadding);

        auto sliderBounds = bounds.removeFromRight (260);
        sliderBounds.removeFromTop (30);
        lineSpacingSlider.setBounds (sliderBounds.removeFromTop (35));
        sliderBounds.removeFromTop (25);
        lineHeightMultipleSlider.setBounds (sliderBounds.removeFromTop (45));

        bounds.removeFromRight (10);
        demoDescription.setBounds (bounds.removeFromTop (lineHeightMultipleSlider.getBottom()));
        update();
    }

    void update()
    {
        ga.clear();

        const auto options = GlyphArrangement::Options{}.withLineSpacing ((float) lineSpacingSlider.getValue())
                                                        .withLineHeightMultiple ((float) lineHeightMultipleSlider.getValue());

        const auto leftBounds = getJustifiedBounds().toFloat();
        ga.addJustifiedText (font,
                             justified,
                             leftBounds.getX(),
                             leftBounds.getY(),
                             leftBounds.getWidth(),
                             Justification::centredTop,
                             options.getLineSpacing());

        const auto rightBounds = getFittedBounds().toFloat();
        ga.addFittedText (font,
                          fitted,
                          rightBounds.getX(),
                          rightBounds.getY(),
                          rightBounds.getWidth(),
                          rightBounds.getHeight(),
                          Justification::centredTop,
                          20,
                          0.0f,
                          options);

        repaint();
    }

    Font font = FontOptions{}.withPointHeight (16.0f);
    GlyphArrangement ga;
    Slider lineSpacingSlider { Slider::LinearHorizontal, Slider::TextBoxLeft };
    Slider lineHeightMultipleSlider { Slider::LinearHorizontal, Slider::TextBoxLeft };
    Label lineSpacingLabel { {}, "Line spacing:" };
    Label lineHeightMultipleLabel { {}, "Line height multiple (fitted text only):" };
    Label demoDescription { {}, "This demo showcases the GlyphArrangement class. Once constructed it "
                                "can be redrawn efficiently. Two important functions are addJustifiedText "
                                "and addFittedText." };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LineSpacingDemo)
};
