/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__
#define __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__

#include "../windows/juce_AlertWindow.h"


//==============================================================================
/**
    A thread that automatically pops up a modal dialog box with a progress bar
    and cancel button while it's busy running.

    These are handy for performing some sort of task while giving the user feedback
    about how long there is to go, etc.

    E.g. @code
    class MyTask  : public ThreadWithProgressWindow
    {
    public:
        MyTask()    : ThreadWithProgressWindow ("busy...", true, true)
        {
        }

        void run()
        {
            for (int i = 0; i < thingsToDo; ++i)
            {
                // must check this as often as possible, because this is
                // how we know if the user's pressed 'cancel'
                if (threadShouldExit())
                    break;

                // this will update the progress bar on the dialog box
                setProgress (i / (double) thingsToDo);


                //   ... do the business here...
            }
        }
    };

    void doTheTask()
    {
        MyTask m;

        if (m.runThread())
        {
            // thread finished normally..
        }
        else
        {
            // user pressed the cancel button..
        }
    }

    @endcode

    @see Thread, AlertWindow
*/
class JUCE_API  ThreadWithProgressWindow  : public Thread,
                                            private Timer
{
public:
    //==============================================================================
    /** Creates the thread.

        Initially, the dialog box won't be visible, it'll only appear when the
        runThread() method is called.

        @param windowTitle              the title to go at the top of the dialog box
        @param hasProgressBar           whether the dialog box should have a progress bar (see
                                        setProgress() )
        @param hasCancelButton          whether the dialog box should have a cancel button
        @param timeOutMsWhenCancelling  when 'cancel' is pressed, this is how long to wait for
                                        the thread to stop before killing it forcibly (see
                                        Thread::stopThread() )
        @param cancelButtonText         the text that should be shown in the cancel button
                                        (if it has one)
        @param componentToCentreAround  if this is non-null, the window will be positioned
                                        so that it's centred around this component.
    */
    ThreadWithProgressWindow (const String& windowTitle,
                              bool hasProgressBar,
                              bool hasCancelButton,
                              int timeOutMsWhenCancelling = 10000,
                              const String& cancelButtonText = "Cancel",
                              Component* componentToCentreAround = nullptr);

    /** Destructor. */
    ~ThreadWithProgressWindow();

    //==============================================================================
    /** Starts the thread and waits for it to finish.

        This will start the thread, make the dialog box appear, and wait until either
        the thread finishes normally, or until the cancel button is pressed.

        Before returning, the dialog box will be hidden.

        @param threadPriority   the priority to use when starting the thread - see
                                Thread::startThread() for values
        @returns true if the thread finished normally; false if the user pressed cancel
    */
    bool runThread (int threadPriority = 5);

    /** The thread should call this periodically to update the position of the progress bar.

        @param newProgress  the progress, from 0.0 to 1.0
        @see setStatusMessage
    */
    void setProgress (double newProgress);

    /** The thread can call this to change the message that's displayed in the dialog box.
    */
    void setStatusMessage (const String& newStatusMessage);

    /** Returns the AlertWindow that is being used.
    */
    AlertWindow* getAlertWindow() const noexcept        { return alertWindow; }

private:
    //==============================================================================
    void timerCallback();

    double progress;
    ScopedPointer <AlertWindow> alertWindow;
    String message;
    CriticalSection messageLock;
    const int timeOutMsWhenCancelling;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadWithProgressWindow)
};

#endif   // __JUCE_THREADWITHPROGRESSWINDOW_JUCEHEADER__
