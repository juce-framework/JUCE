/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

void MACAddress::findAllAddresses (Array<MACAddress>& result)
{
    ifaddrs* addrs = nullptr;

    if (getifaddrs (&addrs) == 0)
    {
        for (const ifaddrs* cursor = addrs; cursor != nullptr; cursor = cursor->ifa_next)
        {
            // Required to avoid misaligned pointer access
            sockaddr sto;
            std::memcpy (&sto, cursor->ifa_addr, sizeof (sockaddr));

            if (sto.sa_family == AF_LINK)
            {
                auto sadd = reinterpret_cast<const sockaddr_dl*> (cursor->ifa_addr);

               #ifndef IFT_ETHER
                enum { IFT_ETHER = 6 };
               #endif

                if (sadd->sdl_type == IFT_ETHER)
                {
                    MACAddress ma (((const uint8*) sadd->sdl_data) + sadd->sdl_nlen);

                    if (! ma.isNull())
                        result.addIfNotAlreadyThere (ma);
                }
            }
        }

        freeifaddrs (addrs);
    }
}

//==============================================================================
bool JUCE_CALLTYPE Process::openEmailWithAttachments ([[maybe_unused]] const String& targetEmailAddress,
                                                      [[maybe_unused]] const String& emailSubject,
                                                      [[maybe_unused]] const String& bodyText,
                                                      [[maybe_unused]] const StringArray& filesToAttach)
{
  #if JUCE_IOS
    //xxx probably need to use MFMailComposeViewController
    jassertfalse;
    return false;
  #else
    JUCE_AUTORELEASEPOOL
    {
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
    }
  #endif
}

//==============================================================================
class URLConnectionStateBase  : public Thread
{
public:
    explicit URLConnectionStateBase (NSURLRequest* req, int maxRedirects)
        : Thread ("http connection"),
          request ([req retain]),
          data ([[NSMutableData data] retain]),
          numRedirectsToFollow (maxRedirects)
    {
    }

    virtual ~URLConnectionStateBase() = default;

    virtual void cancel() = 0;
    virtual bool start (WebInputStream&, WebInputStream::Listener*) = 0;
    virtual int read (char* dest, int numBytes) = 0;

    int64 getContentLength() const noexcept    { return contentLength; }
    NSDictionary* getHeaders() const noexcept  { return headers; }
    int getStatusCode() const noexcept         { return statusCode; }
    NSInteger getErrorCode() const noexcept    { return nsUrlErrorCode; }

protected:
    CriticalSection dataLock, createConnectionLock;
    id delegate = nil;
    NSDictionary* headers = nil;
    NSURLRequest* request = nil;
    NSMutableData* data = nil;
    int64 contentLength = -1;
    int statusCode = 0;
    NSInteger nsUrlErrorCode = 0;

    std::atomic<bool> initialised { false }, hasFailed { false }, hasFinished { false };
    const int numRedirectsToFollow;
    int numRedirects = 0;
    int64 latestTotalBytes = 0;
    bool hasBeenCancelled = false;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (URLConnectionStateBase)
};

#if JUCE_MAC
// This version is only used for backwards-compatibility with older OSX targets,
// so we'll turn off deprecation warnings. This code will be removed at some point
// in the future.
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated")
class URLConnectionStatePreYosemite  : public URLConnectionStateBase
{
public:
    URLConnectionStatePreYosemite (NSURLRequest* req, const int maxRedirects)
        : URLConnectionStateBase (req, maxRedirects)
    {
        static DelegateClass cls;
        delegate = [cls.createInstance() init];
        DelegateClass::setState (delegate, this);
    }

    ~URLConnectionStatePreYosemite() override
    {
        stop();

        [connection release];
        [request release];
        [headers release];
        [delegate release];
        [data release];
    }

    bool start (WebInputStream& inputStream, WebInputStream::Listener* listener) override
    {
        startThread();

        while (isThreadRunning() && ! initialised)
        {
            if (listener != nullptr)
                if (! listener->postDataSendProgress (inputStream, (int) latestTotalBytes, (int) [[request HTTPBody] length]))
                    return false;

            Thread::sleep (1);
        }

        return connection != nil && ! hasFailed;
    }

    void stop()
    {
        {
            const ScopedLock dLock (dataLock);
            const ScopedLock connectionLock (createConnectionLock);

            hasBeenCancelled = true;

            if (connection != nil)
                [connection cancel];
        }

        stopThread (10000);
    }

    void cancel() override
    {
        hasFinished = hasFailed = true;
        stop();
    }

    int read (char* dest, int numBytes) override
    {
        int numDone = 0;

        while (numBytes > 0)
        {
            const ScopedLock sl (dataLock);
            auto available = jmin (numBytes, (int) [data length]);

            if (available > 0)
            {
                [data getBytes: dest length: (NSUInteger) available];
                [data replaceBytesInRange: NSMakeRange (0, (NSUInteger) available) withBytes: nil length: 0];

                numDone += available;
                numBytes -= available;
                dest += available;
            }
            else
            {
                if (hasFailed || hasFinished)
                    break;

                const ScopedUnlock sul (dataLock);
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

        contentLength = [response expectedContentLength];

        [headers release];
        headers = nil;

        if ([response isKindOfClass: [NSHTTPURLResponse class]])
        {
            NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*) response;
            headers = [[httpResponse allHeaderFields] retain];
            statusCode = (int) [httpResponse statusCode];
        }

        initialised = true;
    }

    NSURLRequest* willSendRequest (NSURLRequest* newRequest, NSURLResponse* redirectResponse)
    {
        if (redirectResponse != nullptr)
        {
            if (numRedirects >= numRedirectsToFollow)
                return nil;  // Cancel redirect and allow connection to continue

            ++numRedirects;
        }

        return newRequest;
    }

    void didFailWithError ([[maybe_unused]] NSError* error)
    {
        DBG (nsStringToJuce ([error description]));
        nsUrlErrorCode = [error code];
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

    void didSendBodyData (NSInteger totalBytesWritten, NSInteger /*totalBytesExpected*/)
    {
        latestTotalBytes = static_cast<int> (totalBytesWritten);
    }

    void finishedLoading()
    {
        hasFinished = true;
        initialised = true;
        signalThreadShouldExit();
    }

    void run() override
    {
        {
            const ScopedLock lock (createConnectionLock);

            if (hasBeenCancelled)
                return;

            connection = [[NSURLConnection alloc] initWithRequest: request
                                                         delegate: delegate];
        }

        while (! threadShouldExit())
        {
            JUCE_AUTORELEASEPOOL
            {
                [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
            }
        }
    }

private:
    //==============================================================================
    struct DelegateClass  : public ObjCClass<NSObject>
    {
        DelegateClass()  : ObjCClass<NSObject> ("JUCENetworkDelegate_")
        {
            addIvar<URLConnectionStatePreYosemite*> ("state");

            addMethod (@selector (connection:didReceiveResponse:), didReceiveResponse);
            addMethod (@selector (connection:didFailWithError:),   didFailWithError);
            addMethod (@selector (connection:didReceiveData:),     didReceiveData);
            addMethod (@selector (connection:didSendBodyData:totalBytesWritten:totalBytesExpectedToWrite:),
                                                                   connectionDidSendBodyData);
            addMethod (@selector (connectionDidFinishLoading:),    connectionDidFinishLoading);
            addMethod (@selector (connection:willSendRequest:redirectResponse:), willSendRequest);

            registerClass();
        }

        static void setState (id self, URLConnectionStatePreYosemite* state)  { object_setInstanceVariable (self, "state", state); }
        static URLConnectionStatePreYosemite* getState (id self)              { return getIvar<URLConnectionStatePreYosemite*> (self, "state"); }

    private:
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

        static NSURLRequest* willSendRequest (id self, SEL, NSURLConnection*, NSURLRequest* request, NSURLResponse* response)
        {
            return getState (self)->willSendRequest (request, response);
        }

        static void connectionDidSendBodyData (id self, SEL, NSURLConnection*, NSInteger, NSInteger totalBytesWritten, NSInteger totalBytesExpected)
        {
            getState (self)->didSendBodyData (totalBytesWritten, totalBytesExpected);
        }

        static void connectionDidFinishLoading (id self, SEL, NSURLConnection*)
        {
            getState (self)->finishedLoading();
        }
    };

    NSURLConnection* connection = nil;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (URLConnectionStatePreYosemite)
};
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

//==============================================================================
class API_AVAILABLE (macos (10.9)) URLConnectionState : public URLConnectionStateBase
{
public:
    URLConnectionState (NSURLRequest* req, const int maxRedirects)
        : URLConnectionStateBase (req, maxRedirects)
    {
        static DelegateClass cls;
        delegate = [cls.createInstance() init];
        DelegateClass::setState (delegate, this);
    }

    ~URLConnectionState() override
    {
        signalThreadShouldExit();

        {
            const ScopedLock sl (dataLock);
            isBeingDeleted = true;
            [task cancel];
            DelegateClass::setState (delegate, nullptr);
        }

        stopThread (10000);
        [task release];
        [request release];
        [headers release];

        [session finishTasksAndInvalidate];
        [session release];

        const ScopedLock sl (dataLock);
        [delegate release];
        [data release];
    }

    void cancel() override
    {
        {
            const ScopedLock lock (createConnectionLock);
            hasBeenCancelled = true;
        }

        signalThreadShouldExit();
        stopThread (10000);
    }

    bool start (WebInputStream& inputStream, WebInputStream::Listener* listener) override
    {
        {
            const ScopedLock lock (createConnectionLock);

            if (hasBeenCancelled)
                return false;
        }

        startThread();

        while (isThreadRunning() && ! initialised)
        {
            if (listener != nullptr)
                if (! listener->postDataSendProgress (inputStream, (int) latestTotalBytes, (int) [[request HTTPBody] length]))
                    return false;

            Thread::sleep (1);
        }

        return true;
    }

    int read (char* dest, int numBytes) override
    {
        int numDone = 0;

        while (numBytes > 0)
        {
            const ScopedLock sl (dataLock);
            auto available = jmin (numBytes, (int) [data length]);

            if (available > 0)
            {
                [data getBytes: dest length: (NSUInteger) available];
                [data replaceBytesInRange: NSMakeRange (0, (NSUInteger) available) withBytes: nil length: 0];

                numDone += available;
                numBytes -= available;
                dest += available;
            }
            else
            {
                if (hasFailed || hasFinished)
                    break;

                const ScopedUnlock ul (dataLock);
                Thread::sleep (1);
            }
        }

        return numDone;
    }

    void didReceiveResponse (NSURLResponse* response, id completionHandler)
    {
        {
            const ScopedLock sl (dataLock);
            if (isBeingDeleted)
                return;

            [data setLength: 0];
        }

        contentLength = [response expectedContentLength];

        [headers release];
        headers = nil;

        if ([response isKindOfClass: [NSHTTPURLResponse class]])
        {
            auto httpResponse = (NSHTTPURLResponse*) response;
            headers = [[httpResponse allHeaderFields] retain];
            statusCode = (int) [httpResponse statusCode];
        }

        initialised = true;

        if (completionHandler != nil)
        {
            // Need to wrangle this parameter back into an obj-C block,
            // and call it to allow the transfer to continue..
            void (^callbackBlock)(NSURLSessionResponseDisposition) = completionHandler;
            callbackBlock (NSURLSessionResponseAllow);
        }
    }

    void didComplete (NSError* error)
    {
        const ScopedLock sl (dataLock);

        if (isBeingDeleted)
            return;

       #if JUCE_DEBUG
        if (error != nullptr)
            DBG (nsStringToJuce ([error description]));
       #endif

        hasFailed = (error != nullptr);
        initialised = true;
        signalThreadShouldExit();
    }

    void didReceiveData (NSData* newData)
    {
        const ScopedLock sl (dataLock);

        if (isBeingDeleted)
            return;

        [data appendData: newData];
        initialised = true;
    }

    void didSendBodyData (int64_t totalBytesWritten)
    {
        latestTotalBytes = static_cast<int> (totalBytesWritten);
    }

    void willPerformHTTPRedirection (NSURLRequest* urlRequest, void (^completionHandler)(NSURLRequest *))
    {
        {
            const ScopedLock sl (dataLock);

            if (isBeingDeleted)
                return;
        }

        completionHandler (numRedirects++ < numRedirectsToFollow ? urlRequest : nil);
    }

    void run() override
    {
        jassert (task == nil && session == nil);

        session = [[NSURLSession sessionWithConfiguration: [NSURLSessionConfiguration defaultSessionConfiguration]
                                                 delegate: delegate
                                            delegateQueue: [NSOperationQueue currentQueue]] retain];

        {
            const ScopedLock lock (createConnectionLock);

            if (! hasBeenCancelled)
                task = [session dataTaskWithRequest: request];
        }

        if (task == nil)
            return;

        [task retain];
        [task resume];

        while (! threadShouldExit())
            wait (5);

        hasFinished = true;
        initialised = true;
    }

private:
    //==============================================================================
    struct DelegateClass  : public ObjCClass<NSObject>
    {
        DelegateClass()  : ObjCClass<NSObject> ("JUCE_URLDelegate_")
        {
            addIvar<URLConnectionState*> ("state");

            addMethod (@selector (URLSession:dataTask:didReceiveResponse:completionHandler:),
                                                                            didReceiveResponse);
            addMethod (@selector (URLSession:didBecomeInvalidWithError:),   didBecomeInvalidWithError);
            addMethod (@selector (URLSession:dataTask:didReceiveData:),     didReceiveData);
            addMethod (@selector (URLSession:task:didSendBodyData:totalBytesSent:totalBytesExpectedToSend:),
                                                                            didSendBodyData);
            addMethod (@selector (URLSession:task:willPerformHTTPRedirection:newRequest:completionHandler:),
                                                                            willPerformHTTPRedirection);
            addMethod (@selector (URLSession:task:didCompleteWithError:),   didCompleteWithError);

            registerClass();
        }

        static void setState (id self, URLConnectionState* state)  { object_setInstanceVariable (self, "state", state); }
        static URLConnectionState* getState (id self)              { return getIvar<URLConnectionState*> (self, "state"); }

    private:
        static void didReceiveResponse (id self, SEL, NSURLSession*, NSURLSessionDataTask*, NSURLResponse* response, id completionHandler)
        {
            if (auto state = getState (self))
                state->didReceiveResponse (response, completionHandler);
        }

        static void didBecomeInvalidWithError (id self, SEL, NSURLSession*, NSError* error)
        {
            if (auto state = getState (self))
                state->didComplete (error);
        }

        static void didReceiveData (id self, SEL, NSURLSession*, NSURLSessionDataTask*, NSData* newData)
        {
            if (auto state = getState (self))
                state->didReceiveData (newData);
        }

        static void didSendBodyData (id self, SEL, NSURLSession*, NSURLSessionTask*, int64_t, int64_t totalBytesWritten, int64_t)
        {
            if (auto state = getState (self))
                state->didSendBodyData (totalBytesWritten);
        }

        static void willPerformHTTPRedirection (id self, SEL, NSURLSession*, NSURLSessionTask*, NSHTTPURLResponse*,
                                                NSURLRequest* request, void (^completionHandler)(NSURLRequest *))
        {
            if (auto state = getState (self))
                state->willPerformHTTPRedirection (request, completionHandler);
        }

        static void didCompleteWithError (id self, SEL, NSURLConnection*, NSURLSessionTask*, NSError* error)
        {
            if (auto state = getState (self))
                state->didComplete (error);
        }
    };

    NSURLSession* session = nil;
    NSURLSessionTask* task = nil;
    bool isBeingDeleted = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (URLConnectionState)
};

//==============================================================================
#if JUCE_IOS
struct BackgroundDownloadTask  : public URL::DownloadTask
{
    BackgroundDownloadTask (const URL& urlToUse,
                            const File& targetLocationToUse,
                            const URL::DownloadTaskOptions& options)
         : listener (options.listener),
           uniqueIdentifier (String (urlToUse.toString (true).hashCode64()) + String (Random().nextInt64()))
    {
        targetLocation = targetLocationToUse;
        downloaded = -1;

        static DelegateClass cls;
        delegate = [cls.createInstance() init];
        DelegateClass::setState (delegate, this);

        activeSessions.set (uniqueIdentifier, this);
        auto nsUrl = [NSURL URLWithString: juceStringToNS (urlToUse.toString (true))];

        jassert (nsUrl != nullptr);

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
        NSMutableURLRequest* request = [[NSMutableURLRequest alloc] initWithURL: nsUrl];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        if (options.usePost)
            [request setHTTPMethod: @"POST"];

        StringArray headerLines;
        headerLines.addLines (options.extraHeaders);
        headerLines.removeEmptyStrings (true);

        for (int i = 0; i < headerLines.size(); ++i)
        {
            String key   = headerLines[i].upToFirstOccurrenceOf (":", false, false).trim();
            String value = headerLines[i].fromFirstOccurrenceOf (":", false, false).trim();

            if (key.isNotEmpty() && value.isNotEmpty())
                [request addValue: juceStringToNS (value) forHTTPHeaderField: juceStringToNS (key)];
        }

        auto* configuration = [NSURLSessionConfiguration backgroundSessionConfigurationWithIdentifier: juceStringToNS (uniqueIdentifier)];

        if (options.sharedContainer.isNotEmpty())
            [configuration setSharedContainerIdentifier: juceStringToNS (options.sharedContainer)];

        session = [NSURLSession sessionWithConfiguration: configuration
                                                delegate: delegate
                                           delegateQueue: nullptr];

        if (session != nullptr)
            downloadTask = [session downloadTaskWithRequest:request];

        // Workaround for an Apple bug. See https://github.com/AFNetworking/AFNetworking/issues/2334
        [request HTTPBody];

        [request release];
    }

    ~BackgroundDownloadTask()
    {
        activeSessions.remove (uniqueIdentifier);

        if (httpCode != -1)
            httpCode = 500;

        finished = true;
        connectionEvent.signal();

        [session invalidateAndCancel];
        while (! hasBeenDestroyed)
            destroyEvent.wait();

        [delegate release];
    }

    bool initOK()
    {
        return (downloadTask != nullptr);
    }

    bool connect()
    {
        [downloadTask resume];
        while (downloaded == -1 && finished == false)
            connectionEvent.wait();

        connectFinished = true;
        return ! error;
    }

    //==============================================================================
    URL::DownloadTask::Listener* listener;
    NSObject<NSURLSessionDelegate>* delegate = nil;
    NSURLSession* session = nil;
    NSURLSessionDownloadTask* downloadTask = nil;
    bool connectFinished = false, hasBeenDestroyed = false;
    Atomic<int> calledComplete;
    WaitableEvent connectionEvent, destroyEvent;
    String uniqueIdentifier;

    static HashMap<String, BackgroundDownloadTask*, DefaultHashFunctions, CriticalSection> activeSessions;

    void didWriteData (int64 totalBytesWritten, int64 totalBytesExpectedToWrite)
    {
        downloaded = totalBytesWritten;

        if (contentLength == -1)
            contentLength = totalBytesExpectedToWrite;

        if (connectFinished && error == false && finished == false && listener != nullptr)
            listener->progress (this, totalBytesWritten, contentLength);

        connectionEvent.signal();
    }

    void didFinishDownloadingToURL (NSURL* location)
    {
        auto* fileManager = [NSFileManager defaultManager];
        error = ([fileManager moveItemAtURL: location
                                      toURL: createNSURLFromFile (targetLocation)
                                      error: nil] == NO);
        httpCode = 200;
        finished = true;

        connectionEvent.signal();

        if (listener != nullptr && calledComplete.exchange (1) == 0)
        {
            if (contentLength > 0 && downloaded < contentLength)
            {
                downloaded = contentLength;
                listener->progress (this, downloaded, contentLength);
            }

            listener->finished (this, !error);
        }
    }

    static int getHTTPErrorCode (NSError* nsError)
    {
        // see https://developer.apple.com/reference/foundation/nsurlsessiondownloadtask?language=objc
        switch ([nsError code])
        {
            case NSURLErrorUserAuthenticationRequired:  return 401;
            case NSURLErrorNoPermissionsToReadFile:     return 403;
            case NSURLErrorFileDoesNotExist:            return 404;
            default:                                    return 500;
        }
    }

    void didCompleteWithError (NSError* nsError)
    {
        if (calledComplete.exchange (1) == 0)
        {
            httpCode = nsError != nil ? getHTTPErrorCode (nsError) : -1;
            error = true;
            finished = true;

            if (listener != nullptr)
                listener->finished (this, ! error);
        }

        connectionEvent.signal();
    }

    void didBecomeInvalidWithError()
    {
        hasBeenDestroyed = true;
        destroyEvent.signal();
    }

    //==============================================================================
    void notify()
    {
        if (downloadTask == nullptr) return;

        if (NSError* error = [downloadTask error])
        {
            didCompleteWithError (error);
        }
        else
        {
            const int64 contentLength = [downloadTask countOfBytesExpectedToReceive];

            if ([downloadTask state] == NSURLSessionTaskStateCompleted)
                didWriteData (contentLength, contentLength);
            else
                didWriteData ([downloadTask countOfBytesReceived], contentLength);
        }
    }

    static void invokeNotify (const String& identifier)
    {
        ScopedLock lock (activeSessions.getLock());

        if (auto* task = activeSessions[identifier])
            task->notify();
    }

    //==============================================================================
    struct DelegateClass  : public ObjCClass<NSObject<NSURLSessionDelegate>>
    {
        DelegateClass()  : ObjCClass<NSObject<NSURLSessionDelegate>> ("JUCE_URLDelegate_")
        {
            addIvar<BackgroundDownloadTask*> ("state");

            addMethod (@selector (URLSession:downloadTask:didWriteData:totalBytesWritten:totalBytesExpectedToWrite:), didWriteData);
            addMethod (@selector (URLSession:downloadTask:didFinishDownloadingToURL:),  didFinishDownloadingToURL);
            addMethod (@selector (URLSession:task:didCompleteWithError:),               didCompleteWithError);
            addMethod (@selector (URLSession:didBecomeInvalidWithError:),               didBecomeInvalidWithError);

            registerClass();
        }

        static void setState (id self, BackgroundDownloadTask* state)  { object_setInstanceVariable (self, "state", state); }
        static BackgroundDownloadTask* getState (id self)              { return getIvar<BackgroundDownloadTask*> (self, "state"); }

    private:
        static void didWriteData (id self, SEL, NSURLSession*, NSURLSessionDownloadTask*, int64_t, int64_t totalBytesWritten, int64_t totalBytesExpectedToWrite)
        {
            if (auto state = getState (self))
                state->didWriteData (totalBytesWritten, totalBytesExpectedToWrite);
        }

        static void didFinishDownloadingToURL (id self, SEL, NSURLSession*, NSURLSessionDownloadTask*, NSURL* location)
        {
            if (auto state = getState (self))
                state->didFinishDownloadingToURL (location);
        }

        static void didCompleteWithError (id self, SEL, NSURLSession*, NSURLSessionTask*, NSError* nsError)
        {
            if (auto state = getState (self))
                state->didCompleteWithError (nsError);
        }

        static void didBecomeInvalidWithError (id self, SEL, NSURLSession*, NSURLSessionTask*, NSError*)
        {
            if (auto state = getState (self))
                state->didBecomeInvalidWithError();
        }
    };
};

HashMap<String, BackgroundDownloadTask*, DefaultHashFunctions, CriticalSection> BackgroundDownloadTask::activeSessions;

std::unique_ptr<URL::DownloadTask> URL::downloadToFile (const File& targetLocation, const DownloadTaskOptions& options)
{
    auto downloadTask = std::make_unique<BackgroundDownloadTask> (*this, targetLocation, options);

    if (downloadTask->initOK() && downloadTask->connect())
        return downloadTask;

    return nullptr;
}

void URL::DownloadTask::juce_iosURLSessionNotify (const String& identifier)
{
    BackgroundDownloadTask::invokeNotify (identifier);
}
#else
std::unique_ptr<URL::DownloadTask> URL::downloadToFile (const File& targetLocation, const DownloadTaskOptions& options)
{
    return URL::DownloadTask::createFallbackDownloader (*this, targetLocation, options);
}
#endif

//==============================================================================
class WebInputStream::Pimpl
{
public:
    Pimpl (WebInputStream& pimplOwner, const URL& urlToUse, bool addParametersToBody)
      : owner (pimplOwner),
        url (urlToUse),
        addParametersToRequestBody (addParametersToBody),
        hasBodyDataToSend (addParametersToRequestBody || url.hasBodyDataToSend()),
        httpRequestCmd (hasBodyDataToSend ? "POST" : "GET")
    {
    }

    ~Pimpl()
    {
        connection.reset();
    }

    bool connect (WebInputStream::Listener* webInputListener, [[maybe_unused]] int numRetries = 0)
    {
        {
            const ScopedLock lock (createConnectionLock);

            if (hasBeenCancelled)
                return false;

            createConnection();
        }

        if (connection == nullptr)
            return false;

        if (! connection->start (owner, webInputListener))
        {
            const auto errorCode = connection->getErrorCode();
            connection.reset();

            if (@available (macOS 10.10, *))
                return false;

            // Workaround for macOS versions below 10.10 where HTTPS POST requests with keep-alive
            // fail with the NSURLErrorNetworkConnectionLost error code.
            if (numRetries == 0 && errorCode == NSURLErrorNetworkConnectionLost)
                return connect (webInputListener, ++numRetries);

            return false;
        }

        if (auto* connectionHeaders = connection->getHeaders())
        {
            statusCode = connection->getStatusCode();

            NSEnumerator* enumerator = [connectionHeaders keyEnumerator];

            while (NSString* key = [enumerator nextObject])
                responseHeaders.set (nsStringToJuce (key),
                                     nsStringToJuce ((NSString*) [connectionHeaders objectForKey: key]));

            return true;
        }

        return false;
    }

    void cancel()
    {
        {
            const ScopedLock lock (createConnectionLock);

            if (connection != nullptr)
                connection->cancel();

            hasBeenCancelled = true;
        }
    }

    //==============================================================================
    // WebInputStream methods
    void withExtraHeaders (const String& extraHeaders)
    {
        if (! headers.endsWithChar ('\n') && headers.isNotEmpty())
            headers << "\r\n";

        headers << extraHeaders;

        if (! headers.endsWithChar ('\n') && headers.isNotEmpty())
            headers << "\r\n";
    }

    void withCustomRequestCommand (const String& customRequestCommand)    { httpRequestCmd = customRequestCommand; }
    void withConnectionTimeout (int timeoutInMs)                          { timeOutMs = timeoutInMs; }
    void withNumRedirectsToFollow (int maxRedirectsToFollow)              { numRedirectsToFollow = maxRedirectsToFollow; }
    StringPairArray getRequestHeaders() const                             { return WebInputStream::parseHttpHeaders (headers); }
    StringPairArray getResponseHeaders() const                            { return responseHeaders; }
    int getStatusCode() const                                             { return statusCode; }

    //==============================================================================
    bool isError() const                { return (connection == nullptr || connection->getHeaders() == nullptr); }
    int64 getTotalLength()              { return connection == nullptr ? -1 : connection->getContentLength(); }
    bool isExhausted()                  { return finished; }
    int64 getPosition()                 { return position; }

    int read (void* buffer, int bytesToRead)
    {
        jassert (buffer != nullptr && bytesToRead >= 0);

        if (finished || isError())
            return 0;

        JUCE_AUTORELEASEPOOL
        {
            const int bytesRead = connection->read (static_cast<char*> (buffer), bytesToRead);
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
                return false;

            auto numBytesToSkip = wantedPos - position;
            auto skipBufferSize = (int) jmin (numBytesToSkip, (int64) 16384);
            HeapBlock<char> temp (skipBufferSize);

            while (numBytesToSkip > 0 && ! isExhausted())
                numBytesToSkip -= read (temp, (int) jmin (numBytesToSkip, (int64) skipBufferSize));
        }

        return true;
    }

    int statusCode = 0;

private:
    WebInputStream& owner;
    URL url;
    std::unique_ptr<URLConnectionStateBase> connection;
    String headers;
    MemoryBlock postData;
    int64 position = 0;
    bool finished = false;
    const bool addParametersToRequestBody, hasBodyDataToSend;
    int timeOutMs = 0;
    int numRedirectsToFollow = 5;
    String httpRequestCmd;
    StringPairArray responseHeaders;
    CriticalSection createConnectionLock;
    bool hasBeenCancelled = false;

    void createConnection()
    {
        jassert (connection == nullptr);

        if (NSURL* nsURL = [NSURL URLWithString: juceStringToNS (url.toString (! addParametersToRequestBody))])
        {
            const auto timeOutSeconds = [this]
            {
                if (timeOutMs > 0)
                    return timeOutMs / 1000.0;

                return timeOutMs < 0 ? std::numeric_limits<double>::infinity() : 60.0;
            }();

            if (NSMutableURLRequest* req = [NSMutableURLRequest requestWithURL: nsURL
                                                                   cachePolicy: NSURLRequestReloadIgnoringLocalCacheData
                                                               timeoutInterval: timeOutSeconds])
            {
                if (NSString* httpMethod = [NSString stringWithUTF8String: httpRequestCmd.toRawUTF8()])
                {
                    [req setHTTPMethod: httpMethod];

                    if (hasBodyDataToSend)
                    {
                        WebInputStream::createHeadersAndPostData (url,
                                                                  headers,
                                                                  postData,
                                                                  addParametersToRequestBody);

                        if (! postData.isEmpty())
                            [req setHTTPBody: [NSData dataWithBytes: postData.getData()
                                                             length: postData.getSize()]];
                    }

                    StringArray headerLines;
                    headerLines.addLines (headers);
                    headerLines.removeEmptyStrings (true);

                    for (int i = 0; i < headerLines.size(); ++i)
                    {
                        auto key   = headerLines[i].upToFirstOccurrenceOf (":", false, false).trim();
                        auto value = headerLines[i].fromFirstOccurrenceOf (":", false, false).trim();

                        if (key.isNotEmpty() && value.isNotEmpty())
                            [req addValue: juceStringToNS (value) forHTTPHeaderField: juceStringToNS (key)];
                    }

                    // Workaround for an Apple bug. See https://github.com/AFNetworking/AFNetworking/issues/2334
                    [req HTTPBody];

                    if (@available (macOS 10.10, *))
                        connection = std::make_unique<URLConnectionState> (req, numRedirectsToFollow);
                   #if JUCE_MAC
                    else
                        connection = std::make_unique<URLConnectionStatePreYosemite> (req, numRedirectsToFollow);
                   #endif
                }
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

} // namespace juce
