/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

void Logger::outputDebugString (const String& text)
{
    std::cerr << text << std::endl;
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    return Linux;
}

String SystemStats::getOperatingSystemName()
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
namespace LinuxStatsHelpers
{
    String getConfigFileValue (const char* file, const char* const key)
    {
        StringArray lines;
        File (file).readLines (lines);

        for (int i = lines.size(); --i >= 0;) // (NB - it's important that this runs in reverse order)
            if (lines[i].upToFirstOccurrenceOf (":", false, false).trim().equalsIgnoreCase (key))
                return lines[i].fromFirstOccurrenceOf (":", false, false).trim();

        return String();
    }

    String getCpuInfo (const char* key)
    {
        return getConfigFileValue ("/proc/cpuinfo", key);
    }
}

String SystemStats::getDeviceDescription()
{
    return LinuxStatsHelpers::getCpuInfo ("Hardware");
}

String SystemStats::getCpuVendor()
{
    String v (LinuxStatsHelpers::getCpuInfo ("vendor_id"));

    if (v.isEmpty())
        v = LinuxStatsHelpers::getCpuInfo ("model name");

    return v;
}

String SystemStats::getCpuModel()
{
    return LinuxStatsHelpers::getCpuInfo ("model name");
}

int SystemStats::getCpuSpeedInMegaherz()
{
    return roundToInt (LinuxStatsHelpers::getCpuInfo ("cpu MHz").getFloatValue());
}

int SystemStats::getMemorySizeInMegabytes()
{
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (int) (sysi.totalram * sysi.mem_unit / (1024 * 1024));

    return 0;
}

int SystemStats::getPageSize()
{
    return (int) sysconf (_SC_PAGESIZE);
}

//==============================================================================
String SystemStats::getLogonName()
{
    if (const char* user = getenv ("USER"))
        return CharPointer_UTF8 (user);

    if (struct passwd* const pw = getpwuid (getuid()))
        return CharPointer_UTF8 (pw->pw_name);

    return String();
}

String SystemStats::getFullUserName()
{
    return getLogonName();
}

String SystemStats::getComputerName()
{
    char name [256] = { 0 };
    if (gethostname (name, sizeof (name) - 1) == 0)
        return name;

    return String();
}

static String getLocaleValue (nl_item key)
{
    const char* oldLocale = ::setlocale (LC_ALL, "");
    String result (String::fromUTF8 (nl_langinfo (key)));
    ::setlocale (LC_ALL, oldLocale);
    return result;
}

String SystemStats::getUserLanguage()    { return getLocaleValue (_NL_IDENTIFICATION_LANGUAGE); }
String SystemStats::getUserRegion()      { return getLocaleValue (_NL_IDENTIFICATION_TERRITORY); }
String SystemStats::getDisplayLanguage() { return getUserLanguage() + "-" + getUserRegion(); }

//==============================================================================
void CPUInformation::initialise() noexcept
{
    const String flags (LinuxStatsHelpers::getCpuInfo ("flags"));
    hasMMX   = flags.contains ("mmx");
    hasSSE   = flags.contains ("sse");
    hasSSE2  = flags.contains ("sse2");
    hasSSE3  = flags.contains ("sse3");
    has3DNow = flags.contains ("3dnow");
    hasSSSE3 = flags.contains ("ssse3");
    hasSSE41 = flags.contains ("sse4_1");
    hasSSE42 = flags.contains ("sse4_2");
    hasAVX   = flags.contains ("avx");
    hasAVX2  = flags.contains ("avx2");

    numCpus = LinuxStatsHelpers::getCpuInfo ("processor").getIntValue() + 1;
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() noexcept
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

    return (uint32) (t.tv_sec * 1000 + t.tv_nsec / 1000000);
}

int64 Time::getHighResolutionTicks() noexcept
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

    return (t.tv_sec * (int64) 1000000) + (t.tv_nsec / 1000);
}

int64 Time::getHighResolutionTicksPerSecond() noexcept
{
    return 1000000;  // (microseconds)
}

double Time::getMillisecondCounterHiRes() noexcept
{
    return getHighResolutionTicks() * 0.001;
}

bool Time::setSystemTimeToThisTime() const
{
    timeval t;
    t.tv_sec = millisSinceEpoch / 1000;
    t.tv_usec = (millisSinceEpoch - t.tv_sec * 1000) * 1000;

    return settimeofday (&t, 0) == 0;
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger() noexcept
{
   #if JUCE_BSD
    return false;
   #else
    return LinuxStatsHelpers::getConfigFileValue ("/proc/self/status", "TracerPid")
             .getIntValue() > 0;
   #endif
}
