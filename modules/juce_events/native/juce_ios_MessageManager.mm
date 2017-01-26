/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

void MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    while (! quitMessagePosted)
    {
        JUCE_AUTORELEASEPOOL
        {
            [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                                     beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.001]];
        }
    }
}

void MessageManager::stopDispatchLoop()
{
    if (! SystemStats::isRunningInAppExtensionSandbox())
       [[[UIApplication sharedApplication] delegate] applicationWillTerminate: [UIApplication sharedApplication]];

    exit (0); // iOS apps get no mercy..
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    JUCE_AUTORELEASEPOOL
    {
        jassert (isThisTheMessageThread()); // must only be called by the message thread

        uint32 startTime = Time::getMillisecondCounter();
        NSDate* endDate = [NSDate dateWithTimeIntervalSinceNow: millisecondsToRunFor * 0.001];

        while (! quitMessagePosted)
        {
            JUCE_AUTORELEASEPOOL
            {
                [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                                         beforeDate: endDate];

                if (millisecondsToRunFor >= 0
                     && Time::getMillisecondCounter() >= startTime + (uint32) millisecondsToRunFor)
                    break;
            }
        }

        return ! quitMessagePosted;
    }
}
#endif

//==============================================================================
static ScopedPointer<MessageQueue> messageQueue;

void MessageManager::doPlatformSpecificInitialisation()
{
    if (messageQueue == nullptr)
        messageQueue = new MessageQueue();
}

void MessageManager::doPlatformSpecificShutdown()
{
    messageQueue = nullptr;
}

bool MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    if (messageQueue != nullptr)
        messageQueue->post (message);

    return true;
}

void MessageManager::broadcastMessage (const String&)
{
    // N/A on current iOS
}
