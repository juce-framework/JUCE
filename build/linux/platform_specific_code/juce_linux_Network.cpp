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

#include "linuxincludes.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <linux/if.h>

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/text/juce_StringArray.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/io/network/juce_URL.h"

// we'll borrow the mac's socket-based http streaming code..
#include "../../macosx/platform_specific_code/juce_mac_HTTPStream.h"


//==============================================================================
int SystemStats::getMACAddresses (int64* addresses, int maxNum, const bool littleEndian) throw()
{
    int numResults = 0;

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
                 && ioctl (s, SIOCGIFHWADDR, &ifr) == 0
                 && numResults < maxNum)
            {
                int64 a = 0;
                for (int j = 6; --j >= 0;)
                    a = (a << 8) | ifr.ifr_hwaddr.sa_data[j];

                *addresses++ = a;
                ++numResults;
            }
        }

        close (s);
    }

    return numResults;
}


bool PlatformUtilities::launchEmailWithAttachments (const String& targetEmailAddress,
                                                    const String& emailSubject,
                                                    const String& bodyText,
                                                    const StringArray& filesToAttach)
{
    jassertfalse    // xxx todo

    return false;
}


END_JUCE_NAMESPACE
