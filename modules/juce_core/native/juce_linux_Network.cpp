/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

void MACAddress::findAllAddresses (Array<MACAddress>& result)
{
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
                 && ioctl (s, SIOCGIFHWADDR, &ifr) == 0)
            {
                MACAddress ma ((const uint8*) ifr.ifr_hwaddr.sa_data);

                if (! ma.isNull())
                    result.addIfNotAlreadyThere (ma);
            }
        }

        close (s);
    }
}


bool JUCE_CALLTYPE Process::openEmailWithAttachments (const String& /* targetEmailAddress */,
                                                      const String& /* emailSubject */,
                                                      const String& /* bodyText */,
                                                      const StringArray& /* filesToAttach */)
{
    jassertfalse;    // xxx todo
    return false;
}


//==============================================================================
class WebInputStream  : public InputStream
{
public:
    WebInputStream (const String& address_, bool isPost_, const MemoryBlock& postData_,
                    URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                    const String& headers_, int timeOutMs_, StringPairArray* responseHeaders)
      : statusCode (0), socketHandle (-1), levelsOfRedirection (0),
        address (address_), headers (headers_), postData (postData_), position (0),
        finished (false), isPost (isPost_), timeOutMs (timeOutMs_)
    {
        statusCode = createConnection (progressCallback, progressCallbackContext);

        if (responseHeaders != nullptr && ! isError())
        {
            for (int i = 0; i < headerLines.size(); ++i)
            {
                const String& headersEntry = headerLines[i];
                const String key (headersEntry.upToFirstOccurrenceOf (": ", false, false));
                const String value (headersEntry.fromFirstOccurrenceOf (": ", false, false));
                const String previousValue ((*responseHeaders) [key]);
                responseHeaders->set (key, previousValue.isEmpty() ? value : (previousValue + "," + value));
            }
        }
    }

    ~WebInputStream()
    {
        closeSocket();
    }

    //==============================================================================
    bool isError() const                 { return socketHandle < 0; }
    bool isExhausted() override          { return finished; }
    int64 getPosition() override         { return position; }

    int64 getTotalLength() override
    {
        //xxx to do
        return -1;
    }

    int read (void* buffer, int bytesToRead) override
    {
        if (finished || isError())
            return 0;

        fd_set readbits;
        FD_ZERO (&readbits);
        FD_SET (socketHandle, &readbits);

        struct timeval tv;
        tv.tv_sec = jmax (1, timeOutMs / 1000);
        tv.tv_usec = 0;

        if (select (socketHandle + 1, &readbits, 0, 0, &tv) <= 0)
            return 0;   // (timeout)

        const int bytesRead = jmax (0, (int) recv (socketHandle, buffer, bytesToRead, MSG_WAITALL));
        if (bytesRead == 0)
            finished = true;

        position += bytesRead;
        return bytesRead;
    }

    bool setPosition (int64 wantedPos) override
    {
        if (isError())
            return false;

        if (wantedPos != position)
        {
            finished = false;

            if (wantedPos < position)
            {
                closeSocket();
                position = 0;
                statusCode = createConnection (0, 0);
            }

            skipNextBytes (wantedPos - position);
        }

        return true;
    }

    //==============================================================================
    int statusCode;

private:
    int socketHandle, levelsOfRedirection;
    StringArray headerLines;
    String address, headers;
    MemoryBlock postData;
    int64 position;
    bool finished;
    const bool isPost;
    const int timeOutMs;

    void closeSocket()
    {
        if (socketHandle >= 0)
            close (socketHandle);

        socketHandle = -1;
        levelsOfRedirection = 0;
    }

    int createConnection (URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext)
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
        if (! decomposeURL (address, hostName, hostPath, hostPort))
            return 0;

        String serverName, proxyName, proxyPath;
        int proxyPort = 0;
        int port = 0;

        const String proxyURL (getenv ("http_proxy"));
        if (proxyURL.startsWithIgnoreCase ("http://"))
        {
            if (! decomposeURL (proxyURL, proxyName, proxyPath, proxyPort))
                return 0;

            serverName = proxyName;
            port = proxyPort;
        }
        else
        {
            serverName = hostName;
            port = hostPort;
        }

        struct addrinfo hints;
        zerostruct (hints);

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICSERV;

        struct addrinfo* result = nullptr;
        if (getaddrinfo (serverName.toUTF8(), String (port).toUTF8(), &hints, &result) != 0 || result == 0)
            return 0;

        socketHandle = socket (result->ai_family, result->ai_socktype, 0);

        if (socketHandle == -1)
        {
            freeaddrinfo (result);
            return 0;
        }

        int receiveBufferSize = 16384;
        setsockopt (socketHandle, SOL_SOCKET, SO_RCVBUF, (char*) &receiveBufferSize, sizeof (receiveBufferSize));
        setsockopt (socketHandle, SOL_SOCKET, SO_KEEPALIVE, 0, 0);

      #if JUCE_MAC
        setsockopt (socketHandle, SOL_SOCKET, SO_NOSIGPIPE, 0, 0);
      #endif

        if (connect (socketHandle, result->ai_addr, result->ai_addrlen) == -1)
        {
            closeSocket();
            freeaddrinfo (result);
            return 0;
        }

        freeaddrinfo (result);

        {
            const MemoryBlock requestHeader (createRequestHeader (hostName, hostPort, proxyName, proxyPort,
                                                                  hostPath, address, headers, postData, isPost));

            if (! sendHeader (socketHandle, requestHeader, timeOutTime,
                              progressCallback, progressCallbackContext))
            {
                closeSocket();
                return 0;
            }
        }

        String responseHeader (readResponse (socketHandle, timeOutTime));
        position = 0;

        if (responseHeader.isNotEmpty())
        {
            headerLines = StringArray::fromLines (responseHeader);

            const int status = responseHeader.fromFirstOccurrenceOf (" ", false, false)
                                             .substring (0, 3).getIntValue();

            //int contentLength = findHeaderItem (lines, "Content-Length:").getIntValue();
            //bool isChunked = findHeaderItem (lines, "Transfer-Encoding:").equalsIgnoreCase ("chunked");

            String location (findHeaderItem (headerLines, "Location:"));

            if (status >= 300 && status < 400
                 && location.isNotEmpty() && location != address)
            {
                if (! location.startsWithIgnoreCase ("http://"))
                    location = "http://" + location;

                if (++levelsOfRedirection <= 3)
                {
                    address = location;
                    return createConnection (progressCallback, progressCallbackContext);
                }
            }
            else
            {
                levelsOfRedirection = 0;
                return status;
            }
        }

        closeSocket();
        return 0;
    }

    //==============================================================================
    String readResponse (const int socketHandle, const uint32 timeOutTime)
    {
        int numConsecutiveLFs  = 0;
        MemoryOutputStream buffer;

        while (numConsecutiveLFs < 2
                && buffer.getDataSize() < 32768
                && Time::getMillisecondCounter() <= timeOutTime
                && ! (finished || isError()))
        {
            char c = 0;
            if (read (&c, 1) != 1)
                return String();

            buffer.writeByte (c);

            if (c == '\n')
                ++numConsecutiveLFs;
            else if (c != '\r')
                numConsecutiveLFs = 0;
        }

        const String header (buffer.toString().trimEnd());

        if (header.startsWithIgnoreCase ("HTTP/"))
            return header;

        return String();
    }

    static void writeValueIfNotPresent (MemoryOutputStream& dest, const String& headers, const String& key, const String& value)
    {
        if (! headers.containsIgnoreCase (key))
            dest << "\r\n" << key << ' ' << value;
    }

    static void writeHost (MemoryOutputStream& dest, const bool isPost, const String& path, const String& host, const int port)
    {
        dest << (isPost ? "POST " : "GET ") << path << " HTTP/1.0\r\nHost: " << host;
    }

    static MemoryBlock createRequestHeader (const String& hostName, const int hostPort,
                                            const String& proxyName, const int proxyPort,
                                            const String& hostPath, const String& originalURL,
                                            const String& userHeaders, const MemoryBlock& postData,
                                            const bool isPost)
    {
        MemoryOutputStream header;

        if (proxyName.isEmpty())
            writeHost (header, isPost, hostPath, hostName, hostPort);
        else
            writeHost (header, isPost, originalURL, proxyName, proxyPort);

        writeValueIfNotPresent (header, userHeaders, "User-Agent:", "JUCE/" JUCE_STRINGIFY(JUCE_MAJOR_VERSION)
                                                                        "." JUCE_STRINGIFY(JUCE_MINOR_VERSION)
                                                                        "." JUCE_STRINGIFY(JUCE_BUILDNUMBER));
        writeValueIfNotPresent (header, userHeaders, "Connection:", "close");

        if (isPost)
            writeValueIfNotPresent (header, userHeaders, "Content-Length:", String ((int) postData.getSize()));

        header << "\r\n" << userHeaders
               << "\r\n" << postData;

        return header.getMemoryBlock();
    }

    static bool sendHeader (int socketHandle, const MemoryBlock& requestHeader, const uint32 timeOutTime,
                            URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext)
    {
        size_t totalHeaderSent = 0;

        while (totalHeaderSent < requestHeader.getSize())
        {
            if (Time::getMillisecondCounter() > timeOutTime)
                return false;

            const int numToSend = jmin (1024, (int) (requestHeader.getSize() - totalHeaderSent));

            if (send (socketHandle, static_cast <const char*> (requestHeader.getData()) + totalHeaderSent, numToSend, 0) != numToSend)
                return false;

            totalHeaderSent += numToSend;

            if (progressCallback != nullptr && ! progressCallback (progressCallbackContext, totalHeaderSent, requestHeader.getSize()))
                return false;
        }

        return true;
    }

    static bool decomposeURL (const String& url, String& host, String& path, int& port)
    {
        if (! url.startsWithIgnoreCase ("http://"))
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
            path = "/";

        return true;
    }

    static String findHeaderItem (const StringArray& lines, const String& itemName)
    {
        for (int i = 0; i < lines.size(); ++i)
            if (lines[i].startsWithIgnoreCase (itemName))
                return lines[i].substring (itemName.length()).trim();

        return String();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebInputStream)
};
