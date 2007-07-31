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

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MidiOutput.h"
#include "../../../juce_core/threads/juce_ScopedLock.h"
#include "../../../juce_core/basics/juce_Time.h"


//==============================================================================
MidiOutput::MidiOutput() throw()
    : Thread ("midi out"),
      internal (0),
      firstMessage (0)
{
}

MidiOutput::PendingMessage::PendingMessage (const uint8* const data,
                                            const int len,
                                            const double sampleNumber) throw()
    : message (data, len, sampleNumber)
{
}

void MidiOutput::sendBlockOfMessages (const MidiBuffer& buffer,
                                      const double millisecondCounterToStartAt,
                                      double samplesPerSecondForBuffer) throw()
{
    // You've got to call startBackgroundThread() for this to actually work..
    jassert (isThreadRunning());

    // this needs to be a value in the future - RTFM for this method!
    jassert (millisecondCounterToStartAt > 0);

    samplesPerSecondForBuffer *= 0.001;

    MidiBuffer::Iterator i (buffer);

    const uint8* data;
    int len, time;

    while (i.getNextEvent (data, len, time))
    {
        const double eventTime = millisecondCounterToStartAt + samplesPerSecondForBuffer * time;

        PendingMessage* const m
            = new PendingMessage (data, len, eventTime);

        const ScopedLock sl (lock);

        if (firstMessage == 0)
        {
            firstMessage = m;
            m->next = 0;
        }
        else
        {
            PendingMessage* mm = firstMessage;

            while (mm->next != 0 && mm->next->message.getTimeStamp() <= eventTime)
                mm = mm->next;

            m->next = mm->next;
            mm->next = m;
        }
    }

    notify();
}

void MidiOutput::clearAllPendingMessages() throw()
{
    const ScopedLock sl (lock);

    while (firstMessage != 0)
    {
        PendingMessage* const m = firstMessage;
        firstMessage = firstMessage->next;
        delete m;
    }
}

void MidiOutput::startBackgroundThread() throw()
{
    startThread (9);
}

void MidiOutput::stopBackgroundThread() throw()
{
    stopThread (5000);
}

void MidiOutput::run()
{
    while (! threadShouldExit())
    {
        uint32 now = Time::getMillisecondCounter();
        uint32 eventTime = 0;
        uint32 timeToWait = 500;

        lock.enter();
        PendingMessage* message = firstMessage;

        if (message != 0)
        {
            eventTime = roundDoubleToInt (message->message.getTimeStamp());

            if (eventTime > now + 20)
            {
                timeToWait = jmax (10, eventTime - now - 100);
                message = 0;
            }
            else
            {
                firstMessage = message->next;
            }
        }

        lock.exit();

        if (message != 0)
        {
            if (eventTime > now)
            {
                Time::waitForMillisecondCounter (eventTime);

                if (threadShouldExit())
                    break;
            }

            if (eventTime > now - 200)
                sendMessageNow (message->message);

            delete message;
        }
        else
        {
            jassert (timeToWait < 1000 * 30);
            wait (timeToWait);
        }
    }

    clearAllPendingMessages();
}


END_JUCE_NAMESPACE
