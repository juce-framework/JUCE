/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             LookAndFeelDemo
 version:          1.0.0
 vendor:           juce
 website:          http://juce.com
 description:      Showcases custom look and feel components.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        LookAndFeelDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Custom Look And Feel subclasss.

    Simply override the methods you need to, anything else will be inherited from the base class.
    It's a good idea not to hard code your colors, use the findColor method along with appropriate
    ColorIds so you can set these on a per-component basis.
*/
struct CustomLookAndFeel    : public LookAndFeel_V4
{
    void drawRoundThumb (Graphics& g, float x, float y, float diameter, Color color, float outlineThickness)
    {
        auto halfThickness = outlineThickness * 0.5f;

        Path p;
        p.addEllipse (x + halfThickness,
                      y + halfThickness,
                      diameter - outlineThickness,
                      diameter - outlineThickness);

        DropShadow (Colors::black, 1, {}).drawForPath (g, p);

        g.setColor (color);
        g.fillPath (p);

        g.setColor (color.brighter());
        g.strokePath (p, PathStrokeType (outlineThickness));
    }

    void drawButtonBackground (Graphics& g, Button& button, const Color& backgroundColor,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        auto baseColor = backgroundColor.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                          .withMultipliedAlpha      (button.isEnabled() ? 0.9f : 0.5f);

        if (isButtonDown || isMouseOverButton)
            baseColor = baseColor.contrasting (isButtonDown ? 0.2f : 0.1f);

        auto flatOnLeft   = button.isConnectedOnLeft();
        auto flatOnRight  = button.isConnectedOnRight();
        auto flatOnTop    = button.isConnectedOnTop();
        auto flatOnBottom = button.isConnectedOnBottom();

        auto width  = button.getWidth()  - 1.0f;
        auto height = button.getHeight() - 1.0f;

        if (width > 0 && height > 0)
        {
            auto cornerSize = jmin (15.0f, jmin (width, height) * 0.45f);
            auto lineThickness = cornerSize    * 0.1f;
            auto halfThickness = lineThickness * 0.5f;

            Path outline;
            outline.addRoundedRectangle (0.5f + halfThickness, 0.5f + halfThickness, width - lineThickness, height - lineThickness,
                                         cornerSize, cornerSize,
                                         ! (flatOnLeft  || flatOnTop),
                                         ! (flatOnRight || flatOnTop),
                                         ! (flatOnLeft  || flatOnBottom),
                                         ! (flatOnRight || flatOnBottom));

            auto outlineColor = button.findColor (button.getToggleState() ? TextButton::textColorOnId
                                                                            : TextButton::textColorOffId);

            g.setColor (baseColor);
            g.fillPath (outline);

            if (! button.getToggleState())
            {
                g.setColor (outlineColor);
                g.strokePath (outline, PathStrokeType (lineThickness));
            }
        }
    }

    void drawTickBox (Graphics& g, Component& component,
                      float x, float y, float w, float h,
                      bool ticked,
                      bool isEnabled,
                      bool isMouseOverButton,
                      bool isButtonDown) override
    {
        auto boxSize = w * 0.7f;

        auto isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());

        auto color = component.findColor (TextButton::buttonColorId)
                               .withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f);

        drawRoundThumb (g, x, y + (h - boxSize) * 0.5f, boxSize, color,
                        isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

        if (ticked)
        {
            g.setColor (isEnabled ? findColor (TextButton::buttonOnColorId) : Colors::gray);

            auto scale = 9.0f;
            auto trans = AffineTransform::scale (w / scale, h / scale).translated (x - 2.5f, y + 1.0f);

            g.fillPath (LookAndFeel_V4::getTickShape (6.0f), trans);
        }
    }

    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const Slider::SliderStyle style, Slider& slider) override
    {
        auto sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

        auto isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());

        auto knobColor = slider.findColor (Slider::thumbColorId)
                                .withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                                .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            float kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = x + width * 0.5f;
                ky = sliderPos;
            }
            else
            {
                kx = sliderPos;
                ky = y + height * 0.5f;
            }

            auto outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

            drawRoundThumb (g,
                            kx - sliderRadius,
                            ky - sliderRadius,
                            sliderRadius * 2.0f,
                            knobColor, outlineThickness);
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle style, Slider& slider) override
    {
        g.fillAll (slider.findColor (Slider::backgroundColorId));

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            Path p;

            if (style == Slider::LinearBarVertical)
                p.addRectangle ((float) x, sliderPos, (float) width, 1.0f + height - sliderPos);
            else
                p.addRectangle ((float) x, (float) y, sliderPos - x, (float) height);

            auto baseColor = slider.findColor (Slider::rotarySliderFillColorId)
                                    .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                    .withMultipliedAlpha (0.8f);

            g.setColor (baseColor);
            g.fillPath (p);

            auto lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        }
        else
        {
            drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb      (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height,
                                     float /*sliderPos*/,
                                     float /*minSliderPos*/,
                                     float /*maxSliderPos*/,
                                     const Slider::SliderStyle /*style*/, Slider& slider) override
    {
        auto sliderRadius = getSliderThumbRadius (slider) - 5.0f;
        Path on, off;

        if (slider.isHorizontal())
        {
            auto iy = y + height * 0.5f - sliderRadius * 0.5f;
            Rectangle<float> r (x - sliderRadius * 0.5f, iy, width + sliderRadius, sliderRadius);
            auto onW = r.getWidth() * ((float) slider.valueToProportionOfLength (slider.getValue()));

            on.addRectangle (r.removeFromLeft (onW));
            off.addRectangle (r);
        }
        else
        {
            auto ix = x + width * 0.5f - sliderRadius * 0.5f;
            Rectangle<float> r (ix, y - sliderRadius * 0.5f, sliderRadius, height + sliderRadius);
            auto onH = r.getHeight() * ((float) slider.valueToProportionOfLength (slider.getValue()));

            on.addRectangle (r.removeFromBottom (onH));
            off.addRectangle (r);
        }

        g.setColor (slider.findColor (Slider::rotarySliderFillColorId));
        g.fillPath (on);

        g.setColor (slider.findColor (Slider::trackColorId));
        g.fillPath (off);
    }

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
    {
        auto radius = jmin (width / 2, height / 2) - 2.0f;
        auto centerX = x + width  * 0.5f;
        auto centerY = y + height * 0.5f;
        auto rx = centerX - radius;
        auto ry = centerY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        if (slider.isEnabled())
            g.setColor (slider.findColor (Slider::rotarySliderFillColorId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColor (Color (0x80808080));

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, 0.0);
            g.fillPath (filledArc);
        }

        {
            auto lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
            Path outlineArc;
            outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.0);
            g.strokePath (outlineArc, PathStrokeType (lineThickness));
        }
    }
};

//==============================================================================
/** Another really simple look and feel that is very flat and square.

    This inherits from CustomLookAndFeel above for the linear bar and slider backgrounds.
*/
struct SquareLookAndFeel    : public CustomLookAndFeel
{
    void drawButtonBackground (Graphics& g, Button& button, const Color& backgroundColor,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        auto baseColor = backgroundColor.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                          .withMultipliedAlpha      (button.isEnabled() ? 0.9f : 0.5f);

        if (isButtonDown || isMouseOverButton)
            baseColor = baseColor.contrasting (isButtonDown ? 0.2f : 0.1f);

        auto width  = button.getWidth()  - 1.0f;
        auto height = button.getHeight() - 1.0f;

        if (width > 0 && height > 0)
        {
            g.setGradientFill (ColorGradient::vertical (baseColor, 0.0f,
                                                         baseColor.darker (0.1f), height));

            g.fillRect (button.getLocalBounds());
        }
    }

    void drawTickBox (Graphics& g, Component& component,
                      float x, float y, float w, float h,
                      bool ticked,
                      bool isEnabled,
                      bool /*isMouseOverButton*/,
                      bool /*isButtonDown*/) override
    {
        auto boxSize = w * 0.7f;

        auto isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());

        auto color = component.findColor (TextButton::buttonOnColorId)
                               .withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                               .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f);

        g.setColor (color);

        Rectangle<float> r (x, y + (h - boxSize) * 0.5f, boxSize, boxSize);
        g.fillRect (r);

        if (ticked)
        {
            auto tickPath = LookAndFeel_V4::getTickShape (6.0f);
            g.setColor (isEnabled ? findColor (TextButton::buttonColorId) : Colors::gray);

            auto transform = RectanglePlacement (RectanglePlacement::centered)
                               .getTransformToFit (tickPath.getBounds(),
                                                   r.reduced (r.getHeight() * 0.05f));

            g.fillPath (tickPath, transform);
        }
    }

    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const Slider::SliderStyle style, Slider& slider) override
    {
        auto sliderRadius = (float) getSliderThumbRadius (slider);

        bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());

        auto knobColor = slider.findColor (Slider::rotarySliderFillColorId)
                                .withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                                .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);

        g.setColor (knobColor);

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            float kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = x + width * 0.5f;
                ky = sliderPos;
                g.fillRect (Rectangle<float> (kx - sliderRadius, ky - 2.5f, sliderRadius * 2.0f, 5.0f));
            }
            else
            {
                kx = sliderPos;
                ky = y + height * 0.5f;
                g.fillRect (Rectangle<float> (kx - 2.5f, ky - sliderRadius, 5.0f, sliderRadius * 2.0f));
            }
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
    {
        auto diameter = jmin (width, height) - 4.0f;
        auto radius = (diameter / 2.0f) * std::cos (MathConstants<float>::pi / 4.0f);
        auto centerX = x + width  * 0.5f;
        auto centerY = y + height * 0.5f;
        auto rx = centerX - radius;
        auto ry = centerY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        auto baseColor = slider.isEnabled() ? slider.findColor (Slider::rotarySliderFillColorId).withAlpha (isMouseOver ? 0.8f : 1.0f)
                                             : Color (0x80808080);

        Rectangle<float> r (rx, ry, rw, rw);
        auto transform = AffineTransform::rotation (angle, r.getCenterX(), r.getCenterY());

        auto x1 = r.getTopLeft()   .getX();
        auto y1 = r.getTopLeft()   .getY();
        auto x2 = r.getBottomLeft().getX();
        auto y2 = r.getBottomLeft().getY();

        transform.transformPoints (x1, y1, x2, y2);

        g.setGradientFill (ColorGradient (baseColor, x1, y1,
                                           baseColor.darker (0.1f), x2, y2,
                                           false));

        Path knob;
        knob.addRectangle (r);
        g.fillPath (knob, transform);

        Path needle;
        auto r2 = r * 0.1f;
        needle.addRectangle (r2.withPosition ({ r.getCenterX() - (r2.getWidth() / 2.0f), r.getY() }));

        g.setColor (slider.findColor (Slider::rotarySliderOutlineColorId));
        g.fillPath (needle, AffineTransform::rotation (angle, r.getCenterX(), r.getCenterY()));
    }
};

//==============================================================================
struct LookAndFeelDemoComponent  : public Component
{
    LookAndFeelDemoComponent()
    {
        addAndMakeVisible (rotarySlider);
        rotarySlider.setValue (2.5);

        addAndMakeVisible (verticalSlider);
        verticalSlider.setValue (6.2);

        addAndMakeVisible (barSlider);
        barSlider.setValue (4.5);

        addAndMakeVisible (incDecSlider);
        incDecSlider.setRange (0.0, 10.0, 1.0);
        incDecSlider.setIncDecButtonsMode (Slider::incDecButtonsDraggable_Horizontal);

        addAndMakeVisible (button1);

        addAndMakeVisible (button2);
        button2.setClickingTogglesState (true);
        button2.setToggleState (true, dontSendNotification);

        addAndMakeVisible (button3);

        addAndMakeVisible (button4);
        button4.setToggleState (true, dontSendNotification);

        for (int i = 0; i < 3; ++i)
        {
            auto* b = radioButtons.add (new TextButton ("Button " + String (i + 1)));

            addAndMakeVisible (b);
            b->setRadioGroupId (42);
            b->setClickingTogglesState (true);

            switch (i)
            {
                case 0:     b->setConnectedEdges (Button::ConnectedOnRight);                            break;
                case 1:     b->setConnectedEdges (Button::ConnectedOnRight + Button::ConnectedOnLeft);  break;
                case 2:     b->setConnectedEdges (Button::ConnectedOnLeft);                             break;
                default:    break;
            }
        }

        radioButtons.getUnchecked (2)->setToggleState (true, dontSendNotification);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        auto row = area.removeFromTop (100);

        rotarySlider  .setBounds (row.removeFromLeft (100).reduced (5));
        verticalSlider.setBounds (row.removeFromLeft (100).reduced (5));
        barSlider     .setBounds (row.removeFromLeft (100).reduced (5, 25));
        incDecSlider  .setBounds (row.removeFromLeft (100).reduced (5, 28));

        row = area.removeFromTop (100);
        button1.setBounds (row.removeFromLeft (100).reduced (5));

        auto row2 = row.removeFromTop (row.getHeight() / 2).reduced (0, 10);
        button2.setBounds (row2.removeFromLeft (100).reduced (5, 0));
        button3.setBounds (row2.removeFromLeft (100).reduced (5, 0));
        button4.setBounds (row2.removeFromLeft (100).reduced (5, 0));

        row2 = (row.removeFromTop (row2.getHeight() + 20).reduced (5, 10));

        for (auto* b : radioButtons)
            b->setBounds (row2.removeFromLeft (100));
    }

    Slider rotarySlider    { Slider::RotaryHorizontalVerticalDrag, Slider::NoTextBox},
           verticalSlider  { Slider::LinearVertical, Slider::NoTextBox },
           barSlider       { Slider::LinearBar, Slider::NoTextBox },
           incDecSlider    { Slider::IncDecButtons, Slider::TextBoxBelow };

    TextButton button1  { "Hello World!" },
               button2  { "Hello World!" },
               button3  { "Hello World!" };

    ToggleButton button4 { "Toggle Me" };

    OwnedArray<TextButton> radioButtons;
};

//==============================================================================
class LookAndFeelDemo   : public Component
{
public:
    LookAndFeelDemo()
    {
        descriptionLabel.setMinimumHorizontalScale (1.0f);
        descriptionLabel.setText ("This demonstrates how to create a custom look and feel by overriding only the desired methods.\n\n"
                                  "Components can have their look and feel individually assigned or they will inherit it from their parent. "
                                  "Colors work in a similar way, they can be set for individual components or a look and feel as a whole.",
                                  dontSendNotification);

        addAndMakeVisible (descriptionLabel);
        addAndMakeVisible (lafBox);
        addAndMakeVisible (demoComp);

        addLookAndFeel (new LookAndFeel_V1(), "LookAndFeel_V1");
        addLookAndFeel (new LookAndFeel_V2(), "LookAndFeel_V2");
        addLookAndFeel (new LookAndFeel_V3(), "LookAndFeel_V3");
        addLookAndFeel (new LookAndFeel_V4(), "LookAndFeel_V4 (Dark)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getMidnightColorScheme()), "LookAndFeel_V4 (Midnight)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getGrayColorScheme()),     "LookAndFeel_V4 (Gray)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getLightColorScheme()),    "LookAndFeel_V4 (Light)");

        auto* claf = new CustomLookAndFeel();
        addLookAndFeel (claf, "Custom Look And Feel");
        setupCustomLookAndFeelColors (*claf);

        auto* slaf = new SquareLookAndFeel();
        addLookAndFeel (slaf, "Square Look And Feel");
        setupSquareLookAndFeelColors (*slaf);

        lafBox.onChange = [this] { setAllLookAndFeels (lookAndFeels[lafBox.getSelectedItemIndex()]); };
        lafBox.setSelectedItemIndex (3);

        addAndMakeVisible (randomButton);
        randomButton.onClick = [this] { lafBox.setSelectedItemIndex (Random().nextInt (lafBox.getNumItems())); };

        setSize (500, 500);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                           Color::grayLevel (0.4f)));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);

        descriptionLabel.setBounds (r.removeFromTop (150));
        lafBox          .setBounds (r.removeFromTop (22).removeFromLeft (250));
        randomButton    .setBounds (lafBox.getBounds().withX (lafBox.getRight() + 20).withWidth (140));
        demoComp        .setBounds (r.withTrimmedTop (10));
    }

private:
    Label descriptionLabel;
    ComboBox lafBox;
    TextButton randomButton  { "Assign Randomly" };
    OwnedArray<LookAndFeel> lookAndFeels;
    LookAndFeelDemoComponent demoComp;

    void addLookAndFeel (LookAndFeel* laf, const String& name)
    {
        lookAndFeels.add (laf);
        lafBox.addItem (name, lafBox.getNumItems() + 1);
    }

    void setupCustomLookAndFeelColors (LookAndFeel& laf)
    {
        laf.setColor (Slider::thumbColorId,               Color::grayLevel (0.95f));
        laf.setColor (Slider::textBoxOutlineColorId,      Colors::transparentWhite);
        laf.setColor (Slider::rotarySliderFillColorId,    Color (0xff00b5f6));
        laf.setColor (Slider::rotarySliderOutlineColorId, Colors::white);

        laf.setColor (TextButton::buttonColorId,  Colors::white);
        laf.setColor (TextButton::textColorOffId, Color (0xff00b5f6));

        laf.setColor (TextButton::buttonOnColorId, laf.findColor (TextButton::textColorOffId));
        laf.setColor (TextButton::textColorOnId,   laf.findColor (TextButton::buttonColorId));
    }

    void setupSquareLookAndFeelColors (LookAndFeel& laf)
    {
        auto baseColor = Colors::red;

        laf.setColor (Slider::thumbColorId,               Color::grayLevel (0.95f));
        laf.setColor (Slider::textBoxOutlineColorId,      Colors::transparentWhite);
        laf.setColor (Slider::rotarySliderFillColorId,    baseColor);
        laf.setColor (Slider::rotarySliderOutlineColorId, Colors::white);
        laf.setColor (Slider::trackColorId,               Colors::black);

        laf.setColor (TextButton::buttonColorId,  Colors::white);
        laf.setColor (TextButton::textColorOffId, baseColor);

        laf.setColor (TextButton::buttonOnColorId, laf.findColor (TextButton::textColorOffId));
        laf.setColor (TextButton::textColorOnId,   laf.findColor (TextButton::buttonColorId));
    }

    void setAllLookAndFeels (LookAndFeel* laf)
    {
        for (auto* child : demoComp.getChildren())
            child->setLookAndFeel (laf);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeelDemo)
};
