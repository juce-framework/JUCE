/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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

 name:             AnimationDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays an animated draggable ball.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AnimationDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** This will be the source of our balls and can be dragged around. */
class BallGeneratorComponent    : public Component
{
public:
    BallGeneratorComponent() {}

    void paint (Graphics& g) override
    {
        auto area = getLocalBounds().reduced (2);

        g.setColour (Colours::orange);
        g.drawRoundedRectangle (area.toFloat(), 10.0f, 2.0f);

        g.setColour (findColour (TextButton::textColourOffId));
        g.drawFittedText ("Drag Me!", area, Justification::centred, 1);
    }

    void resized() override
    {
        // Just set the limits of our constrainer so that we don't drag ourselves off the screen
        constrainer.setMinimumOnscreenAmounts (getHeight(), getWidth(),
                                               getHeight(), getWidth());
    }

    void mouseDown (const MouseEvent& e) override
    {
        // Prepares our dragger to drag this Component
        dragger.startDraggingComponent (this, e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        // Moves this Component according to the mouse drag event and applies our constraints to it
        dragger.dragComponent (this, e, &constrainer);
    }

private:
    ComponentBoundsConstrainer constrainer;
    ComponentDragger dragger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BallGeneratorComponent)
};

//==============================================================================
struct BallComponent  : public Component
{
    BallComponent (Point<float> pos)
        : position (pos),
          speed (Random::getSystemRandom().nextFloat() *  4.0f - 2.0f,
                 Random::getSystemRandom().nextFloat() * -6.0f - 2.0f),
          colour (Colours::white)
    {
        setSize (20, 20);
        step();
    }

    bool step()
    {
        position += speed;
        speed.y += 0.1f;

        setCentrePosition ((int) position.x,
                           (int) position.y);

        if (auto* parent = getParentComponent())
            return isPositiveAndBelow (position.x, (float) parent->getWidth())
                && position.y < (float) parent->getHeight();

        return position.y < 400.0f && position.x >= -10.0f;
    }

    void paint (Graphics& g) override
    {
        g.setColour (colour);
        g.fillEllipse (2.0f, 2.0f, (float) getWidth() - 4.0f, (float) getHeight() - 4.0f);

        g.setColour (Colours::darkgrey);
        g.drawEllipse (2.0f, 2.0f, (float) getWidth() - 4.0f, (float) getHeight() - 4.0f, 1.0f);
    }

    Point<float> position, speed;
    Colour colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BallComponent)
};

//==============================================================================
class AnimationDemo  : public Component,
                       private Timer
{
public:
    AnimationDemo()
    {
        setOpaque (true);

        for (auto i = 0; i < 11; ++i)
        {
            auto* b = createButton();
            componentsToAnimate.add (b);
            addAndMakeVisible (b);
            b->onClick = [this] { triggerAnimation(); };
        }

        addAndMakeVisible (ballGenerator);

        cycleCount = 2;
        startTimerHz (60);

        setSize (620, 620);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        ballGenerator.centreWithSize (80, 50);
        triggerAnimation();
    }

private:
    OwnedArray<Component> componentsToAnimate;
    OwnedArray<BallComponent> balls;
    BallGeneratorComponent ballGenerator;

    ComponentAnimator animator;
    int cycleCount;

    bool firstCallback = true;

    Button* createRandomButton()
    {
        DrawablePath normal, over;

        Path star1;
        star1.addStar ({}, 5, 20.0f, 50.0f, 0.2f);
        normal.setPath (star1);
        normal.setFill (Colours::red);

        Path star2;
        star2.addStar ({}, 7, 30.0f, 50.0f, 0.0f);
        over.setPath (star2);
        over.setFill (Colours::pink);
        over.setStrokeFill (Colours::black);
        over.setStrokeThickness (5.0f);

        auto juceIcon = getImageFromAssets ("juce_icon.png");

        DrawableImage down;
        down.setImage (juceIcon);
        down.setOverlayColour (Colours::black.withAlpha (0.3f));

        if (Random::getSystemRandom().nextInt (10) > 2)
        {
            auto type = Random::getSystemRandom().nextInt (3);

            auto* d = new DrawableButton ("Button",
                                          type == 0 ? DrawableButton::ImageOnButtonBackground
                                                    : (type == 1 ? DrawableButton::ImageFitted
                                                                 : DrawableButton::ImageAboveTextLabel));
            d->setImages (&normal,
                          Random::getSystemRandom().nextBool() ? &over : nullptr,
                          Random::getSystemRandom().nextBool() ? &down : nullptr);

            if (Random::getSystemRandom().nextBool())
            {
                d->setColour (DrawableButton::backgroundColourId,   getRandomBrightColour());
                d->setColour (DrawableButton::backgroundOnColourId, getRandomBrightColour());
            }

            d->setClickingTogglesState (Random::getSystemRandom().nextBool());
            return d;
        }

        auto* b = new ImageButton ("ImageButton");

        b->setImages (true, true, true,
                      juceIcon, 0.7f, Colours::transparentBlack,
                      juceIcon, 1.0f, getRandomDarkColour()  .withAlpha (0.2f),
                      juceIcon, 1.0f, getRandomBrightColour().withAlpha (0.8f),
                      0.5f);
        return b;
    }

    Button* createButton()
    {
        auto juceIcon = getImageFromAssets ("juce_icon.png").rescaled (128, 128);

        auto* b = new ImageButton ("ImageButton");

        b->setImages (true, true, true,
                      juceIcon, 1.0f, Colours::transparentBlack,
                      juceIcon, 1.0f, Colours::white,
                      juceIcon, 1.0f, Colours::white,
                      0.5f);

        return b;
    }

    void triggerAnimation()
    {
        auto width = getWidth();
        auto height = getHeight();

        bool useWidth = (height > width);

        for (auto* component : componentsToAnimate)
        {
            auto newIndex = (componentsToAnimate.indexOf (component) + 3 * cycleCount)
                             % componentsToAnimate.size();

            auto angle = (float) newIndex * MathConstants<float>::twoPi / (float) componentsToAnimate.size();

            auto radius = useWidth ? (float) width  * 0.35f
                                   : (float) height * 0.35f;

            Rectangle<int> r (getWidth()  / 2 + (int) (radius * std::sin (angle)) - 50,
                              getHeight() / 2 + (int) (radius * std::cos (angle)) - 50,
                              100, 100);

            animator.animateComponent (component, r.reduced (10), 1.0f,
                                       900 + (int) (300 * std::sin (angle)),
                                       false, 0.0, 0.0);
        }

        ++cycleCount;
    }

    void timerCallback() override
    {
        if (firstCallback)
        {
            triggerAnimation();
            firstCallback = false;
        }

        // Go through each of our balls and update their position
        for (int i = balls.size(); --i >= 0;)
            if (! balls.getUnchecked (i)->step())
                balls.remove (i);

        // Randomly generate new balls
        if (Random::getSystemRandom().nextInt (100) < 4)
            addAndMakeVisible (balls.add (new BallComponent (ballGenerator.getBounds().getCentre().toFloat())));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationDemo)
};
