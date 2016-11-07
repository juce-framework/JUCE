/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

ScopedAutoReleasePool::ScopedAutoReleasePool()
{
    pool = [[NSAutoreleasePool alloc] init];
}

ScopedAutoReleasePool::~ScopedAutoReleasePool()
{
    [((NSAutoreleasePool*) pool) release];
}

//==============================================================================
void Logger::outputDebugString (const String& text)
{
    // Would prefer to use std::cerr here, but avoiding it for
    // the moment, due to clang JIT linkage problems.
    fputs (text.toRawUTF8(), stderr);
    fputs ("\n", stderr);
    fflush (stderr);
}

//==============================================================================
namespace SystemStatsHelpers
{
   #if JUCE_INTEL && ! JUCE_NO_INLINE_ASM
    static void doCPUID (uint32& a, uint32& b, uint32& c, uint32& d, uint32 type)
    {
        uint32 la = a, lb = b, lc = c, ld = d;

        asm ("mov %%ebx, %%esi \n\t"
             "cpuid \n\t"
             "xchg %%esi, %%ebx"
               : "=a" (la), "=S" (lb), "=c" (lc), "=d" (ld) : "a" (type)
           #if JUCE_64BIT
                  , "b" (lb), "c" (lc), "d" (ld)
           #endif
        );

        a = la; b = lb; c = lc; d = ld;
    }
   #endif
}

//==============================================================================
void CPUInformation::initialise() noexcept
{
   #if JUCE_INTEL && ! JUCE_NO_INLINE_ASM
    uint32 a = 0, b = 0, d = 0, c = 0;
    SystemStatsHelpers::doCPUID (a, b, c, d, 1);

    hasMMX   = (d & (1u << 23)) != 0;
    hasSSE   = (d & (1u << 25)) != 0;
    hasSSE2  = (d & (1u << 26)) != 0;
    has3DNow = (b & (1u << 31)) != 0;
    hasSSE3  = (c & (1u <<  0)) != 0;
    hasSSSE3 = (c & (1u <<  9)) != 0;
    hasSSE41 = (c & (1u << 19)) != 0;
    hasSSE42 = (c & (1u << 20)) != 0;
    hasAVX   = (c & (1u << 28)) != 0;

    SystemStatsHelpers::doCPUID (a, b, c, d, 7);
    hasAVX2  = (b & (1u <<  5)) != 0;
   #endif

    numCpus = (int) [[NSProcessInfo processInfo] activeProcessorCount];
}

//==============================================================================
#if ! JUCE_IOS
static String getOSXVersion()
{
    JUCE_AUTORELEASEPOOL
    {
        NSDictionary* dict = [NSDictionary dictionaryWithContentsOfFile:
                                    nsStringLiteral ("/System/Library/CoreServices/SystemVersion.plist")];

        return nsStringToJuce ([dict objectForKey: nsStringLiteral ("ProductVersion")]);
    }
}
#endif

SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
   #if JUCE_IOS
    return iOS;
   #else
    StringArray parts;
    parts.addTokens (getOSXVersion(), ".", StringRef());

    jassert (parts[0].getIntValue() == 10);
    const int major = parts[1].getIntValue();
    jassert (major > 2);

    return (OperatingSystemType) (major + MacOSX_10_4 - 4);
   #endif
}

String SystemStats::getOperatingSystemName()
{
   #if JUCE_IOS
    return "iOS " + nsStringToJuce ([[UIDevice currentDevice] systemVersion]);
   #else
    return "Mac OSX " + getOSXVersion();
   #endif
}

String SystemStats::getDeviceDescription()
{
   #if JUCE_IOS
    return nsStringToJuce ([[UIDevice currentDevice] model]);
   #else
    return String();
   #endif
}

bool SystemStats::isOperatingSystem64Bit()
{
   #if JUCE_IOS
    return false;
   #elif JUCE_64BIT
    return true;
   #else
    return getOperatingSystemType() >= MacOSX_10_6;
   #endif
}

int SystemStats::getMemorySizeInMegabytes()
{
    uint64 mem = 0;
    size_t memSize = sizeof (mem);
    int mib[] = { CTL_HW, HW_MEMSIZE };
    sysctl (mib, 2, &mem, &memSize, 0, 0);
    return (int) (mem / (1024 * 1024));
}

String SystemStats::getCpuVendor()
{
   #if JUCE_INTEL && ! JUCE_NO_INLINE_ASM
    uint32 dummy = 0;
    uint32 vendor[4] = { 0 };

    SystemStatsHelpers::doCPUID (dummy, vendor[0], vendor[2], vendor[1], 0);

    return String (reinterpret_cast<const char*> (vendor), 12);
   #else
    return String();
   #endif
}

int SystemStats::getCpuSpeedInMegaherz()
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

//==============================================================================
String SystemStats::getLogonName()
{
    return nsStringToJuce (NSUserName());
}

String SystemStats::getFullUserName()
{
    return nsStringToJuce (NSFullUserName());
}

String SystemStats::getComputerName()
{
    char name [256] = { 0 };
    if (gethostname (name, sizeof (name) - 1) == 0)
        return String (name).upToLastOccurrenceOf (".local", false, true);

    return String();
}

static String getLocaleValue (CFStringRef key)
{
    CFLocaleRef cfLocale = CFLocaleCopyCurrent();
    const String result (String::fromCFString ((CFStringRef) CFLocaleGetValue (cfLocale, key)));
    CFRelease (cfLocale);
    return result;
}

String SystemStats::getUserLanguage()   { return getLocaleValue (kCFLocaleLanguageCode); }
String SystemStats::getUserRegion()     { return getLocaleValue (kCFLocaleCountryCode); }

String SystemStats::getDisplayLanguage()
{
    CFArrayRef cfPrefLangs = CFLocaleCopyPreferredLanguages();
    const String result (String::fromCFString ((CFStringRef) CFArrayGetValueAtIndex (cfPrefLangs, 0)));
    CFRelease (cfPrefLangs);
    return result;
}

//==============================================================================
/*  NB: these are kept outside the HiResCounterInfo struct and initialised to 1 to avoid
    division-by-zero errors if some other static constructor calls us before this file's
    static constructors have had a chance to fill them in correctly..
*/
static uint64 hiResCounterNumerator = 0, hiResCounterDenominator = 1;

class HiResCounterInfo
{
public:
    HiResCounterInfo()
    {
        mach_timebase_info_data_t timebase;
        (void) mach_timebase_info (&timebase);

        if (timebase.numer % 1000000 == 0)
        {
            hiResCounterNumerator   = timebase.numer / 1000000;
            hiResCounterDenominator = timebase.denom;
        }
        else
        {
            hiResCounterNumerator   = timebase.numer;
            hiResCounterDenominator = timebase.denom * (uint64) 1000000;
        }

        highResTimerFrequency = (timebase.denom * (uint64) 1000000000) / timebase.numer;
        highResTimerToMillisecRatio = hiResCounterNumerator / (double) hiResCounterDenominator;
    }

    uint32 millisecondsSinceStartup() const noexcept
    {
        return (uint32) ((mach_absolute_time() * hiResCounterNumerator) / hiResCounterDenominator);
    }

    double getMillisecondCounterHiRes() const noexcept
    {
        return mach_absolute_time() * highResTimerToMillisecRatio;
    }

    int64 highResTimerFrequency;

private:
    double highResTimerToMillisecRatio;
};

static HiResCounterInfo hiResCounterInfo;

uint32 juce_millisecondsSinceStartup() noexcept         { return hiResCounterInfo.millisecondsSinceStartup(); }
double Time::getMillisecondCounterHiRes() noexcept      { return hiResCounterInfo.getMillisecondCounterHiRes(); }
int64  Time::getHighResolutionTicksPerSecond() noexcept { return hiResCounterInfo.highResTimerFrequency; }
int64  Time::getHighResolutionTicks() noexcept          { return (int64) mach_absolute_time(); }

bool Time::setSystemTimeToThisTime() const
{
    jassertfalse;
    return false;
}

//==============================================================================
int SystemStats::getPageSize()
{
    return (int) NSPageSize();
}
