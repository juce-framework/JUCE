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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
void Logger::outputDebugString (const String& text) throw()
{
    fputs (text.toUTF8(), stdout);
    fputs ("\n", stdout);
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

bool SystemStats::isOperatingSystem64Bit() throw()
{
#if JUCE_64BIT
    return true;
#else
    //xxx not sure how to find this out?..
    return false;
#endif
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

#endif
