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

enum { magicCoordWorkerConnectionHeader = 0x712baf04 };

static const char* startMessage = "__ipc_st";
static const char* killMessage  = "__ipc_k_";
static const char* pingMessage  = "__ipc_p_";
enum { specialMessageSize = 8, defaultTimeoutMs = 8000 };

static bool isMessageType (const MemoryBlock& mb, const char* messageType) noexcept
{
    return mb.matches (messageType, (size_t) specialMessageSize);
}

static String getCommandLinePrefix (const String& commandLineUniqueID)
{
    return "--" + commandLineUniqueID + ":";
}

//==============================================================================
// This thread sends and receives ping messages every second, so that it
// can find out if the other process has stopped running.
struct ChildProcessPingThread  : public Thread,
                                 private AsyncUpdater
{
    ChildProcessPingThread (int timeout)  : Thread ("IPC ping"), timeoutMs (timeout)
    {
        pingReceived();
    }

    void startPinging()                     { startThread (Priority::low); }

    void pingReceived() noexcept            { countdown = timeoutMs / 1000 + 1; }
    void triggerConnectionLostMessage()     { triggerAsyncUpdate(); }

    virtual bool sendPingMessage (const MemoryBlock&) = 0;
    virtual void pingFailed() = 0;

    int timeoutMs;

    using AsyncUpdater::cancelPendingUpdate;

private:
    Atomic<int> countdown;

    void handleAsyncUpdate() override   { pingFailed(); }

    void run() override
    {
        while (! threadShouldExit())
        {
            if (--countdown <= 0 || ! sendPingMessage ({ pingMessage, specialMessageSize }))
            {
                triggerConnectionLostMessage();
                break;
            }

            wait (1000);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildProcessPingThread)
};

//==============================================================================
struct ChildProcessCoordinator::Connection  : public InterprocessConnection,
                                              private ChildProcessPingThread
{
    Connection (ChildProcessCoordinator& m, const String& pipeName, int timeout)
        : InterprocessConnection (false, magicCoordWorkerConnectionHeader),
          ChildProcessPingThread (timeout),
          owner (m)
    {
        createPipe (pipeName, timeoutMs);
    }

    ~Connection() override
    {
        cancelPendingUpdate();
        stopThread (10000);
    }

    using ChildProcessPingThread::startPinging;

private:
    void connectionMade() override  {}
    void connectionLost() override  { owner.handleConnectionLost(); }

    bool sendPingMessage (const MemoryBlock& m) override    { return owner.sendMessageToWorker (m); }
    void pingFailed() override                              { connectionLost(); }

    void messageReceived (const MemoryBlock& m) override
    {
        pingReceived();

        if (m.getSize() != specialMessageSize || ! isMessageType (m, pingMessage))
            owner.handleMessageFromWorker (m);
    }

    ChildProcessCoordinator& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};

//==============================================================================
ChildProcessCoordinator::ChildProcessCoordinator() = default;

ChildProcessCoordinator::~ChildProcessCoordinator()
{
    killWorkerProcess();
}

void ChildProcessCoordinator::handleConnectionLost() {}

void ChildProcessCoordinator::handleMessageFromWorker (const MemoryBlock& mb)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)
    handleMessageFromSlave (mb);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    JUCE_END_IGNORE_WARNINGS_MSVC
}

bool ChildProcessCoordinator::sendMessageToWorker (const MemoryBlock& mb)
{
    if (connection != nullptr)
        return connection->sendMessage (mb);

    jassertfalse; // this can only be used when the connection is active!
    return false;
}

bool ChildProcessCoordinator::launchWorkerProcess (const File& executable, const String& commandLineUniqueID,
                                                   int timeoutMs, int streamFlags)
{
    killWorkerProcess();

    auto pipeName = "p" + String::toHexString (Random().nextInt64());

    StringArray args;
    args.add (executable.getFullPathName());
    args.add (getCommandLinePrefix (commandLineUniqueID) + pipeName);

    childProcess.reset (new ChildProcess());

    if (childProcess->start (args, streamFlags))
    {
        connection.reset (new Connection (*this, pipeName, timeoutMs <= 0 ? defaultTimeoutMs : timeoutMs));

        if (connection->isConnected())
        {
            connection->startPinging();
            sendMessageToWorker ({ startMessage, specialMessageSize });
            return true;
        }

        connection.reset();
    }

    return false;
}

void ChildProcessCoordinator::killWorkerProcess()
{
    if (connection != nullptr)
    {
        sendMessageToWorker ({ killMessage, specialMessageSize });
        connection->disconnect();
        connection.reset();
    }

    childProcess.reset();
}

//==============================================================================
struct ChildProcessWorker::Connection  : public InterprocessConnection,
                                         private ChildProcessPingThread
{
    Connection (ChildProcessWorker& p, const String& pipeName, int timeout)
        : InterprocessConnection (false, magicCoordWorkerConnectionHeader),
          ChildProcessPingThread (timeout),
          owner (p)
    {
        connectToPipe (pipeName, timeoutMs);
    }

    ~Connection() override
    {
        cancelPendingUpdate();
        stopThread (10000);
        disconnect();
    }

    using ChildProcessPingThread::startPinging;

private:
    ChildProcessWorker& owner;

    void connectionMade() override  {}
    void connectionLost() override  { owner.handleConnectionLost(); }

    bool sendPingMessage (const MemoryBlock& m) override    { return owner.sendMessageToCoordinator (m); }
    void pingFailed() override                              { connectionLost(); }

    void messageReceived (const MemoryBlock& m) override
    {
        pingReceived();

        if (isMessageType (m, pingMessage))
            return;

        if (isMessageType (m, killMessage))
            return triggerConnectionLostMessage();

        if (isMessageType (m, startMessage))
            return owner.handleConnectionMade();

        owner.handleMessageFromCoordinator (m);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};

//==============================================================================
ChildProcessWorker::ChildProcessWorker() = default;
ChildProcessWorker::~ChildProcessWorker() = default;

void ChildProcessWorker::handleConnectionMade() {}
void ChildProcessWorker::handleConnectionLost() {}

void ChildProcessWorker::handleMessageFromCoordinator (const MemoryBlock& mb)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)
    handleMessageFromMaster (mb);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    JUCE_END_IGNORE_WARNINGS_MSVC
}

bool ChildProcessWorker::sendMessageToCoordinator (const MemoryBlock& mb)
{
    if (connection != nullptr)
        return connection->sendMessage (mb);

    jassertfalse; // this can only be used when the connection is active!
    return false;
}

bool ChildProcessWorker::initialiseFromCommandLine (const String& commandLine,
                                                    const String& commandLineUniqueID,
                                                    int timeoutMs)
{
    auto prefix = getCommandLinePrefix (commandLineUniqueID);

    if (commandLine.trim().startsWith (prefix))
    {
        auto pipeName = commandLine.fromFirstOccurrenceOf (prefix, false, false)
                                   .upToFirstOccurrenceOf (" ", false, false).trim();

        if (pipeName.isNotEmpty())
        {
            connection.reset (new Connection (*this, pipeName, timeoutMs <= 0 ? defaultTimeoutMs : timeoutMs));

            if (connection->isConnected())
                connection->startPinging();
            else
                connection.reset();
        }
    }

    return connection != nullptr;
}

} // namespace juce
