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

void Logger::outputDebugString (const String& text)
{
    OutputDebugString ((text + "\n").toWideCharPointer());
}

//==============================================================================
#ifdef JUCE_DLL_BUILD
 JUCE_API void* juceDLL_malloc (size_t sz)    { return std::malloc (sz); }
 JUCE_API void  juceDLL_free (void* block)    { std::free (block); }
#endif

//==============================================================================
#if JUCE_USE_MSVC_INTRINSICS

// CPU info functions using intrinsics...

#pragma intrinsic (__cpuid)
#pragma intrinsic (__rdtsc)

static void callCPUID (int result[4], int infoType)
{
    __cpuid (result, infoType);
}

#else

static void callCPUID (int result[4], int infoType)
{
   #if ! JUCE_MINGW
    __try
   #endif
    {
       #if JUCE_GCC || JUCE_CLANG
        __asm__ __volatile__ ("cpuid" : "=a" (result[0]), "=b" (result[1]), "=c" (result[2]),"=d" (result[3]) : "a" (infoType));
       #else
        __asm
        {
            mov    esi, result
            mov    eax, infoType
            xor    ecx, ecx
            cpuid
            mov    dword ptr [esi +  0], eax
            mov    dword ptr [esi +  4], ebx
            mov    dword ptr [esi +  8], ecx
            mov    dword ptr [esi + 12], edx
        }
       #endif
    }
   #if ! JUCE_MINGW
    __except (EXCEPTION_EXECUTE_HANDLER) {}
   #endif
}

#endif

String SystemStats::getCpuVendor()
{
    int info[4] = { 0 };
    callCPUID (info, 0);

    char v [12];
    memcpy (v, info + 1, 4);
    memcpy (v + 4, info + 3, 4);
    memcpy (v + 8, info + 2, 4);

    return String (v, 12);
}

//==============================================================================
void CPUInformation::initialise() noexcept
{
    int info[4] = { 0 };
    callCPUID (info, 1);

    // NB: IsProcessorFeaturePresent doesn't work on XP
    hasMMX   = (info[3] & (1 << 23)) != 0;
    hasSSE   = (info[3] & (1 << 25)) != 0;
    hasSSE2  = (info[3] & (1 << 26)) != 0;
    hasSSE3  = (info[2] & (1 <<  0)) != 0;
    hasAVX   = (info[2] & (1 << 28)) != 0;
    hasSSSE3 = (info[2] & (1 <<  9)) != 0;
    hasSSE41 = (info[2] & (1 << 19)) != 0;
    hasSSE42 = (info[2] & (1 << 20)) != 0;
    has3DNow = (info[1] & (1 << 31)) != 0;

    callCPUID (info, 7);

    hasAVX2 = (info[1] & (1 << 5)) != 0;

    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo (&systemInfo);
    numCpus = (int) systemInfo.dwNumberOfProcessors;
}

#if JUCE_MSVC && JUCE_CHECK_MEMORY_LEAKS
struct DebugFlagsInitialiser
{
    DebugFlagsInitialiser()
    {
        _CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }
};

static DebugFlagsInitialiser debugFlagsInitialiser;
#endif

//==============================================================================
static bool isWindowsVersionOrLater (SystemStats::OperatingSystemType target)
{
    OSVERSIONINFOEX info;
    zerostruct (info);
    info.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);

    if (target >= SystemStats::Windows10)
    {
        info.dwMajorVersion = 10;
        info.dwMinorVersion = 0;
    }
    else if (target >= SystemStats::WinVista)
    {
        info.dwMajorVersion = 6;

        switch (target)
        {
            case SystemStats::WinVista:    break;
            case SystemStats::Windows7:    info.dwMinorVersion = 1; break;
            case SystemStats::Windows8_0:  info.dwMinorVersion = 2; break;
            case SystemStats::Windows8_1:  info.dwMinorVersion = 3; break;
            default:                       jassertfalse; break;
        }
    }
    else
    {
        info.dwMajorVersion = 5;
        info.dwMinorVersion = target >= SystemStats::WinXP ? 1 : 0;
    }

    DWORDLONG mask = 0;

    VER_SET_CONDITION (mask, VER_MAJORVERSION,     VER_GREATER_EQUAL);
    VER_SET_CONDITION (mask, VER_MINORVERSION,     VER_GREATER_EQUAL);
    VER_SET_CONDITION (mask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    VER_SET_CONDITION (mask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL);

    return VerifyVersionInfo (&info,
                              VER_MAJORVERSION | VER_MINORVERSION
                               | VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
                              mask) != FALSE;
}

SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    const SystemStats::OperatingSystemType types[]
            = { Windows10, Windows8_1, Windows8_0, Windows7, WinVista, WinXP, Win2000 };

    for (int i = 0; i < numElementsInArray (types); ++i)
        if (isWindowsVersionOrLater (types[i]))
            return types[i];

    jassertfalse;  // need to support whatever new version is running!
    return UnknownOS;
}

String SystemStats::getOperatingSystemName()
{
    const char* name = "Unknown OS";

    switch (getOperatingSystemType())
    {
        case Windows10:         name = "Windows 10";        break;
        case Windows8_1:        name = "Windows 8.1";       break;
        case Windows8_0:        name = "Windows 8.0";       break;
        case Windows7:          name = "Windows 7";         break;
        case WinVista:          name = "Windows Vista";     break;
        case WinXP:             name = "Windows XP";        break;
        case Win2000:           name = "Windows 2000";      break;
        default:                jassertfalse; break; // !! new type of OS?
    }

    return name;
}

String SystemStats::getDeviceDescription()
{
    return String();
}

bool SystemStats::isOperatingSystem64Bit()
{
   #if JUCE_64BIT
    return true;
   #else
    typedef BOOL (WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    LPFN_ISWOW64PROCESS fnIsWow64Process
        = (LPFN_ISWOW64PROCESS) GetProcAddress (GetModuleHandleA ("kernel32"), "IsWow64Process");

    BOOL isWow64 = FALSE;

    return fnIsWow64Process != nullptr
            && fnIsWow64Process (GetCurrentProcess(), &isWow64)
            && isWow64 != FALSE;
   #endif
}

//==============================================================================
int SystemStats::getMemorySizeInMegabytes()
{
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof (mem);
    GlobalMemoryStatusEx (&mem);
    return (int) (mem.ullTotalPhys / (1024 * 1024)) + 1;
}

//==============================================================================
String SystemStats::getEnvironmentVariable (const String& name, const String& defaultValue)
{
    DWORD len = GetEnvironmentVariableW (name.toWideCharPointer(), nullptr, 0);
    if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
        return String (defaultValue);

    HeapBlock<WCHAR> buffer (len);
    len = GetEnvironmentVariableW (name.toWideCharPointer(), buffer, len);

    return String (CharPointer_wchar_t (buffer),
                   CharPointer_wchar_t (buffer + len));
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() noexcept
{
    return (uint32) timeGetTime();
}

//==============================================================================
class HiResCounterHandler
{
public:
    HiResCounterHandler()
        : hiResTicksOffset (0)
    {
        // This macro allows you to override the default timer-period
        // used on Windows. By default this is set to 1, because that has
        // always been the value used in JUCE apps, and changing it could
        // affect the behaviour of existing code, but you may wish to make
        // it larger (or set it to 0 to use the system default) to make your
        // app less demanding on the CPU.
        // For more info, see win32 documentation about the timeBeginPeriod
        // function.
       #ifndef JUCE_WIN32_TIMER_PERIOD
        #define JUCE_WIN32_TIMER_PERIOD 1
       #endif

       #if JUCE_WIN32_TIMER_PERIOD > 0
        const MMRESULT res = timeBeginPeriod (JUCE_WIN32_TIMER_PERIOD);
        ignoreUnused (res);
        jassert (res == TIMERR_NOERROR);
       #endif

        LARGE_INTEGER f;
        QueryPerformanceFrequency (&f);
        hiResTicksPerSecond = f.QuadPart;
        hiResTicksScaleFactor = 1000.0 / hiResTicksPerSecond;
    }

    inline int64 getHighResolutionTicks() noexcept
    {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter (&ticks);

        const int64 mainCounterAsHiResTicks = (juce_millisecondsSinceStartup() * hiResTicksPerSecond) / 1000;
        const int64 newOffset = mainCounterAsHiResTicks - ticks.QuadPart;

        // fix for a very obscure PCI hardware bug that can make the counter
        // sometimes jump forwards by a few seconds..
        const int64 offsetDrift = abs64 (newOffset - hiResTicksOffset);

        if (offsetDrift > (hiResTicksPerSecond >> 1))
            hiResTicksOffset = newOffset;

        return ticks.QuadPart + hiResTicksOffset;
    }

    inline double getMillisecondCounterHiRes() noexcept
    {
        return getHighResolutionTicks() * hiResTicksScaleFactor;
    }

    int64 hiResTicksPerSecond, hiResTicksOffset;
    double hiResTicksScaleFactor;
};

static HiResCounterHandler hiResCounterHandler;

int64  Time::getHighResolutionTicksPerSecond() noexcept  { return hiResCounterHandler.hiResTicksPerSecond; }
int64  Time::getHighResolutionTicks() noexcept           { return hiResCounterHandler.getHighResolutionTicks(); }
double Time::getMillisecondCounterHiRes() noexcept       { return hiResCounterHandler.getMillisecondCounterHiRes(); }

//==============================================================================
static int64 juce_getClockCycleCounter() noexcept
{
   #if JUCE_USE_MSVC_INTRINSICS
    // MS intrinsics version...
    return (int64) __rdtsc();

   #elif JUCE_GCC || JUCE_CLANG
    // GNU inline asm version...
    unsigned int hi = 0, lo = 0;

    __asm__ __volatile__ (
        "xor %%eax, %%eax               \n\
         xor %%edx, %%edx               \n\
         rdtsc                          \n\
         movl %%eax, %[lo]              \n\
         movl %%edx, %[hi]"
         :
         : [hi] "m" (hi),
           [lo] "m" (lo)
         : "cc", "eax", "ebx", "ecx", "edx", "memory");

    return (int64) ((((uint64) hi) << 32) | lo);
   #else
    // MSVC inline asm version...
    unsigned int hi = 0, lo = 0;

    __asm
    {
        xor eax, eax
        xor edx, edx
        rdtsc
        mov lo, eax
        mov hi, edx
    }

    return (int64) ((((uint64) hi) << 32) | lo);
   #endif
}

int SystemStats::getCpuSpeedInMegaherz()
{
    const int64 cycles = juce_getClockCycleCounter();
    const uint32 millis = Time::getMillisecondCounter();
    int lastResult = 0;

    for (;;)
    {
        int n = 1000000;
        while (--n > 0) {}

        const uint32 millisElapsed = Time::getMillisecondCounter() - millis;
        const int64 cyclesNow = juce_getClockCycleCounter();

        if (millisElapsed > 80)
        {
            const int newResult = (int) (((cyclesNow - cycles) / millisElapsed) / 1000);

            if (millisElapsed > 500 || (lastResult == newResult && newResult > 100))
                return newResult;

            lastResult = newResult;
        }
    }
}


//==============================================================================
bool Time::setSystemTimeToThisTime() const
{
    SYSTEMTIME st;

    st.wDayOfWeek = 0;
    st.wYear           = (WORD) getYear();
    st.wMonth          = (WORD) (getMonth() + 1);
    st.wDay            = (WORD) getDayOfMonth();
    st.wHour           = (WORD) getHours();
    st.wMinute         = (WORD) getMinutes();
    st.wSecond         = (WORD) getSeconds();
    st.wMilliseconds   = (WORD) (millisSinceEpoch % 1000);

    // do this twice because of daylight saving conversion problems - the
    // first one sets it up, the second one kicks it in.
    return SetLocalTime (&st) != 0
            && SetLocalTime (&st) != 0;
}

int SystemStats::getPageSize()
{
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo (&systemInfo);

    return (int) systemInfo.dwPageSize;
}

//==============================================================================
String SystemStats::getLogonName()
{
    TCHAR text [256] = { 0 };
    DWORD len = (DWORD) numElementsInArray (text) - 1;
    GetUserName (text, &len);
    return String (text, len);
}

String SystemStats::getFullUserName()
{
    return getLogonName();
}

String SystemStats::getComputerName()
{
    TCHAR text[128] = { 0 };
    DWORD len = (DWORD) numElementsInArray (text) - 1;
    GetComputerName (text, &len);
    return String (text, len);
}

static String getLocaleValue (LCID locale, LCTYPE key, const char* defaultValue)
{
    TCHAR buffer [256] = { 0 };
    if (GetLocaleInfo (locale, key, buffer, 255) > 0)
        return buffer;

    return defaultValue;
}

String SystemStats::getUserLanguage()     { return getLocaleValue (LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME,  "en"); }
String SystemStats::getUserRegion()       { return getLocaleValue (LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, "US"); }

String SystemStats::getDisplayLanguage()
{
    DynamicLibrary dll ("kernel32.dll");
    JUCE_LOAD_WINAPI_FUNCTION (dll, GetUserDefaultUILanguage, getUserDefaultUILanguage, LANGID, (void))

    if (getUserDefaultUILanguage == nullptr)
        return "en";

    const DWORD langID = MAKELCID (getUserDefaultUILanguage(), SORT_DEFAULT);

    String mainLang (getLocaleValue (langID, LOCALE_SISO639LANGNAME, "en"));
    String region   (getLocaleValue (langID, LOCALE_SISO3166CTRYNAME, nullptr));

    if (region.isNotEmpty())
        mainLang << '-' << region;

    return mainLang;
}
