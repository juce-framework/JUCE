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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_ThreadWithProgressWindow.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../text/juce_LocalisedStrings.h"


//==============================================================================
ThreadWithProgressWindow::ThreadWithProgressWindow (const String& title,
                                                    const bool hasProgressBar,
                                                    const bool hasCancelButton,
                                                    const int timeOutMsWhenCancelling_,
                                                    const String& cancelButtonText)
  : Thread ("Juce Progress Window"),
    progress (0.0),
    timeOutMsWhenCancelling (timeOutMsWhenCancelling_)
{
    alertWindow = LookAndFeel::getDefaultLookAndFeel()
                    .createAlertWindow (title, String::empty, cancelButtonText,
                                        String::empty, String::empty,
                                        AlertWindow::NoIcon, hasCancelButton ? 1 : 0, 0);

    if (hasProgressBar)
        alertWindow->addProgressBarComponent (progress);
}

ThreadWithProgressWindow::~ThreadWithProgressWindow()
{
    stopThread (timeOutMsWhenCancelling);
}

bool ThreadWithProgressWindow::runThread (const int priority)
{
    startThread (priority);
    startTimer (100);

    {
        const ScopedLock sl (messageLock);
        alertWindow->setMessage (message);
    }

    const bool finishedNaturally = alertWindow->runModalLoop() != 0;

    stopThread (timeOutMsWhenCancelling);

    alertWindow->setVisible (false);

    return finishedNaturally;
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
    if (! isThreadRunning())
    {
        // thread has finished normally..
        alertWindow->exitModalState (1);
        alertWindow->setVisible (false);
    }
    else
    {
        const ScopedLock sl (messageLock);
        alertWindow->setMessage (message);
    }
}

END_JUCE_NAMESPACE
