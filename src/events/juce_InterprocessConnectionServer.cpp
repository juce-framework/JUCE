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

#include "juce_InterprocessConnectionServer.h"
#include "../containers/juce_ScopedPointer.h"


//==============================================================================
InterprocessConnectionServer::InterprocessConnectionServer()
    : Thread ("Juce IPC server")
{
}

InterprocessConnectionServer::~InterprocessConnectionServer()
{
    stop();
}

//==============================================================================
bool InterprocessConnectionServer::beginWaitingForSocket (const int portNumber)
{
    stop();

    socket = new StreamingSocket();

    if (socket->createListener (portNumber))
    {
        startThread();
        return true;
    }

    socket = 0;
    return false;
}

void InterprocessConnectionServer::stop()
{
    signalThreadShouldExit();

    if (socket != 0)
        socket->close();

    stopThread (4000);
    socket = 0;
}

void InterprocessConnectionServer::run()
{
    while ((! threadShouldExit()) && socket != 0)
    {
        ScopedPointer <StreamingSocket> clientSocket (socket->waitForNextConnection());

        if (clientSocket != 0)
        {
            InterprocessConnection* newConnection = createConnectionObject();

            if (newConnection != 0)
                newConnection->initialiseWithSocket (clientSocket.release());
        }
    }
}


END_JUCE_NAMESPACE
