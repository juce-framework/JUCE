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

#include "../JuceDemoHeader.h"

//==============================================================================
/** This will be the source of our balls and can be dragged around. */
class BallGeneratorComponent    : public Component
{
public:
    BallGeneratorComponent()
    {
    }

    void paint (Graphics& g) override
    {
        Rectangle<float> area (getLocalBounds().toFloat().reduced (2.0f));
        g.setColour (Colours::orange.withAlpha (0.6f));
        g.fillRoundedRectangle (area, 10.0f);

        g.setColour (Colours::darkgrey);
        g.drawRoundedRectangle (area, 10.0f, 2.0f);

        AttributedString s;
        s.setJustification (Justification::centred);
        s.setWordWrap (AttributedString::none);
        s.append ("Balls!\n"
                  "(Drag Me)");
        s.setColour (Colours::black);
        s.draw (g, area);
    }

    void resized() override
    {
        // Just set the limits of our constrainer so that we don't drag ourselves off the screen
        constrainer.setMinimumOnscreenAmounts (getHeight(), getWidth(), getHeight(), getWidth());
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
    BallComponent (const Point<float>& pos)
        : position (pos),
          speed (Random::getSystemRandom().nextFloat() *  4.0f - 2.0f,
                 Random::getSystemRandom().nextFloat() * -6.0f - 2.0f),
          colour (getRandomBrightColour().withAlpha (0.4f))
    {
        setSize (20, 20);
        step();
    }

    bool step()
    {
        position += speed;
        speed.y += 0.05f;

        setCentrePosition ((int) position.x,
                           (int) position.y);

        if (Component* parent = getParentComponent())
            return isPositiveAndBelow (position.x, (float) parent->getWidth())
                     && position.y < (float) parent->getHeight();

        return position.y < 400.0f && position.x >= -10.0f;
    }

    void paint (Graphics& g)
    {
        g.setColour (colour);
        g.fillEllipse (2.0f, 2.0f, getWidth() - 4.0f, getHeight() - 4.0f);

        g.setColour (Colours::darkgrey);
        g.drawEllipse (2.0f, 2.0f, getWidth() - 4.0f, getHeight() - 4.0f, 1.0f);
    }

    Point<float> position, speed;
    Colour colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BallComponent)
};

//==============================================================================
class AnimationDemo  : public Component,
                       private Button::Listener,
                       private Timer
{
public:
    AnimationDemo()
    {
        setOpaque (true);

        for (int i = 11; --i >= 0;)
        {
            Button* b = createRandomButton();
            componentsToAnimate.add (b);
            addAndMakeVisible (b);
            b->addListener (this);
        }

        ballGenerator = new BallGeneratorComponent();
        componentsToAnimate.add (ballGenerator);
        addAndMakeVisible (ballGenerator);
        ballGenerator->setBounds (200, 500, 70, 50);

        int w = 160, h = 80;

        for (int i = 0; i < componentsToAnimate.size(); ++i)
        {
            Rectangle<int> r (w * (i % 3), h * (i / 3), w, h);
            componentsToAnimate.getUnchecked(i)->setBounds (r.reduced (10));
        }

        startTimer (1000 / 60);
    }

    void paint (Graphics& g) override
    {
        fillBrushedAluminiumBackground (g);
    }

private:
    OwnedArray<Component> componentsToAnimate;
    OwnedArray<BallComponent> balls;
    BallGeneratorComponent* ballGenerator;

    ComponentAnimator animator;

    Button* createRandomButton()
    {
        DrawablePath normal, over;

        Path star1;
        star1.addStar (Point<float>(), 5, 20.0f, 50.0f, 0.2f);
        normal.setPath (star1);
        normal.setFill (Colours::red);

        Path star2;
        star2.addStar (Point<float>(), 7, 30.0f, 50.0f, 0.0f);
        over.setPath (star2);
        over.setFill (Colours::pink);
        over.setStrokeFill (Colours::black);
        over.setStrokeThickness (5.0f);

        DrawableImage down;
        down.setImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
        down.setOverlayColour (Colours::black.withAlpha (0.3f));

        if (Random::getSystemRandom().nextInt (10) > 2)
        {
            int type = Random::getSystemRandom().nextInt (3);

            DrawableButton* d = new DrawableButton ("Button",
                                                    type == 0 ? DrawableButton::ImageOnButtonBackground
                                                              : (type == 1 ? DrawableButton::ImageFitted
                                                                           : DrawableButton::ImageAboveTextLabel));
            d->setImages (&normal,
                          Random::getSystemRandom().nextBool() ? &over : nullptr,
                          Random::getSystemRandom().nextBool() ? &down : nullptr);

            if (Random::getSystemRandom().nextBool())
            {
                d->setColour (DrawableButton::backgroundColourId, getRandomBrightColour());
                d->setColour (DrawableButton::backgroundOnColourId, getRandomBrightColour());
            }

            d->setClickingTogglesState (Random::getSystemRandom().nextBool());
            return d;
        }

        ImageButton* b = new ImageButton ("ImageButton");

        Image image = ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize);
        b->setImages (true, true, true,
                      image, 0.7f, Colours::transparentBlack,
                      image, 1.0f, getRandomDarkColour().withAlpha (0.2f),
                      image, 1.0f, getRandomBrightColour().withAlpha (0.8f),
                      0.5f);
        return b;
    }

    void buttonClicked (Button*) override
    {
        for (int i = 0; i < componentsToAnimate.size(); ++i)
            componentsToAnimate.swap (i, Random::getSystemRandom().nextInt (componentsToAnimate.size()));

        int w = 160, h = 80;

        for (int i = 0; i < componentsToAnimate.size(); ++i)
        {
            Rectangle<int> r (w * (i % 3), h * (i / 3), w, h);

            animator.animateComponent (componentsToAnimate.getUnchecked(i),
                                       r.reduced (10),
                                       Random::getSystemRandom().nextBool() ? 1.0f : 0.4f,
                                       500 + Random::getSystemRandom().nextInt (2000),
                                       false,
                                       Random::getSystemRandom().nextDouble(),
                                       Random::getSystemRandom().nextDouble());
        }
    }

    void timerCallback() override
    {
        // Go through each of our balls and update their position
        for (int i = balls.size(); --i >= 0;)
            if (! balls.getUnchecked(i)->step())
                balls.remove (i);

        // Randomly generate new balls
        if (Random::getSystemRandom().nextInt (200) < 4)
        {
            BallComponent* ball = new BallComponent (ballGenerator->getBounds().getCentre().toFloat());
            addAndMakeVisible (ball);
            balls.add (ball);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<AnimationDemo> demo ("10 Components: Animation");
