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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MidiOutput.h"
#include "../../threads/juce_ScopedLock.h"
#include "../../core/juce_Time.h"


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

    const double timeScaleFactor = 1000.0 / samplesPerSecondForBuffer;

    MidiBuffer::Iterator i (buffer);

    const uint8* data;
    int len, time;

    while (i.getNextEvent (data, len, time))
    {
        const double eventTime = millisecondCounterToStartAt + timeScaleFactor * time;

        PendingMessage* const m
            = new PendingMessage (data, len, eventTime);

        const ScopedLock sl (lock);

        if (firstMessage == 0 || firstMessage->message.getTimeStamp() > eventTime)
        {
            m->next = firstMessage;
            firstMessage = m;
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
            eventTime = roundToInt (message->message.getTimeStamp());

            if (eventTime > now + 20)
            {
                timeToWait = eventTime - (now + 20);
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
