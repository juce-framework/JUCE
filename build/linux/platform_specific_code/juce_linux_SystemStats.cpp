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
#include "../../../src/juce_core/basics/juce_StandardHeader.h"
#include <sys/sysinfo.h>
#include <dlfcn.h>


#ifndef CPU_ISSET
  #undef SUPPORT_AFFINITIES
#endif

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/io/files/juce_File.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_appframework/events/juce_Timer.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"


//==============================================================================
/*static juce_noinline unsigned int getCPUIDWord (int* familyModel, int* extFeatures) throw()
{
    unsigned int cpu = 0;
    unsigned int ext = 0;
    unsigned int family = 0;
    unsigned int dummy = 0;

#if JUCE_64BIT
    __asm__ ("cpuid"
               : "=a" (family), "=b" (ext), "=c" (dummy), "=d" (cpu) : "a" (1));

#else
    __asm__ ("push %%ebx; cpuid; mov %%ebx, %%edi; pop %%ebx"
               : "=a" (family), "=D" (ext), "=c" (dummy), "=d" (cpu) : "a" (1));
#endif

    if (familyModel != 0)
        *familyModel = family;

    if (extFeatures != 0)
        *extFeatures = ext;

    return cpu;
}*/

//==============================================================================
void Logger::outputDebugString (const String& text) throw()
{
    fprintf (stdout, text.toUTF8());
    fprintf (stdout, "\n");
}

void Logger::outputDebugPrintf (const tchar* format, ...) throw()
{
    String text;
    va_list args;
    va_start (args, format);
    text.vprintf(format, args);
    outputDebugString(text);
}

SystemStats::OperatingSystemType SystemStats::getOperatingSystemType() throw()
{
    return Linux;
}

const String SystemStats::getOperatingSystemName() throw()
{
    return T("Linux");
}

static const String getCpuInfo (const char* key, bool lastOne = false) throw()
{
    String info;
    char buf [256];

    FILE* f = fopen ("/proc/cpuinfo", "r");

    while (f != 0 && fgets (buf, sizeof(buf), f))
    {
        if (strncmp (buf, key, strlen (key)) == 0)
        {
            char* p = buf;

            while (*p && *p != '\n')
                ++p;

            if (*p != 0)
                *p = 0;

            p = buf;

            while (*p != 0 && *p != ':')
                ++p;

            if (*p != 0 && *(p + 1) != 0)
                info = p + 2;

            if (! lastOne)
                break;
        }
    }

    fclose (f);
    return info;
}

bool SystemStats::hasMMX() throw()
{
    return getCpuInfo ("flags").contains (T("mmx"));
}

bool SystemStats::hasSSE() throw()
{
    return getCpuInfo ("flags").contains (T("sse"));
}

bool SystemStats::hasSSE2() throw()
{
    return getCpuInfo ("flags").contains (T("sse2"));
}

bool SystemStats::has3DNow() throw()
{
    return getCpuInfo ("flags").contains (T("3dnow"));
}

const String SystemStats::getCpuVendor() throw()
{
    return getCpuInfo ("vendor_id");
}

int SystemStats::getCpuSpeedInMegaherz() throw()
{
    const String speed (getCpuInfo ("cpu MHz"));

    return (int) (speed.getFloatValue() + 0.5f);
}

int SystemStats::getMemorySizeInMegabytes() throw()
{
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (sysi.totalram * sysi.mem_unit / (1024 * 1024));

    return 0;
}

uint32 juce_millisecondsSinceStartup() throw()
{
    static unsigned int calibrate = 0;
    static bool calibrated = false;
    timeval t;
    unsigned int ret = 0;

    if (! gettimeofday (&t, 0))
    {
        if (! calibrated)
        {
            struct sysinfo sysi;

            if (sysinfo (&sysi) == 0)
                // Safe to assume system was not brought up earlier than 1970!
                calibrate = t.tv_sec - sysi.uptime;

            calibrated = true;
        }

        ret = 1000 * (t.tv_sec - calibrate) + (t.tv_usec / 1000);
    }

    return ret;
}

double Time::getMillisecondCounterHiRes() throw()
{
    return getHighResolutionTicks() * 0.001;
}

int64 Time::getHighResolutionTicks() throw()
{
    timeval t;
    if (gettimeofday (&t, 0))
        return 0;

    return ((int64) t.tv_sec * (int64) 1000000) + (int64) t.tv_usec;
}

int64 Time::getHighResolutionTicksPerSecond() throw()
{
    // Microseconds
    return 1000000;
}

bool Time::setSystemTimeToThisTime() const throw()
{
    timeval t;
    t.tv_sec = millisSinceEpoch % 1000000;
    t.tv_usec = millisSinceEpoch - t.tv_sec;

    return settimeofday (&t, NULL) ? false : true;
}

int SystemStats::getPageSize() throw()
{
    static int systemPageSize = 0;

    if (systemPageSize == 0)
        systemPageSize = sysconf (_SC_PAGESIZE);

    return systemPageSize;
}

int SystemStats::getNumCpus() throw()
{
    const int lastCpu = getCpuInfo ("processor", true).getIntValue();

    return lastCpu + 1;
}


//==============================================================================
void SystemStats::initialiseStats() throw()
{
    // Process starts off as root when running suid
    Process::lowerPrivilege();

    String s (SystemStats::getJUCEVersion());
}

void PlatformUtilities::fpuReset()
{
}

END_JUCE_NAMESPACE
