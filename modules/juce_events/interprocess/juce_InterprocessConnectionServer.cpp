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

InterprocessConnectionServer::InterprocessConnectionServer()
    : Thread ("Juce IPC server")
{
}

InterprocessConnectionServer::~InterprocessConnectionServer()
{
    stop();
}

//==============================================================================
bool InterprocessConnectionServer::beginWaitingForSocket (const int portNumber, const String& bindAddress)
{
    stop();

    socket = new StreamingSocket();

    if (socket->createListener (portNumber, bindAddress))
    {
        startThread();
        return true;
    }

    socket = nullptr;
    return false;
}

void InterprocessConnectionServer::stop()
{
    signalThreadShouldExit();

    if (socket != nullptr)
        socket->close();

    stopThread (4000);
    socket = nullptr;
}

void InterprocessConnectionServer::run()
{
    while ((! threadShouldExit()) && socket != nullptr)
    {
        ScopedPointer<StreamingSocket> clientSocket (socket->waitForNextConnection());

        if (clientSocket != nullptr)
            if (InterprocessConnection* newConnection = createConnectionObject())
                newConnection->initialiseWithSocket (clientSocket.release());
    }
}
