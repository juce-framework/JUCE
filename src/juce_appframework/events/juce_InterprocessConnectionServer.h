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

#ifndef __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__
#define __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__

#include "juce_InterprocessConnection.h"


//==============================================================================
/**
    An object that waits for client sockets to connect to a port on this host, and
    creates InterprocessConnection objects for each one.

    To use this, create a class derived from it which implements the createConnectionObject()
    method, so that it creates suitable connection objects for each client that tries
    to connect.

    @see InterprocessConnection
*/
class JUCE_API  InterprocessConnectionServer    : private Thread
{
public:
    //==============================================================================
    /** Creates an uninitialised server object.
    */
    InterprocessConnectionServer();

    /** Destructor. */
    ~InterprocessConnectionServer();

    //==============================================================================
    /** Starts an internal thread which listens on the given port number.

        While this is running, in another process tries to connect with the
        InterprocessConnection::connectToSocket() method, this object will call
        createConnectionObject() to create a connection to that client.

        Use stop() to stop the thread running.

        @see createConnectionObject, stop
    */
    bool beginWaitingForSocket (const int portNumber);

    /** Terminates the listener thread, if it's active.

        @see beginWaitingForSocket
    */
    void stop();

protected:
    /** Creates a suitable connection object for a client process that wants to
        connect to this one.

        This will be called by the listener thread when a client process tries
        to connect, and must return a new InterprocessConnection object that will
        then run as this end of the connection.

        @see InterprocessConnection
    */
    virtual InterprocessConnection* createConnectionObject() = 0;


public:
    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Socket* volatile socket;

    void run();

    InterprocessConnectionServer (const InterprocessConnectionServer&);
    const InterprocessConnectionServer& operator= (const InterprocessConnectionServer&);
};


#endif   // __JUCE_INTERPROCESSCONNECTIONSERVER_JUCEHEADER__
