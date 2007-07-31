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

#ifndef __JUCE_MAC_HTTPSTREAM_JUCEHEADER__
#define __JUCE_MAC_HTTPSTREAM_JUCEHEADER__

// (This file gets included by the mac + linux networking code)


//==============================================================================
/** A HTTP input stream that uses sockets.
*/
class JUCE_HTTPSocketStream
{
public:
    //==============================================================================
    JUCE_HTTPSocketStream()
        : statusCode (0),
          readPosition (0),
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
               const String& optionalPostText,
               const bool isPost)
    {
        closeSocket();

        String hostName, hostPath;
        int hostPort;

        if (! decomposeURL (url, hostName, hostPath, hostPort))
            return false;

        struct hostent* const host
            = gethostbyname ((const char*) hostName.toUTF8());

        if (host == 0)
            return false;

        struct sockaddr_in address;
        zerostruct (address);
        memcpy ((void*) &address.sin_addr, (const void*) host->h_addr, host->h_length);
        address.sin_family = host->h_addrtype;
        address.sin_port = htons (hostPort);

        socketHandle = socket (host->h_addrtype, SOCK_STREAM, 0);

        if (socketHandle == -1)
            return false;

        int receiveBufferSize = 16384;
        setsockopt (socketHandle, SOL_SOCKET, SO_RCVBUF, (char*) &receiveBufferSize, sizeof (receiveBufferSize));
        setsockopt (socketHandle, SOL_SOCKET, SO_KEEPALIVE, 0, 0);

        if (connect (socketHandle, (struct sockaddr*) &address, sizeof (address)) == -1)
        {
            closeSocket();
            return false;
        }

        String proxyURL (getenv ("http_proxy"));

        if (! proxyURL.startsWithIgnoreCase (T("http://")))
            proxyURL = String::empty;

        const String requestHeader (createRequestHeader (hostName, hostPath,
                                                         proxyURL, url,
                                                         hostPort, optionalPostText,
                                                         isPost));

        const char* const utf8Header = (const char*) requestHeader.toUTF8();
        const int headerLen = strlen (utf8Header);

        if (! send (socketHandle, utf8Header, headerLen, 0) == headerLen)
        {
            closeSocket();
            return false;
        }

        const String responseHeader (readResponse());

        if (responseHeader.isNotEmpty())
        {
            //DBG (responseHeader);

            StringArray lines;
            lines.addLines (responseHeader);

            statusCode = responseHeader.fromFirstOccurrenceOf (T(" "), false, false)
                                .substring (4).getIntValue();

            //int contentLength = findHeaderItem (lines, T("Content-Length:")).getIntValue();
            //bool isChunked = findHeaderItem (lines, T("Transfer-Encoding:")).equalsIgnoreCase ("chunked");

            String location (findHeaderItem (lines, T("Location:")));

            if (statusCode >= 300 && statusCode < 400
                 && location.isNotEmpty())
            {
                if (! location.startsWithIgnoreCase (T("http://")))
                    location = T("http://") + location;

                if (levelsOfRedirection++ < 3)
                    return open (location, optionalPostText, isPost);
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
    int statusCode, readPosition;

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

    const String createRequestHeader (const String& hostName,
                                      const String& hostPath,
                                      const String& proxyURL,
                                      const String& originalURL,
                                      const int hostPort,
                                      const String& optionalPostText,
                                      const bool isPost)
    {
        String header (isPost ? "POST " : "GET ");

        if (proxyURL.isEmpty())
        {
            header << hostPath << " HTTP/1.1\r\nHost: "
                   << hostName << ':' << hostPort;
        }
        else
        {
            String proxyName, proxyPath;
            int proxyPort;

            if (! decomposeURL (proxyURL, proxyName, proxyPath, proxyPort))
                return String::empty;

            header << originalURL << " HTTP/1.1\r\nHost: "
                   << proxyName << ':' << proxyPort;

            /* xxx needs finishing
            const char* proxyAuth = getenv ("http_proxy_auth");
            if (proxyAuth != 0)
                header << T("\r\nProxy-Authorization: ") << Base64Encode (proxyAuth);
            */
        }

        header << "\r\nUser-Agent: JUCE/"
               << JUCE_MAJOR_VERSION << '.' << JUCE_MINOR_VERSION
               << "\r\nConnection: Close\r\n";

        if (isPost && optionalPostText.isNotEmpty())
        {
            const char* const postTextUTF8 = (const char*) optionalPostText.toUTF8();

            header << "Content-type: application/x-www-form-urlencoded\r\nContent-length: "
                   << (int) strlen (postTextUTF8) << "\r\n\r\n"
                   << optionalPostText;
        }

        header << "\r\n";
        //DBG (header);
        return header;
    }

    const String readResponse()
    {
        int bytesRead = 0, numConsecutiveLFs  = 0;
        MemoryBlock buffer (1024, true);

        while (numConsecutiveLFs < 2 && bytesRead < 32768)
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
                             const String& optionalPostText,
                             const bool isPost)
{
    JUCE_HTTPSocketStream* const s = new JUCE_HTTPSocketStream();

    if (s->open (url, optionalPostText, isPost))
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

int juce_getStatusCodeFor (void* handle)
{
    JUCE_HTTPSocketStream* const s = (JUCE_HTTPSocketStream*) handle;

    if (s != 0)
        return s->statusCode;

    return 0;
}

int juce_readFromInternetFile (void* handle, void* buffer, int bytesToRead)
{
    JUCE_HTTPSocketStream* const s = (JUCE_HTTPSocketStream*) handle;

    if (s != 0)
        return s->read (buffer, bytesToRead);

    return 0;
}

int juce_seekInInternetFile (void* handle, int newPosition)
{
    JUCE_HTTPSocketStream* const s = (JUCE_HTTPSocketStream*) handle;

    if (s != 0)
        return s->readPosition;

    return 0;
}


#endif   // __JUCE_MAC_HTTPSTREAM_JUCEHEADER__
