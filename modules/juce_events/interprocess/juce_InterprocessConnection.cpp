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

struct InterprocessConnection::ConnectionThread final : public Thread
{
    ConnectionThread (InterprocessConnection& c)  : Thread (SystemStats::getJUCEVersion() + ": IPC"), owner (c) {}
    void run() override     { owner.runThread(); }

    InterprocessConnection& owner;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectionThread)
};

class SafeActionImpl
{
public:
    explicit SafeActionImpl (InterprocessConnection& p)
        : ref (p) {}

    template <typename Fn>
    void ifSafe (Fn&& fn)
    {
        const ScopedLock lock (mutex);

        if (safe)
            fn (ref);
    }

    void setSafe (bool s)
    {
        const ScopedLock lock (mutex);
        safe = s;
    }

    bool isSafe()
    {
        const ScopedLock lock (mutex);
        return safe;
    }

private:
    CriticalSection mutex;
    InterprocessConnection& ref;
    bool safe = false;
};

class InterprocessConnection::SafeAction final : public SafeActionImpl
{
    using SafeActionImpl::SafeActionImpl;
};

//==============================================================================
InterprocessConnection::InterprocessConnection (bool callbacksOnMessageThread, uint32 magicMessageHeaderNumber)
    : useMessageThread (callbacksOnMessageThread),
      magicMessageHeader (magicMessageHeaderNumber),
      safeAction (std::make_shared<SafeAction> (*this))
{
    thread.reset (new ConnectionThread (*this));
}

InterprocessConnection::~InterprocessConnection()
{
    // You *must* call `disconnect` in the destructor of your derived class to ensure
    // that any pending messages are not delivered. If the messages were delivered after
    // destroying the derived class, we'd end up calling the pure virtual implementations
    // of `messageReceived`, `connectionMade` and `connectionLost` which is definitely
    // not a good idea!
    jassert (! safeAction->isSafe());

    callbackConnectionState = false;
    disconnect (4000, Notify::no);
    thread.reset();
}

//==============================================================================
bool InterprocessConnection::connectToSocket (const String& hostName,
                                              int portNumber, int timeOutMillisecs)
{
    disconnect();

    auto s = std::make_unique<StreamingSocket>();

    if (s->connect (hostName, portNumber, timeOutMillisecs))
    {
        const ScopedWriteLock sl (pipeAndSocketLock);
        initialiseWithSocket (std::move (s));
        return true;
    }

    return false;
}

bool InterprocessConnection::connectToPipe (const String& pipeName, int timeoutMs)
{
    disconnect();

    auto newPipe = std::make_unique<NamedPipe>();

    if (newPipe->openExisting (pipeName))
    {
        const ScopedWriteLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = timeoutMs;
        initialiseWithPipe (std::move (newPipe));
        return true;
    }

    return false;
}

bool InterprocessConnection::createPipe (const String& pipeName, int timeoutMs, bool mustNotExist)
{
    disconnect();

    auto newPipe = std::make_unique<NamedPipe>();

    if (newPipe->createNewPipe (pipeName, mustNotExist))
    {
        const ScopedWriteLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = timeoutMs;
        initialiseWithPipe (std::move (newPipe));
        return true;
    }

    return false;
}

void InterprocessConnection::disconnect (int timeoutMs, Notify notify)
{
    thread->signalThreadShouldExit();

    {
        const ScopedReadLock sl (pipeAndSocketLock);
        if (socket != nullptr)  socket->close();
        if (pipe != nullptr)    pipe->close();
    }

    thread->stopThread (timeoutMs);
    deletePipeAndSocket();

    if (notify == Notify::yes)
        connectionLostInt();

    callbackConnectionState = false;
    safeAction->setSafe (false);
}

void InterprocessConnection::deletePipeAndSocket()
{
    const ScopedWriteLock sl (pipeAndSocketLock);
    socket.reset();
    pipe.reset();
}

bool InterprocessConnection::isConnected() const
{
    const ScopedReadLock sl (pipeAndSocketLock);

    return ((socket != nullptr && socket->isConnected())
              || (pipe != nullptr && pipe->isOpen()))
            && threadIsRunning;
}

String InterprocessConnection::getConnectedHostName() const
{
    {
        const ScopedReadLock sl (pipeAndSocketLock);

        if (pipe == nullptr && socket == nullptr)
            return {};

        if (socket != nullptr && ! socket->isLocal())
            return socket->getHostName();
    }

    return IPAddress::local().toString();
}

//==============================================================================
bool InterprocessConnection::sendMessage (const MemoryBlock& message)
{
    uint32 messageHeader[2] = { ByteOrder::swapIfBigEndian (magicMessageHeader),
                                ByteOrder::swapIfBigEndian ((uint32) message.getSize()) };

    MemoryBlock messageData (sizeof (messageHeader) + message.getSize());
    messageData.copyFrom (messageHeader, 0, sizeof (messageHeader));
    messageData.copyFrom (message.getData(), sizeof (messageHeader), message.getSize());

    return writeData (messageData.getData(), (int) messageData.getSize()) == (int) messageData.getSize();
}

int InterprocessConnection::writeData (void* data, int dataSize)
{
    const ScopedReadLock sl (pipeAndSocketLock);

    if (socket != nullptr)
        return socket->write (data, dataSize);

    if (pipe != nullptr)
        return pipe->write (data, dataSize, pipeReceiveMessageTimeout);

    return 0;
}

//==============================================================================
void InterprocessConnection::initialise()
{
    safeAction->setSafe (true);
    threadIsRunning = true;
    connectionMadeInt();
    thread->startThread();
}

void InterprocessConnection::initialiseWithSocket (std::unique_ptr<StreamingSocket> newSocket)
{
    jassert (socket == nullptr && pipe == nullptr);
    socket = std::move (newSocket);
    initialise();
}

void InterprocessConnection::initialiseWithPipe (std::unique_ptr<NamedPipe> newPipe)
{
    jassert (socket == nullptr && pipe == nullptr);
    pipe = std::move (newPipe);
    initialise();
}

//==============================================================================
struct ConnectionStateMessage final : public MessageManager::MessageBase
{
    ConnectionStateMessage (std::shared_ptr<SafeActionImpl> ipc, bool connected) noexcept
        : safeAction (ipc), connectionMade (connected)
    {}

    void messageCallback() override
    {
        safeAction->ifSafe ([this] (InterprocessConnection& owner)
        {
            if (connectionMade)
                owner.connectionMade();
            else
                owner.connectionLost();
        });
    }

    std::shared_ptr<SafeActionImpl> safeAction;
    bool connectionMade;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectionStateMessage)
};

void InterprocessConnection::connectionMadeInt()
{
    if (! callbackConnectionState)
    {
        callbackConnectionState = true;

        if (useMessageThread)
            (new ConnectionStateMessage (safeAction, true))->post();
        else
            connectionMade();
    }
}

void InterprocessConnection::connectionLostInt()
{
    if (callbackConnectionState)
    {
        callbackConnectionState = false;

        if (useMessageThread)
            (new ConnectionStateMessage (safeAction, false))->post();
        else
            connectionLost();
    }
}

struct DataDeliveryMessage final : public Message
{
    DataDeliveryMessage (std::shared_ptr<SafeActionImpl> ipc, const MemoryBlock& d)
        : safeAction (ipc), data (d)
    {}

    void messageCallback() override
    {
        safeAction->ifSafe ([this] (InterprocessConnection& owner)
        {
            owner.messageReceived (data);
        });
    }

    std::shared_ptr<SafeActionImpl> safeAction;
    MemoryBlock data;
};

void InterprocessConnection::deliverDataInt (const MemoryBlock& data)
{
    jassert (callbackConnectionState);

    if (useMessageThread)
        (new DataDeliveryMessage (safeAction, data))->post();
    else
        messageReceived (data);
}

//==============================================================================
int InterprocessConnection::readData (void* data, int num)
{
    const ScopedReadLock sl (pipeAndSocketLock);

    if (socket != nullptr)
        return socket->read (data, num, true);

    if (pipe != nullptr)
        return pipe->read (data, num, pipeReceiveMessageTimeout);

    jassertfalse;
    return -1;
}

bool InterprocessConnection::readNextMessage()
{
    uint32 messageHeader[2];
    auto bytes = readData (messageHeader, sizeof (messageHeader));

    if (bytes == (int) sizeof (messageHeader)
         && ByteOrder::swapIfBigEndian (messageHeader[0]) == magicMessageHeader)
    {
        auto bytesInMessage = (int) ByteOrder::swapIfBigEndian (messageHeader[1]);

        if (bytesInMessage > 0)
        {
            MemoryBlock messageData ((size_t) bytesInMessage, true);
            int bytesRead = 0;

            while (bytesInMessage > 0)
            {
                if (thread->threadShouldExit())
                    return false;

                auto numThisTime = jmin (bytesInMessage, 65536);
                auto bytesIn = readData (addBytesToPointer (messageData.getData(), bytesRead), numThisTime);

                if (bytesIn <= 0)
                    break;

                bytesRead += bytesIn;
                bytesInMessage -= bytesIn;
            }

            if (bytesRead >= 0)
                deliverDataInt (messageData);
        }

        return true;
    }

    if (bytes < 0)
    {
        if (socket != nullptr)
            deletePipeAndSocket();

        connectionLostInt();
    }

    return false;
}

void InterprocessConnection::runThread()
{
    while (! thread->threadShouldExit())
    {
        if (socket != nullptr)
        {
            auto ready = socket->waitUntilReady (true, 100);

            if (ready < 0)
            {
                deletePipeAndSocket();
                connectionLostInt();
                break;
            }

            if (ready == 0)
            {
                thread->wait (1);
                continue;
            }
        }
        else if (pipe != nullptr)
        {
            if (! pipe->isOpen())
            {
                deletePipeAndSocket();
                connectionLostInt();
                break;
            }
        }
        else
        {
            break;
        }

        if (thread->threadShouldExit() || ! readNextMessage())
            break;
    }

    threadIsRunning = false;
}

} // namespace juce
