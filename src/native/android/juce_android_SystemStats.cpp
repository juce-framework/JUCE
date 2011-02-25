/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
namespace AndroidStatsHelpers
{
    const String getSystemProperty (const String& name)
    {
        return juceString (LocalRef<jstring> ((jstring) getEnv()->CallStaticObjectMethod (android.systemClass,
                                                                                          android.getProperty,
                                                                                          javaString (name).get())));
    }
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    return Android;
}

const String SystemStats::getOperatingSystemName()
{
    return "Android " + AndroidStatsHelpers::getSystemProperty ("os.version");
}

bool SystemStats::isOperatingSystem64Bit()
{
  #if JUCE_64BIT
    return true;
  #else
    return false;
  #endif
}

const String SystemStats::getCpuVendor()
{
    return AndroidStatsHelpers::getSystemProperty ("os.arch");
}

int SystemStats::getCpuSpeedInMegaherz()
{
    return 0; // TODO
}

int SystemStats::getMemorySizeInMegabytes()
{
    // xxx they forgot to implement sysinfo in the library, dammit! Should put this stuff back when they fix it.
/*    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (sysi.totalram * sysi.mem_unit / (1024 * 1024));
 */
    DBG ("warning! memory size is unavailable due to an Android bug!");
    return 0;
}

int SystemStats::getPageSize()
{
    return sysconf (_SC_PAGESIZE);
}

//==============================================================================
const String SystemStats::getLogonName()
{
    const char* user = getenv ("USER");

    if (user == 0)
    {
        struct passwd* const pw = getpwuid (getuid());
        if (pw != 0)
            user = pw->pw_name;
    }

    return CharPointer_UTF8 (user);
}

const String SystemStats::getFullUserName()
{
    return getLogonName();
}

//==============================================================================
void SystemStats::initialiseStats()
{
    // TODO
    cpuFlags.hasMMX = false;
    cpuFlags.hasSSE = false;
    cpuFlags.hasSSE2 = false;
    cpuFlags.has3DNow = false;

    cpuFlags.numCpus = jmax (1, sysconf (_SC_NPROCESSORS_ONLN));
}

void PlatformUtilities::fpuReset() {}

//==============================================================================
uint32 juce_millisecondsSinceStartup() throw()
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

int64 Time::getHighResolutionTicks() throw()
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

    return (t.tv_sec * (int64) 1000000) + (t.tv_nsec / (int64) 1000);
}

int64 Time::getHighResolutionTicksPerSecond() throw()
{
    return 1000000;  // (microseconds)
}

double Time::getMillisecondCounterHiRes() throw()
{
    return getHighResolutionTicks() * 0.001;
}

bool Time::setSystemTimeToThisTime() const
{
    jassertfalse;
    return false;
}


#endif
