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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
void Logger::outputDebugString (const String& text)
{
    std::cerr << text << std::endl;
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    return Linux;
}

const String SystemStats::getOperatingSystemName()
{
    return "Linux";
}

bool SystemStats::isOperatingSystem64Bit()
{
#if JUCE_64BIT
    return true;
#else
    //xxx not sure how to find this out?..
    return false;
#endif
}

//==============================================================================
static const String juce_getCpuInfo (const char* const key)
{
    StringArray lines;
    lines.addLines (File ("/proc/cpuinfo").loadFileAsString());

    for (int i = lines.size(); --i >= 0;) // (NB - it's important that this runs in reverse order)
        if (lines[i].startsWithIgnoreCase (key))
            return lines[i].fromFirstOccurrenceOf (":", false, false).trim();

    return String::empty;
}

bool SystemStats::hasMMX()      { return juce_getCpuInfo ("flags").contains ("mmx"); }
bool SystemStats::hasSSE()      { return juce_getCpuInfo ("flags").contains ("sse"); }
bool SystemStats::hasSSE2()     { return juce_getCpuInfo ("flags").contains ("sse2"); }
bool SystemStats::has3DNow()    { return juce_getCpuInfo ("flags").contains ("3dnow"); }

const String SystemStats::getCpuVendor()
{
    return juce_getCpuInfo ("vendor_id");
}

int SystemStats::getCpuSpeedInMegaherz()
{
    return roundToInt (juce_getCpuInfo ("cpu MHz").getFloatValue());
}

int SystemStats::getMemorySizeInMegabytes()
{
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (sysi.totalram * sysi.mem_unit / (1024 * 1024));

    return 0;
}

int SystemStats::getPageSize()
{
    return sysconf (_SC_PAGESIZE);
}

int SystemStats::getNumCpus()
{
    return juce_getCpuInfo ("processor").getIntValue() + 1;
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

    return String::fromUTF8 (user);
}

const String SystemStats::getFullUserName()
{
    return getLogonName();
}

//==============================================================================
void SystemStats::initialiseStats()
{
}

void PlatformUtilities::fpuReset()
{
}

//==============================================================================
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
    return 1000000;  // (microseconds)
}

bool Time::setSystemTimeToThisTime() const
{
    timeval t;
    t.tv_sec = millisSinceEpoch % 1000000;
    t.tv_usec = millisSinceEpoch - t.tv_sec;

    return settimeofday (&t, 0) ? false : true;
}


#endif
