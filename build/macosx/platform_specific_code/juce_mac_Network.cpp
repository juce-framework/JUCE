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
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <Carbon/Carbon.h>

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
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"
#include "../../../src/juce_core/text/juce_StringArray.h"
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

int SystemStats::getMACAddresses (int64* addresses, int maxNum) throw()
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

END_JUCE_NAMESPACE
