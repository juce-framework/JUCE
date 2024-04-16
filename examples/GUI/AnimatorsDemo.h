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

 name:             AnimatorsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Application demonstrating how the JUCE provided Animator
                   classes can be used to create dynamic, composable, smooth
                   animations.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_animation
 exporters:        xcode_mac, vs2022, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AnimatorsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

struct Paintable
{
    virtual ~Paintable() = default;
    virtual void paint (Graphics& g) const = 0;
};

namespace Shapes
{

class Arc final : public Paintable
{
    static constexpr auto pi = MathConstants<float>::pi;
    static constexpr auto arcStart = pi / 2.0f;

public:
    Arc() = default;

    Arc (Point<float> centreIn, float radiusIn, float thicknessIn, Colour colourIn)
        : centre (centreIn),
          initialRadius (radiusIn - thicknessIn / 2),
          initialThickness (thicknessIn),
          colour (colourIn)
    {
    }

    void paint (Graphics& g) const override
    {
        if (! active)
            return;

        const Graphics::ScopedSaveState s { g };

        Path p;
        p.addCentredArc (centre.getX(),
                         centre.getY(),
                         radius,
                         radius,
                         0.0f,
                         arcStart,
                         arcStart + sweepAngle,
                         true);

        g.setColour (colour);
        g.strokePath (p, { thickness, PathStrokeType::mitered });
    }

    void setActive (bool activeIn)
    {
        active = activeIn;
    }

    auto getFanoutAnimator() const { return fanoutAnimator; }

    auto getFillAnimator() const   { return fillAnimator; }

    Point<float> centre;
    float initialRadius = 0.0f, initialThickness = 0.0f;
    Colour colour = Colours::white;

private:
    bool active = false;
    float radius = 0.0f, thickness = 0.0f, sweepAngle = 0.0f;

    void reset()
    {
        sweepAngle = 0.0f;
        radius = initialRadius;
        thickness = initialThickness;
    }

    Animator fanoutAnimator = ValueAnimatorBuilder{}
                                  .withOnStartCallback ([this]
                                                        {
                                                            reset();
                                                            active = true;
                                                        })
                                  .withValueChangedCallback ([this] (auto value)
                                                              {
                                                                  sweepAngle = makeAnimationLimits (2.1f * MathConstants<float>::pi).lerp (value);
                                                              })
                                  .withEasing (Easings::createLinear())
                                  .build();

    Animator fillAnimator = ValueAnimatorBuilder{}
                                .withOnStartReturningValueChangedCallback (
                                    [this]
                                    {
                                        const auto thicknessChange = radius - 5.0f;
                                        const std::tuple begin { initialRadius, initialThickness };
                                        const std::tuple end { initialRadius - thicknessChange / 2.0f, initialThickness + thicknessChange };
                                        const auto limits = makeAnimationLimits (begin, end);

                                        return [this, limits] (auto value)
                                        {
                                            std::tie (radius, thickness) = limits.lerp (value);
                                        };
                                    })
                                .withEasing (Easings::createLinear())
                                .build();
};

class Circle : public Paintable
{
public:
    Circle (Point<float> centreIn, float radiusIn, Colour colourIn)
        : centre (centreIn), radius (radiusIn), colour (colourIn)
    {
    }

    void paint (Graphics& g) const override
    {
        if (! active)
            return;

        const Graphics::ScopedSaveState s { g };

        g.setColour (colour);
        g.fillEllipse (centre.getX() - radius, centre.getY() - radius, 2.0f * radius, 2.0f * radius);
    }

    void setActive (bool activeIn) { active = activeIn; }

private:
    Point<float> centre;
    float radius = 0.0f;
    Colour colour;
    bool active = false;
};

/* Can return a subpath based on a proportion between [0, 1.0]. Useful for creating an animation
   where a path is drawn over time.
*/
class PartialPath
{
public:
    explicit PartialPath (std::initializer_list<Point<float>> points)
    {
        bool pathIsEmpty = false;

        for (auto p : points)
        {
            if (std::exchange (pathIsEmpty, true) == false)
            {
                path.startNewSubPath (p);
                pts.push_back (std::make_pair (p, 0.0f));
            }
            else
            {
                path.lineTo (p);
                pts.push_back (std::make_pair (p, path.getLength()));
            }
        }
    }

    Path getPartialPath (float proportion) const
    {
        proportion = jmin (1.0f, proportion);

        Path partialPath;
        bool pathIsEmpty = false;

        if (pts.size() < 2)
            return partialPath;

        const auto proportionalLength = path.getLength() * proportion;

        const auto lineTo = [&partialPath, &pathIsEmpty] (Point<float> p)
        {
            if (std::exchange (pathIsEmpty, true) == false)
                partialPath.startNewSubPath (p);
            else
                partialPath.lineTo (p);
        };

        for (const auto& point : pts)
        {
            if (point.second > proportionalLength)
            {
                lineTo (path.getPointAlongPath (proportionalLength));
                break;
            }

            lineTo (point.first);
        }

        return partialPath;
    }

private:
    Path path;
    std::vector<std::pair<Point<float>, float>> pts;
};

class Checkmark : public Paintable
{
public:
    Checkmark (Rectangle<float> placementIn, float thicknessIn)
        : placement (placementIn),
          partialPath { { placement.getX(), placement.getY() + 0.7f * placement.getHeight() },
                        { placement.getX() + 0.4f * placement.getWidth(), placement.getBottom() },
                        { placement.getRight(), placement.getY() + 0.2f * placement.getHeight() } },
          thickness (thicknessIn)
    {
    }

    void paint (Graphics& g) const override
    {
        if (exactlyEqual (progress, 0.0f))
            return;

        const Graphics::ScopedSaveState s { g };

        g.setColour (colour);

        const auto p = partialPath.getPartialPath (progress);
        g.strokePath (p, PathStrokeType { thickness, PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded });
    }

    void setProgress (float p) { progress = p; }

private:
    Rectangle<float> placement;
    PartialPath partialPath;
    float progress = 0.0f;
    float thickness = 10.0f;
    Colour colour { Colours::white };
};

} // namespace Shapes

inline auto createComponentMover (Component& component, Rectangle<int> targetBounds)
{
    const auto boundsToTuple = [] (auto b)
    {
        return std::make_tuple (b.getX(), b.getY(), b.getWidth(), b.getHeight());
    };

    const auto begin = boundsToTuple (component.getBoundsInParent());
    const auto end   = boundsToTuple (targetBounds);
    const auto limits = makeAnimationLimits (begin, end);

    return [&component, limits] (auto v)
    {
        const auto [x, y, w, h] = limits.lerp (v);
        component.setBounds (x, y, w, h);
    };
}

class AnimatedCheckmark : public Paintable
{
public:
    AnimatedCheckmark (Point<float> centre, float radius, float thickness, Colour colour)
        : arc (centre, radius, thickness, colour),
          circle (centre, radius, colour),
          checkmark (Rectangle<float> { centre.getX() - radius,
                                        centre.getY() - radius,
                                        2.0f * radius,
                                        2.0f * radius }.reduced (radius * 0.4f),
                     thickness)
    {
    }

    void paint (Graphics& g) const override
    {
        arc.paint (g);
        circle.paint (g);
        checkmark.paint (g);
    }

    auto getAnimator() const { return animator; }

private:
    Shapes::Arc arc;
    Shapes::Circle circle;
    Shapes::Checkmark checkmark;

    Animator animator = [&]
    {
        circle.setActive (false);

        const auto checkmarkAnimator = ValueAnimatorBuilder{}
                                           .withEasing (Easings::createEaseOutBack())
                                           .withDurationMs (450)
                                           .withValueChangedCallback ([this] (auto value)
                                                                      {
                                                                          checkmark.setProgress (value);
                                                                      })
                                           .build();

        return AnimatorSetBuilder (arc.getFanoutAnimator())
            .followedBy (arc.getFillAnimator())
            .followedBy ([this]
                         {
                             arc.setActive (false);
                             circle.setActive (true);
                         })
            .followedBy (checkmarkAnimator)
            .build();
    }();
};

class FallingBall : public Component
{
private:
    template <typename T>
    auto getDimensions (Rectangle<T> r)
    {
        return std::make_tuple (r.getX(), r.getY(), r.getWidth(), r.getHeight());
    }

public:
    void paint (Graphics& g) override
    {
        g.setColour (Colour { 0xff179af0 });
        const auto [x, y, width, height] = getDimensions (getLocalBounds().toFloat());
        g.fillEllipse (x, y, width, height);
    }

    Animator updateAndGetAnimator()
    {
        newBounds = getBoundsInParent().withY (getParentHeight() - getHeight());
        return fallAnimator;
    }

private:
    Rectangle<int> newBounds;

    Animator fallAnimator = ValueAnimatorBuilder{}
                                .withOnStartReturningValueChangedCallback ([this] { return createComponentMover (*this, newBounds); })
                                .withEasing (Easings::createBounce())
                                .withDurationMs (600)
                                .build();
};

class PulsingCheckmark : public Component
{
public:
    PulsingCheckmark (Point<float> centre, float radius)
        : checkmark ({ radius, radius }, radius, radius / 6.25f, Colour { 0xff1bc211 })
    {
        const auto widthAndHeight = 2 * (int) radius;
        const auto x = (int) (centre.getX() - radius);
        const auto y = (int) (centre.getY() - radius);

        setBounds (x, y, widthAndHeight, widthAndHeight);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        checkmark.paint (g);
    }

    void mouseDown (const MouseEvent&) override
    {
        animator.complete();
    }

    Animator getAnimator() const { return animator; }

private:
    //==============================================================================
    AnimatedCheckmark checkmark;

    Animator animator = [&]
    {
        const auto checkmarkAnimator = checkmark.getAnimator();
        const auto checkmarkDuration = checkmarkAnimator.getDurationMs();

        const auto pulseAnimator = ValueAnimatorBuilder{}
                                       .withEasing (Easings::createOnOffRamp())
                                       .withOnStartReturningValueChangedCallback (
                                           [this]
                                           {
                                               const auto radius = (float) getWidth() / 2.0f;
                                               const auto centreInParent = getBoundsInParent().toFloat().getPosition()
                                                                           + Point<float> { radius, radius };

                                               return [this, centreInParent] (auto value)
                                               {
                                                   setTransform (AffineTransform::translation (-centreInParent)
                                                                     .followedBy (AffineTransform::scale (1.0f + 0.2f * value))
                                                                     .followedBy (AffineTransform::translation (centreInParent)));
                                               };
                                           })
                                       .build();

        const auto timeBeforePulseAnimation = checkmarkDuration - pulseAnimator.getDurationMs();

        const auto repaintAnimator = ValueAnimatorBuilder{}
                                         .withValueChangedCallback ([this] (auto) { repaint(); })
                                         .runningInfinitely()
                                         .build();

        return AnimatorSetBuilder (checkmarkAnimator)
            .togetherWith (repaintAnimator)
            .togetherWith (timeBeforePulseAnimation)
            .followedBy (pulseAnimator)
            .followedBy ([repaintAnimator] { repaintAnimator.complete(); })
            .build();
    }();
};

// Displays the PulsingCheckmark as it looks when its animation is complete
class CompletedCheckmark : public Component
{
public:
    explicit CompletedCheckmark (std::function<void()> onClickIn)
        : onClick (std::move (onClickIn)) {}

    void resized() override
    {
        const auto getCentre  = [this]
        {
            return Point<float> { (float) getWidth() / 2, (float) getHeight() / 2 };
        };

        const auto radius = (float) std::min (getWidth(), getHeight()) / 2.0f;
        checkmark = std::make_unique<PulsingCheckmark> (getCentre(), radius);
        checkmark->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*checkmark);
        checkmark->getAnimator().start();
        checkmark->getAnimator().complete();
        checkmark->getAnimator().update (0);
    }

    void mouseDown (const MouseEvent&) override
    {
        NullCheckedInvocation::invoke (onClick);
    }

private:
    std::unique_ptr<PulsingCheckmark> checkmark;
    std::function<void()> onClick;
};

class BallToolComponent : public Component
{
public:
    explicit BallToolComponent (std::function<void()> onClickIn)
        : onClick (std::move (onClickIn)) {}

    void paint (Graphics& g) override
    {
        g.setColour (Colour { 0xff179af0 });
        g.fillEllipse (getLocalBounds().toFloat());
    }

    void mouseDown (const MouseEvent&) override
    {
        NullCheckedInvocation::invoke (onClick);
    }

private:
    std::function<void()> onClick;
};

class AnimatorsDemo : public Component
{
public:
    //==============================================================================
    AnimatorsDemo()
    {
        welcomeComponent.setOnAnimatedClickEnd ([this]
                                                {
                                                    toolsPanel.open();
                                                });
        addAndMakeVisible (welcomeComponent);

        toolsPanel.onClose = [this] { welcomeComponent.reset(); };
        toolsPanel.addToolComponent (std::make_unique<CompletedCheckmark> ([this] { selectedTool = SelectedTool::checkmark; }));
        toolsPanel.addToolComponent (std::make_unique<BallToolComponent> ([this]
                                                                          {
                                                                              selectedTool = SelectedTool::ball;
                                                                          }));
        addAndMakeVisible (toolsPanel);

        setSize (600, 400);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        for (const auto& paintable : objects)
            paintable.second->paint (g);
    }

    void resized() override
    {
        welcomeComponent.setBounds (getLocalBounds());
    }

    void mouseDown (const MouseEvent& event) override
    {
        switch (selectedTool)
        {
            case SelectedTool::checkmark:
                makeCheckmark (event.getPosition().toFloat());
                break;

            case SelectedTool::ball:
                makeBall (event.getPosition().toFloat());
                break;

            case SelectedTool::none:
                toolsPanel.wobbleLabel();
                break;
        }
    }

private:
    void makeCheckmark (Point<float> centre)
    {
        auto checkmark = std::make_unique<PulsingCheckmark> (centre, 50.0f);
        updater.addAnimator (checkmark->getAnimator(), [this] { toolComponent.reset(); });
        checkmark->getAnimator().start();
        addAndMakeVisible (*checkmark, 0);
        toolComponent = std::move (checkmark);
    }

    void makeBall (Point<float> centre)
    {
        auto ball = std::make_unique<FallingBall>();
        addAndMakeVisible (*ball, 0);
        const auto radius = 50.0f;
        const Rectangle<float> bounds { centre.getX() - radius, centre.getY() - radius, 2 * radius, 2 * radius };
        ball->setBounds (bounds.toNearestInt());
        auto animator = ball->updateAndGetAnimator();
        updater.addAnimator (animator, [this] { toolComponent.reset(); });
        animator.start();
        toolComponent = std::move (ball);
    }

    //==============================================================================
    VBlankAnimatorUpdater updater { this };

    struct WelcomeComponent : public Component
    {
        WelcomeComponent()
        {
            startButton.onClick = [this]
            {
                animateForward = true;
                buttonAnimator.start();
                updater.addAnimator (buttonAnimator, [this] { updater.removeAnimator (buttonAnimator); });
            };

            addAndMakeVisible (startButton);
        }

        void setOnAnimatedClickEnd (std::function<void()> onClickIn)
        {
            onClick = std::move (onClickIn);
        }

        void reset()
        {
            animateForward = false;
            buttonAnimator.start();
            updater.addAnimator (buttonAnimator, [this] { updater.removeAnimator (buttonAnimator); });
        }

        void resized() override
        {
            startButton.setBounds (getLocalBounds().withSizeKeepingCentre (140, 40));
        }

    private:
        TextButton startButton { "Start demo" };
        bool animateForward = false;
        Animator buttonAnimator = [&]
        {
            return ValueAnimatorBuilder{}
                .withOnStartCallback ([this] { setVisible (true); })
                .withValueChangedCallback ([this] (auto value)
                                           {
                                               const auto progress = animateForward ? value : (1.0 - value);
                                               setAlpha (1.0f - (float) progress);
                                           })
                .withOnCompleteCallback ([this]
                                         {
                                             setVisible (! animateForward);

                                             if (animateForward)
                                                NullCheckedInvocation::invoke (onClick);
                                         })
                .build();
        }();

        VBlankAnimatorUpdater updater { this };
        std::function<void()> onClick;
    };

    class WobblyLabel : public Component
    {
    public:
        explicit WobblyLabel (const String& text)
            : label ({}, text)
        {
            label.setJustificationType (Justification::right);
            addAndMakeVisible (label);
        }

        void resized() override
        {
            label.setBounds (getLocalBounds().withX ((int) offset));
        }

        void wobble()
        {
            updater.addAnimator (animator, [this] { updater.removeAnimator (animator); });
            animator.start();
        }

    private:
        float offset = 0.0f;
        Label label;
        Animator animator = ValueAnimatorBuilder{}.withValueChangedCallback (
                                                       [this] (float progress)
                                                       {
                                                           offset = 10.0f * std::sin (progress * 20.0f) * (1.0f - progress);
                                                           resized();
                                                       })
                                                  .withDurationMs (600)
                                                  .build();
        VBlankAnimatorUpdater updater { this };
    };

    struct ToolsPanel : public Component
    {
        static constexpr auto margin = 15;

        ToolsPanel()
        {
            shadower.setOwner (this);
            closeButton.onClick = [this] { close(); };
            addAndMakeVisible (closeButton);
            addAndMakeVisible (label);
            addAndMakeVisible (instructions);
            addChildComponent (selectionComponent, 0);

            instructions.setJustificationType (Justification::centred);
        }

        void paint (Graphics& g) override
        {
            g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).brighter (0.1f));
        }

        void resized() override
        {
            closeButton.setBounds (getLocalBounds().removeFromTop (40)
                                                   .removeFromRight (40)
                                                   .reduced (5));

            auto bounds = getLocalBounds();
            instructions.setBounds (bounds.removeFromBottom (30));

            FlexBox flexBox;
            flexBox.flexDirection = FlexBox::Direction::row;
            flexBox.flexWrap = FlexBox::Wrap::noWrap;
            flexBox.justifyContent = FlexBox::JustifyContent::center;
            flexBox.alignItems = FlexBox::AlignItems::center;

            const auto height = (float) (bounds.getHeight() - 2 * margin);

            flexBox.items.add (FlexItem (label).withWidth (200.0f).withHeight (height));

            for (auto& c : toolComponents)
                flexBox.items.add (FlexItem (*c).withWidth (height).withHeight (height).withMargin (margin));

            flexBox.performLayout (bounds);
        }

        //==============================================================================
        void open()
        {
            shouldOpen = true;
            updater.addAnimator (slideInAnimator, [this] { updater.removeAnimator (slideInAnimator); });
            slideInAnimator.start();
        }

        void close()
        {
            shouldOpen = false;
            updater.addAnimator (slideInAnimator, [this]
                                                  {
                                                      NullCheckedInvocation::invoke (onClose);
                                                      updater.removeAnimator (slideInAnimator);
                                                  });
            slideInAnimator.start();
        }

        void addToolComponent (std::unique_ptr<Component> component)
        {
            addAndMakeVisible (*component);
            component->addMouseListener (this, false);
            toolComponents.push_back (std::move (component));
        }

        //==============================================================================
        void mouseUp (const MouseEvent& event) override
        {
            if (event.originalComponent == this)
                return;

            const auto targetBounds = event.originalComponent->getBounds().expanded (std::min (10, margin));

            if (! selectionComponent.isVisible())
            {
                selectionComponent.setBounds (targetBounds);
                selectionComponent.appear (updater);
            }
            else
            {
                selectionComponent.moveTo (updater, targetBounds);
            }
        }

        void wobbleLabel()
        {
            label.wobble();
        }

    private:
        //==============================================================================
        class SelectionComponent : public Component
        {
        public:
            SelectionComponent()
            {
                setVisible (false);
            }

            void paint (Graphics& g) override
            {
                arc.paint (g);
            }

            void appear (VBlankAnimatorUpdater& animatorUpdater)
            {
                appearAnimator.start();
                animatorUpdater.addAnimator (appearAnimator,
                                             [this, &animatorUpdater]
                                             {
                                                 animatorUpdater.removeAnimator (appearAnimator);
                                             });
            }

            void moveTo (VBlankAnimatorUpdater& animatorUpdater, Rectangle<int> newBoundsIn)
            {
                newBounds = newBoundsIn;
                moveToNewBoundsAnimator.start();
                animatorUpdater.addAnimator (moveToNewBoundsAnimator,
                                             [this, &animatorUpdater]
                                             {
                                                 animatorUpdater.removeAnimator (moveToNewBoundsAnimator);
                                             });
            }

        private:
            Shapes::Arc arc;
            Rectangle<int> newBounds;

            Animator appearAnimator = [&]
            {
                const auto repaintAnimator =
                    ValueAnimatorBuilder{}
                        .withValueChangedCallback ([this] (auto) { repaint(); })
                        .runningInfinitely()
                        .build();

                return AnimatorSetBuilder { [this]
                                            {
                                                setVisible (true);
                                                arc.centre        = Point { getWidth(), getHeight() }.toFloat() / 2;
                                                arc.initialRadius = (float) (getWidth()) / 2 - 2.0f;
                                                arc.initialThickness = 4.0f;
                                            } }
                    .followedBy (repaintAnimator)
                    .togetherWith (arc.getFanoutAnimator())
                    .followedBy ([repaintAnimator] { repaintAnimator.complete(); })
                    .withTimeTransform ([] (auto v) { return 1.5 * v; })
                    .build();
            }();

            Animator moveToNewBoundsAnimator = ValueAnimatorBuilder{}
                                                   .withOnStartReturningValueChangedCallback ([this] { return createComponentMover (*this, newBounds); })
                                                   .withEasing (Easings::createSpring())
                                                   .withDurationMs (600)
                                                   .build();
        };

        WobblyLabel label { "Select animation:" };
        Label instructions { "", "Click below to animate" };
        SelectionComponent selectionComponent;
        std::vector<std::unique_ptr<Component>> toolComponents;
        DropShadow shadow { Colour { 0x90000000 }, 12, {} };
        DropShadower shadower { shadow };
        TextButton closeButton { "X" };

        Animator slideInAnimator = ValueAnimatorBuilder{}
                                       .withEasing (Easings::createEaseInOutCubic())
                                       .withOnStartReturningValueChangedCallback (
                                           [this]
                                           {
                                               const auto width = getParentWidth() - 2 * margin;
                                               const auto height = 130;
                                               setBounds (-width, margin, width, height);
                                               setVisible (true);

                                               const auto limits = makeAnimationLimits (-width, margin);

                                               return [this, limits] (auto value)
                                               {
                                                   const auto progress = std::clamp (shouldOpen ? value : 1.0 - value, 0.0, 1.0);
                                                   setTopLeftPosition (roundToInt (limits.lerp ((float) progress)), margin);
                                               };
                                           })
                                       .withDurationMs (500)
                                       .build();

        VBlankAnimatorUpdater updater { this };
        bool shouldOpen = true;

    public:
        std::function<void()> onClose;
    };

    enum class SelectedTool
    {
        none,
        checkmark,
        ball
    };

    WelcomeComponent welcomeComponent;
    ToolsPanel toolsPanel;
    SelectedTool selectedTool = SelectedTool::none;
    std::unique_ptr<Component> toolComponent;
    std::map<Paintable*, std::unique_ptr<Paintable>> objects;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatorsDemo)
};
