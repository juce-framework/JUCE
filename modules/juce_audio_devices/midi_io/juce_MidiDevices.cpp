/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class MidiDeviceListConnectionBroadcaster : private AsyncUpdater
{
public:
    ~MidiDeviceListConnectionBroadcaster() override
    {
        cancelPendingUpdate();
    }

    MidiDeviceListConnection::Key add (std::function<void()> callback)
    {
        JUCE_ASSERT_MESSAGE_THREAD
        return callbacks.emplace (key++, std::move (callback)).first->first;
    }

    void remove (const MidiDeviceListConnection::Key k)
    {
        JUCE_ASSERT_MESSAGE_THREAD
        callbacks.erase (k);
    }

    void notify()
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            cancelPendingUpdate();

            const State newState;

            if (std::exchange (lastNotifiedState, newState) != newState)
                for (auto it = callbacks.begin(); it != callbacks.end();)
                    NullCheckedInvocation::invoke ((it++)->second);
        }
        else
        {
            triggerAsyncUpdate();
        }
    }

    static auto& get()
    {
        static MidiDeviceListConnectionBroadcaster result;
        return result;
    }

private:
    MidiDeviceListConnectionBroadcaster() = default;

    class State
    {
        Array<MidiDeviceInfo> ins = MidiInput::getAvailableDevices(), outs = MidiOutput::getAvailableDevices();
        auto tie() const { return std::tie (ins, outs); }

    public:
        bool operator== (const State& other) const { return tie() == other.tie(); }
        bool operator!= (const State& other) const { return tie() != other.tie(); }
    };

    void handleAsyncUpdate() override
    {
        notify();
    }

    std::map<MidiDeviceListConnection::Key, std::function<void()>> callbacks;
    State lastNotifiedState;
    MidiDeviceListConnection::Key key = 0;
};

//==============================================================================
MidiDeviceListConnection::~MidiDeviceListConnection() noexcept
{
    if (broadcaster != nullptr)
        broadcaster->remove (key);
}

//==============================================================================
void MidiInputCallback::handlePartialSysexMessage ([[maybe_unused]] MidiInput* source,
                                                   [[maybe_unused]] const uint8* messageData,
                                                   [[maybe_unused]] int numBytesSoFar,
                                                   [[maybe_unused]] double timestamp) {}

//==============================================================================
MidiOutput::MidiOutput (const String& deviceName, const String& deviceIdentifier)
    : Thread ("midi out"), deviceInfo (deviceName, deviceIdentifier)
{
}

void MidiOutput::sendBlockOfMessagesNow (const MidiBuffer& buffer)
{
    for (const auto metadata : buffer)
        sendMessageNow (metadata.getMessage());
}

void MidiOutput::sendBlockOfMessages (const MidiBuffer& buffer,
                                      double millisecondCounterToStartAt,
                                      double samplesPerSecondForBuffer)
{
    // You've got to call startBackgroundThread() for this to actually work..
    jassert (isThreadRunning());

    // this needs to be a value in the future - RTFM for this method!
    jassert (millisecondCounterToStartAt > 0);

    auto timeScaleFactor = 1000.0 / samplesPerSecondForBuffer;

    for (const auto metadata : buffer)
    {
        auto eventTime = millisecondCounterToStartAt + timeScaleFactor * metadata.samplePosition;
        auto* m = new PendingMessage (metadata.data, metadata.numBytes, eventTime);

        const ScopedLock sl (lock);

        if (firstMessage == nullptr || firstMessage->message.getTimeStamp() > eventTime)
        {
            m->next = firstMessage;
            firstMessage = m;
        }
        else
        {
            auto* mm = firstMessage;

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
        auto* m = firstMessage;
        firstMessage = firstMessage->next;
        delete m;
    }
}

void MidiOutput::startBackgroundThread()
{
    startThread (Priority::high);
}

void MidiOutput::stopBackgroundThread()
{
    stopThread (5000);
}

void MidiOutput::run()
{
    while (! threadShouldExit())
    {
        auto now = Time::getMillisecondCounter();
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
            std::unique_ptr<PendingMessage> messageDeleter (message);

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

} // namespace juce
