/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ThreadWithProgressWindow::ThreadWithProgressWindow (const String& title,
                                                    const bool hasProgressBar,
                                                    const bool hasCancelButton,
                                                    const int cancellingTimeOutMs,
                                                    const String& cancelButtonText,
                                                    Component* componentToCentreAround)
   : Thread ("ThreadWithProgressWindow"),
     progress (0.0),
     timeOutMsWhenCancelling (cancellingTimeOutMs),
     wasCancelledByUser (false)
{
    alertWindow.reset (LookAndFeel::getDefaultLookAndFeel()
                           .createAlertWindow (title, {},
                                               cancelButtonText.isEmpty() ? TRANS("Cancel")
                                                                          : cancelButtonText,
                                               {}, {}, MessageBoxIconType::NoIcon, hasCancelButton ? 1 : 0,
                                               componentToCentreAround));

    // if there are no buttons, we won't allow the user to interrupt the thread.
    alertWindow->setEscapeKeyCancels (false);

    if (hasProgressBar)
        alertWindow->addProgressBarComponent (progress);
}

ThreadWithProgressWindow::~ThreadWithProgressWindow()
{
    stopThread (timeOutMsWhenCancelling);
}

void ThreadWithProgressWindow::launchThread (Priority threadPriority)
{
    JUCE_ASSERT_MESSAGE_THREAD

    startThread (threadPriority);
    startTimer (100);

    {
        const ScopedLock sl (messageLock);
        alertWindow->setMessage (message);
    }

    alertWindow->enterModalState();
}

void ThreadWithProgressWindow::setProgress (const double newProgress)
{
    progress = newProgress;
}

void ThreadWithProgressWindow::setStatusMessage (const String& newStatusMessage)
{
    const ScopedLock sl (messageLock);
    message = newStatusMessage;
}

void ThreadWithProgressWindow::timerCallback()
{
    bool threadStillRunning = isThreadRunning();

    if (! (threadStillRunning && alertWindow->isCurrentlyModal (false)))
    {
        stopTimer();
        stopThread (timeOutMsWhenCancelling);
        alertWindow->exitModalState (1);
        alertWindow->setVisible (false);

        wasCancelledByUser = threadStillRunning;
        threadComplete (threadStillRunning);
        return; // (this may be deleted now)
    }

    const ScopedLock sl (messageLock);
    alertWindow->setMessage (message);
}

void ThreadWithProgressWindow::threadComplete (bool) {}

#if JUCE_MODAL_LOOPS_PERMITTED
bool ThreadWithProgressWindow::runThread (Priority threadPriority)
{
    launchThread (threadPriority);

    while (isTimerRunning())
        MessageManager::getInstance()->runDispatchLoopUntil (5);

    return ! wasCancelledByUser;
}
#endif

} // namespace juce
