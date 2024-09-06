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

 name:             AnimationEasingDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Application for comparing animation easings.

 dependencies:     juce_gui_basics, juce_animation

 exporters:        xcode_mac, vs2022, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AnimationEasingDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

struct AnimationEasingDemoConstants
{
    static constexpr auto smallGapSize = 5;
    static constexpr auto mediumGapSize = smallGapSize * 2;
    static constexpr auto largeGapSize = mediumGapSize * 2;
    static constexpr auto defaultComponentHeight = 35;
    inline static const auto cp1AccentColour = Colour { 0xffff0088 };
    inline static const auto cp2AccentColour = Colour { 0xff00aabb };
};

struct AnimationEasingDemoHelpers
{
    static int calculateSectionSize (int originalSize, int numberOfSections, int gapSize)
    {
        const auto totalGapSize = (float) gapSize * ((float) numberOfSections - 1.0f);
        const auto totalSizeOfAllSections = (float) originalSize - totalGapSize;
        return roundToInt (totalSizeOfAllSections / (float) numberOfSections);
    }

    static void layoutComponentsHorizontally (Rectangle<int> bounds, const std::vector<Component*>& components, int gapSize = AnimationEasingDemoConstants::smallGapSize)
    {
        const auto componentWidth = calculateSectionSize (bounds.getWidth(), (int) components.size(), gapSize);

        for (auto* component : components)
        {
            const auto newComponentBounds = bounds.removeFromLeft (componentWidth);

            if (component != nullptr)
                component->setBounds (newComponentBounds);

            bounds.removeFromLeft (gapSize);
        }
    }

    static void layoutComponentsVertically (Rectangle<int> bounds, const std::vector<Component*>& components, int gapSize = AnimationEasingDemoConstants::smallGapSize)
    {
        const auto componentHeight = calculateSectionSize (bounds.getHeight(), (int) components.size(), gapSize);

        for (auto* component : components)
        {
            const auto newComponentBounds = bounds.removeFromTop (componentHeight);

            if (component != nullptr)
                component->setBounds (newComponentBounds);

            bounds.removeFromTop (gapSize);
        }
    }

    static void layoutComponentsVerticallyOrHorizontally (Rectangle<int> bounds, const std::vector<Component*>& components, int gapSize = AnimationEasingDemoConstants::smallGapSize)
    {
        if (bounds.getHeight() > bounds.getWidth())
            layoutComponentsVertically (bounds, components, gapSize);
        else
            layoutComponentsHorizontally (bounds, components, gapSize);
    }
};

struct AnimationSettings
{
    Value shouldAnimatePosition;
    Value shouldAnimateSize;
    Value shouldAnimateAlpha ;
    Value durationMs;
};

class AnimationSettingsComponent final : public Component
{
public:
    explicit AnimationSettingsComponent (const AnimationSettings& settingsIn)
    {
        playbackControls.button.onClick = [&] { NullCheckedInvocation::invoke (onAnimate); };

        durationControls.slider.getValueObject().referTo (settingsIn.durationMs);
        playbackControls.positionToggle.getToggleStateValue().referTo (settingsIn.shouldAnimatePosition);
        playbackControls.sizeToggle.getToggleStateValue().referTo (settingsIn.shouldAnimateSize);
        playbackControls.alphaToggle.getToggleStateValue().referTo (settingsIn.shouldAnimateAlpha);

        addAndMakeVisible (durationControls);
        addAndMakeVisible (playbackControls);
    }

    void resized() final
    {
        AnimationEasingDemoHelpers::layoutComponentsVertically (getLocalBounds(), { &playbackControls, &durationControls });
    }

    std::function<void()> onAnimate;

private:
    struct DurationControls final : public Component
    {
        DurationControls()
        {
            slider.setRange (50.0, 5000.0, 10.0);
            slider.setValue (1000.0);
            slider.setTextValueSuffix (" ms");

            addAndMakeVisible (label);
            addAndMakeVisible (slider);
        }

        void resized() final
        {
            auto bounds = getLocalBounds();
            const auto labelWidth = GlyphArrangement::getStringWidthInt (label.getFont(), label.getText())
                                  + AnimationEasingDemoConstants::largeGapSize;
            label.setBounds (bounds.removeFromLeft (labelWidth));
            slider.setBounds (bounds);
        }

        Label label { "", "Duration:" };
        Slider slider { Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight };
    };

    struct PlaybackControls final : public Component
    {
        PlaybackControls()
        {
            addAndMakeVisible (button);
            addAndMakeVisible (positionToggle);
            addAndMakeVisible (sizeToggle);
            addAndMakeVisible (alphaToggle);
        }

        void resized() final
        {
            AnimationEasingDemoHelpers::layoutComponentsHorizontally (getLocalBounds(), { &button, nullptr, &positionToggle, &sizeToggle, &alphaToggle });
        }

        TextButton button { "Animate" };
        ToggleButton positionToggle { "Position" };
        ToggleButton sizeToggle { "Size" };
        ToggleButton alphaToggle { "Alpha" };
    };

    DurationControls durationControls;
    PlaybackControls playbackControls;
};

static Point<float> convertPointInBoundsToBezierPoint (const Point<float>& point, const Rectangle<float>& bounds)
{
    return { jlimit (0.0f, 1.0f, jmap (point.getX(), bounds.getX(), bounds.getRight(),  0.0f, 1.0f)),
                                 jmap (point.getY(), bounds.getBottom(), bounds.getY(), 0.0f, 1.0f) };
}

static Point<float> convertBezierPointToPointInBounds (const Point<float>& bezierPoint, const Rectangle<float>& bounds)
{
    return bounds.getRelativePoint (bezierPoint.getX(), 1.0f - bezierPoint.getY());
}

struct CubicBezier
{
    CubicBezier() = default;
    CubicBezier (const Point<float>& cp1In,
                 const Point<float>& cp2In)
        : cp1 (cp1In),
          cp2 (cp2In) {}

    Point<float> cp0 { 0.0f, 0.0f };
    Point<float> cp1 {};
    Point<float> cp2 {};
    Point<float> cp3 { 1.0f, 1.0f };

    bool operator== (const CubicBezier& other)
    {
        const auto tie = [](const CubicBezier& x) { return std::tie (x.cp0, x.cp1, x.cp2, x.cp3); };
        return tie (*this) == tie (other);
    }
};

class CubicBezierSettingsComponent final : public Component
{
public:
    CubicBezierSettingsComponent()
    {
        textEditor.setFont (FontOptions (18.0f));
        textEditor.setColour (TextEditor::ColourIds::backgroundColourId, {});
        textEditor.setColour (TextEditor::ColourIds::highlightColourId, {});
        textEditor.setColour (TextEditor::ColourIds::outlineColourId, {});
        textEditor.setColour (TextEditor::ColourIds::focusedOutlineColourId, {});
        textEditor.setColour (TextEditor::ColourIds::shadowColourId, {});
        textEditor.setJustification (Justification::centred);

        updateText();
        textEditor.onTextChange = [&]
        {
            const auto strippedText = textEditor.getText().retainCharacters ("0123456789.-,");
            const auto stringValues = StringArray::fromTokens (strippedText, ",", "");

            if (stringValues.size() == 4)
            {
                setCubicBezierCurve ({{ jlimit (0.0f, 1.0f, stringValues[0].getFloatValue()), stringValues[1].getFloatValue() },
                                      { jlimit (0.0f, 1.0f, stringValues[2].getFloatValue()), stringValues[3].getFloatValue() }});
            }

            updateText();
        };

        addAndMakeVisible (textEditor);
    }

    void setCubicBezierCurve (const CubicBezier& newBezierCurve)
    {
        if (std::exchange (bezierCurve, newBezierCurve) == newBezierCurve)
           return;

        updateText();
        NullCheckedInvocation::invoke (onValueChange);
    }

    CubicBezier getCubicBezierCurve() const
    {
        return bezierCurve;
    }

    void resized() final
    {
        textEditor.setBounds (getLocalBounds());
    }

    std::function<void()> onValueChange;

private:
    void updateText()
    {
        ScopedValueSetter<std::function<void()>> pauseOnTextChangeCallbacks (textEditor.onTextChange, nullptr);

        ScopeGuard restoreCaretPosition {[p = textEditor.getCaretPosition(), this]
        {
            textEditor.setCaretPosition (p);
        }};

        textEditor.clear();

        textEditor.setColour (TextEditor::ColourIds::textColourId, Colours::white);
        textEditor.insertTextAtCaret ("cubicBezier (");

        textEditor.setColour (TextEditor::ColourIds::textColourId, AnimationEasingDemoConstants::cp1AccentColour);
        textEditor.insertTextAtCaret (String (bezierCurve.cp1.getX(), 2));

        textEditor.setColour (TextEditor::ColourIds::textColourId, Colours::white);
        textEditor.insertTextAtCaret (", ");

        textEditor.setColour (TextEditor::ColourIds::textColourId, AnimationEasingDemoConstants::cp1AccentColour);
        textEditor.insertTextAtCaret (String (bezierCurve.cp1.getY(), 2));

        textEditor.setColour (TextEditor::ColourIds::textColourId, Colours::white);
        textEditor.insertTextAtCaret (", ");

        textEditor.setColour (TextEditor::ColourIds::textColourId, AnimationEasingDemoConstants::cp2AccentColour);
        textEditor.insertTextAtCaret (String (bezierCurve.cp2.getX(), 2));

        textEditor.setColour (TextEditor::ColourIds::textColourId, Colours::white);
        textEditor.insertTextAtCaret (", ");

        textEditor.setColour (TextEditor::ColourIds::textColourId, AnimationEasingDemoConstants::cp2AccentColour);
        textEditor.insertTextAtCaret (String (bezierCurve.cp2.getY(), 2));

        textEditor.setColour (TextEditor::ColourIds::textColourId, Colours::white);
        textEditor.insertTextAtCaret (")");
    }

    CubicBezier bezierCurve;
    TextEditor textEditor;
};

class CubicBezierGraphComponent final : public Component
{
public:
    void setCubicBezierCurve (const CubicBezier& newBezierCurve)
    {
        if (std::exchange (bezierCurve, newBezierCurve) == newBezierCurve)
           return;

        NullCheckedInvocation::invoke (onValueChange);
        repaint();
    }

    CubicBezier getCubicBezierCurve() const
    {
        return bezierCurve;
    }

    void paint (Graphics& g) final
    {
        const auto bounds = getGraphArea();
        const auto lineThickness = 6.0f;
        const auto cp0 = getControlPointOnGraph (bezierCurve.cp0);
        const auto cp1 = getControlPointOnGraph (bezierCurve.cp1);
        const auto cp2 = getControlPointOnGraph (bezierCurve.cp2);
        const auto cp3 = getControlPointOnGraph (bezierCurve.cp3);
        const auto outlineColour = getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::outline);
        const auto highlightColour = getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::highlightedFill);
        const auto foregroundColour = getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::defaultText);

        // graph background
        drawColouredLines (g, bounds, 15, { Colour{}, highlightColour.withAlpha (0.2f) });

        // graph outline
        g.setColour (outlineColour);
        g.drawRect (bounds);

        // semi-transparent linear line
        g.setColour (foregroundColour.withAlpha (0.15f));
        g.drawLine ({ cp0, cp3 }, lineThickness);

        // cubic-bezier curve
        Path curve;
        curve.startNewSubPath (cp0);
        curve.cubicTo (cp1, cp2, cp3);

        g.setColour (foregroundColour);
        g.strokePath (curve, PathStrokeType (lineThickness));

        // lines between control points
        g.setColour (foregroundColour);
        g.drawLine ({ cp0, cp1 }, 2.0f);
        g.drawLine ({ cp2, cp3 }, 2.0f);

        // control points
        drawControlPoint (g, cp0, highlightColour.brighter());
        drawControlPoint (g, cp3, highlightColour.brighter());
        drawControlPoint (g, cp1, AnimationEasingDemoConstants::cp1AccentColour);
        drawControlPoint (g, cp2, AnimationEasingDemoConstants::cp2AccentColour);
    }

    void mouseDown (const MouseEvent& event) final
    {
        const auto pos = event.position;
        const auto distanceToCp1 = pos.getDistanceFrom (getControlPointOnGraph (bezierCurve.cp1));
        const auto distanceToCp2 = pos.getDistanceFrom (getControlPointOnGraph (bezierCurve.cp2));

        selectedControlPoint = distanceToCp1 <= distanceToCp2 ? &bezierCurve.cp1
                                                              : &bezierCurve.cp2;
        updateSelectedControlPoint (pos);
    }

    void mouseDrag (const MouseEvent& event) final
    {
        updateSelectedControlPoint (event.position);
    }

    std::function<void()> onValueChange;

private:
    Point<float> getControlPointOnGraph (const Point<float>& relativeControlPoint)
    {
        return convertBezierPointToPointInBounds (relativeControlPoint, getGraphArea());
    }

    void updateSelectedControlPoint (const Point<float>& newPoint)
    {
        jassert (selectedControlPoint != nullptr);

        const auto newControlPoint = convertPointInBoundsToBezierPoint (newPoint, getGraphArea());

        if (*selectedControlPoint == newControlPoint)
            return;

        *selectedControlPoint = newControlPoint;
        NullCheckedInvocation::invoke (onValueChange);
        repaint();
    }

    Rectangle<float> getGraphArea() const
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto size = std::min (bounds.getWidth(), bounds.getHeight());
        return bounds.withSizeKeepingCentre (size, size).reduced (AnimationEasingDemoConstants::largeGapSize);
    }

    void drawControlPoint (Graphics& g, const Point<float>& point, Colour colour)
    {
        const auto size = jlimit (10.0f, 35.0f, (float) std::min (getWidth(), getHeight()) / 12.0f);
        Rectangle<float> bounds;
        bounds.setSize (size, size);
        bounds.setCentre (point);

        g.setColour (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::outline));
        g.drawEllipse (bounds, 2.0f);

        g.setColour (colour);
        g.fillEllipse (bounds);
    }

    void drawColouredLines (Graphics& g, Rectangle<float> bounds, int numLines, std::vector<Colour> colours)
    {
        const auto lineHeight = bounds.getHeight() / (float) numLines;

        for (size_t line = 0; line < (size_t) numLines; ++line)
        {
            g.setColour (colours[line % colours.size()]);
            g.fillRect (bounds.removeFromTop (lineHeight));
        }
    }

    CubicBezier bezierCurve;
    Point<float>* selectedControlPoint{};
};

class AnimationView : public Component
{
public:
    AnimationView (const AnimationSettings& animationSettingsIn,
                   std::function<ValueAnimatorBuilder::EasingFn()> easingFunctionFactoryIn)
        : animationSettings (animationSettingsIn),
          easingFunctionFactory (std::move (easingFunctionFactoryIn))
    {
        jassert (easingFunctionFactory != nullptr);

        componentToAnimate.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (componentToAnimate);
    }

    void animate()
    {
        showSettingsPage (false);

        const auto valueChangedCallback = [&](float v)
        {
            animateFrame (animationSettings.shouldAnimatePosition.getValue() ? v : 1.0f,
                          animationSettings.shouldAnimateSize.getValue()     ? v : 1.0f,
                          animationSettings.shouldAnimateAlpha.getValue()    ? v : 1.0f);
        };

        const auto animateIn = ValueAnimatorBuilder{}.withEasing (easingFunctionFactory())
                                                     .withDurationMs (animationSettings.durationMs.getValue())
                                                     .withValueChangedCallback (valueChangedCallback);

        const auto animateOut = animateIn.withValueChangedCallback ([=](auto v){ valueChangedCallback (1.0f - v); });

        animator = std::make_unique<Animator> (AnimatorSetBuilder (animateOut.build())
                                                      .followedBy (400.0)
                                                      .followedBy (animateIn.build())
                                                      .build());

        updater.addAnimator (*animator);
        animator->start();
    }

    void mouseDown (const MouseEvent&) final
    {
        animate();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        componentToAnimate.setBounds (bounds);
        settingsPageBackground.setBounds (bounds);

        editViewButton.setBounds (bounds.removeFromTop   (AnimationEasingDemoConstants::defaultComponentHeight)
                                        .removeFromRight (AnimationEasingDemoConstants::defaultComponentHeight * 2));

        if (customSettingsPage != nullptr)
            customSettingsPage->setBounds (bounds);
    }

protected:
    void setCustomSettingsPage (Component& settingsPage)
    {
        customSettingsPage = &settingsPage;
        editViewButton.onClick = [&] { toggleSettingsPage(); };
        addChildComponent (settingsPageBackground);
        addChildComponent (customSettingsPage);

        editViewButton.setButtonText ("Edit");
        addAndMakeVisible (editViewButton);
    }

private:
    void showSettingsPage (bool shouldShowSettingsPage)
    {
        if (customSettingsPage == nullptr)
            return;

        editViewButton.setButtonText (shouldShowSettingsPage ? "View" : "Edit");
        settingsPageBackground.setVisible (shouldShowSettingsPage);
        customSettingsPage->setVisible (shouldShowSettingsPage);
    }

    void toggleSettingsPage()
    {
        showSettingsPage (editViewButton.getButtonText() == "Edit");
    }

    struct JUCELogoComponent final : Component
    {
        void paint (Graphics& g) final
        {
            const auto bounds = getLocalBounds();
            g.setColour (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::highlightedFill));
            g.fillRect (bounds);

            g.setColour (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::defaultText));

            const auto logo = getJUCELogoPath();
            g.addTransform (logo.getTransformToScaleToFit (bounds.toFloat().reduced (AnimationEasingDemoConstants::mediumGapSize), true));
            g.fillPath (logo);
        }
    };

    void animateFrame (float position, float size, float alpha)
    {
        // Transforms don't work when scaling to 0 so this work around hides
        // the component when the size is 0. As the alpha is also animated it's
        // guaranteed to be correctly set when the size is not 0
        if (approximatelyEqual (size, 0.0f))
        {
            componentToAnimate.setAlpha (0.0f);
            return;
        }

        const auto bounds = getLocalBounds().toFloat();
        const auto centre = bounds.getCentre();
        const auto xLimits = makeAnimationLimits (-bounds.getWidth(), 0.0f);
        componentToAnimate.setTransform (AffineTransform{}.scaled (size, size, centre.getX(), centre.getY())
                                                          .translated (xLimits.lerp (position), 0.0f));
        componentToAnimate.setAlpha (alpha);
    }

    struct BackgroundFill : Component
    {
        void paint (Graphics& g)
        {
            g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
        }
    };

    AnimationSettings animationSettings;
    std::function<ValueAnimatorBuilder::EasingFn()> easingFunctionFactory {};
    BackgroundFill settingsPageBackground;
    Component* customSettingsPage {};
    TextButton editViewButton;
    std::unique_ptr<Animator> animator;
    VBlankAnimatorUpdater updater { this };
    JUCELogoComponent componentToAnimate;
};

class StandardEasingAnimationView final : public AnimationView
{
public:
    StandardEasingAnimationView (const AnimationSettings& settings,
                                 ValueAnimatorBuilder::EasingFn easingFunction)
        : AnimationView (settings, [=] { return easingFunction; })
    {}
};

class CubicBezierEasingAnimationView final : public AnimationView
{
public:
    CubicBezierEasingAnimationView (const AnimationSettings& settings)
        : AnimationView (settings,
                         [&] { return Easings::createCubicBezier (bezierCurve.cp1, bezierCurve.cp2); })
    {
        settingsPage.graph.onValueChange = [&] { setCubicBezierCurve (settingsPage.graph.getCubicBezierCurve()); };
        settingsPage.settings.onValueChange = [&] { setCubicBezierCurve (settingsPage.settings.getCubicBezierCurve()); };
        settingsPage.graph.setCubicBezierCurve ({ { 0.2f, 0.0f }, { 0.0f, 1.0f } });

        setCustomSettingsPage (settingsPage);
    }

private:
    struct SettingsPage : Component
    {
        SettingsPage()
        {
            addAndMakeVisible (graph);
            addAndMakeVisible (settings);
        }

        void resized() final
        {
            auto bounds = getLocalBounds();
            settings.setBounds (bounds.removeFromBottom (AnimationEasingDemoConstants::defaultComponentHeight));
            graph.setBounds (bounds);
        }

        CubicBezierGraphComponent graph;
        CubicBezierSettingsComponent settings;
    };

    void setCubicBezierCurve (const CubicBezier& newBezierCurve)
    {
        bezierCurve = newBezierCurve;
        settingsPage.graph.setCubicBezierCurve (newBezierCurve);
        settingsPage.settings.setCubicBezierCurve (newBezierCurve);
    }


    CubicBezier bezierCurve;
    SettingsPage settingsPage;
};

struct SliderAndLabel final : public Component
{
    SliderAndLabel()
    {
        addAndMakeVisible (slider);
        addAndMakeVisible (label);
    }

    void resized() final
    {
        auto bounds = getLocalBounds();
        label.setBounds (bounds.removeFromTop (bounds.getHeight() / 2)
                               .removeFromBottom (AnimationEasingDemoConstants::defaultComponentHeight));
        slider.setBounds (bounds.removeFromTop (AnimationEasingDemoConstants::defaultComponentHeight));
    }

    Slider slider;
    Label label;
};

class SpringEasingAnimationView final : public AnimationView
{
public:
    SpringEasingAnimationView (const AnimationSettings& settings)
        : AnimationView (settings,
                         [&] { return Easings::createSpring (SpringEasingOptions{}.withFrequency (getFrequency())
                                                                                  .withAttenuation (getAttenuation())
                                                                                  .withExtraAttenuationRange (getExtraAttenuationRange())); })
    {
        setCustomSettingsPage (settingsPage);
    }

private:
    float getFrequency() { return (float) settingsPage.frequency.slider.getValue(); }
    float getAttenuation() { return (float) settingsPage.attenuation.slider.getValue(); }
    float getExtraAttenuationRange() { return (float) settingsPage.extraAttenuationRange.slider.getValue(); }

    struct SettingsPage : Component
    {
        SettingsPage()
        {
            frequency.label.setText ("Frequency", NotificationType::dontSendNotification);
            frequency.slider.setRange (1.0, 10.0, 1.0);
            frequency.slider.setValue (3.0);

            attenuation.label.setText ("Attenuation", NotificationType::dontSendNotification);
            attenuation.slider.setRange (1.0, 10.0, 1.0);
            attenuation.slider.setValue (3.0);

            extraAttenuationRange.label.setText ("Extra attenuation range", NotificationType::dontSendNotification);
            extraAttenuationRange.slider.setRange (0.05, 0.98, 0.01);
            extraAttenuationRange.slider.setValue (0.25);

            addAndMakeVisible (frequency);
            addAndMakeVisible (attenuation);
            addAndMakeVisible (extraAttenuationRange);
        }

        void resized() final
        {
            AnimationEasingDemoHelpers::layoutComponentsVertically (getLocalBounds(), { &frequency, &attenuation, &extraAttenuationRange });
        }

        SliderAndLabel frequency;
        SliderAndLabel attenuation;
        SliderAndLabel extraAttenuationRange;
    };

    SettingsPage settingsPage;
};

class BounceOutEasingAnimationView final : public AnimationView
{
public:
    BounceOutEasingAnimationView (const AnimationSettings& settings)
        : AnimationView (settings,
                         [&] { return Easings::createBounce (roundToInt (numberOfBounces.slider.getValue())); })
    {
        numberOfBounces.label.setText ("Number of bounces", NotificationType::dontSendNotification);
        numberOfBounces.slider.setRange (1.0, 10.0, 1.0);
        numberOfBounces.slider.setValue (3.0);

        setCustomSettingsPage (numberOfBounces);
    }

private:
    SliderAndLabel numberOfBounces;
};

class AnimationSelectorAndView final : public Component
{
    struct AnimationViewAndName
    {
        String name;
        std::unique_ptr<AnimationView> component;
    };

public:
    AnimationSelectorAndView (const AnimationSettings& settings)
    {
        views.push_back ({ "linear",         std::make_unique <StandardEasingAnimationView>    (settings, Easings::createLinear()) });
        views.push_back ({ "ease (default)", std::make_unique <StandardEasingAnimationView>    (settings, Easings::createEase()) });
        views.push_back ({ "easeIn",         std::make_unique <StandardEasingAnimationView>    (settings, Easings::createEaseIn()) });
        views.push_back ({ "easeOut",        std::make_unique <StandardEasingAnimationView>    (settings, Easings::createEaseOut()) });
        views.push_back ({ "easeInOut",      std::make_unique <StandardEasingAnimationView>    (settings, Easings::createEaseInOut()) });
        views.push_back ({ "easeOutBack",    std::make_unique <StandardEasingAnimationView>    (settings, Easings::createEaseOutBack()) });
        views.push_back ({ "easeInOutCubic", std::make_unique <StandardEasingAnimationView>    (settings, Easings::createEaseInOutCubic()) });
        views.push_back ({ "cubicBezier",    std::make_unique <CubicBezierEasingAnimationView> (settings) });
        views.push_back ({ "spring",         std::make_unique <SpringEasingAnimationView>      (settings) });
        views.push_back ({ "bounce",         std::make_unique <BounceOutEasingAnimationView>   (settings) });

        for (auto [itemId, view] : enumerate (views, int { 1 }))
            easingSelector.addItem (view.name, itemId);

        // select "ease" as the default
        easingSelector.setSelectedItemIndex (1);
        easingSelector.onChange = [&]
        {
            for (auto& view : views)
                view.component->setVisible (false);

            views[(size_t) easingSelector.getSelectedItemIndex()].component->setVisible (true);
        };

        for (auto& view : views)
            addChildComponent (view.component.get());

        addAndMakeVisible (easingSelector);
    }

    void animate()
    {
        for (auto& view : views)
            view.component->animate();
    }

    void resized() final
    {
        auto bounds = getLocalBounds();

        easingSelector.setBounds (bounds.removeFromTop (AnimationEasingDemoConstants::defaultComponentHeight));

        bounds.removeFromTop (AnimationEasingDemoConstants::smallGapSize);

        for (auto& view : views)
            view.component->setBounds (bounds);
    }

private:
    ComboBox easingSelector;
    std::vector<AnimationViewAndName> views;
};

class AnimationEasingDemo final : public Component
{
public:
    //==============================================================================
    AnimationEasingDemo()
    {
        animationSettings.durationMs = 1000.0;
        animationSettings.shouldAnimatePosition = true;
        animationSettings.shouldAnimateSize = false;
        animationSettings.shouldAnimateAlpha = false;

        animationSettingsComponent.onAnimate = [&]
        {
            animationView1.animate();
            animationView2.animate();
        };

        addAndMakeVisible (animationSettingsComponent);
        addAndMakeVisible (animationView1);
        addAndMakeVisible (animationView2);

        setSize (600, 400);
    }

    //==============================================================================
    void paint (Graphics& g) final
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (AnimationEasingDemoConstants::largeGapSize);

        animationSettingsComponent.setBounds (bounds.removeFromTop (AnimationEasingDemoConstants::defaultComponentHeight * 2));
        bounds.removeFromTop (AnimationEasingDemoConstants::smallGapSize);
        AnimationEasingDemoHelpers::layoutComponentsVerticallyOrHorizontally (bounds, { &animationView1, &animationView2 });
    }

private:
    //==============================================================================
    AnimationSettings animationSettings;
    AnimationSettingsComponent animationSettingsComponent { animationSettings };
    AnimationSelectorAndView animationView1 { animationSettings };
    AnimationSelectorAndView animationView2 { animationSettings };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationEasingDemo)
};
