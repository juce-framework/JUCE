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

#ifndef JUCE_INTERPROCESSCONNECTIONSERVER_H_INCLUDED
#define JUCE_INTERPROCESSCONNECTIONSERVER_H_INCLUDED


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
    bool beginWaitingForSocket (int portNumber);

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


private:
    //==============================================================================
    ScopedPointer <StreamingSocket> socket;

    void run() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InterprocessConnectionServer)
};


#endif   // JUCE_INTERPROCESSCONNECTIONSERVER_H_INCLUDED
