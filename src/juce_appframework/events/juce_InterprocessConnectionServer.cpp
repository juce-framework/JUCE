/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_InterprocessConnectionServer.h"


//==============================================================================
InterprocessConnectionServer::InterprocessConnectionServer()
    : Thread ("Juce IPC server"),
      socket (0)
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

    socket = new Socket();

    if (socket->createListener (portNumber))
    {
        startThread();
        return true;
    }

    deleteAndZero (socket);
    return false;
}

void InterprocessConnectionServer::stop()
{
    signalThreadShouldExit();

    if (socket != 0)
        socket->close();

    stopThread (4000);

    deleteAndZero (socket);
}

void InterprocessConnectionServer::run()
{
    while ((! threadShouldExit()) && socket != 0)
    {
        Socket* const clientSocket = socket->waitForNextConnection();

        if (clientSocket != 0)
        {
            InterprocessConnection* newConnection = createConnectionObject();

            if (newConnection != 0)
            {
                newConnection->initialiseWithSocket (clientSocket);
            }
            else
            {
                delete clientSocket;
            }
        }
    }
}


END_JUCE_NAMESPACE
