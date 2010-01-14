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

static int64 highResTimerFrequency = 0;
static double highResTimerToMillisecRatio = 0;

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

static unsigned int getCPUIDWord (unsigned int& familyModel, unsigned int& extFeatures)
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
void SystemStats::initialiseStats() throw()
{
    static bool initialised = false;

    if (! initialised)
    {
        initialised = true;

#if JUCE_MAC
        // extremely annoying: adding this line stops the apple menu items from working. Of
        // course, not adding it means that carbon windows (e.g. in plugins) won't get
        // any events.
        //NSApplicationLoad();
        [NSApplication sharedApplication];
#endif

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

        mach_timebase_info_data_t timebase;
        (void) mach_timebase_info (&timebase);
        highResTimerFrequency = (int64) (1.0e9 * timebase.denom / timebase.numer);
        highResTimerToMillisecRatio = timebase.numer / (1.0e6 * timebase.denom);

        String s (SystemStats::getJUCEVersion());

        rlimit lim;
        getrlimit (RLIMIT_NOFILE, &lim);
        lim.rlim_cur = lim.rlim_max = RLIM_INFINITY;
        setrlimit (RLIMIT_NOFILE, &lim);
    }
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

bool SystemStats::isOperatingSystem64Bit() throw()
{
#if JUCE_64BIT
    return true;
#else
    //xxx not sure how to find this out?..
    return false;
#endif
}

int SystemStats::getMemorySizeInMegabytes() throw()
{
    uint64 mem = 0;
    size_t memSize = sizeof (mem);
    int mib[] = { CTL_HW, HW_MEMSIZE };
    sysctl (mib, 2, &mem, &memSize, 0, 0);
    return (int) (mem / (1024 * 1024));
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
    uint64 speedHz = 0;
    size_t speedSize = sizeof (speedHz);
    int mib[] = { CTL_HW, HW_CPU_FREQ };
    sysctl (mib, 2, &speedHz, &speedSize, 0, 0);

#if JUCE_BIG_ENDIAN
    if (speedSize == 4)
        speedHz >>= 32;
#endif
    return (int) (speedHz / 1000000);
}

int SystemStats::getNumCpus() throw()
{
#if JUCE_IPHONE || (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
    return (int) [[NSProcessInfo processInfo] activeProcessorCount];
#else
    return MPProcessors();
#endif
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() throw()
{
    return (uint32) (mach_absolute_time() * highResTimerToMillisecRatio);
}

double Time::getMillisecondCounterHiRes() throw()
{
    return mach_absolute_time() * highResTimerToMillisecRatio;
}

int64 Time::getHighResolutionTicks() throw()
{
    return (int64) mach_absolute_time();
}

int64 Time::getHighResolutionTicksPerSecond() throw()
{
    return highResTimerFrequency;
}

int64 SystemStats::getClockCycleCounter() throw()
{
    return (int64) mach_absolute_time();
}

bool Time::setSystemTimeToThisTime() const throw()
{
    jassertfalse
    return false;
}

//==============================================================================
int SystemStats::getPageSize() throw()
{
    return (int) NSPageSize();
}

void PlatformUtilities::fpuReset()
{
}


#endif
