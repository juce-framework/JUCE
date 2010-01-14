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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
int SystemStats::getMACAddresses (int64* addresses, int maxNum, const bool littleEndian) throw()
{
    int numResults = 0;

    const int s = socket (AF_INET, SOCK_DGRAM, 0);
    if (s != -1)
    {
        char buf [1024];
        struct ifconf ifc;
        ifc.ifc_len = sizeof (buf);
        ifc.ifc_buf = buf;
        ioctl (s, SIOCGIFCONF, &ifc);

        for (unsigned int i = 0; i < ifc.ifc_len / sizeof (struct ifreq); ++i)
        {
            struct ifreq ifr;
            strcpy (ifr.ifr_name, ifc.ifc_req[i].ifr_name);

            if (ioctl (s, SIOCGIFFLAGS, &ifr) == 0
                 && (ifr.ifr_flags & IFF_LOOPBACK) == 0
                 && ioctl (s, SIOCGIFHWADDR, &ifr) == 0
                 && numResults < maxNum)
            {
                int64 a = 0;
                for (int j = 6; --j >= 0;)
                    a = (a << 8) | (uint8) ifr.ifr_hwaddr.sa_data [littleEndian ? j : (5 - j)];

                *addresses++ = a;
                ++numResults;
            }
        }

        close (s);
    }

    return numResults;
}


bool PlatformUtilities::launchEmailWithAttachments (const String& targetEmailAddress,
                                                    const String& emailSubject,
                                                    const String& bodyText,
                                                    const StringArray& filesToAttach)
{
    jassertfalse    // xxx todo

    return false;
}

//==============================================================================
/** A HTTP input stream that uses sockets.
 */
class JUCE_HTTPSocketStream
{
public:
    //==============================================================================
    JUCE_HTTPSocketStream()
        : readPosition (0),
          socketHandle (-1),
          levelsOfRedirection (0),
          timeoutSeconds (15)
    {
    }

    ~JUCE_HTTPSocketStream()
    {
        closeSocket();
    }

    //==============================================================================
    bool open (const String& url,
               const String& headers,
               const MemoryBlock& postData,
               const bool isPost,
               URL::OpenStreamProgressCallback* callback,
               void* callbackContext,
               int timeOutMs)
    {
        closeSocket();

        uint32 timeOutTime = Time::getMillisecondCounter();

        if (timeOutMs == 0)
            timeOutTime += 60000;
        else if (timeOutMs < 0)
            timeOutTime = 0xffffffff;
        else
            timeOutTime += timeOutMs;

        String hostName, hostPath;
        int hostPort;

        if (! decomposeURL (url, hostName, hostPath, hostPort))
            return false;

        const struct hostent* host = 0;
        int port = 0;

        String proxyName, proxyPath;
        int proxyPort = 0;

        String proxyURL (getenv ("http_proxy"));
        if (proxyURL.startsWithIgnoreCase (T("http://")))
        {
            if (! decomposeURL (proxyURL, proxyName, proxyPath, proxyPort))
                return false;

            host = gethostbyname ((const char*) proxyName.toUTF8());
            port = proxyPort;
        }
        else
        {
            host = gethostbyname ((const char*) hostName.toUTF8());
            port = hostPort;
        }

        if (host == 0)
            return false;

        struct sockaddr_in address;
        zerostruct (address);
        memcpy ((void*) &address.sin_addr, (const void*) host->h_addr, host->h_length);
        address.sin_family = host->h_addrtype;
        address.sin_port = htons (port);

        socketHandle = socket (host->h_addrtype, SOCK_STREAM, 0);

        if (socketHandle == -1)
            return false;

        int receiveBufferSize = 16384;
        setsockopt (socketHandle, SOL_SOCKET, SO_RCVBUF, (char*) &receiveBufferSize, sizeof (receiveBufferSize));
        setsockopt (socketHandle, SOL_SOCKET, SO_KEEPALIVE, 0, 0);

#if JUCE_MAC
        setsockopt (socketHandle, SOL_SOCKET, SO_NOSIGPIPE, 0, 0);
#endif

        if (connect (socketHandle, (struct sockaddr*) &address, sizeof (address)) == -1)
        {
            closeSocket();
            return false;
        }

        const MemoryBlock requestHeader (createRequestHeader (hostName, hostPort,
                                                              proxyName, proxyPort,
                                                              hostPath, url,
                                                              headers, postData,
                                                              isPost));
        size_t totalHeaderSent = 0;

        while (totalHeaderSent < requestHeader.getSize())
        {
            if (Time::getMillisecondCounter() > timeOutTime)
            {
                closeSocket();
                return false;
            }

            const int numToSend = jmin (1024, (int) (requestHeader.getSize() - totalHeaderSent));

            if (send (socketHandle,
                      ((const char*) requestHeader.getData()) + totalHeaderSent,
                      numToSend, 0)
                != numToSend)
            {
                closeSocket();
                return false;
            }

            totalHeaderSent += numToSend;

            if (callback != 0 && ! callback (callbackContext, totalHeaderSent, requestHeader.getSize()))
            {
                closeSocket();
                return false;
            }
        }

        const String responseHeader (readResponse (timeOutTime));

        if (responseHeader.isNotEmpty())
        {
            //DBG (responseHeader);

            StringArray lines;
            lines.addLines (responseHeader);

            // NB - using charToString() here instead of just T(" "), because that was
            // causing a mysterious gcc internal compiler error...
            const int statusCode = responseHeader.fromFirstOccurrenceOf (String::charToString (T(' ')), false, false)
                                                 .substring (0, 3).getIntValue();

            //int contentLength = findHeaderItem (lines, T("Content-Length:")).getIntValue();
            //bool isChunked = findHeaderItem (lines, T("Transfer-Encoding:")).equalsIgnoreCase ("chunked");

            String location (findHeaderItem (lines, T("Location:")));

            if (statusCode >= 300 && statusCode < 400
                && location.isNotEmpty())
            {
                if (! location.startsWithIgnoreCase (T("http://")))
                    location = T("http://") + location;

                if (levelsOfRedirection++ < 3)
                    return open (location, headers, postData, isPost, callback, callbackContext, timeOutMs);
            }
            else
            {
                levelsOfRedirection = 0;
                return true;
            }
        }

        closeSocket();
        return false;
    }

    //==============================================================================
    int read (void* buffer, int bytesToRead)
    {
        fd_set readbits;
        FD_ZERO (&readbits);
        FD_SET (socketHandle, &readbits);

        struct timeval tv;
        tv.tv_sec = timeoutSeconds;
        tv.tv_usec = 0;

        if (select (socketHandle + 1, &readbits, 0, 0, &tv) <= 0)
            return 0;   // (timeout)

        const int bytesRead = jmax (0, recv (socketHandle, buffer, bytesToRead, MSG_WAITALL));
        readPosition += bytesRead;
        return bytesRead;
    }

    //==============================================================================
    int readPosition;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int socketHandle, levelsOfRedirection;
    const int timeoutSeconds;

    //==============================================================================
    void closeSocket()
    {
        if (socketHandle >= 0)
            close (socketHandle);

        socketHandle = -1;
    }

    const MemoryBlock createRequestHeader (const String& hostName,
                                           const int hostPort,
                                           const String& proxyName,
                                           const int proxyPort,
                                           const String& hostPath,
                                           const String& originalURL,
                                           const String& headers,
                                           const MemoryBlock& postData,
                                           const bool isPost)
    {
        String header (isPost ? "POST " : "GET ");

        if (proxyName.isEmpty())
        {
            header << hostPath << " HTTP/1.0\r\nHost: "
            << hostName << ':' << hostPort;
        }
        else
        {
            header << originalURL << " HTTP/1.0\r\nHost: "
            << proxyName << ':' << proxyPort;
        }

        header << "\r\nUser-Agent: JUCE/"
        << JUCE_MAJOR_VERSION << '.' << JUCE_MINOR_VERSION
        << "\r\nConnection: Close\r\nContent-Length: "
        << postData.getSize() << "\r\n"
        << headers << "\r\n";

        MemoryBlock mb;
        mb.append (header.toUTF8(), (int) strlen (header.toUTF8()));
        mb.append (postData.getData(), postData.getSize());

        return mb;
    }

    const String readResponse (const uint32 timeOutTime)
    {
        int bytesRead = 0, numConsecutiveLFs  = 0;
        MemoryBlock buffer (1024, true);

        while (numConsecutiveLFs < 2 && bytesRead < 32768
               && Time::getMillisecondCounter() <= timeOutTime)
        {
            fd_set readbits;
            FD_ZERO (&readbits);
            FD_SET (socketHandle, &readbits);

            struct timeval tv;
            tv.tv_sec = timeoutSeconds;
            tv.tv_usec = 0;

            if (select (socketHandle + 1, &readbits, 0, 0, &tv) <= 0)
                return String::empty;  // (timeout)

            buffer.ensureSize (bytesRead + 8, true);
            char* const dest = (char*) buffer.getData() + bytesRead;

            if (recv (socketHandle, dest, 1, 0) == -1)
                return String::empty;

            const char lastByte = *dest;
            ++bytesRead;

            if (lastByte == '\n')
                ++numConsecutiveLFs;
            else if (lastByte != '\r')
                numConsecutiveLFs = 0;
        }

        const String header (String::fromUTF8 ((const uint8*) buffer.getData()));

        if (header.startsWithIgnoreCase (T("HTTP/")))
            return header.trimEnd();

        return String::empty;
    }

    //==============================================================================
    static bool decomposeURL (const String& url,
                              String& host, String& path, int& port)
    {
        if (! url.startsWithIgnoreCase (T("http://")))
            return false;

        const int nextSlash = url.indexOfChar (7, '/');
        int nextColon = url.indexOfChar (7, ':');
        if (nextColon > nextSlash && nextSlash > 0)
            nextColon = -1;

        if (nextColon >= 0)
        {
            host = url.substring (7, nextColon);

            if (nextSlash >= 0)
                port = url.substring (nextColon + 1, nextSlash).getIntValue();
            else
                port = url.substring (nextColon + 1).getIntValue();
        }
        else
        {
            port = 80;

            if (nextSlash >= 0)
                host = url.substring (7, nextSlash);
            else
                host = url.substring (7);
        }

        if (nextSlash >= 0)
            path = url.substring (nextSlash);
        else
            path = T("/");

        return true;
    }

    //==============================================================================
    static const String findHeaderItem (const StringArray& lines, const String& itemName)
    {
        for (int i = 0; i < lines.size(); ++i)
            if (lines[i].startsWithIgnoreCase (itemName))
                return lines[i].substring (itemName.length()).trim();

        return String::empty;
    }
};

//==============================================================================
bool juce_isOnLine()
{
    return true;
}

void* juce_openInternetFile (const String& url,
                             const String& headers,
                             const MemoryBlock& postData,
                             const bool isPost,
                             URL::OpenStreamProgressCallback* callback,
                             void* callbackContext,
                             int timeOutMs)
{
    JUCE_HTTPSocketStream* const s = new JUCE_HTTPSocketStream();

    if (s->open (url, headers, postData, isPost,
                 callback, callbackContext, timeOutMs))
        return s;

    delete s;
    return 0;
}

void juce_closeInternetFile (void* handle)
{
    JUCE_HTTPSocketStream* const s = (JUCE_HTTPSocketStream*) handle;

    if (s != 0)
        delete s;
}

int juce_readFromInternetFile (void* handle, void* buffer, int bytesToRead)
{
    JUCE_HTTPSocketStream* const s = (JUCE_HTTPSocketStream*) handle;

    if (s != 0)
        return s->read (buffer, bytesToRead);

    return 0;
}

int64 juce_getInternetFileContentLength (void* handle)
{
    JUCE_HTTPSocketStream* const s = (JUCE_HTTPSocketStream*) handle;

    if (s != 0)
    {
        //xxx todo
        jassertfalse
    }

    return -1;
}

int juce_seekInInternetFile (void* handle, int newPosition)
{
    JUCE_HTTPSocketStream* const s = (JUCE_HTTPSocketStream*) handle;

    if (s != 0)
        return s->readPosition;

    return 0;
}

#endif
