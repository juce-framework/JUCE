/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

class WebInputStream  : public InputStream
{
public:
    WebInputStream (const String& address, bool isPost, const MemoryBlock& postData,
                    URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                    const String& headers, int timeOutMs, StringPairArray* responseHeaders,
                    const int maxRedirects, const String& httpRequest)
        : multi (nullptr), curl (nullptr), headerList (nullptr), lastError (CURLE_OK),
          contentLength (-1), streamPos (0),
          finished (false), skipBytes (0),
          postBuffer (nullptr), postPosition (0)
    {
        statusCode = -1;

        if (init() && setOptions (address, timeOutMs, (responseHeaders != nullptr),
                                  maxRedirects, headers, isPost, httpRequest, postData.getSize()))
        {
            connect (responseHeaders, isPost, postData, progressCallback, progressCallbackContext);
        }
        else
        {
            cleanup();
        }
    }

    ~WebInputStream()
    {
        cleanup();
    }

    //==============================================================================
    // Input Stream overrides
    bool isError() const                 { return curl == nullptr || lastError != CURLE_OK; }
    bool isExhausted() override          { return (isError() || finished) && curlBuffer.getSize() == 0; }
    int64 getPosition() override         { return streamPos; }
    int64 getTotalLength() override      { return contentLength; }

    int read (void* buffer, int bytesToRead) override
    {
        return readOrSkip (buffer, bytesToRead, false);
    }

    bool setPosition (int64 wantedPos) override
    {
        const int amountToSkip = static_cast<int> (wantedPos - getPosition());

        if (amountToSkip < 0)
            return false;

        if (amountToSkip == 0)
            return true;

        const int actuallySkipped = readOrSkip (nullptr, amountToSkip, true);

        return actuallySkipped == amountToSkip;
    }

    //==============================================================================
    int statusCode;

private:
    //==============================================================================
    bool init()
    {
        multi = curl_multi_init();

        if (multi != nullptr)
        {
            curl = curl_easy_init();

            if (curl != nullptr)
                if (curl_multi_add_handle (multi, curl) == CURLM_OK)
                    return true;
        }

        cleanup();
        return false;
    }

    void cleanup()
    {
        if (curl != nullptr)
        {
            curl_multi_remove_handle (multi, curl);

            if (headerList != nullptr)
            {
                curl_slist_free_all (headerList);
                headerList = nullptr;
            }

            curl_easy_cleanup (curl);
            curl = nullptr;
        }

        if (multi != nullptr)
        {
            curl_multi_cleanup (multi);
            multi = nullptr;
        }
    }

    //==============================================================================
    bool setOptions (const String& address, int timeOutMs, bool wantsHeaders,
                     const int maxRedirects, const String& headers,
                     bool isPost, const String& httpRequest, size_t postSize)
    {
        if (curl_easy_setopt (curl, CURLOPT_URL, address.toRawUTF8()) == CURLE_OK
             && curl_easy_setopt (curl, CURLOPT_WRITEDATA, this) == CURLE_OK
             && curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, StaticCurlWrite) == CURLE_OK
             && curl_easy_setopt (curl, CURLOPT_MAXREDIRS, static_cast<long> (maxRedirects)) == CURLE_OK)
        {
            if (isPost)
            {
                if (curl_easy_setopt (curl, CURLOPT_READDATA, this) != CURLE_OK
                     || curl_easy_setopt (curl, CURLOPT_READFUNCTION, StaticCurlRead) != CURLE_OK)
                    return false;

                if (curl_easy_setopt (curl, CURLOPT_POST, 1) != CURLE_OK
                     || curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t> (postSize)) != CURLE_OK)
                    return false;
            }

            // handle special http request commands
            bool hasSpecialRequestCmd = isPost ? (httpRequest != "POST") : (httpRequest != "GET");
            if (hasSpecialRequestCmd)
            {
                if (curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, httpRequest.toRawUTF8()) != CURLE_OK)
                    return false;
            }

            // do we want to parse the headers
            if (wantsHeaders)
            {
                if (curl_easy_setopt (curl, CURLOPT_HEADERDATA, this) != CURLE_OK
                     || curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, StaticCurlHeader) != CURLE_OK)
                    return false;
            }

            if (headers.isNotEmpty())
            {
                const StringArray headerLines = StringArray::fromLines (headers);

                // fromLines will always return at least one line if the string is not empty
                jassert (headerLines.size() > 0);
                headerList = curl_slist_append (headerList, headerLines [0].toRawUTF8());

                for (int i = 1; (i < headerLines.size() && headerList != nullptr); ++i)
                    headerList = curl_slist_append (headerList, headerLines [i].toRawUTF8());

                if (headerList == nullptr)
                    return false;

                if (curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headerList) != CURLE_OK)
                    return false;
            }

            if (timeOutMs > 0)
            {
                long timeOutSecs = ((long) timeOutMs + 999) / 1000;

                if (curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, timeOutSecs) != CURLE_OK
                     || curl_easy_setopt (curl, CURLOPT_LOW_SPEED_LIMIT, 100) != CURLE_OK
                     || curl_easy_setopt (curl, CURLOPT_LOW_SPEED_TIME, timeOutSecs) != CURLE_OK)
                    return false;
            }

            return true;
        }

        return false;
    }

    void connect (StringPairArray* responseHeaders, bool isPost, const MemoryBlock& postData,
                  URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext)
    {
        if (isPost)
            postBuffer = &postData;

        size_t lastPos = static_cast<size_t> (-1);

        // step until either: 1) there is an error 2) the transaction is complete
        // or 3) data is in the in buffer
        while ((! finished) && curlBuffer.getSize() == 0 && curl != nullptr)
        {
            singleStep();

            // call callbacks if this is a post request
            if (isPost && progressCallback != nullptr && lastPos != postPosition)
            {
                lastPos = postPosition;

                if (! progressCallback (progressCallbackContext,
                                        static_cast<int> (lastPos),
                                        static_cast<int> (postData.getSize())))
                {
                    // user has decided to abort the transaction
                    cleanup();
                    return;
                }
            }
        }

        long responseCode;
        if (curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &responseCode) == CURLE_OK)
            statusCode = static_cast<int> (responseCode);

        // parse headers
        if (responseHeaders != nullptr)
            parseHttpHeaders (*responseHeaders);

        // get content length size
        double curlLength;
        if (curl_easy_getinfo (curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &curlLength) == CURLE_OK)
            contentLength = static_cast<int64> (curlLength);
    }

    void finish()
    {
        if (curl == nullptr)
            return;

        for (;;)
        {
            int cnt = 0;

            if (CURLMsg* msg = curl_multi_info_read (multi, &cnt))
            {
                if (msg->msg == CURLMSG_DONE && msg->easy_handle == curl)
                {
                    lastError = msg->data.result; // this is the error that stopped our process from continuing
                    break;
                }
            }
            else
            {
                break;
            }
        }

        finished = true;
    }

    //==============================================================================
    void singleStep()
    {
        if (curl == nullptr || lastError != CURLE_OK)
            return;

        fd_set fdread, fdwrite, fdexcep;
        int maxfd = -1;
        long curl_timeo;

        if ((lastError = (int) curl_multi_timeout (multi, &curl_timeo)) != CURLM_OK)
            return;

        // why 980? see http://curl.haxx.se/libcurl/c/curl_multi_timeout.html
        if (curl_timeo < 0)
            curl_timeo = 980;

        struct timeval tv;
        tv.tv_sec = curl_timeo / 1000;
        tv.tv_usec = (curl_timeo % 1000) * 1000;

        FD_ZERO (&fdread);
        FD_ZERO (&fdwrite);
        FD_ZERO (&fdexcep);


        if ((lastError = (int) curl_multi_fdset (multi, &fdread, &fdwrite, &fdexcep, &maxfd)) != CURLM_OK)
            return;

        if (maxfd != -1)
        {
            if (select (maxfd + 1, &fdread, &fdwrite, &fdexcep, &tv) < 0)
            {
                lastError = -1;
                return;
            }
        }
        else
        {
            // if curl does not return any sockets for to wait on, then the doc says to wait 100 ms
            Thread::sleep (100);
        }

        int still_running = 0;
        int curlRet;

        while ((curlRet = (int) curl_multi_perform (multi, &still_running)) == CURLM_CALL_MULTI_PERFORM)
        {}

        if ((lastError = curlRet) != CURLM_OK)
            return;

        if (still_running <= 0)
            finish();
    }

    int readOrSkip (void* buffer, int bytesToRead, bool skip)
    {
        if (bytesToRead <= 0)
            return 0;

        size_t pos = 0;
        size_t len = static_cast<size_t> (bytesToRead);

        while (len > 0)
        {
            size_t bufferBytes = curlBuffer.getSize();
            bool removeSection = true;

            if (bufferBytes == 0)
            {
                // do not call curl again if we are finished
                if (finished || curl == nullptr)
                    return static_cast<int> (pos);

                skipBytes = skip ? len : 0;
                singleStep();

                // update the amount that was read/skipped from curl
                bufferBytes = skip ? len - skipBytes : curlBuffer.getSize();
                removeSection = ! skip;
            }

            // can we copy data from the internal buffer?
            if (bufferBytes > 0)
            {
                size_t max = jmin (len, bufferBytes);

                if (! skip)
                    memcpy (addBytesToPointer (buffer, pos), curlBuffer.getData(), max);

                pos += max;
                streamPos += static_cast<int64> (max);
                len -= max;

                if (removeSection)
                    curlBuffer.removeSection (0, max);
            }
        }

        return static_cast<int> (pos);
    }


    //==============================================================================
    void parseHttpHeaders (StringPairArray& responseHeaders)
    {
        StringArray headerLines = StringArray::fromLines (curlHeaders);

        // ignore the first line as this is the status line
        for (int i = 1; i < headerLines.size(); ++i)
        {
            const String& headersEntry = headerLines[i];

            if (headersEntry.isNotEmpty())
            {
                const String key (headersEntry.upToFirstOccurrenceOf (": ", false, false));
                const String value (headersEntry.fromFirstOccurrenceOf (": ", false, false));
                const String previousValue (responseHeaders [key]);
                responseHeaders.set (key, previousValue.isEmpty() ? value : (previousValue + "," + value));
            }
        }
    }


    //==============================================================================
    // CURL callbacks
    size_t curlWriteCallback (char* ptr, size_t size, size_t nmemb)
    {
        if (curl == nullptr || lastError != CURLE_OK)
            return 0;

        const size_t len = size * nmemb;

        // skip bytes if necessary
        size_t max = jmin (skipBytes, len);
        skipBytes -= max;

        if (len > max)
            curlBuffer.append (ptr + max, len - max);

        return len;
    }

    size_t curlReadCallback (char* ptr, size_t size, size_t nmemb)
    {
        if (curl == nullptr || postBuffer == nullptr || lastError != CURLE_OK)
            return 0;

        const size_t len = size * nmemb;

        size_t max = jmin (postBuffer->getSize() - postPosition, len);
        memcpy (ptr, (char*)postBuffer->getData() + postPosition, max);
        postPosition += max;

        return max;
    }

    size_t curlHeaderCallback (char* ptr, size_t size, size_t nmemb)
    {
        if (curl == nullptr || lastError != CURLE_OK)
            return 0;

        size_t len = size * nmemb;

        curlHeaders += String (ptr, len);
        return len;
    }

    //==============================================================================
    // Static method wrappers
    static size_t StaticCurlWrite (char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        WebInputStream* wi = reinterpret_cast<WebInputStream*> (userdata);
        return wi->curlWriteCallback (ptr, size, nmemb);
    }

    static size_t StaticCurlRead (char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        WebInputStream* wi = reinterpret_cast<WebInputStream*> (userdata);
        return wi->curlReadCallback (ptr, size, nmemb);
    }

    static size_t StaticCurlHeader (char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        WebInputStream* wi = reinterpret_cast<WebInputStream*> (userdata);
        return wi->curlHeaderCallback (ptr, size, nmemb);
    }

private:
    CURLM* multi;
    CURL* curl;
    struct curl_slist* headerList;
    int lastError;

    //==============================================================================
    // internal buffers and buffer positions
    int64 contentLength, streamPos;
    MemoryBlock curlBuffer;
    String curlHeaders;
    bool finished;
    size_t skipBytes;

    //==============================================================================
    // Http POST variables
    const MemoryBlock* postBuffer;
    size_t postPosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebInputStream)
};
