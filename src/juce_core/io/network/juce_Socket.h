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

#ifndef __JUCE_SOCKET_JUCEHEADER__
#define __JUCE_SOCKET_JUCEHEADER__

#include "../../text/juce_String.h"


//==============================================================================
/**
    A wrapper for a streaming (TCP) socket.

    This allows low-level use of sockets; for an easier-to-use messaging layer on top of
    sockets, you could also try the InterprocessConnection class.

    @see DatagramSocket, InterprocessConnection, InterprocessConnectionServer
*/
class JUCE_API  StreamingSocket
{
public:
    //==============================================================================
    /** Creates an uninitialised socket.

        To connect it, use the connect() method, after which you can read() or write()
        to it.

        To wait for other sockets to connect to this one, the createListener() method
        enters "listener" mode, and can be used to spawn new sockets for each connection
        that comes along.
    */
    StreamingSocket();

    /** Destructor. */
    ~StreamingSocket();

    //==============================================================================
    /** Binds the socket to the specified local port.

        @returns    true on success; false may indicate that another socket is already bound
                    on the same port
    */
    bool bindToPort (const int localPortNumber);

    /** Tries to connect the socket to hostname:port.

        If timeOutMillisecs is 0, then this method will block until the operating system
        rejects the connection (which could take a long time).

        @returns true if it succeeds.
        @see isConnected
    */
    bool connect (const String& remoteHostname,
                  const int remotePortNumber,
                  const int timeOutMillisecs = 3000);

    /** True if the socket is currently connected. */
    bool isConnected() const throw()                            { return connected; }

    /** Closes the connection. */
    void close();

    /** Returns the name of the currently connected host. */
    const String& getHostName() const throw()                   { return hostName; }

    /** Returns the port number that's currently open. */
    int getPort() const throw()                                 { return portNumber; }

    /** True if the socket is connected to this machine rather than over the network. */
    bool isLocal() const throw();

    //==============================================================================
    /** Waits until the socket is ready for reading or writing.

        If readyForReading is true, it will wait until the socket is ready for
        reading; if false, it will wait until it's ready for writing.

        If the timeout is < 0, it will wait forever, or else will give up after
        the specified time.

        If the socket is ready on return, this returns 1. If it times-out before
        the socket becomes ready, it returns 0. If an error occurs, it returns -1.
    */
    int waitUntilReady (const bool readyForReading,
                        const int timeoutMsecs) const;

    /** Reads bytes from the socket.

        If blockUntilSpecifiedAmountHasArrived is true, the method will block until
        maxBytesToRead bytes have been read, (or until an error occurs). If this
        flag is false, the method will return as much data as is currently available
        without blocking.

        @returns the number of bytes read, or -1 if there was an error.
        @see waitUntilReady
    */
    int read (void* destBuffer, const int maxBytesToRead,
              const bool blockUntilSpecifiedAmountHasArrived);

    /** Writes bytes to the socket from a buffer.

        Note that this method will block unless you have checked the socket is ready
        for writing before calling it (see the waitUntilReady() method).

        @returns the number of bytes written, or -1 if there was an error.
    */
    int write (const void* sourceBuffer, const int numBytesToWrite);

    //==============================================================================
    /** Puts this socket into "listener" mode.

        When in this mode, your thread can call waitForNextConnection() repeatedly,
        which will spawn new sockets for each new connection, so that these can
        be handled in parallel by other threads.

        This returns true if it manages to open the socket successfully.

        @see waitForNextConnection
    */
    bool createListener (const int portNumber);

    /** When in "listener" mode, this waits for a connection and spawns it as a new
        socket.

        The object that gets returned will be owned by the caller.

        This method can only be called after using createListener().

        @see createListener
    */
    StreamingSocket* waitForNextConnection() const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String hostName;
    int volatile portNumber, handle;
    bool connected, isListener;

    StreamingSocket (const String& hostname, const int portNumber, const int handle);
    StreamingSocket (const StreamingSocket&);
    const StreamingSocket& operator= (const StreamingSocket&);
};


//==============================================================================
/**
    A wrapper for a datagram (UDP) socket.

    This allows low-level use of sockets; for an easier-to-use messaging layer on top of
    sockets, you could also try the InterprocessConnection class.

    @see StreamingSocket, InterprocessConnection, InterprocessConnectionServer
*/
class JUCE_API  DatagramSocket
{
public:
    //==============================================================================
    /**
        Creates an (uninitialised) datagram socket.

        The localPortNumber is the port on which to bind this socket. If this value is 0,
        the port number is assigned by the operating system.

        To use the socket for sending, call the connect() method. This will not immediately
        make a connection, but will save the destination you've provided. After this, you can
        call read() or write().

        If enableBroadcasting is true, the socket will be allowed to send broadcast messages
        (may require extra privileges on linux)

        To wait for other sockets to connect to this one, call waitForNextConnection().
    */
    DatagramSocket (const int localPortNumber,
                    const bool enableBroadcasting = false);

    /** Destructor. */
    ~DatagramSocket();

    //==============================================================================
    /** Binds the socket to the specified local port.

        @returns    true on success; false may indicate that another socket is already bound
                    on the same port
    */
    bool bindToPort (const int localPortNumber);

    /** Tries to connect the socket to hostname:port.

        If timeOutMillisecs is 0, then this method will block until the operating system
        rejects the connection (which could take a long time).

        @returns true if it succeeds.
        @see isConnected
    */
    bool connect (const String& remoteHostname,
                  const int remotePortNumber,
                  const int timeOutMillisecs = 3000);

    /** True if the socket is currently connected. */
    bool isConnected() const throw()                            { return connected; }

    /** Closes the connection. */
    void close();

    /** Returns the name of the currently connected host. */
    const String& getHostName() const throw()                   { return hostName; }

    /** Returns the port number that's currently open. */
    int getPort() const throw()                                 { return portNumber; }

    /** True if the socket is connected to this machine rather than over the network. */
    bool isLocal() const throw();

    //==============================================================================
    /** Waits until the socket is ready for reading or writing.

        If readyForReading is true, it will wait until the socket is ready for
        reading; if false, it will wait until it's ready for writing.

        If the timeout is < 0, it will wait forever, or else will give up after
        the specified time.

        If the socket is ready on return, this returns 1. If it times-out before
        the socket becomes ready, it returns 0. If an error occurs, it returns -1.
    */
    int waitUntilReady (const bool readyForReading,
                        const int timeoutMsecs) const;

    /** Reads bytes from the socket.

        If blockUntilSpecifiedAmountHasArrived is true, the method will block until
        maxBytesToRead bytes have been read, (or until an error occurs). If this
        flag is false, the method will return as much data as is currently available
        without blocking.

        @returns the number of bytes read, or -1 if there was an error.
        @see waitUntilReady
    */
    int read (void* destBuffer, const int maxBytesToRead,
              const bool blockUntilSpecifiedAmountHasArrived);

    /** Writes bytes to the socket from a buffer.

        Note that this method will block unless you have checked the socket is ready
        for writing before calling it (see the waitUntilReady() method).

        @returns the number of bytes written, or -1 if there was an error.
    */
    int write (const void* sourceBuffer, const int numBytesToWrite);

    //==============================================================================
    /** This waits for incoming data to be sent, and returns a socket that can be used
        to read it.

        The object that gets returned is owned by the caller, and can't be used for
        sending, but can be used to read the data.
    */
    DatagramSocket* waitForNextConnection() const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String hostName;
    int volatile portNumber, handle;
    bool connected, allowBroadcast;
    void* serverAddress;

    DatagramSocket (const String& hostname, const int portNumber, const int handle, const int localPortNumber);
    DatagramSocket (const DatagramSocket&);
    const DatagramSocket& operator= (const DatagramSocket&);
};


#endif   // __JUCE_SOCKET_JUCEHEADER__
