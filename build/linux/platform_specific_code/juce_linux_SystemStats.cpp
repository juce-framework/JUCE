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

static struct _LogicalCpuInfo
{
    bool htSupported;
    bool htAvailable;
    int numPackages;
    int numLogicalPerPackage;
    uint32 physicalAffinityMask;
} logicalCpuInfo;

//==============================================================================
static juce_noinline unsigned int getCPUIDWord (int* familyModel, int* extFeatures) throw()
{
    unsigned int cpu = 0;
    unsigned int ext = 0;
    unsigned int family = 0;
    unsigned int dummy = 0;

    __asm__ ("push %%ebx; cpuid; mov %%ebx, %%edi; pop %%ebx"
               : "=a" (family), "=D" (ext), "=c" (dummy),"=d" (cpu)
               : "a" (1));

    if (familyModel != 0)
        *familyModel = family;

    if (extFeatures != 0)
        *extFeatures = ext;

    return cpu;
}

void juce_initLogicalCpuInfo() throw()
{
    int familyModelWord, extFeaturesWord;
    int featuresWord = getCPUIDWord (&familyModelWord, &extFeaturesWord);

    logicalCpuInfo.htSupported = false;
    logicalCpuInfo.htAvailable = false;
    logicalCpuInfo.numLogicalPerPackage = 1;
    logicalCpuInfo.numPackages = 0;
    logicalCpuInfo.physicalAffinityMask = 0;

#if SUPPORT_AFFINITIES
    cpu_set_t processAffinity;

    /*
       N.B. If this line causes a compile error, then you've probably not got the latest
       version of glibc installed.

       If you don't want to update your copy of glibc and don't care about cpu affinities,
       then you can just disable all this stuff by removing the SUPPORT_AFFINITIES macro
       from the linuxincludes.h file.
    */
    if (sched_getaffinity (getpid(),
                           sizeof (cpu_set_t),
                           &processAffinity) != sizeof (cpu_set_t))
    {
        return;
    }

    // Checks: CPUID supported, model >= Pentium 4, Hyperthreading bit set, logical CPUs per package > 1
    if (featuresWord == 0
        || ((familyModelWord >> 8) & 0xf) < 15
        || (featuresWord & (1 << 28)) == 0
        || ((extFeaturesWord >> 16) & 0xff) < 2)
    {
        for (int i = 0; i < 64; ++i)
            if (CPU_ISSET (i, &processAffinity))
                logicalCpuInfo.physicalAffinityMask |= (1 << i);

        return;
    }

    logicalCpuInfo.htSupported = true;
    logicalCpuInfo.numLogicalPerPackage = (extFeaturesWord >> 16) & 0xff;

    cpu_set_t affinityMask;
    cpu_set_t physAff;
    CPU_ZERO (&physAff);

    unsigned char i = 1;
    unsigned char physIdMask = 0xFF;
    unsigned char physIdShift = 0;

    //unsigned char apicId, logId, physId;

    while (i < logicalCpuInfo.numLogicalPerPackage)
    {
        i *= 2;
        physIdMask <<= 1;
        physIdShift++;
    }

    CPU_SET (0, &affinityMask);
    logicalCpuInfo.numPackages = 0;

//xxx revisit this at some point..
/*    while ((affinityMask != 0) && (affinityMask <= processAffinity))
    {
        int ret;
        if (! sched_setaffinity (getpid(), sizeof (cpu_set_t), &affinityMask))
        {
            sched_yield(); // schedule onto correct CPU

            featuresWord = getCPUIDWord(&familyModelWord, &extFeaturesWord);
            apicId = (unsigned char)(extFeaturesWord >> 24);
            logId = (unsigned char)(apicId & ~physIdMask);
            physId = (unsigned char)(apicId >> physIdShift);

            if (logId != 0)
                logicalCpuInfo.htAvailable = true;

            if ((((int)logId) % logicalCpuInfo.numLogicalPerPackage) == 0)
            {
                // This is a physical CPU
                physAff |= affinityMask;
                logicalCpuInfo.numPackages++;
            }
        }

        affinityMask = affinityMask << 1;
    }

    sched_setaffinity (getpid(), sizeof(unsigned long), &processAffinity);
*/

    logicalCpuInfo.physicalAffinityMask = 0;

    for (int i = 0; i < 64; ++i)
        if (CPU_ISSET (i, &physAff))
            logicalCpuInfo.physicalAffinityMask |= (1 << i);

#endif
}

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

bool SystemStats::hasHyperThreading() throw()
{
    return logicalCpuInfo.htAvailable;
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
    return getHighResolutionTicks() * (1.0 / 1000000.0);
}

int64 Time::getHighResolutionTicks() throw()
{
    timeval t;
    if (gettimeofday (&t,NULL))
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

int SystemStats::getNumPhysicalCpus() throw()
{
    if (logicalCpuInfo.numPackages)
        return logicalCpuInfo.numPackages;

    return getNumLogicalCpus();
}

int SystemStats::getNumLogicalCpus() throw()
{
    const int lastCpu = getCpuInfo ("processor", true).getIntValue();

    return lastCpu + 1;
}

uint32 SystemStats::getPhysicalAffinityMask() throw()
{
#if SUPPORT_AFFINITIES
    return logicalCpuInfo.physicalAffinityMask;
#else
    /* affinities aren't supported because either the appropriate header files weren't found,
       or the SUPPORT_AFFINITIES macro was turned off in linuxheaders.h
    */
    jassertfalse
    return 0;
#endif

}

//==============================================================================
void SystemStats::initialiseStats() throw()
{
    // Process starts off as root when running suid
    Process::lowerPrivilege();

    String s (SystemStats::getJUCEVersion());

    juce_initLogicalCpuInfo();
}

void PlatformUtilities::fpuReset()
{
}

END_JUCE_NAMESPACE
