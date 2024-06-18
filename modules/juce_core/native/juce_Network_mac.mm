/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
struct SessionListener
{
    virtual ~SessionListener() = default;
    virtual void didComplete (NSError*) = 0;
    virtual void didReceiveResponse (NSURLResponse*, void (^) (NSURLSessionResponseDisposition)) = 0;
    virtual void didReceiveData (NSData*) = 0;
    virtual void didSendBodyData (int64_t) = 0;
    virtual void willPerformHTTPRedirection (NSURLRequest*, void (^) (NSURLRequest *)) = 0;
};

class SharedSession
{
public:
    SharedSession()
    {
        DelegateClass::setState (delegate.get(), this);
    }

    ~SharedSession()
    {
        std::unique_lock lock { mutex };
        [session.get() finishTasksAndInvalidate];
        condvar.wait (lock, [&] { return state == State::stopped; });
    }

    NSUniquePtr<NSURLSessionTask> addTask (NSURLRequest* request, SessionListener* listener)
    {
        std::unique_lock lock { mutex };

        if (state != State::running)
            return nullptr;

        NSUniquePtr<NSURLSessionTask> task { [[session.get() dataTaskWithRequest: request] retain] };
        listenerForTask[[task.get() taskIdentifier]] = listener;
        return task;
    }

    void removeTask (NSURLSessionTask* task)
    {
        std::unique_lock lock { mutex };
        condvar.wait (lock, [&] { return listenerForTask.find ([task taskIdentifier]) == listenerForTask.end(); });
    }

private:
    void didBecomeInvalid ([[maybe_unused]] NSError* error)
    {
       #if JUCE_DEBUG
        if (error != nullptr)
            DBG (nsStringToJuce ([error description]));
       #endif

        const auto toNotify = [&]
        {
            const std::scoped_lock lock { mutex };
            state = State::stopRequested;
            // Take a copy of listenerForTask so that we don't need to hold the lock while
            // iterating through the remaining listeners.
            return listenerForTask;
        }();

        for (const auto& pair : toNotify)
            pair.second->didComplete (error);

        const std::scoped_lock lock { mutex };
        listenerForTask.clear();
        state = State::stopped;

        // Important: we keep the lock held while calling condvar.notify_one().
        // If we don't, then it's possible that when the destructor runs, it will wake
        // before we notify the condvar on this thread, allowing the destructor to continue
        // and destroying the condition variable. When didBecomeInvalid resumes, the condition
        // variable will have been destroyed.
        condvar.notify_one();
    }

    void didComplete (NSURLSessionTask* task, [[maybe_unused]] NSError* error)
    {
       #if JUCE_DEBUG
        if (error != nullptr)
            DBG (nsStringToJuce ([error description]));
       #endif

        auto* listener = getListener (task);

        if (listener == nullptr)
            return;

        listener->didComplete (error);

        {
            const std::scoped_lock lock { mutex };
            listenerForTask.erase ([task taskIdentifier]);
        }

        condvar.notify_one();
    }

    void didReceiveResponse (NSURLSessionTask* task,
                             NSURLResponse* response,
                             void (^completionHandler) (NSURLSessionResponseDisposition))
    {
        if (auto* listener = getListener (task))
            listener->didReceiveResponse (response, completionHandler);
    }

    void didReceiveData (NSURLSessionTask* task, NSData* newData)
    {
        if (auto* listener = getListener (task))
            listener->didReceiveData (newData);
    }

    void didSendBodyData (NSURLSessionTask* task, int64_t totalBytesWritten)
    {
        if (auto* listener = getListener (task))
            listener->didSendBodyData (totalBytesWritten);
    }

    void willPerformHTTPRedirection (NSURLSessionTask* task,
                                     NSURLRequest* urlRequest,
                                     void (^completionHandler) (NSURLRequest *))
    {
        if (auto* listener = getListener (task))
            listener->willPerformHTTPRedirection (urlRequest, completionHandler);
    }

    SessionListener* getListener (NSURLSessionTask* t)
    {
        const std::scoped_lock lock { mutex };
        const auto iter = listenerForTask.find ([t taskIdentifier]);
        return iter != listenerForTask.end() ? iter->second : nullptr;
    }

    struct DelegateClass final : public ObjCClass<NSObject<NSURLSessionTaskDelegate>>
    {
        DelegateClass()
            : ObjCClass ("JUCE_URLDelegate_")
        {
            addIvar<SharedSession*> ("state");

            addMethod (@selector (URLSession:didBecomeInvalidWithError:),
                       [] (id self, SEL, NSURLSession*, NSError* error)
                       {
                           getState (self)->didBecomeInvalid (error);
                       });

            addMethod (@selector (URLSession:dataTask:didReceiveResponse:completionHandler:),
                       [] (id self,
                           SEL,
                           NSURLSession*,
                           NSURLSessionDataTask* task,
                           NSURLResponse* response,
                           void (^completionHandler) (NSURLSessionResponseDisposition))
                       {
                           getState (self)->didReceiveResponse (task, response, completionHandler);
                       });

            addMethod (@selector (URLSession:dataTask:didReceiveData:),
                       [] (id self, SEL, NSURLSession*, NSURLSessionDataTask* task, NSData* newData)
                       {
                           getState (self)->didReceiveData (task, newData);
                       });

            addMethod (@selector (URLSession:task:didSendBodyData:totalBytesSent:totalBytesExpectedToSend:),
                       [] (id self,
                           SEL,
                           NSURLSession*,
                           NSURLSessionTask* task,
                           int64_t,
                           int64_t totalBytesWritten,
                           int64_t)
                       {
                           getState (self)->didSendBodyData (task, totalBytesWritten);
                       });

            addMethod (@selector (URLSession:task:willPerformHTTPRedirection:newRequest:completionHandler:),
                       [] (id self,
                           SEL,
                           NSURLSession*,
                           NSURLSessionTask* task,
                           NSHTTPURLResponse*,
                           NSURLRequest* req,
                           void (^completionHandler) (NSURLRequest *))
                       {
                           getState (self)->willPerformHTTPRedirection (task, req, completionHandler);
                       });

            addMethod (@selector (URLSession:task:didCompleteWithError:),
                       [] (id self, SEL, NSURLConnection*, NSURLSessionTask* task, NSError* error)
                       {
                           getState (self)->didComplete (task, error);
                       });

            registerClass();
        }

        static void setState (NSObject* self, SharedSession* state)  { object_setInstanceVariable (self, "state", state); }
        static SharedSession* getState (NSObject* self)              { return getIvar<SharedSession*> (self, "state"); }

        static DelegateClass& get()
        {
            static DelegateClass cls;
            return cls;
        }
    };

    enum class State
    {
        running,
        stopRequested,
        stopped,
    };

    std::mutex mutex;
    std::condition_variable condvar;

    NSUniquePtr<NSObject<NSURLSessionTaskDelegate>> delegate { [DelegateClass::get().createInstance() init] };
    NSUniquePtr<NSURLSession> session
    {
        [[NSURLSession sessionWithConfiguration: [NSURLSessionConfiguration defaultSessionConfiguration]
                                       delegate: delegate.get()
                                  delegateQueue: nil] retain]
    };

    std::map<NSUInteger, SessionListener*> listenerForTask;

    State state = State::running;
};

class TaskToken
{
public:
    TaskToken() = default;

    explicit TaskToken (NSURLRequest* request, SessionListener* l)
        : task ([&]
                {
                    SharedResourcePointer<SharedSession> session;
                    return session->addTask (request, l);
                }())
    {
        if (auto* t = task.get())
            [t resume];
    }

    TaskToken (TaskToken&&) noexcept = default;
    TaskToken& operator= (TaskToken&&) noexcept = default;

    ~TaskToken()
    {
        SharedResourcePointer<SharedSession> session;

        if (auto* toRemove = task.get())
            session->removeTask (toRemove);
    }

    void cancel()
    {
        if (auto* toCancel = task.get())
            [toCancel cancel];
    }

private:
    NSUniquePtr<NSURLSessionTask> task;
};

//==============================================================================
class URLConnectionState final : private SessionListener
{
public:
    URLConnectionState (NSUniquePtr<NSURLRequest> req, const int maxRedirects)
        : request (std::move (req)),
          numRedirects (maxRedirects)
    {
    }

    ~URLConnectionState() override
    {
        cancel();
    }

    void cancel()
    {
        const std::scoped_lock lock { mutex };
        token.cancel();
    }

    int64 getContentLength() const noexcept
    {
        const std::scoped_lock lock { mutex };
        return contentLength;
    }

    NSUniquePtr<NSDictionary> getHeaders() const noexcept
    {
        const std::scoped_lock lock { mutex };
        return NSUniquePtr<NSDictionary> { [headers.get() copy] };
    }

    int getStatusCode() const noexcept
    {
        const std::scoped_lock lock { mutex };
        return statusCode;
    }

    bool start (WebInputStream& inputStream, WebInputStream::Listener* listener)
    {
        TaskToken newToken { request.get(), this };

        std::unique_lock lock { mutex };
        token = std::move (newToken);

        while (! condvar.wait_for (lock,
                                   std::chrono::milliseconds { 1 },
                                   [&] { return state != State::beforeStart; }))
        {
            if (listener != nullptr
                && ! listener->postDataSendProgress (inputStream,
                                                     (int) latestTotalBytes,
                                                     (int) [[request.get() HTTPBody] length]))
            {
                return false;
            }
        }

        return true;
    }

    int read (char* dest, int numBytes)
    {
        int numDone = 0;

        while (numBytes > 0)
        {
            std::unique_lock lock { mutex };

            const auto getNumAvailable = [&] { return jmin (numBytes, (int) [data.get() length]); };
            condvar.wait (lock, [&] { return getNumAvailable() > 0 || state == State::requestFinished; });

            const auto available = getNumAvailable();

            if (available <= 0)
                break;

            [data.get() getBytes: dest length: (NSUInteger) available];
            [data.get() replaceBytesInRange: NSMakeRange (0, (NSUInteger) available) withBytes: nil length: 0];

            numDone += available;
            numBytes -= available;
            dest += available;
        }

        return numDone;
    }

private:
    void didComplete (NSError*) override
    {
        {
            const std::scoped_lock lock { mutex };
            state = State::requestFinished;
        }

        condvar.notify_one();
    }

    void didReceiveResponse (NSURLResponse* response, void (^completionHandler) (NSURLSessionResponseDisposition)) override
    {
        {
            const std::scoped_lock lock { mutex };

            contentLength = [response expectedContentLength];

            if ([response isKindOfClass: [NSHTTPURLResponse class]])
            {
                auto httpResponse = (NSHTTPURLResponse*) response;
                headers.reset ([[httpResponse allHeaderFields] retain]);
                statusCode = (int) [httpResponse statusCode];
            }

            if (state == State::beforeStart)
                state = State::started;
        }

        condvar.notify_one();
        completionHandler (NSURLSessionResponseAllow);
    }

    void didReceiveData (NSData* newData) override
    {
        {
            const std::scoped_lock lock { mutex };
            [data.get() appendData: newData];

            if (state == State::beforeStart)
                state = State::started;
        }

        condvar.notify_one();
    }

    void didSendBodyData (int64_t totalBytesWritten) override
    {
        const std::scoped_lock lock { mutex };
        latestTotalBytes = totalBytesWritten;
    }

    void willPerformHTTPRedirection (NSURLRequest* urlRequest, void (^completionHandler) (NSURLRequest *)) override
    {
        // No lock required here because numRedirects is only accessed from the session's work queue
        // after the task has started.
        completionHandler (--numRedirects >= 0 ? urlRequest : nil);
    }

    enum class State
    {
        beforeStart,
        started,
        requestFinished
    };

    mutable std::mutex mutex;
    std::condition_variable condvar;

    NSUniquePtr<NSDictionary> headers;
    NSUniquePtr<NSURLRequest> request;
    NSUniquePtr<NSMutableData> data { [[NSMutableData data] retain] };

    int64 latestTotalBytes = 0;
    int64 contentLength = -1;
    int statusCode = 0;
    int numRedirects = 0;
    State state = State::beforeStart;

    SharedResourcePointer<SharedSession> session;
    TaskToken token;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (URLConnectionState)
};

//==============================================================================
#if JUCE_IOS
struct BackgroundDownloadTask final : public URL::DownloadTask
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
    struct DelegateClass final : public ObjCClass<NSObject<NSURLSessionDelegate>>
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

        if (! connection.has_value())
            return false;

        if (! connection->start (owner, webInputListener))
        {
            connection.reset();
            return false;
        }

        const auto connectionHeaders = connection->getHeaders();

        if (connectionHeaders == nullptr)
            return false;

        statusCode = connection->getStatusCode();

        NSEnumerator* enumerator = [connectionHeaders.get() keyEnumerator];

        while (NSString* key = [enumerator nextObject])
            responseHeaders.set (nsStringToJuce (key),
                                 nsStringToJuce ((NSString*) [connectionHeaders.get() objectForKey: key]));

        return true;
    }

    void cancel()
    {
        const ScopedLock lock (createConnectionLock);

        if (connection.has_value())
            connection->cancel();

        hasBeenCancelled = true;
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
    bool isError() const                { return (! connection.has_value() || connection->getHeaders() == nullptr); }
    int64 getTotalLength()              { return ! connection.has_value() ? -1 : connection->getContentLength(); }
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
    std::optional<URLConnectionState> connection;
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
        jassert (! connection.has_value());

        NSUniquePtr<NSURL> nsURL { [[NSURL URLWithString: juceStringToNS (url.toString (! addParametersToRequestBody))] retain] };

        if (nsURL == nullptr)
            return;

        const auto timeOutSeconds = [this]
        {
            if (timeOutMs > 0)
                return timeOutMs / 1000.0;

            return timeOutMs < 0 ? std::numeric_limits<double>::infinity() : 60.0;
        }();

        NSUniquePtr<NSMutableURLRequest> req { [[NSMutableURLRequest requestWithURL: nsURL.get()
                                                                        cachePolicy: NSURLRequestReloadIgnoringLocalCacheData
                                                                    timeoutInterval: timeOutSeconds] retain] };

        if (req == nullptr)
            return;

        NSUniquePtr<NSString> httpMethod { [[NSString stringWithUTF8String: httpRequestCmd.toRawUTF8()] retain] };

        if (httpMethod == nullptr)
            return;

        [req.get() setHTTPMethod: httpMethod.get()];

        if (hasBodyDataToSend)
        {
            WebInputStream::createHeadersAndPostData (url,
                                                      headers,
                                                      postData,
                                                      addParametersToRequestBody);

            if (! postData.isEmpty())
                [req.get() setHTTPBody: [NSData dataWithBytes: postData.getData()
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
                [req.get() addValue: juceStringToNS (value) forHTTPHeaderField: juceStringToNS (key)];
        }

        // Workaround for an Apple bug. See https://github.com/AFNetworking/AFNetworking/issues/2334
        [req.get() HTTPBody];

        connection.emplace (std::move (req), numRedirectsToFollow);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

} // namespace juce
