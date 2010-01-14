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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
int SystemStats::getMACAddresses (int64* addresses, int maxNum, const bool littleEndian) throw()
{
    #ifndef IFT_ETHER
     #define IFT_ETHER 6
    #endif

    ifaddrs* addrs = 0;
    int numResults = 0;

    if (getifaddrs (&addrs) == 0)
    {
        const ifaddrs* cursor = addrs;

        while (cursor != 0 && numResults < maxNum)
        {
            if (cursor->ifa_addr->sa_family == AF_LINK)
            {
                const sockaddr_dl* const sadd = (const sockaddr_dl*) cursor->ifa_addr;

                if (sadd->sdl_type == IFT_ETHER)
                {
                    const uint8* const addr = ((const uint8*) sadd->sdl_data) + sadd->sdl_nlen;

                    uint64 a = 0;
                    for (int i = 6; --i >= 0;)
                        a = (a << 8) | addr [littleEndian ? i : (5 - i)];

                    *addresses++ = (int64) a;
                    ++numResults;
                }
            }

            cursor = cursor->ifa_next;
        }

        freeifaddrs (addrs);
    }

    return numResults;
}

//==============================================================================
bool PlatformUtilities::launchEmailWithAttachments (const String& targetEmailAddress,
                                                    const String& emailSubject,
                                                    const String& bodyText,
                                                    const StringArray& filesToAttach)
{
#if JUCE_IPHONE
    //xxx probably need to use MFMailComposeViewController
    jassertfalse
    return false;
#else
    const ScopedAutoReleasePool pool;

    String script;
    script << "tell application \"Mail\"\r\n"
              "set newMessage to make new outgoing message with properties {subject:\""
           << emailSubject.replace (T("\""), T("\\\""))
           << "\", content:\""
           << bodyText.replace (T("\""), T("\\\""))
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
               << filesToAttach[i].replace (T("\""), T("\\\""))
               << "\"} at after the last paragraph\r\n"
                  "end tell\r\n";
    }

    script << "end tell\r\n"
              "end tell\r\n";

    NSAppleScript* s = [[NSAppleScript alloc]
                            initWithSource: juceStringToNS (script)];
    NSDictionary* error = 0;
    const bool ok = [s executeAndReturnError: &error] != nil;
    [s release];

    return ok;
#endif
}

//==============================================================================
END_JUCE_NAMESPACE

using namespace JUCE_NAMESPACE;

//==============================================================================
#define JuceURLConnection MakeObjCClassName(JuceURLConnection)

@interface JuceURLConnection  : NSObject
{
@public
    NSURLRequest* request;
    NSURLConnection* connection;
    NSMutableData* data;
    Thread* runLoopThread;
    bool initialised, hasFailed, hasFinished;
    int position;
    int64 contentLength;
    NSLock* dataLock;
}

- (JuceURLConnection*) initWithRequest: (NSURLRequest*) req withCallback: (URL::OpenStreamProgressCallback*) callback withContext: (void*) context;
- (void) dealloc;
- (void) connection: (NSURLConnection*) connection didReceiveResponse: (NSURLResponse*) response;
- (void) connection: (NSURLConnection*) connection didFailWithError: (NSError*) error;
- (void) connection: (NSURLConnection*) connection didReceiveData: (NSData*) data;
- (void) connectionDidFinishLoading: (NSURLConnection*) connection;

- (BOOL) isOpen;
- (int) read: (char*) dest numBytes: (int) num;
- (int) readPosition;
- (void) stop;
- (void) createConnection;

@end

class JuceURLConnectionMessageThread  : public Thread
{
    JuceURLConnection* owner;

public:
    JuceURLConnectionMessageThread (JuceURLConnection* owner_)
        : Thread (T("http connection")),
          owner (owner_)
    {
    }

    ~JuceURLConnectionMessageThread()
    {
        stopThread (10000);
    }

    void run()
    {
        [owner createConnection];

        while (! threadShouldExit())
        {
            const ScopedAutoReleasePool pool;
            [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
        }
    }
};


@implementation JuceURLConnection

- (JuceURLConnection*) initWithRequest: (NSURLRequest*) req
                          withCallback: (URL::OpenStreamProgressCallback*) callback
                           withContext: (void*) context;
{
    [super init];
    request = req;
    [request retain];
    data = [[NSMutableData data] retain];
    dataLock = [[NSLock alloc] init];
    connection = 0;
    initialised = false;
    hasFailed = false;
    hasFinished = false;
    contentLength = -1;

    runLoopThread = new JuceURLConnectionMessageThread (self);
    runLoopThread->startThread();

    while (runLoopThread->isThreadRunning() && ! initialised)
    {
        if (callback != 0)
            callback (context, -1, (int) [[request HTTPBody] length]);

        Thread::sleep (1);
    }

    return self;
}

- (void) dealloc
{
    [self stop];

    delete runLoopThread;
    [connection release];
    [data release];
    [dataLock release];
    [request release];
    [super dealloc];
}

- (void) createConnection
{
    connection = [[NSURLConnection alloc] initWithRequest: request
                                                 delegate: [self retain]];

    if (connection == nil)
        runLoopThread->signalThreadShouldExit();
}

- (void) connection: (NSURLConnection*) connection didReceiveResponse: (NSURLResponse*) response
{
    [dataLock lock];
    [data setLength: 0];
    [dataLock unlock];
    initialised = true;
    contentLength = [response expectedContentLength];
}

- (void) connection: (NSURLConnection*) connection didFailWithError: (NSError*) error
{
    DBG (nsStringToJuce ([error description]));
    hasFailed = true;
    initialised = true;
    runLoopThread->signalThreadShouldExit();
}

- (void) connection: (NSURLConnection*) connection didReceiveData: (NSData*) newData
{
    [dataLock lock];
    [data appendData: newData];
    [dataLock unlock];
    initialised = true;
}

- (void) connectionDidFinishLoading: (NSURLConnection*) connection
{
    hasFinished = true;
    initialised = true;
    runLoopThread->signalThreadShouldExit();
}

- (BOOL) isOpen
{
    return connection != 0 && ! hasFailed;
}

- (int) readPosition
{
    return position;
}

- (int) read: (char*) dest numBytes: (int) numNeeded
{
    int numDone = 0;

    while (numNeeded > 0)
    {
        int available = jmin (numNeeded, (int) [data length]);

        if (available > 0)
        {
            [dataLock lock];
            [data getBytes: dest length: available];
            [data replaceBytesInRange: NSMakeRange (0, available) withBytes: nil length: 0];
            [dataLock unlock];

            numDone += available;
            numNeeded -= available;
            dest += available;
        }
        else
        {
            if (hasFailed || hasFinished)
                break;

            Thread::sleep (1);
        }
    }

    position += numDone;
    return numDone;
}

- (void) stop
{
    [connection cancel];
    runLoopThread->stopThread (10000);
}

@end
BEGIN_JUCE_NAMESPACE


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
    const ScopedAutoReleasePool pool;

    NSMutableURLRequest* req = [NSMutableURLRequest
            requestWithURL: [NSURL URLWithString: juceStringToNS (url)]
               cachePolicy: NSURLRequestUseProtocolCachePolicy
           timeoutInterval: timeOutMs <= 0 ? 60.0 : (timeOutMs / 1000.0)];

    if (req == nil)
        return 0;

    [req setHTTPMethod: isPost ? @"POST" : @"GET"];
    //[req setCachePolicy: NSURLRequestReloadIgnoringLocalAndRemoteCacheData];

    StringArray headerLines;
    headerLines.addLines (headers);
    headerLines.removeEmptyStrings (true);

    for (int i = 0; i < headerLines.size(); ++i)
    {
        const String key (headerLines[i].upToFirstOccurrenceOf (T(":"), false, false).trim());
        const String value (headerLines[i].fromFirstOccurrenceOf (T(":"), false, false).trim());

        if (key.isNotEmpty() && value.isNotEmpty())
            [req addValue: juceStringToNS (value) forHTTPHeaderField: juceStringToNS (key)];
    }

    if (isPost && postData.getSize() > 0)
    {
        [req setHTTPBody: [NSData dataWithBytes: postData.getData()
                                         length: postData.getSize()]];
    }

    JuceURLConnection* const s = [[JuceURLConnection alloc] initWithRequest: req
                                                               withCallback: callback
                                                                withContext: callbackContext];

    if ([s isOpen])
        return s;

    [s release];
    return 0;
}

void juce_closeInternetFile (void* handle)
{
    JuceURLConnection* const s = (JuceURLConnection*) handle;

    if (s != 0)
    {
        const ScopedAutoReleasePool pool;
        [s stop];
        [s release];
    }
}

int juce_readFromInternetFile (void* handle, void* buffer, int bytesToRead)
{
    JuceURLConnection* const s = (JuceURLConnection*) handle;

    if (s != 0)
    {
        const ScopedAutoReleasePool pool;
        return [s read: (char*) buffer numBytes: bytesToRead];
    }

    return 0;
}

int64 juce_getInternetFileContentLength (void* handle)
{
    JuceURLConnection* const s = (JuceURLConnection*) handle;

    if (s != 0)
        return s->contentLength;

    return -1;
}

int juce_seekInInternetFile (void* handle, int newPosition)
{
    JuceURLConnection* const s = (JuceURLConnection*) handle;

    if (s != 0)
        return [s readPosition];

    return 0;
}

#endif
