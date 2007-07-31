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

#ifdef _WIN32
  #include "../../../../build/win32/platform_specific_code/win32_headers.h"
  #include <winsock2.h>

  #ifdef _MSC_VER
    #pragma warning (disable : 4127 4389 4018)
  #endif

#else
  #ifndef LINUX
    #include <Carbon/Carbon.h>
  #endif

  #include <sys/types.h>
  #include <netdb.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <sys/errno.h>
  #include <netinet/tcp.h>
  #include <netinet/in.h>
  #include <fcntl.h>
  #include <unistd.h>
#endif

#include "../../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Socket.h"
#include "../../threads/juce_ScopedLock.h"
#include "../../threads/juce_Thread.h"


#if JUCE_WIN32
 static CriticalSection socketInitLock;
 static int numActiveSockets = 0;
#endif

//==============================================================================
Socket::Socket()
    : portNumber (0),
      handle (-1),
      connected (false),
      isListener (false)
{
#if JUCE_WIN32
    const ScopedLock sl (socketInitLock);

    if (numActiveSockets++ == 0)
    {
        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD (1, 1);
        WSAStartup (wVersionRequested, &wsaData);
    }
#endif
}

Socket::Socket (const String& hostName_, const int portNumber_, const int handle_)
    : hostName (hostName_),
      portNumber (portNumber_),
      handle (handle_),
      connected (true),
      isListener (false)
{
#if JUCE_WIN32
    socketInitLock.enter();
    ++numActiveSockets;
    socketInitLock.exit();
#endif

    resetSocketOptions();
}

Socket::~Socket()
{
    close();

#if JUCE_WIN32
    const ScopedLock sl (socketInitLock);

    if (--numActiveSockets == 0)
        WSACleanup();
#endif
}

//==============================================================================
bool Socket::resetSocketOptions()
{
    const int sndBufSize = 65536;
    const int rcvBufSize = 65536;
    const int one = 1;

    return setsockopt (handle, SOL_SOCKET, SO_RCVBUF, (const char*) &rcvBufSize, sizeof (int)) == 0
            && setsockopt (handle, SOL_SOCKET, SO_SNDBUF, (const char*) &sndBufSize, sizeof (int)) == 0
            && setsockopt (handle, IPPROTO_TCP, TCP_NODELAY, (const char*) &one, sizeof (int)) == 0;
}

//==============================================================================
int Socket::read (void* destBuffer, const int maxBytesToRead)
{
    if (isListener || ! connected)
        return -1;

    int bytesRead = 0;

    while (bytesRead < maxBytesToRead)
    {
        int bytesThisTime;

#if JUCE_WIN32
        bytesThisTime = recv (handle, ((char*) destBuffer) + bytesRead, maxBytesToRead - bytesRead, 0);
#else
        while ((bytesThisTime = ::read (handle, ((char*) destBuffer) + bytesRead, maxBytesToRead - bytesRead)) < 0
                 && errno == EINTR
                 && connected)
        {
        }
#endif

        if (bytesThisTime <= 0 || ! connected)
        {
            if (bytesRead == 0)
                bytesRead = -1;

            break;
        }

        bytesRead += bytesThisTime;
    }

    return bytesRead;
}

int Socket::write (const void* sourceBuffer, int numBytesToWrite)
{
    if (isListener || ! connected)
        return -1;

#if JUCE_WIN32
    return send (handle, (const char*) sourceBuffer, numBytesToWrite, 0);
#else
    int result;

    while ((result = ::write (handle, sourceBuffer, numBytesToWrite)) < 0
            && errno == EINTR)
    {
    }

    return result;
#endif
}

//==============================================================================
int Socket::isReady (int timeoutMsecs)
{
    if (! connected)
        return -1;

    struct timeval timeout;
    struct timeval* timeoutp;

    if (timeoutMsecs >= 0)
    {
        timeout.tv_sec = timeoutMsecs / 1000;
        timeout.tv_usec = (timeoutMsecs % 1000) * 1000;
        timeoutp = &timeout;
    }
    else
    {
        timeoutp = 0;
    }

    fd_set readbits;
    FD_ZERO (&readbits);
    FD_SET (handle, &readbits);

#if JUCE_WIN32
    if (select (handle + 1, &readbits, 0, 0, timeoutp) < 0)
        return -1;
#else
    int result;
    while ((result = select (handle + 1, &readbits, 0, 0, timeoutp)) < 0
            && errno == EINTR)
    {
    }

    if (result < 0)
        return -1;
#endif

    if (FD_ISSET (handle, &readbits))
        return 1;

    return 0;
}

//==============================================================================
bool Socket::connect (const String& newHostName,
                      int newPortNumber,
                      int timeOutMillisecs)
{
    if (isListener)
    {
        jassertfalse    // a listener socket can't connect to another one!
        return false;
    }

    if (connected)
        close();

    hostName = newHostName;
    portNumber = newPortNumber;
    isListener = false;

    struct hostent* hostEnt = gethostbyname (hostName);

    if (! hostEnt)
        return false;

    struct in_addr targetAddress;
    memcpy (&targetAddress.s_addr,
            *(hostEnt->h_addr_list),
            sizeof (targetAddress.s_addr));

    struct sockaddr_in servTmpAddr;
    zerostruct (servTmpAddr);
    servTmpAddr.sin_family = PF_INET;
    servTmpAddr.sin_addr = targetAddress;
    servTmpAddr.sin_port = htons ((uint16) portNumber);

    handle = (int) socket (AF_INET, SOCK_STREAM, 0);

    if (handle < 0)
        return false;

    while (timeOutMillisecs > 0 || timeOutMillisecs < 0)
    {
        if (handle < 0)
            return false;

        if (::connect (handle, (struct sockaddr*) &servTmpAddr, sizeof (struct sockaddr_in)) >= 0)
        {
            connected = true;
            break;
        }

        if (timeOutMillisecs > 0)
        {
            const int timeToSleep = jmin (timeOutMillisecs, 1000);
            timeOutMillisecs -= timeToSleep;
            Thread::sleep (timeToSleep);
        }
    }

    if (! (connected && resetSocketOptions()))
    {
        close();
        return false;
    }

    return true;
}

void Socket::close()
{
#if JUCE_WIN32
    closesocket (handle);
#else
    if (connected)
    {
        connected = false;

        if (isListener)
        {
            // need to do this to interrupt the accept() function..
            Socket temp;
            temp.connect ("localhost", portNumber, 1000);
        }
    }

    ::close (handle);
#endif

    hostName = String::empty;
    portNumber = 0;
    handle = -1;
    connected = false;
    isListener = false;
}

//==============================================================================
bool Socket::createListener (int newPortNumber)
{
    if (connected)
        close();

    hostName = "listener";
    portNumber = newPortNumber;
    isListener = true;

    struct sockaddr_in servTmpAddr;
    zerostruct (servTmpAddr);
    servTmpAddr.sin_family = PF_INET;
    servTmpAddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servTmpAddr.sin_port = htons ((uint16) portNumber);

    handle = (int) socket (AF_INET, SOCK_STREAM, 0);

    if (handle < 0)
        return false;

    const int reuse = 1;
    setsockopt (handle, SOL_SOCKET, SO_REUSEADDR, (const char*) &reuse, sizeof (reuse));

    if (bind (handle, (struct sockaddr*) &servTmpAddr, sizeof (struct sockaddr_in)) < 0
         || listen (handle, SOMAXCONN) < 0)
    {
        close();
        return false;
    }

    connected = true;
    return true;
}

Socket* Socket::waitForNextConnection()
{
    jassert (isListener || ! connected); // to call this method, you first have to use createListener() to
                                         // prepare this socket as a listener.

    if (connected && isListener)
    {
        struct sockaddr address;

#if defined (JUCE_LINUX) || (defined (JUCE_MAC) && ! MACOS_10_2_OR_EARLIER)
        socklen_t len = sizeof (sockaddr);
#else
        int len = sizeof (sockaddr);
#endif
        const int newSocket = (int) accept (handle, &address, &len);

        if (newSocket >= 0 && connected)
            return new Socket (inet_ntoa (((struct sockaddr_in*) &address)->sin_addr),
                               portNumber, newSocket);
    }

    return 0;
}

//==============================================================================
bool Socket::isLocal() const throw()
{
    return hostName == T("127.0.0.1");
}


END_JUCE_NAMESPACE
