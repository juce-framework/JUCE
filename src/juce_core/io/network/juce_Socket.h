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
    A wrapper for a socket.

    Allows low-level use of sockets; for an easier-to-use messaging layer on top of
    sockets, you could also try the InterprocessConnection class.

    @see InterprocessConnection, InterprocessConnectionServer
*/
class JUCE_API  Socket
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
    Socket();

    /** Destructor. */
    ~Socket();

    //==============================================================================
    // Tests if the socket is ready
    // Returns: 1 == yes, 0 == no, -1 == error
    int isReady (int timeoutMsecs = 0);

    //==============================================================================
    /** Reads bytes from the socket (blocking).

        Returns the number of bytes read, or -1 if there was an error.
    */
    int read (void* destBuffer, const int maxBytesToRead);

    /** Writes bytes to the socket from a buffer.

        This may block on error conditions.

        Returns the number of bytes written, or -1 if there was an error.
    */
    int write (const void* sourceBuffer, int numBytesToWrite);

    //==============================================================================
    /** Tries to connect the socket to hostname:port.

        Returns true if it succeeds.

        @see isConnected
    */
    bool connect (const String& hostname,
                  int portNumber,
                  int timeOutMillisecs = 3000);

    /** Closes the connection. */
    void close();

    //==============================================================================
    /** True if the socket is currently connected. */
    bool isConnected() const throw()                            { return connected; }

    /** Returns the name of the currently connected host. */
    const String& getHostName() const throw()                   { return hostName; }

    /** Returns the port number that's currently open. */
    int getPort() const throw()                                 { return portNumber; }

    /** True if the socket is connected to this machine rather than over the network. */
    bool isLocal() const throw();

    //==============================================================================
    /** Puts this socket into "listener" mode.

        When in this mode, your thread can call waitForNextConnection() repeatedly,
        which will spawn new sockets for each new connection, so that these can
        be handled in parallel by other threads.

        This returns true if it manages to open the socket successfully.

        @see waitForNextConnection
    */
    bool createListener (int portNumber);

    /** When in "listener" mode, this waits for a connection and spawns it as a new
        socket.

        The object that gets returned will be owned by the caller.

        This method can only be called after using createListener().

        @see createListener
    */
    Socket* waitForNextConnection();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String hostName;
    int volatile portNumber, handle;
    bool connected, isListener;

    Socket (const String& hostname, const int portNumber, const int handle);
    Socket (const Socket&);
    const Socket& operator= (const Socket&);

    bool resetSocketOptions();
};


#endif   // __JUCE_SOCKET_JUCEHEADER__
