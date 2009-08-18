/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"


//==============================================================================
class BouncingBallComp  : public Component
{
    float x, y, size, dx, dy, w, h, parentWidth, parentHeight;
    float innerX, innerY;
    Colour colour;
    Thread::ThreadID threadId;

public:
    BouncingBallComp()
    {
        x = Random::getSystemRandom().nextFloat() * 200.0f;
        y = Random::getSystemRandom().nextFloat() * 200.0f;
        parentWidth = 50;
        parentHeight = 50;
        innerX = 0;
        innerY = 0;
        threadId = 0;

        const float speed = 5.0f; // give each ball a fixed speed so we can
                                  // see the effects of thread priority on how fast
                                  // they actually go.
        const float angle = Random::getSystemRandom().nextFloat() * float_Pi * 2.0f;

        dx = sinf (angle) * speed;
        dy = cosf (angle) * speed;

        size = Random::getSystemRandom().nextFloat() * 30.0f + 30.0f;

        colour = Colour (Random::getSystemRandom().nextInt())
                    .withAlpha (0.5f)
                    .withBrightness (0.7f);
    }

    ~BouncingBallComp()
    {
    }

    void paint (Graphics& g)
    {
        g.setColour (colour);
        g.fillEllipse (innerX, innerY, size, size);

        g.setColour (Colours::black);
        g.setFont (10.0f);
        g.drawText (String::toHexString ((int64) threadId), 0, 0, getWidth(), getHeight(), Justification::centred, false);
    }

    void parentSizeChanged()
    {
        parentWidth = getParentWidth() - size;
        parentHeight = getParentHeight() - size;
    }

    void moveBall()
    {
        threadId = Thread::getCurrentThreadId(); // this is so the component can print the thread ID inside the ball

        x += dx;
        y += dy;

        if (x < 0)
            dx = fabsf (dx);

        if (x > parentWidth)
            dx = -fabsf (dx);

        if (y < 0)
            dy = fabsf (dy);

        if (y > parentHeight)
            dy = -fabsf (dy);

        setBounds (((int) x) - 2,
                   ((int) y) - 2,
                   ((int) size) + 4,
                   ((int) size) + 4);

        innerX = x - getX();
        innerY = y - getY();

        repaint();
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
class DemoThread    : public BouncingBallComp,
                      public Thread
{
    int interval;

public:
    DemoThread()
        : Thread (T("Juce Demo Thread"))
    {
        interval = Random::getSystemRandom().nextInt (50) + 6;

        // give the threads a random priority, so some will move more
        // smoothly than others..
        startThread (Random::getSystemRandom().nextInt (3) + 3);
    }

    ~DemoThread()
    {
        // allow the thread 2 seconds to stop cleanly - should be plenty of time.
        stopThread (2000);
    }

    void run()
    {
        // this is the code that runs this thread - we'll loop continuously,
        // updating the co-ordinates of our blob.

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

    juce_UseDebuggingNewOperator
};


//==============================================================================
class DemoThreadPoolJob  : public BouncingBallComp,
                           public ThreadPoolJob
{
public:
    DemoThreadPoolJob()
        : ThreadPoolJob (T("Demo Threadpool Job"))
    {
    }

    ~DemoThreadPoolJob()
    {
    }

    JobStatus runJob()
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

    juce_UseDebuggingNewOperator
};

//==============================================================================
class ThreadingDemo  : public Component,
                       public Timer,
                       public ButtonListener
{
    bool isUsingPool;
    ThreadPool pool;
    TextButton* controlButton;

public:
    //==============================================================================
    ThreadingDemo()
        : pool (3)
    {
        isUsingPool = false;

        setName (T("Multithreading"));

        setOpaque (true);
    }

    ~ThreadingDemo()
    {
        pool.removeAllJobs (true, 2000);

        deleteAllChildren();
    }

    // this gets called when a component is added or removed from a parent component.
    void parentHierarchyChanged()
    {
        // we'll use this as an opportunity to start and stop the threads, so that
        // we don't leave them going when the component's not actually visible.
        stopTimer();

        pool.removeAllJobs (true, 4000);
        deleteAllChildren();

        addAndMakeVisible (controlButton = new TextButton (T("Thread type")));
        controlButton->changeWidthToFitText (20);
        controlButton->setTopLeftPosition (20, 20);
        controlButton->setTriggeredOnMouseDown (true);
        controlButton->setAlwaysOnTop (true);
        controlButton->addButtonListener (this);

        if (isShowing())
        {
            while (getNumChildComponents() < 5)
                addABall();

            startTimer (2000);
        }
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
    }

    void setUsingPool (bool usePool)
    {
        isUsingPool = usePool;
        parentHierarchyChanged(); // resets everything
    }

    void addABall()
    {
        if (isUsingPool)
        {
            DemoThreadPoolJob* newComp = new DemoThreadPoolJob();
            addAndMakeVisible (newComp);
            newComp->parentSizeChanged();

            pool.addJob (newComp);
        }
        else
        {
            DemoThread* newComp = new DemoThread();
            addAndMakeVisible (newComp);
            newComp->parentSizeChanged();
        }
    }

    void removeABall()
    {
        if (isUsingPool)
        {
            ThreadPoolJob* jobToRemove = pool.getJob (Random::getSystemRandom().nextInt (pool.getNumJobs()));

            if (jobToRemove != 0)
            {
                pool.removeJob (jobToRemove, true, 4000);
                delete jobToRemove;
            }
        }
        else
        {
            if (getNumChildComponents() > 1)
            {
                Component* ball = getChildComponent (1 + Random::getSystemRandom().nextInt (getNumChildComponents() - 1));

                if (dynamic_cast <Button*> (ball) == 0) // don't delete our button!
                    delete ball;
            }
        }
    }

    void timerCallback()
    {
        if (Random::getSystemRandom().nextBool())
        {
            if (getNumChildComponents() <= 10)
                addABall();
        }
        else
        {
            if (getNumChildComponents() > 3)
                removeABall();
        }
    }

    void buttonClicked (Button* button)
    {
        PopupMenu m;
        m.addItem (1, T("Use one thread per ball"), true, ! isUsingPool);
        m.addItem (2, T("Use a thread pool"), true, isUsingPool);

        const int res = m.showAt (button);

        if (res != 0)
            setUsingPool (res == 2);
    }
};



//==============================================================================
Component* createThreadingDemo()
{
    return new ThreadingDemo();
}
