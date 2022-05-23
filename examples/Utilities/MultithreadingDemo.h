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

 name:             MultithreadingDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Demonstrates multi-threading.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MultithreadingDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class BouncingBall   : private ComponentListener
{
public:
    BouncingBall (Component& comp)
        : containerComponent (comp)
    {
        containerComponent.addComponentListener (this);

        auto speed = 5.0f; // give each ball a fixed speed so we can
                           // see the effects of thread priority on how fast
                           // they actually go.

        auto angle = Random::getSystemRandom().nextFloat() * MathConstants<float>::twoPi;

        dx = std::sin (angle) * speed;
        dy = std::cos (angle) * speed;

        colour = Colour ((juce::uint32) Random::getSystemRandom().nextInt())
                    .withAlpha (0.5f)
                    .withBrightness (0.7f);

        componentMovedOrResized (containerComponent, true, true);

        x = Random::getSystemRandom().nextFloat() * parentWidth;
        y = Random::getSystemRandom().nextFloat() * parentHeight;
    }

    ~BouncingBall() override
    {
        containerComponent.removeComponentListener (this);
    }

    // This will be called from the message thread
    void draw (Graphics& g)
    {
        const ScopedLock lock (drawing);

        g.setColour (colour);
        g.fillEllipse (x, y, size, size);

        g.setColour (Colours::black);
        g.setFont (10.0f);
        g.drawText (String::toHexString ((int64) threadId), Rectangle<float> (x, y, size, size), Justification::centred, false);
    }

    void moveBall()
    {
        const ScopedLock lock (drawing);

        threadId = Thread::getCurrentThreadId(); // this is so the component can print the thread ID inside the ball

        x += dx;
        y += dy;

        if (x < 0)
            dx = std::abs (dx);

        if (x > parentWidth)
            dx = -std::abs (dx);

        if (y < 0)
            dy = std::abs (dy);

        if (y > parentHeight)
            dy = -std::abs (dy);
    }

private:
    void componentMovedOrResized (Component& comp, bool, bool) override
    {
        const ScopedLock lock (drawing);

        parentWidth  = (float) comp.getWidth()  - size;
        parentHeight = (float) comp.getHeight() - size;
    }

    float x = 0.0f, y = 0.0f,
          size = Random::getSystemRandom().nextFloat() * 30.0f + 30.0f,
          dx = 0.0f, dy = 0.0f,
          parentWidth = 50.0f, parentHeight = 50.0f;

    Colour colour;
    Thread::ThreadID threadId = {};
    CriticalSection drawing;

    Component& containerComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BouncingBall)
};

//==============================================================================
class DemoThread    : public BouncingBall,
                      public Thread
{
public:
    DemoThread (Component& containerComp)
        : BouncingBall (containerComp),
          Thread ("JUCE Demo Thread")
    {
        // give the threads a random priority, so some will move more
        // smoothly than others..
        startThread (Random::getSystemRandom().nextInt (3) + 3);
    }

    ~DemoThread() override
    {
        // allow the thread 2 seconds to stop cleanly - should be plenty of time.
        stopThread (2000);
    }

    void run() override
    {
        // this is the code that runs this thread - we'll loop continuously,
        // updating the coordinates of our blob.

        // threadShouldExit() returns true when the stopThread() method has been
        // called, so we should check it often, and exit as soon as it gets flagged.
        while (! threadShouldExit())
        {
            // sleep a bit so the threads don't all grind the CPU to a halt..
            wait (interval);

            // because this is a background thread, we mustn't do any UI work without
            // first grabbing a MessageManagerLock..
            const MessageManagerLock mml (Thread::getCurrentThread());

            if (! mml.lockWasGained())  // if something is trying to kill this job, the lock
                return;                 // will fail, in which case we'd better return..

            // now we've got the UI thread locked, we can mess about with the components
            moveBall();
        }
    }

private:
    int interval = Random::getSystemRandom().nextInt (50) + 6;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoThread)
};


//==============================================================================
class DemoThreadPoolJob  : public BouncingBall,
                           public ThreadPoolJob
{
public:
    DemoThreadPoolJob (Component& containerComp)
        : BouncingBall (containerComp),
          ThreadPoolJob ("Demo Threadpool Job")
    {}

    JobStatus runJob() override
    {
        // this is the code that runs this job. It'll be repeatedly called until we return
        // jobHasFinished instead of jobNeedsRunningAgain.
        Thread::sleep (30);

        // because this is a background thread, we mustn't do any UI work without
        // first grabbing a MessageManagerLock..
        const MessageManagerLock mml (this);

        // before moving the ball, we need to check whether the lock was actually gained, because
        // if something is trying to stop this job, it will have failed..
        if (mml.lockWasGained())
            moveBall();

        return jobNeedsRunningAgain;
    }

    void removedFromQueue()
    {
        // This is called to tell us that our job has been removed from the pool.
        // In this case there's no need to do anything here.
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoThreadPoolJob)
};

//==============================================================================
class MultithreadingDemo   : public Component,
                             private Timer
{
public:
    //==============================================================================
    MultithreadingDemo()
    {
        setOpaque (true);

        addAndMakeVisible (controlButton);
        controlButton.changeWidthToFitText (24);
        controlButton.setTopLeftPosition (20, 20);
        controlButton.setTriggeredOnMouseDown (true);
        controlButton.setAlwaysOnTop (true);
        controlButton.onClick = [this] { showMenu(); };

        setSize (500, 500);

        resetAllBalls();

        startTimerHz (60);
    }

    ~MultithreadingDemo() override
    {
        pool.removeAllJobs (true, 2000);
    }

    void resetAllBalls()
    {
        pool.removeAllJobs (true, 4000);
        balls.clear();

        for (int i = 0; i < 5; ++i)
            addABall();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));

        for (auto* ball : balls)
            ball->draw (g);
    }

private:
    //==============================================================================
    void setUsingPool (bool usePool)
    {
        isUsingPool = usePool;
        resetAllBalls();
    }

    void addABall()
    {
        if (isUsingPool)
        {
            auto newBall = std::make_unique<DemoThreadPoolJob> (*this);
            pool.addJob (newBall.get(), false);
            balls.add (newBall.release());

        }
        else
        {
            balls.add (new DemoThread (*this));
        }
    }

    void timerCallback() override
    {
        repaint();
    }

    void showMenu()
    {
        PopupMenu m;
        m.addItem (1, "Use one thread per ball", true, ! isUsingPool);
        m.addItem (2, "Use a thread pool",       true,   isUsingPool);

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (controlButton),
                         ModalCallbackFunction::forComponent (menuItemChosenCallback, this));
    }

    static void menuItemChosenCallback (int result, MultithreadingDemo* demoComponent)
    {
        if (result != 0 && demoComponent != nullptr)
            demoComponent->setUsingPool (result == 2);
    }

    //==============================================================================
    ThreadPool pool           { 3 };
    TextButton controlButton  { "Thread type" };
    bool isUsingPool = false;

    OwnedArray<BouncingBall> balls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultithreadingDemo)
};
