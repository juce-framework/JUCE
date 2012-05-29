/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

void MACAddress::findAllAddresses (Array<MACAddress>& result)
{
    ifaddrs* addrs = nullptr;

    if (getifaddrs (&addrs) == 0)
    {
        for (const ifaddrs* cursor = addrs; cursor != nullptr; cursor = cursor->ifa_next)
        {
            sockaddr_storage* sto = (sockaddr_storage*) cursor->ifa_addr;
            if (sto->ss_family == AF_LINK)
            {
                const sockaddr_dl* const sadd = (const sockaddr_dl*) cursor->ifa_addr;

                #ifndef IFT_ETHER
                 #define IFT_ETHER 6
                #endif

                if (sadd->sdl_type == IFT_ETHER)
                    result.addIfNotAlreadyThere (MACAddress (((const uint8*) sadd->sdl_data) + sadd->sdl_nlen));
            }
        }

        freeifaddrs (addrs);
    }
}

//==============================================================================
bool Process::openEmailWithAttachments (const String& targetEmailAddress,
                                        const String& emailSubject,
                                        const String& bodyText,
                                        const StringArray& filesToAttach)
{
  #if JUCE_IOS
    //xxx probably need to use MFMailComposeViewController
    jassertfalse;
    return false;
  #else
    JUCE_AUTORELEASEPOOL

    String script;
    script << "tell application \"Mail\"\r\n"
              "set newMessage to make new outgoing message with properties {subject:\""
           << emailSubject.replace ("\"", "\\\"")
           << "\", content:\""
           << bodyText.replace ("\"", "\\\"")
           << "\" & return & return}\r\n"
              "tell newMessage\r\n"
              "set visible to true\r\n"
              "set sender to \"sdfsdfsdfewf\"\r\n"
              "make new to recipient at end of to recipients with properties {address:\""
           << targetEmailAddress
           << "\"}\r\n";

    for (int i = 0; i < filesToAttach.size(); ++i)
    {
        script << "tell content\r\n"
                  "make new attachment with properties {file name:\""
               << filesToAttach[i].replace ("\"", "\\\"")
               << "\"} at after the last paragraph\r\n"
                  "end tell\r\n";
    }

    script << "end tell\r\n"
              "end tell\r\n";

    NSAppleScript* s = [[NSAppleScript alloc] initWithSource: juceStringToNS (script)];
    NSDictionary* error = nil;
    const bool ok = [s executeAndReturnError: &error] != nil;
    [s release];

    return ok;
  #endif
}

//==============================================================================
class URLConnectionState   : public Thread
{
public:
    URLConnectionState (NSObject* owner_, NSURLRequest* req)
        : Thread ("http connection"),
          contentLength (-1),
          owner (owner_),
          request ([req retain]),
          connection (nil),
          data ([[NSMutableData data] retain]),
          headers (nil),
          initialised (false),
          hasFailed (false),
          hasFinished (false)
    {
    }

    ~URLConnectionState()
    {
        stopThread (10000);
        [connection release];
        [data release];
        [request release];
        [headers release];
    }

    bool start (URL::OpenStreamProgressCallback* callback, void* context)
    {
        startThread();

        while (isThreadRunning() && ! initialised)
        {
            if (callback != nullptr)
                callback (context, -1, (int) [[request HTTPBody] length]);

            Thread::sleep (1);
        }

        return connection != nil && ! hasFailed;
    }

    void stop()
    {
        [connection cancel];
        stopThread (10000);
    }

    int read (char* dest, int numBytes)
    {
        int numDone = 0;

        while (numBytes > 0)
        {
            const int available = jmin (numBytes, (int) [data length]);

            if (available > 0)
            {
                const ScopedLock sl (dataLock);
                [data getBytes: dest length: available];
                [data replaceBytesInRange: NSMakeRange (0, available) withBytes: nil length: 0];

                numDone += available;
                numBytes -= available;
                dest += available;
            }
            else
            {
                if (hasFailed || hasFinished)
                    break;

                Thread::sleep (1);
            }
        }

        return numDone;
    }

    void didReceiveResponse (NSURLResponse* response)
    {
        {
            const ScopedLock sl (dataLock);
            [data setLength: 0];
        }

        initialised = true;
        contentLength = [response expectedContentLength];

        [headers release];
        headers = nil;

        if ([response isKindOfClass: [NSHTTPURLResponse class]])
            headers = [[((NSHTTPURLResponse*) response) allHeaderFields] retain];
    }

    void didFailWithError (NSError* error)
    {
        DBG (nsStringToJuce ([error description])); (void) error;
        hasFailed = true;
        initialised = true;
        signalThreadShouldExit();
    }

    void didReceiveData (NSData* newData)
    {
        const ScopedLock sl (dataLock);
        [data appendData: newData];
        initialised = true;
    }

    void finishedLoading()
    {
        hasFinished = true;
        initialised = true;
        signalThreadShouldExit();
    }

    void run()
    {
        NSUInteger oldRetainCount = [owner retainCount];
        connection = [[NSURLConnection alloc] initWithRequest: request
                                                     delegate: owner];

        if (oldRetainCount == [owner retainCount])
            [owner retain]; // newer SDK should already retain this, but there were problems in older versions..

        if (connection != nil)
        {
            while (! threadShouldExit())
            {
                JUCE_AUTORELEASEPOOL
                [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
            }
        }
    }

    int64 contentLength;
    CriticalSection dataLock;
    NSObject* owner;
    NSURLRequest* request;
    NSURLConnection* connection;
    NSMutableData* data;
    NSDictionary* headers;
    bool initialised, hasFailed, hasFinished;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (URLConnectionState);
};

//==============================================================================
struct URLConnectionDelegateClass  : public ObjCClass<NSObject>
{
    URLConnectionDelegateClass()  : ObjCClass ("JUCEAppDelegate_")
    {
        addIvar <URLConnectionState*> ("state");

        addMethod (@selector (dealloc),                        dealloc,                    "v@:");
        addMethod (@selector (connection:didReceiveResponse:), didReceiveResponse,         "v@:@@");
        addMethod (@selector (connection:didFailWithError:),   didFailWithError,           "v@:@@");
        addMethod (@selector (connection:didReceiveData:),     didReceiveData,             "v@:@@");
        addMethod (@selector (connectionDidFinishLoading:),    connectionDidFinishLoading, "v@:@");

        registerClass();
    }

    static void setState (id self, URLConnectionState* state)
    {
        object_setInstanceVariable (self, "state", state);
    }

    static URLConnectionState* getState (id self)
    {
        return getIvar<URLConnectionState*> (self, "state");
    }

private:
    static void dealloc (id self, SEL sel)
    {
        getState (self)->stop();
        delete getState (self);
        sendSuperclassMessage (self, @selector (dealloc));
    }

    static void didReceiveResponse (id self, SEL, NSURLConnection*, NSURLResponse* response)
    {
        getState (self)->didReceiveResponse (response);
    }

    static void didFailWithError (id self, SEL, NSURLConnection*, NSError* error)
    {
        getState (self)->didFailWithError (error);
    }

    static void didReceiveData (id self, SEL, NSURLConnection*, NSData* newData)
    {
        getState (self)->didReceiveData (newData);
    }

    static void connectionDidFinishLoading (id self, SEL, NSURLConnection*)
    {
        getState (self)->finishedLoading();
    }
};

//==============================================================================
class WebInputStream  : public InputStream
{
public:
    WebInputStream (const String& address_, bool isPost_, const MemoryBlock& postData_,
                    URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                    const String& headers_, int timeOutMs_, StringPairArray* responseHeaders)
      : connection (nil),
        address (address_), headers (headers_), postData (postData_), position (0),
        finished (false), isPost (isPost_), timeOutMs (timeOutMs_)
    {
        JUCE_AUTORELEASEPOOL
        connection = createConnection (progressCallback, progressCallbackContext);

        if (responseHeaders != nullptr && connection != nil)
        {
            URLConnectionState* const state = URLConnectionDelegateClass::getState (connection);

            if (state->headers != nil)
            {
                NSEnumerator* enumerator = [state->headers keyEnumerator];
                NSString* key;

                while ((key = [enumerator nextObject]) != nil)
                    responseHeaders->set (nsStringToJuce (key),
                                          nsStringToJuce ((NSString*) [state->headers objectForKey: key]));
            }
        }
    }

    ~WebInputStream()
    {
        close();
    }

    //==============================================================================
    bool isError() const        { return connection == nil; }
    int64 getTotalLength()      { return connection == nil ? -1 : URLConnectionDelegateClass::getState (connection)->contentLength; }
    bool isExhausted()          { return finished; }
    int64 getPosition()         { return position; }

    int read (void* buffer, int bytesToRead)
    {
        jassert (buffer != nullptr && bytesToRead >= 0);

        if (finished || isError())
        {
            return 0;
        }
        else
        {
            JUCE_AUTORELEASEPOOL

            URLConnectionState* const state = URLConnectionDelegateClass::getState (connection);

            const int bytesRead = state->read (static_cast <char*> (buffer), bytesToRead);
            position += bytesRead;

            if (bytesRead == 0)
                finished = true;

            return bytesRead;
        }
    }

    bool setPosition (int64 wantedPos)
    {
        if (wantedPos != position)
        {
            finished = false;

            if (wantedPos < position)
            {
                close();
                position = 0;
                connection = createConnection (0, 0);
            }

            skipNextBytes (wantedPos - position);
        }

        return true;
    }

    //==============================================================================
private:
    NSObject* connection;
    String address, headers;
    MemoryBlock postData;
    int64 position;
    bool finished;
    const bool isPost;
    const int timeOutMs;

    void close()
    {
        if (connection != nil)
        {
            URLConnectionDelegateClass::getState (connection)->stop();
            [connection release];
            connection = nil;
        }
    }

    NSObject* createConnection (URL::OpenStreamProgressCallback* progressCallback,
                                void* progressCallbackContext)
    {
        NSMutableURLRequest* req = [NSMutableURLRequest  requestWithURL: [NSURL URLWithString: juceStringToNS (address)]
                                                            cachePolicy: NSURLRequestReloadIgnoringLocalCacheData
                                                        timeoutInterval: timeOutMs <= 0 ? 60.0 : (timeOutMs / 1000.0)];

        if (req == nil)
            return nil;

        [req setHTTPMethod: nsStringLiteral (isPost ? "POST" : "GET")];
        //[req setCachePolicy: NSURLRequestReloadIgnoringLocalAndRemoteCacheData];

        StringArray headerLines;
        headerLines.addLines (headers);
        headerLines.removeEmptyStrings (true);

        for (int i = 0; i < headerLines.size(); ++i)
        {
            const String key (headerLines[i].upToFirstOccurrenceOf (":", false, false).trim());
            const String value (headerLines[i].fromFirstOccurrenceOf (":", false, false).trim());

            if (key.isNotEmpty() && value.isNotEmpty())
                [req addValue: juceStringToNS (value) forHTTPHeaderField: juceStringToNS (key)];
        }

        if (isPost && postData.getSize() > 0)
            [req setHTTPBody: [NSData dataWithBytes: postData.getData()
                                             length: postData.getSize()]];

        static URLConnectionDelegateClass cls;
        NSObject* const s = [cls.createInstance() init];

        URLConnectionState* state = new URLConnectionState (s, req);
        URLConnectionDelegateClass::setState (s, state);

        if (state->start (progressCallback, progressCallbackContext))
            return s;

        [s release];
        return nil;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebInputStream);
};

InputStream* URL::createNativeStream (const String& address, bool isPost, const MemoryBlock& postData,
                                      OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                                      const String& headers, const int timeOutMs, StringPairArray* responseHeaders)
{
    ScopedPointer <WebInputStream> wi (new WebInputStream (address, isPost, postData,
                                                           progressCallback, progressCallbackContext,
                                                           headers, timeOutMs, responseHeaders));

    return wi->isError() ? nullptr : wi.release();
}
