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

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#import <Cocoa/Cocoa.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/network/IOEthernetInterface.h>
#import <IOKit/network/IONetworkInterface.h>
#import <IOKit/network/IOEthernetController.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/text/juce_String.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/threads/juce_ScopedLock.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"
#include "../../../src/juce_core/text/juce_StringArray.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/io/network/juce_URL.h"

#include "juce_mac_HTTPStream.h"

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
class AutoPool
{
public:
    AutoPool()      { pool = [[NSAutoreleasePool alloc] init]; }
    ~AutoPool()     { [pool release]; }

private:
    NSAutoreleasePool* pool;
};

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


END_JUCE_NAMESPACE
