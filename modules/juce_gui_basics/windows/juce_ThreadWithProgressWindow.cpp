/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

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
    alertWindow = LookAndFeel::getDefaultLookAndFeel()
                    .createAlertWindow (title, String(),
                                        cancelButtonText.isEmpty() ? TRANS("Cancel")
                                                                   : cancelButtonText,
                                        String(), String(),
                                        AlertWindow::NoIcon, hasCancelButton ? 1 : 0,
                                        componentToCentreAround);

    // if there are no buttons, we won't allow the user to interrupt the thread.
    alertWindow->setEscapeKeyCancels (false);

    if (hasProgressBar)
        alertWindow->addProgressBarComponent (progress);
}

ThreadWithProgressWindow::~ThreadWithProgressWindow()
{
    stopThread (timeOutMsWhenCancelling);
}

void ThreadWithProgressWindow::launchThread (int priority)
{
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    startThread (priority);
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

    if (! (threadStillRunning && alertWindow->isCurrentlyModal()))
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
bool ThreadWithProgressWindow::runThread (const int priority)
{
    launchThread (priority);

    while (isTimerRunning())
        MessageManager::getInstance()->runDispatchLoopUntil (5);

    return ! wasCancelledByUser;
}
#endif
