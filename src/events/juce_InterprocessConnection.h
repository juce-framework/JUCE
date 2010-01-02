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

#ifndef __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__
#define __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__

#include "juce_MessageListener.h"
#include "../threads/juce_Thread.h"
#include "../io/network/juce_Socket.h"
#include "../io/files/juce_NamedPipe.h"
#include "../containers/juce_ScopedPointer.h"
class InterprocessConnectionServer;


//==============================================================================
/**
    Manages a simple two-way messaging connection to another process, using either
    a socket or a named pipe as the transport medium.

    To connect to a waiting socket or an open pipe, use the connectToSocket() or
    connectToPipe() methods. If this succeeds, messages can be sent to the other end,
    and incoming messages will result in a callback via the messageReceived()
    method.

    To open a pipe and wait for another client to connect to it, use the createPipe()
    method.

    To act as a socket server and create connections for one or more client, see the
    InterprocessConnectionServer class.

    @see InterprocessConnectionServer, Socket, NamedPipe
*/
class JUCE_API  InterprocessConnection    : public Thread,
                                            private MessageListener
{
public:
    //==============================================================================
    /** Creates a connection.

        Connections are created manually, connecting them with the connectToSocket()
        or connectToPipe() methods, or they are created automatically by a InterprocessConnectionServer
        when a client wants to connect.

        @param callbacksOnMessageThread     if true, callbacks to the connectionMade(),
                                            connectionLost() and messageReceived() methods will
                                            always be made using the message thread; if false,
                                            these will be called immediately on the connection's
                                            own thread.
        @param magicMessageHeaderNumber     a magic number to use in the header to check the
                                            validity of the data blocks being sent and received. This
                                            can be any number, but the sender and receiver must obviously
                                            use matching values or they won't recognise each other.
    */
    InterprocessConnection (const bool callbacksOnMessageThread = true,
                            const uint32 magicMessageHeaderNumber = 0xf2b49e2c);

    /** Destructor. */
    ~InterprocessConnection();

    //==============================================================================
    /** Tries to connect this object to a socket.

        For this to work, the machine on the other end needs to have a InterprocessConnectionServer
        object waiting to receive client connections on this port number.

        @param hostName             the host computer, either a network address or name
        @param portNumber           the socket port number to try to connect to
        @param timeOutMillisecs     how long to keep trying before giving up
        @returns true if the connection is established successfully
        @see Socket
    */
    bool connectToSocket (const String& hostName,
                          const int portNumber,
                          const int timeOutMillisecs);

    /** Tries to connect the object to an existing named pipe.

        For this to work, another process on the same computer must already have opened
        an InterprocessConnection object and used createPipe() to create a pipe for this
        to connect to.

        You can optionally specify a timeout length to be passed to the NamedPipe::read() method.

        @returns true if it connects successfully.
        @see createPipe, NamedPipe
    */
    bool connectToPipe (const String& pipeName,
                        const int pipeReceiveMessageTimeoutMs = -1);

    /** Tries to create a new pipe for other processes to connect to.

        This creates a pipe with the given name, so that other processes can use
        connectToPipe() to connect to the other end.

        You can optionally specify a timeout length to be passed to the NamedPipe::read() method.

        If another process is already using this pipe, this will fail and return false.
    */
    bool createPipe (const String& pipeName,
                     const int pipeReceiveMessageTimeoutMs = -1);

    /** Disconnects and closes any currently-open sockets or pipes. */
    void disconnect();

    /** True if a socket or pipe is currently active. */
    bool isConnected() const;

    /** Returns the socket that this connection is using (or null if it uses a pipe). */
    StreamingSocket* getSocket() const throw()                  { return socket; }

    /** Returns the pipe that this connection is using (or null if it uses a socket). */
    NamedPipe* getPipe() const throw()                          { return pipe; }

    /** Returns the name of the machine at the other end of this connection.

        This will return an empty string if the other machine isn't known for
        some reason.
    */
    const String getConnectedHostName() const;

    //==============================================================================
    /** Tries to send a message to the other end of this connection.

        This will fail if it's not connected, or if there's some kind of write error. If
        it succeeds, the connection object at the other end will receive the message by
        a callback to its messageReceived() method.

        @see messageReceived
    */
    bool sendMessage (const MemoryBlock& message);

    //==============================================================================
    /** Called when the connection is first connected.

        If the connection was created with the callbacksOnMessageThread flag set, then
        this will be called on the message thread; otherwise it will be called on a server
        thread.
    */
    virtual void connectionMade() = 0;

    /** Called when the connection is broken.

        If the connection was created with the callbacksOnMessageThread flag set, then
        this will be called on the message thread; otherwise it will be called on a server
        thread.
    */
    virtual void connectionLost() = 0;

    /** Called when a message arrives.

        When the object at the other end of this connection sends us a message with sendMessage(),
        this callback is used to deliver it to us.

        If the connection was created with the callbacksOnMessageThread flag set, then
        this will be called on the message thread; otherwise it will be called on a server
        thread.

        @see sendMessage
    */
    virtual void messageReceived (const MemoryBlock& message) = 0;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    CriticalSection pipeAndSocketLock;
    ScopedPointer <StreamingSocket> socket;
    ScopedPointer <NamedPipe> pipe;
    bool callbackConnectionState;
    const bool useMessageThread;
    const uint32 magicMessageHeader;
    int pipeReceiveMessageTimeout;

    //==============================================================================
    friend class InterprocessConnectionServer;

    void initialiseWithSocket (StreamingSocket* const socket_);
    void initialiseWithPipe (NamedPipe* const pipe_);

    void handleMessage (const Message& message);

    void connectionMadeInt();
    void connectionLostInt();
    void deliverDataInt (const MemoryBlock& data);

    bool readNextMessageInt();
    void run();

    InterprocessConnection (const InterprocessConnection&);
    const InterprocessConnection& operator= (const InterprocessConnection&);
};

#endif   // __JUCE_INTERPROCESSCONNECTION_JUCEHEADER__
