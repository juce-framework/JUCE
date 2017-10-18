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

#include "../JuceDemoHeader.h"


//==============================================================================
/** Custom Look And Feel subclasss.

    Simply override the methods you need to, anything else will be inherited from the base class.
    It's a good idea not to hard code your colours, use the findColour method along with appropriate
    ColourIds so you can set these on a per-component basis.
 */
struct CustomLookAndFeel    : public LookAndFeel_V4
{
    void drawRoundThumb (Graphics& g, float x, float y, float diameter, Colour colour, float outlineThickness)
    {
        const Rectangle<float> a (x, y, diameter, diameter);
        auto halfThickness = outlineThickness * 0.5f;

        Path p;
        p.addEllipse (x + halfThickness,
                      y + halfThickness,
                      diameter - outlineThickness,
                      diameter - outlineThickness);

        DropShadow (Colours::black, 1, {}).drawForPath (g, p);

        g.setColour (colour);
        g.fillPath (p);

        g.setColour (colour.brighter());
        g.strokePath (p, PathStrokeType (outlineThickness));
    }

    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                          .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f);

        if (isButtonDown || isMouseOverButton)
            baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);

        bool flatOnLeft   = button.isConnectedOnLeft();
        bool flatOnRight  = button.isConnectedOnRight();
        bool flatOnTop    = button.isConnectedOnTop();
        bool flatOnBottom = button.isConnectedOnBottom();

        auto width  = button.getWidth() - 1.0f;
        auto height = button.getHeight() - 1.0f;

        if (width > 0 && height > 0)
        {
            auto cornerSize = jmin (15.0f, jmin (width, height) * 0.45f);
            auto lineThickness = cornerSize * 0.1f;
            auto halfThickness = lineThickness * 0.5f;

            Path outline;
            outline.addRoundedRectangle (0.5f + halfThickness, 0.5f + halfThickness, width - lineThickness, height - lineThickness,
                                         cornerSize, cornerSize,
                                         ! (flatOnLeft  || flatOnTop),
                                         ! (flatOnRight || flatOnTop),
                                         ! (flatOnLeft  || flatOnBottom),
                                         ! (flatOnRight || flatOnBottom));

            auto outlineColour = button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                                            : TextButton::textColourOffId);

            g.setColour (baseColour);
            g.fillPath (outline);

            if (! button.getToggleState())
            {
                g.setColour (outlineColour);
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
        const float boxSize = w * 0.7f;

        bool isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());

        auto colour = component.findColour (TextButton::buttonColourId)
                           .withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                           .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f);

        drawRoundThumb (g, x, y + (h - boxSize) * 0.5f, boxSize, colour,
                        isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

        if (ticked)
        {
            const Path tick (LookAndFeel_V4::getTickShape (6.0f));
            g.setColour (isEnabled ? findColour (TextButton::buttonOnColourId) : Colours::grey);

            const float scale = 9.0f;
            const AffineTransform trans (AffineTransform::scale (w / scale, h / scale)
                                             .translated (x - 2.5f, y + 1.0f));
            g.fillPath (tick, trans);
        }
    }

    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const Slider::SliderStyle style, Slider& slider) override
    {
        auto sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

        bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());

        auto knobColour = slider.findColour (Slider::thumbColourId)
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
                            knobColour, outlineThickness);
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
        g.fillAll (slider.findColour (Slider::backgroundColourId));

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            Path p;

            if (style == Slider::LinearBarVertical)
                p.addRectangle ((float) x, sliderPos, (float) width, 1.0f + height - sliderPos);
            else
                p.addRectangle ((float) x, (float) y, sliderPos - x, (float) height);

            auto baseColour = slider.findColour (Slider::rotarySliderFillColourId)
                                  .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                  .withMultipliedAlpha (0.8f);

            g.setColour (baseColour);
            g.fillPath (p);

            auto lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        }
        else
        {
            drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
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

        g.setColour (slider.findColour (Slider::rotarySliderFillColourId));
        g.fillPath (on);

        g.setColour (slider.findColour (Slider::trackColourId));
        g.fillPath (off);
    }

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
    {
        auto radius = jmin (width / 2, height / 2) - 2.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

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
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                          .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f);

        if (isButtonDown || isMouseOverButton)
            baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);

        auto width  = button.getWidth() - 1.0f;
        auto height = button.getHeight() - 1.0f;

        if (width > 0 && height > 0)
        {
            g.setGradientFill (ColourGradient (baseColour, 0.0f, 0.0f,
                                               baseColour.darker (0.1f), 0.0f, height,
                                               false));

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

        bool isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());

        auto colour = component.findColour (TextButton::buttonOnColourId)
                         .withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                         .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f);

        g.setColour (colour);

        Rectangle<float> r (x, y + (h - boxSize) * 0.5f, boxSize, boxSize);
        g.fillRect (r);

        if (ticked)
        {
            auto tickPath = LookAndFeel_V4::getTickShape (6.0f);
            g.setColour (isEnabled ? findColour (TextButton::buttonColourId) : Colours::grey);

            auto transform = RectanglePlacement (RectanglePlacement::centred)
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

        auto knobColour = slider.findColour (Slider::rotarySliderFillColourId)
                            .withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                            .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);

        g.setColour (knobColour);

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
        auto radius = (diameter / 2.0f) * std::cos (float_Pi / 4.0f);
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        auto baseColour = slider.isEnabled() ? slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 0.8f : 1.0f)
                                             : Colour (0x80808080);

        Rectangle<float> r (rx, ry, rw, rw);
        auto transform = AffineTransform::rotation (angle, r.getCentreX(), r.getCentreY());

        auto x1 = r.getTopLeft().getX();
        auto y1 = r.getTopLeft().getY();
        auto x2 = r.getBottomLeft().getX();
        auto y2 = r.getBottomLeft().getY();

        transform.transformPoints (x1, y1, x2, y2);

        g.setGradientFill (ColourGradient (baseColour, x1, y1,
                                           baseColour.darker (0.1f), x2, y2,
                                           false));

        Path knob;
        knob.addRectangle (r);
        g.fillPath (knob, transform);

        Path needle;
        auto r2 = r * 0.1f;
        needle.addRectangle (r2.withPosition ({ r.getCentreX() - (r2.getWidth() / 2.0f), r.getY() }));

        g.setColour (slider.findColour (Slider::rotarySliderOutlineColourId));
        g.fillPath (needle, AffineTransform::rotation (angle, r.getCentreX(), r.getCentreY()));
    }
};

//==============================================================================
struct LookAndFeelDemoComponent  : public Component
{
    LookAndFeelDemoComponent()
    {
        addAndMakeVisible (rotarySlider);
        rotarySlider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        rotarySlider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        rotarySlider.setValue (2.5);

        addAndMakeVisible (verticalSlider);
        verticalSlider.setSliderStyle (Slider::LinearVertical);
        verticalSlider.setTextBoxStyle (Slider::NoTextBox, false, 90, 20);
        verticalSlider.setValue (6.2);

        addAndMakeVisible (barSlider);
        barSlider.setSliderStyle (Slider::LinearBar);
        barSlider.setValue (4.5);

        addAndMakeVisible (incDecSlider);
        incDecSlider.setSliderStyle (Slider::IncDecButtons);
        incDecSlider.setRange (0.0, 10.0, 1.0);
        incDecSlider.setIncDecButtonsMode (Slider::incDecButtonsDraggable_Horizontal);
        incDecSlider.setTextBoxStyle (Slider::TextBoxBelow, false, 90, 20);

        addAndMakeVisible (button1);
        button1.setButtonText ("Hello World!");

        addAndMakeVisible (button2);
        button2.setButtonText ("Hello World!");
        button2.setClickingTogglesState (true);
        button2.setToggleState (true, dontSendNotification);

        addAndMakeVisible (button3);
        button3.setButtonText ("Hello World!");

        addAndMakeVisible (button4);
        button4.setButtonText ("Toggle Me");
        button4.setToggleState (true, dontSendNotification);

        for (int i = 0; i < 3; ++i)
        {
            auto* b = radioButtons.add (new TextButton());
            addAndMakeVisible (b);
            b->setRadioGroupId (42);
            b->setClickingTogglesState (true);
            b->setButtonText ("Button " + String (i + 1));

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

        rotarySlider.setBounds (row.removeFromLeft (100).reduced (5));
        verticalSlider.setBounds (row.removeFromLeft (100).reduced (5));
        barSlider.setBounds (row.removeFromLeft (100).reduced (5, 25));
        incDecSlider.setBounds (row.removeFromLeft (100).reduced (5, 28));

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

    Slider rotarySlider, verticalSlider, barSlider, incDecSlider;
    TextButton button1, button2, button3;
    ToggleButton button4;
    OwnedArray<TextButton> radioButtons;
};

//==============================================================================
class LookAndFeelDemo   : public Component,
                          private ComboBox::Listener,
                          private Button::Listener
{
public:
    LookAndFeelDemo()
    {
        descriptionLabel.setMinimumHorizontalScale (1.0f);
        descriptionLabel.setText ("This demonstrates how to create a custom look and feel by overriding only the desired methods.\n\n"
                                  "Components can have their look and feel individually assigned or they will inherit it from their parent. "
                                  "Colours work in a similar way, they can be set for individual components or a look and feel as a whole.",
                                  dontSendNotification);

        addAndMakeVisible (descriptionLabel);
        addAndMakeVisible (lafBox);
        addAndMakeVisible (demoComp);

        addLookAndFeel (new LookAndFeel_V1(), "LookAndFeel_V1");
        addLookAndFeel (new LookAndFeel_V2(), "LookAndFeel_V2");
        addLookAndFeel (new LookAndFeel_V3(), "LookAndFeel_V3");
        addLookAndFeel (new LookAndFeel_V4(), "LookAndFeel_V4 (Dark)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getMidnightColourScheme()), "LookAndFeel_V4 (Midnight)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getGreyColourScheme()), "LookAndFeel_V4 (Grey)");
        addLookAndFeel (new LookAndFeel_V4 (LookAndFeel_V4::getLightColourScheme()), "LookAndFeel_V4 (Light)");

        auto claf = new CustomLookAndFeel();
        addLookAndFeel (claf, "Custom Look And Feel");
        setupCustomLookAndFeelColours (*claf);

        auto slaf = new SquareLookAndFeel();
        addLookAndFeel (slaf, "Square Look And Feel");
        setupSquareLookAndFeelColours (*slaf);

        lafBox.addListener (this);
        lafBox.setSelectedItemIndex (3);

        addAndMakeVisible (randomButton);
        randomButton.setButtonText ("Assign Randomly");
        randomButton.addListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colour::greyLevel (0.4f)));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);

        demoComp.setBounds (r);
        descriptionLabel.setBounds (r.removeFromTop (200));
        lafBox.setBounds (r.removeFromTop (22).removeFromLeft (250));
        randomButton.setBounds (lafBox.getBounds().withX (lafBox.getRight() + 20).withWidth (140));

        demoComp.setBounds (r.withTrimmedTop (10));
    }

private:
    Label descriptionLabel;
    ComboBox lafBox;
    TextButton randomButton;
    OwnedArray<LookAndFeel> lookAndFeels;
    LookAndFeelDemoComponent demoComp;

    void addLookAndFeel (LookAndFeel* laf, const String& name)
    {
        lookAndFeels.add (laf);
        lafBox.addItem (name, lafBox.getNumItems() + 1);
    }

    void setupCustomLookAndFeelColours (LookAndFeel& laf)
    {
        laf.setColour (Slider::thumbColourId, Colour::greyLevel (0.95f));
        laf.setColour (Slider::textBoxOutlineColourId, Colours::transparentWhite);
        laf.setColour (Slider::rotarySliderFillColourId, Colour (0xff00b5f6));
        laf.setColour (Slider::rotarySliderOutlineColourId, Colours::white);

        laf.setColour (TextButton::buttonColourId, Colours::white);
        laf.setColour (TextButton::textColourOffId, Colour (0xff00b5f6));

        laf.setColour (TextButton::buttonOnColourId, laf.findColour (TextButton::textColourOffId));
        laf.setColour (TextButton::textColourOnId, laf.findColour (TextButton::buttonColourId));
    }

    void setupSquareLookAndFeelColours (LookAndFeel& laf)
    {
        auto baseColour = Colours::red;

        laf.setColour (Slider::thumbColourId, Colour::greyLevel (0.95f));
        laf.setColour (Slider::textBoxOutlineColourId, Colours::transparentWhite);
        laf.setColour (Slider::rotarySliderFillColourId, baseColour);
        laf.setColour (Slider::rotarySliderOutlineColourId, Colours::white);
        laf.setColour (Slider::trackColourId, Colours::black);

        laf.setColour (TextButton::buttonColourId, Colours::white);
        laf.setColour (TextButton::textColourOffId, baseColour);

        laf.setColour (TextButton::buttonOnColourId, laf.findColour (TextButton::textColourOffId));
        laf.setColour (TextButton::textColourOnId, laf.findColour (TextButton::buttonColourId));
    }

    void setAllLookAndFeels (LookAndFeel* laf)
    {
        for (auto* child : demoComp.getChildren())
            child->setLookAndFeel (laf);
    }

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged == &lafBox)
            setAllLookAndFeels (lookAndFeels[lafBox.getSelectedItemIndex()]);
    }

    void buttonClicked (Button* b) override
    {
        if (b == &randomButton)
            lafBox.setSelectedItemIndex (Random::getSystemRandom().nextInt (lafBox.getNumItems()));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeelDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<LookAndFeelDemo> demo ("10 Components: Look And Feel");
