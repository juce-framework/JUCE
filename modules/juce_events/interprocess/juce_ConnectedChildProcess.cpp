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

enum { magicMastSlaveConnectionHeader = 0x712baf04 };

static const char* startMessage = "__ipc_st";
static const char* killMessage  = "__ipc_k_";
static const char* pingMessage  = "__ipc_p_";
enum { specialMessageSize = 8, defaultTimeoutMs = 8000 };

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

    static bool isPingMessage (const MemoryBlock& m) noexcept
    {
        return memcmp (m.getData(), pingMessage, specialMessageSize) == 0;
    }

    void pingReceived() noexcept            { countdown = timeoutMs / 1000 + 1; }
    void triggerConnectionLostMessage()     { triggerAsyncUpdate(); }

    virtual bool sendPingMessage (const MemoryBlock&) = 0;
    virtual void pingFailed() = 0;

    int timeoutMs;

private:
    Atomic<int> countdown;

    void handleAsyncUpdate() override   { pingFailed(); }

    void run() override
    {
        while (! threadShouldExit())
        {
            if (--countdown <= 0 || ! sendPingMessage (MemoryBlock (pingMessage, specialMessageSize)))
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
struct ChildProcessMaster::Connection  : public InterprocessConnection,
                                         private ChildProcessPingThread
{
    Connection (ChildProcessMaster& m, const String& pipeName, int timeout)
        : InterprocessConnection (false, magicMastSlaveConnectionHeader),
          ChildProcessPingThread (timeout),
          owner (m)
    {
        if (createPipe (pipeName, timeoutMs))
            startThread (4);
    }

    ~Connection()
    {
        stopThread (10000);
    }

private:
    void connectionMade() override  {}
    void connectionLost() override  { owner.handleConnectionLost(); }

    bool sendPingMessage (const MemoryBlock& m) override    { return owner.sendMessageToSlave (m); }
    void pingFailed() override                              { connectionLost(); }

    void messageReceived (const MemoryBlock& m) override
    {
        pingReceived();

        if (m.getSize() != specialMessageSize || ! isPingMessage (m))
            owner.handleMessageFromSlave (m);
    }

    ChildProcessMaster& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};

//==============================================================================
ChildProcessMaster::ChildProcessMaster() {}

ChildProcessMaster::~ChildProcessMaster()
{
    if (connection != nullptr)
    {
        sendMessageToSlave (MemoryBlock (killMessage, specialMessageSize));
        connection->disconnect();
        connection = nullptr;
    }
}

void ChildProcessMaster::handleConnectionLost() {}

bool ChildProcessMaster::sendMessageToSlave (const MemoryBlock& mb)
{
    if (connection != nullptr)
        return connection->sendMessage (mb);

    jassertfalse; // this can only be used when the connection is active!
    return false;
}

bool ChildProcessMaster::launchSlaveProcess (const File& executable, const String& commandLineUniqueID, int timeoutMs)
{
    connection = nullptr;
    jassert (childProcess.kill());

    const String pipeName ("p" + String::toHexString (Random().nextInt64()));

    StringArray args;
    args.add (executable.getFullPathName());
    args.add (getCommandLinePrefix (commandLineUniqueID) + pipeName);

    if (childProcess.start (args))
    {
        connection = new Connection (*this, pipeName, timeoutMs <= 0 ? defaultTimeoutMs : timeoutMs);

        if (connection->isConnected())
        {
            sendMessageToSlave (MemoryBlock (startMessage, specialMessageSize));
            return true;
        }

        connection = nullptr;
    }

    return false;
}

//==============================================================================
struct ChildProcessSlave::Connection  : public InterprocessConnection,
                                        private ChildProcessPingThread
{
    Connection (ChildProcessSlave& p, const String& pipeName, int timeout)
        : InterprocessConnection (false, magicMastSlaveConnectionHeader),
          ChildProcessPingThread (timeout),
          owner (p)
    {
        connectToPipe (pipeName, timeoutMs);
        startThread (4);
    }

    ~Connection()
    {
        stopThread (10000);
    }

private:
    ChildProcessSlave& owner;

    void connectionMade() override  {}
    void connectionLost() override  { owner.handleConnectionLost(); }

    bool sendPingMessage (const MemoryBlock& m) override    { return owner.sendMessageToMaster (m); }
    void pingFailed() override                              { connectionLost(); }

    void messageReceived (const MemoryBlock& m) override
    {
        pingReceived();

        if (m.getSize() == specialMessageSize)
        {
            if (isPingMessage (m))
                return;

            if (memcmp (m.getData(), killMessage, specialMessageSize) == 0)
            {
                triggerConnectionLostMessage();
                return;
            }

            if (memcmp (m.getData(), startMessage, specialMessageSize) == 0)
            {
                owner.handleConnectionMade();
                return;
            }
        }

        owner.handleMessageFromMaster (m);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};

//==============================================================================
ChildProcessSlave::ChildProcessSlave() {}
ChildProcessSlave::~ChildProcessSlave() {}

void ChildProcessSlave::handleConnectionMade() {}
void ChildProcessSlave::handleConnectionLost() {}

bool ChildProcessSlave::sendMessageToMaster (const MemoryBlock& mb)
{
    if (connection != nullptr)
        return connection->sendMessage (mb);

    jassertfalse; // this can only be used when the connection is active!
    return false;
}

bool ChildProcessSlave::initialiseFromCommandLine (const String& commandLine,
                                                   const String& commandLineUniqueID,
                                                   int timeoutMs)
{
    String prefix (getCommandLinePrefix (commandLineUniqueID));

    if (commandLine.trim().startsWith (prefix))
    {
        String pipeName (commandLine.fromFirstOccurrenceOf (prefix, false, false)
                                    .upToFirstOccurrenceOf (" ", false, false).trim());

        if (pipeName.isNotEmpty())
        {
            connection = new Connection (*this, pipeName, timeoutMs <= 0 ? defaultTimeoutMs : timeoutMs);

            if (! connection->isConnected())
                connection = nullptr;
        }
    }

    return connection != nullptr;
}
