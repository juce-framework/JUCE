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
#include <Carbon/Carbon.h>
#include <CoreAudio/HostTime.h>
#include <ctime>
#include <sys/resource.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/application/juce_Application.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"

static int64 highResTimerFrequency;

#if JUCE_INTEL

static void juce_getCpuVendor (char* const v) throw()
{
    int vendor[4];
    zerostruct (vendor);
    int dummy = 0;

    asm ("mov %%ebx, %%esi \n\t"
         "cpuid \n\t"
         "xchg %%esi, %%ebx"
           : "=a" (dummy), "=S" (vendor[0]), "=c" (vendor[2]), "=d" (vendor[1]) : "a" (0));

    memcpy (v, vendor, 16);
}

static unsigned int getCPUIDWord (unsigned int& familyModel, unsigned int& extFeatures) throw()
{
    unsigned int cpu = 0;
    unsigned int ext = 0;
    unsigned int family = 0;
    unsigned int dummy = 0;

    asm ("mov %%ebx, %%esi \n\t"
         "cpuid \n\t"
         "xchg %%esi, %%ebx"
           : "=a" (family), "=S" (ext), "=c" (dummy), "=d" (cpu) : "a" (1));

    familyModel = family;
    extFeatures = ext;
    return cpu;
}

struct CPUFlags
{
    bool hasMMX : 1;
    bool hasSSE : 1;
    bool hasSSE2 : 1;
    bool has3DNow : 1;
};

static CPUFlags cpuFlags;

#endif

//==============================================================================
void Logger::outputDebugString (const String& text) throw()
{
    String withLineFeed (text + T("\n"));
    const char* const utf8 = withLineFeed.toUTF8();
    fwrite (utf8, strlen (utf8), 1, stdout);
}

void Logger::outputDebugPrintf (const tchar* format, ...) throw()
{
    String text;
    va_list args;
    va_start (args, format);
    text.vprintf(format, args);
    outputDebugString (text);
}

int SystemStats::getMemorySizeInMegabytes() throw()
{
    long bytes;
    if (Gestalt (gestaltPhysicalRAMSize, &bytes) == noErr)
        return (int) (((unsigned long) bytes) / (1024 * 1024));

    return 0;
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType() throw()
{
    return MacOSX;
}

const String SystemStats::getOperatingSystemName() throw()
{
    return T("Mac OS X");
}

//==============================================================================
void SystemStats::initialiseStats() throw()
{
    static bool initialised = false;

    if (! initialised)
    {
        initialised = true;

#if JUCE_INTEL
        {
            unsigned int familyModel, extFeatures;
            const unsigned int features = getCPUIDWord (familyModel, extFeatures);

            cpuFlags.hasMMX = ((features & (1 << 23)) != 0);
            cpuFlags.hasSSE = ((features & (1 << 25)) != 0);
            cpuFlags.hasSSE2 = ((features & (1 << 26)) != 0);
            cpuFlags.has3DNow = ((extFeatures & (1 << 31)) != 0);
        }
#endif

        highResTimerFrequency = (int64) AudioGetHostClockFrequency();

        if (JUCEApplication::getInstance() != 0)
            RegisterAppearanceClient();

        TXNInitTextension (0, 0, kTXNWantMoviesMask | kTXNWantGraphicsMask);

        String s (SystemStats::getJUCEVersion());

        rlimit lim;
        getrlimit (RLIMIT_NOFILE, &lim);
        lim.rlim_cur = lim.rlim_max = RLIM_INFINITY;
        setrlimit (RLIMIT_NOFILE, &lim);
    }
}

bool SystemStats::hasMMX() throw()
{
#if JUCE_INTEL
    return cpuFlags.hasMMX;
#else
    return false;
#endif
}

bool SystemStats::hasSSE() throw()
{
#if JUCE_INTEL
    return cpuFlags.hasSSE;
#else
    return false;
#endif
}

bool SystemStats::hasSSE2() throw()
{
#if JUCE_INTEL
    return cpuFlags.hasSSE2;
#else
    return false;
#endif
}

bool SystemStats::has3DNow() throw()
{
#if JUCE_INTEL
    return cpuFlags.has3DNow;
#else
    return false;
#endif
}

const String SystemStats::getCpuVendor() throw()
{
#if JUCE_INTEL
    char v [16];
    juce_getCpuVendor (v);
    return String (v, 16);
#else
    return String::empty;
#endif
}

int SystemStats::getCpuSpeedInMegaherz() throw()
{
    return GetCPUSpeed();
}

int SystemStats::getNumCpus() throw()
{
    return MPProcessors();
}

//==============================================================================
static int64 juce_getMicroseconds() throw()
{
    UnsignedWide t;
    Microseconds (&t);
    return (((int64) t.hi) << 32) | t.lo;
}

uint32 juce_millisecondsSinceStartup() throw()
{
    return (uint32) (juce_getMicroseconds() / 1000);
}

double Time::getMillisecondCounterHiRes() throw()
{
    // xxx might be more accurate to use a scaled AudioGetCurrentHostTime?
    return juce_getMicroseconds() * 0.001;
}

int64 Time::getHighResolutionTicks() throw()
{
    return (int64) AudioGetCurrentHostTime();
}

int64 Time::getHighResolutionTicksPerSecond() throw()
{
    return highResTimerFrequency;
}

int64 SystemStats::getClockCycleCounter() throw()
{
    jassertfalse
    return 0;
}

bool Time::setSystemTimeToThisTime() const throw()
{
    jassertfalse
    return false;
}

//==============================================================================
int SystemStats::getPageSize() throw()
{
    jassertfalse
    return 512; //xxx
}

void PlatformUtilities::fpuReset()
{
}

END_JUCE_NAMESPACE
