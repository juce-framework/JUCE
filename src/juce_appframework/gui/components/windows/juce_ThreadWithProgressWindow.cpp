/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_ThreadWithProgressWindow.h"
#include "../../../../juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
ThreadWithProgressWindow::ThreadWithProgressWindow (const String& title,
                                                    const bool hasProgressBar,
                                                    const bool hasCancelButton,
                                                    const int timeOutMsWhenCancelling_)
  : Thread ("Juce Progress Window"),
    progress (0.0),
    alertWindow (title, String::empty, AlertWindow::NoIcon),
    timeOutMsWhenCancelling (timeOutMsWhenCancelling_)
{
    if (hasProgressBar)
        alertWindow.addProgressBarComponent (progress);

    if (hasCancelButton)
        alertWindow.addButton (TRANS("cancel"), 1);
}

ThreadWithProgressWindow::~ThreadWithProgressWindow()
{
    stopThread (timeOutMsWhenCancelling);
}

bool ThreadWithProgressWindow::runThread (const int priority)
{
    startThread (priority);
    startTimer (100);

    alertWindow.setMessage (message);

    const bool wasCancelled = alertWindow.runModalLoop() != 0;

    stopThread (timeOutMsWhenCancelling);

    alertWindow.setVisible (false);

    return ! wasCancelled;
}

void ThreadWithProgressWindow::setProgress (const double newProgress)
{
    progress = newProgress;
}

void ThreadWithProgressWindow::setStatusMessage (const String& newStatusMessage)
{
    message = newStatusMessage;
}

void ThreadWithProgressWindow::timerCallback()
{
    if (! isThreadRunning())
    {
        // thread has finished normally..
        alertWindow.setVisible (false);
    }
    else
    {
        alertWindow.setMessage (message);
    }
}

END_JUCE_NAMESPACE
