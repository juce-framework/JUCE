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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_InterprocessConnection.h"
#include "../threads/juce_ScopedLock.h"
#include "../containers/juce_ScopedPointer.h"


//==============================================================================
InterprocessConnection::InterprocessConnection (const bool callbacksOnMessageThread,
                                                const uint32 magicMessageHeaderNumber)
    : Thread ("Juce IPC connection"),
      callbackConnectionState (false),
      useMessageThread (callbacksOnMessageThread),
      magicMessageHeader (magicMessageHeaderNumber),
      pipeReceiveMessageTimeout (-1)
{
}

InterprocessConnection::~InterprocessConnection()
{
    callbackConnectionState = false;
    disconnect();
}


//==============================================================================
bool InterprocessConnection::connectToSocket (const String& hostName,
                                              const int portNumber,
                                              const int timeOutMillisecs)
{
    disconnect();

    const ScopedLock sl (pipeAndSocketLock);
    socket = new StreamingSocket();

    if (socket->connect (hostName, portNumber, timeOutMillisecs))
    {
        connectionMadeInt();
        startThread();
        return true;
    }
    else
    {
        socket = 0;
        return false;
    }
}

bool InterprocessConnection::connectToPipe (const String& pipeName,
                                            const int pipeReceiveMessageTimeoutMs)
{
    disconnect();

    ScopedPointer <NamedPipe> newPipe (new NamedPipe());

    if (newPipe->openExisting (pipeName))
    {
        const ScopedLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = pipeReceiveMessageTimeoutMs;
        initialiseWithPipe (newPipe.release());
        return true;
    }

    return false;
}

bool InterprocessConnection::createPipe (const String& pipeName,
                                         const int pipeReceiveMessageTimeoutMs)
{
    disconnect();

    ScopedPointer <NamedPipe> newPipe (new NamedPipe());

    if (newPipe->createNewPipe (pipeName))
    {
        const ScopedLock sl (pipeAndSocketLock);
        pipeReceiveMessageTimeout = pipeReceiveMessageTimeoutMs;
        initialiseWithPipe (newPipe.release());
        return true;
    }

    return false;
}

void InterprocessConnection::disconnect()
{
    if (socket != 0)
        socket->close();

    if (pipe != 0)
    {
        pipe->cancelPendingReads();
        pipe->close();
    }

    stopThread (4000);

    {
        const ScopedLock sl (pipeAndSocketLock);
        socket = 0;
        pipe = 0;
    }

    connectionLostInt();
}

bool InterprocessConnection::isConnected() const
{
    const ScopedLock sl (pipeAndSocketLock);

    return ((socket != 0 && socket->isConnected())
              || (pipe != 0 && pipe->isOpen()))
            && isThreadRunning();
}

const String InterprocessConnection::getConnectedHostName() const
{
    if (pipe != 0)
    {
        return "localhost";
    }
    else if (socket != 0)
    {
        if (! socket->isLocal())
            return socket->getHostName();

        return "localhost";
    }

    return String::empty;
}

//==============================================================================
bool InterprocessConnection::sendMessage (const MemoryBlock& message)
{
    uint32 messageHeader[2];
    messageHeader [0] = ByteOrder::swapIfBigEndian (magicMessageHeader);
    messageHeader [1] = ByteOrder::swapIfBigEndian ((uint32) message.getSize());

    MemoryBlock messageData (sizeof (messageHeader) + message.getSize());
    messageData.copyFrom (messageHeader, 0, sizeof (messageHeader));
    messageData.copyFrom (message.getData(), sizeof (messageHeader), message.getSize());

    size_t bytesWritten = 0;

    const ScopedLock sl (pipeAndSocketLock);

    if (socket != 0)
    {
        bytesWritten = socket->write (messageData.getData(), (int) messageData.getSize());
    }
    else if (pipe != 0)
    {
        bytesWritten = pipe->write (messageData.getData(), (int) messageData.getSize());
    }

    if (bytesWritten < 0)
    {
        // error..
        return false;
    }

    return (bytesWritten == messageData.getSize());
}

//==============================================================================
void InterprocessConnection::initialiseWithSocket (StreamingSocket* const socket_)
{
    jassert (socket == 0);
    socket = socket_;
    connectionMadeInt();
    startThread();
}

void InterprocessConnection::initialiseWithPipe (NamedPipe* const pipe_)
{
    jassert (pipe == 0);
    pipe = pipe_;
    connectionMadeInt();
    startThread();
}

const int messageMagicNumber = 0xb734128b;

void InterprocessConnection::handleMessage (const Message& message)
{
    if (message.intParameter1 == messageMagicNumber)
    {
        switch (message.intParameter2)
        {
        case 0:
            {
                ScopedPointer <MemoryBlock> data ((MemoryBlock*) message.pointerParameter);
                messageReceived (*data);
                break;
            }

        case 1:
            connectionMade();
            break;

        case 2:
            connectionLost();
            break;
        }
    }
}

void InterprocessConnection::connectionMadeInt()
{
    if (! callbackConnectionState)
    {
        callbackConnectionState = true;

        if (useMessageThread)
            postMessage (new Message (messageMagicNumber, 1, 0, 0));
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
            postMessage (new Message (messageMagicNumber, 2, 0, 0));
        else
            connectionLost();
    }
}

void InterprocessConnection::deliverDataInt (const MemoryBlock& data)
{
    jassert (callbackConnectionState);

    if (useMessageThread)
        postMessage (new Message (messageMagicNumber, 0, 0, new MemoryBlock (data)));
    else
        messageReceived (data);
}

//==============================================================================
bool InterprocessConnection::readNextMessageInt()
{
    const int maximumMessageSize = 1024 * 1024 * 10; // sanity check

    uint32 messageHeader[2];
    const int bytes = (socket != 0) ? socket->read (messageHeader, sizeof (messageHeader), true)
                                    : pipe->read (messageHeader, sizeof (messageHeader), pipeReceiveMessageTimeout);

    if (bytes == sizeof (messageHeader)
         && ByteOrder::swapIfBigEndian (messageHeader[0]) == magicMessageHeader)
    {
        const int bytesInMessage = (int) ByteOrder::swapIfBigEndian (messageHeader[1]);

        if (bytesInMessage > 0 && bytesInMessage < maximumMessageSize)
        {
            MemoryBlock messageData (bytesInMessage, true);
            int bytesRead = 0;

            while (bytesRead < bytesInMessage)
            {
                if (threadShouldExit())
                    return false;

                const int numThisTime = jmin (bytesInMessage, 65536);
                const int bytesIn = (socket != 0) ? socket->read (((char*) messageData.getData()) + bytesRead, numThisTime, true)
                                                  : pipe->read (((char*) messageData.getData()) + bytesRead, numThisTime,
                                                                pipeReceiveMessageTimeout);

                if (bytesIn <= 0)
                    break;

                bytesRead += bytesIn;
            }

            if (bytesRead >= 0)
                deliverDataInt (messageData);
        }
    }
    else if (bytes < 0)
    {
        {
            const ScopedLock sl (pipeAndSocketLock);
            socket = 0;
        }

        connectionLostInt();
        return false;
    }

    return true;
}

void InterprocessConnection::run()
{
    while (! threadShouldExit())
    {
        if (socket != 0)
        {
            const int ready = socket->waitUntilReady (true, 0);

            if (ready < 0)
            {
                {
                    const ScopedLock sl (pipeAndSocketLock);
                    socket = 0;
                }

                connectionLostInt();
                break;
            }
            else if (ready > 0)
            {
                if (! readNextMessageInt())
                    break;
            }
            else
            {
                Thread::sleep (2);
            }
        }
        else if (pipe != 0)
        {
            if (! pipe->isOpen())
            {
                {
                    const ScopedLock sl (pipeAndSocketLock);
                    pipe = 0;
                }

                connectionLostInt();
                break;
            }
            else
            {
                if (! readNextMessageInt())
                    break;
            }
        }
        else
        {
            break;
        }
    }
}


END_JUCE_NAMESPACE
