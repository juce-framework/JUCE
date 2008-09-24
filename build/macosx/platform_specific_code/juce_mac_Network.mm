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

#include "juce_mac_NativeHeaders.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/text/juce_String.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/basics/juce_Logger.h"
#include "../../../src/juce_core/threads/juce_ScopedLock.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"
#include "../../../src/juce_core/text/juce_StringArray.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/io/network/juce_URL.h"

//#include "juce_mac_HTTPStream.h"

//==============================================================================
static bool GetEthernetIterator (io_iterator_t* matchingServices) throw()
{
    mach_port_t masterPort;

    if (IOMasterPort (MACH_PORT_NULL, &masterPort) == KERN_SUCCESS)
    {
        CFMutableDictionaryRef dict = IOServiceMatching (kIOEthernetInterfaceClass);

        if (dict != 0)
        {
            CFMutableDictionaryRef propDict = CFDictionaryCreateMutable (kCFAllocatorDefault,
                                                                         0,
                                                                         &kCFTypeDictionaryKeyCallBacks,
                                                                         &kCFTypeDictionaryValueCallBacks);

            if (propDict != 0)
            {
                CFDictionarySetValue (propDict, CFSTR (kIOPrimaryInterface), kCFBooleanTrue);

                CFDictionarySetValue (dict, CFSTR (kIOPropertyMatchKey), propDict);
                CFRelease (propDict);
            }
        }

        return IOServiceGetMatchingServices (masterPort, dict, matchingServices) == KERN_SUCCESS;
    }

    return false;
}

int SystemStats::getMACAddresses (int64* addresses, int maxNum, const bool littleEndian) throw()
{
    int numResults = 0;
    io_iterator_t it;

    if (GetEthernetIterator (&it))
    {
        io_object_t i;

        while ((i = IOIteratorNext (it)) != 0)
        {
            io_object_t controller;

            if (IORegistryEntryGetParentEntry (i, kIOServicePlane, &controller) == KERN_SUCCESS)
            {
                CFTypeRef data = IORegistryEntryCreateCFProperty (controller,
                                                                  CFSTR (kIOMACAddress),
                                                                  kCFAllocatorDefault,
                                                                  0);
                if (data != 0)
                {
                    UInt8 addr [kIOEthernetAddressSize];
                    zeromem (addr, sizeof (addr));

                    CFDataGetBytes ((CFDataRef) data, CFRangeMake (0, sizeof (addr)), addr);
                    CFRelease (data);

                    int64 a = 0;
                    for (int i = 6; --i >= 0;)
                        a = (a << 8) | addr[i];

                    if (! littleEndian)
                        a = (int64) swapByteOrder ((uint64) a);

                    if (numResults < maxNum)
                    {
                        *addresses++ = a;
                        ++numResults;
                    }
                }

                IOObjectRelease (controller);
            }

            IOObjectRelease (i);
        }

        IOObjectRelease (it);
    }

    return numResults;
}

//==============================================================================
bool PlatformUtilities::launchEmailWithAttachments (const String& targetEmailAddress,
                                                    const String& emailSubject,
                                                    const String& bodyText,
                                                    const StringArray& filesToAttach)
{
    const AutoPool pool;

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
                            initWithSource: [NSString stringWithUTF8String: (const char*) script.toUTF8()]];
    NSDictionary* error = 0;
    const bool ok = [s executeAndReturnError: &error] != nil;
    [s release];

    return ok;
}

//==============================================================================
END_JUCE_NAMESPACE

using namespace JUCE_NAMESPACE;

//==============================================================================
@interface JuceURLConnection  : NSObject
{
    NSURLRequest* request;
    NSURLConnection* connection;
    NSMutableData* data;
    Thread* runLoopThread;
    bool initialised, hasFailed, hasFinished;
    int position;
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
        startThread();
    }

    ~JuceURLConnectionMessageThread()
    {
        stopThread (5000);
    }

    void run()
    {
        AutoPool pool;
        [owner createConnection];

        while (! threadShouldExit())
        {
            AutoPool pool;
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

    runLoopThread = new JuceURLConnectionMessageThread (self);

    while (runLoopThread->isThreadRunning() && ! initialised)
    {
        if (callback != 0)
            callback (context, -1, [[request HTTPBody] length]);

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
                                                 delegate: self];

    if (connection == nil)
        runLoopThread->signalThreadShouldExit();
}

- (void) connection: (NSURLConnection*) connection didReceiveResponse: (NSURLResponse*) response
{
    [dataLock lock];
    [data setLength: 0];
    [dataLock unlock];
    initialised = true;
}

- (void) connection: (NSURLConnection*) connection didFailWithError: (NSError*) error
{
    NSLog ([error description]);
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
        int available = jmin (numNeeded, [data length]);

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
    runLoopThread->stopThread (5000);
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
    AutoPool pool;

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
        AutoPool pool;
        [s stop];
        [s release];
    }
}

int juce_readFromInternetFile (void* handle, void* buffer, int bytesToRead)
{
    JuceURLConnection* const s = (JuceURLConnection*) handle;
    
    if (s != 0)
    {
        AutoPool pool;
        return [s read: (char*) buffer numBytes: bytesToRead];
    }

    return 0;
}

int juce_seekInInternetFile (void* handle, int newPosition)
{
    JuceURLConnection* const s = (JuceURLConnection*) handle;
    
    if (s != 0)
        return [s readPosition];

    return 0;
}


END_JUCE_NAMESPACE
