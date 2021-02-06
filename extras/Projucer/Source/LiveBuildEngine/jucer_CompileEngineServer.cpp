/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "jucer_CompileEngineServer.h"
#include "jucer_CompileEngineDLL.h"
#include "jucer_MessageIDs.h"
#include "jucer_CppHelpers.h"
#include "jucer_SourceCodeRange.h"
#include "jucer_ClassDatabase.h"
#include "jucer_DiagnosticMessage.h"
#include "jucer_ProjectBuildInfo.h"
#include "jucer_ClientServerMessages.h"

#if JUCE_LINUX || JUCE_BSD
 #include <sys/types.h>
 #include <unistd.h>
#endif

#ifndef RUN_CLANG_IN_CHILD_PROCESS
 #error
#endif

#if RUN_CLANG_IN_CHILD_PROCESS
 static bool parentProcessHasExited();
#endif

#if JUCE_WINDOWS
 static void setParentProcessID (int);
 static int getCurrentProcessID();
#else
 #include <unistd.h>
#endif

//==============================================================================
/** Detects whether this process has hung, and kills it if so. */
struct ZombiePatrol    : private Thread,
                         private AsyncUpdater,
                         private Timer
{
    explicit ZombiePatrol (MessageHandler& mh)
       : Thread ("Ping"), owner (mh)
    {
        startThread (2);
        startTimer (1000);
    }

    ~ZombiePatrol() override
    {
        stopThread (1000);
    }

private:
    MessageHandler& owner;
    int failedPings = 0;

    void run() override
    {
        while (! threadShouldExit())
        {
           #if RUN_CLANG_IN_CHILD_PROCESS
            if (parentProcessHasExited())
            {
                killProcess();
                break;
            }
           #endif

            wait (1000);
        }
    }

    void handleAsyncUpdate() override
    {
        DBG ("Server: quitting");
        stopTimer();
        ProjucerApplication::getApp().systemRequestedQuit();
    }

    void timerCallback() override
    {
        if (! MessageTypes::sendPing (owner))
        {
            if (++failedPings == 10)
            {
                killProcess();
                return;
            }
        }
        else
        {
            failedPings = 0;
        }
    }

    void killProcess()
    {
        triggerAsyncUpdate(); // give the messagequeue a chance to do things cleanly.
        static UnstoppableKillerThread* k = new UnstoppableKillerThread(); // (allowed to leak, but static so only one is created)
        ignoreUnused (k);
    }

    struct UnstoppableKillerThread  : public Thread
    {
        UnstoppableKillerThread() : Thread ("Killer")  { startThread(); }

        void run() override
        {
            wait (15000);

            if (! threadShouldExit())
                Process::terminate();
        }
    };
};

//==============================================================================
class ServerIPC  : public InterprocessConnection,
                   public MessageHandler
{
public:
    explicit ServerIPC (const StringArray& info)
       : InterprocessConnection (true), liveCodeBuilder (nullptr)
    {
        if (! createPipe (info[0], -1))
        {
            Logger::writeToLog ("*** Couldn't create pipe!");
            ProjucerApplication::getApp().systemRequestedQuit();
            return;
        }

        if (dll.isLoaded())
            liveCodeBuilder = dll.projucer_createBuilder (sendMessageCallback, this, info[1].toRawUTF8(), info[2].toRawUTF8());

       #if JUCE_WINDOWS
        setParentProcessID (info[3].getHexValue32());
       #endif

        zombieKiller.reset (new ZombiePatrol (*this));
    }

    ~ServerIPC() override
    {
        zombieKiller.reset();

        if (dll.isLoaded())
            dll.projucer_deleteBuilder (liveCodeBuilder);

        dll.shutdown();

        DBG ("Server: finished closing down");
    }

    void connectionMade() override
    {
        DBG ("Server: client connected");
    }

    void connectionLost() override
    {
        Logger::writeToLog ("Server: client lost");
        JUCEApplication::quit();
    }

    void sendQuitMessageToIDE()
    {
        MessageTypes::sendShouldCloseIDE (*this);
    }

    bool sendMessage (const ValueTree& m) override
    {
        return InterprocessConnection::sendMessage (MessageHandler::convertMessage (m));
    }

    void messageReceived (const MemoryBlock& message) override
    {
        jassert (dll.isLoaded());
        dll.projucer_sendMessage (liveCodeBuilder, message.getData(), message.getSize());
    }

    static bool sendMessageCallback (void* userInfo, const void* data, size_t dataSize)
    {
        return static_cast<InterprocessConnection*> (static_cast<ServerIPC*> (userInfo))
                  ->sendMessage (MemoryBlock (data, dataSize));
    }

    CompileEngineDLL dll;
    LiveCodeBuilder liveCodeBuilder;
    std::unique_ptr<ZombiePatrol> zombieKiller;
};

//==============================================================================
const char* commandPrefix = "--server:";
const char* commandTokenSeparator = "\x01";

String createCommandLineForLaunchingServer (const String& pipeName, const String& projectUID, const File& cacheLocation)
{
    StringArray info;
    info.add (pipeName);
    info.add (projectUID);
    info.add (cacheLocation.getFullPathName());

   #if JUCE_WINDOWS
    info.add (String::toHexString (getCurrentProcessID()));
   #endif

    const File exe (File::getSpecialLocation (File::currentExecutableFile).getFullPathName());

    return "\"" + exe.getFullPathName() + "\" " + commandPrefix + info.joinIntoString (commandTokenSeparator);
}

static ServerIPC* currentServer = nullptr;

static void crashCallback (const char* message)
{
    if (currentServer != nullptr)
    {
       #if RUN_CLANG_IN_CHILD_PROCESS
        MessageTypes::sendCrash (*currentServer, message);
        Logger::writeToLog ("*** Crashed! " + String (message));
       #else
        ignoreUnused (message);
        jassertfalse;
       #endif

        currentServer->disconnect();
    }
}

static void quitCallback()
{
    ProjucerApplication::getApp().systemRequestedQuit();
}

void* createClangServer (const String& commandLine)
{
    StringArray info;
    info.addTokens (commandLine.fromFirstOccurrenceOf (commandPrefix, false, false), commandTokenSeparator, "");

    std::unique_ptr<ServerIPC> ipc (new ServerIPC (info));

    if (ipc->dll.isLoaded())
    {
        ipc->dll.initialise (crashCallback, quitCallback, (bool) RUN_CLANG_IN_CHILD_PROCESS);

        currentServer = ipc.release();
        return currentServer;
    }

    return nullptr;
}

void destroyClangServer (void* server)
{
    currentServer = nullptr;
    delete static_cast<ServerIPC*> (server);
}

void sendQuitMessageToIDE (void* server)
{
    static_cast<ServerIPC*> (server)->sendQuitMessageToIDE();
}

//==============================================================================
#if JUCE_WINDOWS
 #define STRICT 1
 #define WIN32_LEAN_AND_MEAN 1
 #include <windows.h>

 static HANDLE parentProcessHandle = nullptr;
 static void setParentProcessID (int pid)  { parentProcessHandle = OpenProcess (SYNCHRONIZE, FALSE, (DWORD) pid); }
 static int getCurrentProcessID()          { return (int) GetCurrentProcessId(); }
#endif

#if RUN_CLANG_IN_CHILD_PROCESS
bool parentProcessHasExited()
{
   #if JUCE_WINDOWS
    return WaitForSingleObject (parentProcessHandle, 0) == WAIT_OBJECT_0;
   #else
    return getppid() == 1;
   #endif
}
#endif
