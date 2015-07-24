/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

struct MidiOutput::PendingMessage
{
    PendingMessage (const void* const data, const int len, const double timeStamp)
        : message (data, len, timeStamp)
    {}

    MidiMessage message;
    PendingMessage* next;
};

MidiOutput::MidiOutput()
    : Thread ("midi out"),
      internal (nullptr),
      firstMessage (nullptr)
{
}

void MidiOutput::sendBlockOfMessages (const MidiBuffer& buffer,
                                      const double millisecondCounterToStartAt,
                                      double samplesPerSecondForBuffer)
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

        PendingMessage* const m = new PendingMessage (data, len, eventTime);

        const ScopedLock sl (lock);

        if (firstMessage == nullptr || firstMessage->message.getTimeStamp() > eventTime)
        {
            m->next = firstMessage;
            firstMessage = m;
        }
        else
        {
            PendingMessage* mm = firstMessage;

            while (mm->next != nullptr && mm->next->message.getTimeStamp() <= eventTime)
                mm = mm->next;

            m->next = mm->next;
            mm->next = m;
        }
    }

    notify();
}

void MidiOutput::clearAllPendingMessages()
{
    const ScopedLock sl (lock);

    while (firstMessage != nullptr)
    {
        PendingMessage* const m = firstMessage;
        firstMessage = firstMessage->next;
        delete m;
    }
}

void MidiOutput::startBackgroundThread()
{
    startThread (9);
}

void MidiOutput::stopBackgroundThread()
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

        PendingMessage* message;

        {
            const ScopedLock sl (lock);
            message = firstMessage;

            if (message != nullptr)
            {
                eventTime = (uint32) roundToInt (message->message.getTimeStamp());

                if (eventTime > now + 20)
                {
                    timeToWait = eventTime - (now + 20);
                    message = nullptr;
                }
                else
                {
                    firstMessage = message->next;
                }
            }
        }

        if (message != nullptr)
        {
            const ScopedPointer<PendingMessage> messageDeleter (message);

            if (eventTime > now)
            {
                Time::waitForMillisecondCounter (eventTime);

                if (threadShouldExit())
                    break;
            }

            if (eventTime > now - 200)
                sendMessageNow (message->message);
        }
        else
        {
            jassert (timeToWait < 1000 * 30);
            wait ((int) timeToWait);
        }
    }

    clearAllPendingMessages();
}
