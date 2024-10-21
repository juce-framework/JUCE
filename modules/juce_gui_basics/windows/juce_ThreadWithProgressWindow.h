/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

    @tags{GUI}
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
                                        (if it has one). Leave this empty for the default "Cancel"
        @param componentToCentreAround  if this is non-null, the window will be positioned
                                        so that it's centred around this component.
    */
    ThreadWithProgressWindow (const String& windowTitle,
                              bool hasProgressBar,
                              bool hasCancelButton,
                              int timeOutMsWhenCancelling = 10000,
                              const String& cancelButtonText = String(),
                              Component* componentToCentreAround = nullptr);

    /** Destructor. */
    ~ThreadWithProgressWindow() override;

    //==============================================================================
   #if JUCE_MODAL_LOOPS_PERMITTED
    /** Starts the thread and waits for it to finish.

        This will start the thread, make the dialog box appear, and wait until either
        the thread finishes normally, or until the cancel button is pressed.

        Before returning, the dialog box will be hidden.

        @param priority   the priority to use when starting the thread - see
                          Thread::Priority for values
        @returns true if the thread finished normally; false if the user pressed cancel
    */
    bool runThread (Priority priority = Priority::normal);
   #endif

    /** Starts the thread and returns.

        This will start the thread and make the dialog box appear in a modal state. When
        the thread finishes normally, or the cancel button is pressed, the window will be
        hidden and the threadComplete() method will be called.

        @param priority   the priority to use when starting the thread - see
                          Thread::Priority for values
    */
    void launchThread (Priority priority = Priority::normal);

    /** The thread should call this periodically to update the position of the progress bar.

        @param newProgress  the progress, from 0.0 to 1.0
        @see setStatusMessage
    */
    void setProgress (double newProgress);

    /** The thread can call this to change the message that's displayed in the dialog box. */
    void setStatusMessage (const String& newStatusMessage);

    /** Returns the AlertWindow that is being used. */
    AlertWindow* getAlertWindow() const noexcept        { return alertWindow.get(); }

    //==============================================================================
    /** This method is called (on the message thread) when the operation has finished.
        You may choose to use this callback to delete the ThreadWithProgressWindow object.
    */
    virtual void threadComplete (bool userPressedCancel);

private:
    //==============================================================================
    void timerCallback() override;

    double progress;
    std::unique_ptr<AlertWindow> alertWindow;
    String message;
    CriticalSection messageLock;
    const int timeOutMsWhenCancelling;
    bool wasCancelledByUser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadWithProgressWindow)
};

} // namespace juce
