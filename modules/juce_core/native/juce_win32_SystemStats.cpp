/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
#if JUCE_USE_INTRINSICS

// CPU info functions using intrinsics...

#pragma intrinsic (__cpuid)
#pragma intrinsic (__rdtsc)

String SystemStats::getCpuVendor()
{
    int info [4];
    __cpuid (info, 0);

    char v [12];
    memcpy (v, info + 1, 4);
    memcpy (v + 4, info + 3, 4);
    memcpy (v + 8, info + 2, 4);

    return String (v, 12);
}

#else

//==============================================================================
// CPU info functions using old fashioned inline asm...

static void juce_getCpuVendor (char* const v)
{
    int vendor[4] = { 0 };

   #if ! JUCE_MINGW
    __try
   #endif
    {
       #if JUCE_GCC
        unsigned int dummy = 0;
        __asm__ ("cpuid" : "=a" (dummy), "=b" (vendor[0]), "=c" (vendor[2]),"=d" (vendor[1]) : "a" (0));
       #else
        __asm
        {
            mov eax, 0
            cpuid
            mov [vendor], ebx
            mov [vendor + 4], edx
            mov [vendor + 8], ecx
        }
       #endif
    }
   #if ! JUCE_MINGW
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
   #endif

    memcpy (v, vendor, 16);
}

String SystemStats::getCpuVendor()
{
    char v [16];
    juce_getCpuVendor (v);
    return String (v, 16);
}
#endif


//==============================================================================
SystemStats::CPUFlags::CPUFlags()
{
    hasMMX   = IsProcessorFeaturePresent (PF_MMX_INSTRUCTIONS_AVAILABLE) != 0;
    hasSSE   = IsProcessorFeaturePresent (PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0;
    hasSSE2  = IsProcessorFeaturePresent (PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0;
   #ifdef PF_AMD3D_INSTRUCTIONS_AVAILABLE
    has3DNow = IsProcessorFeaturePresent (PF_AMD3D_INSTRUCTIONS_AVAILABLE) != 0;
   #else
    has3DNow = IsProcessorFeaturePresent (PF_3DNOW_INSTRUCTIONS_AVAILABLE) != 0;
   #endif

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
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof (info);
    GetVersionEx (&info);

    if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        if (info.dwMajorVersion == 5)
            return (info.dwMinorVersion == 0) ? Win2000 : WinXP;

        if (info.dwMajorVersion == 6)
        {
            switch (info.dwMinorVersion)
            {
                case 0:  return WinVista;
                case 1:  return Windows7;
                case 2:  return Windows8;

                default:
                    jassertfalse;  // new version needs to be added here!
                    return Windows8;
            }
        }
    }

    jassertfalse;  // need to support whatever new version is running!
    return UnknownOS;
}

String SystemStats::getOperatingSystemName()
{
    const char* name = "Unknown OS";

    switch (getOperatingSystemType())
    {
        case Windows7:          name = "Windows 7";         break;
        case Windows8:          name = "Windows 8";         break;
        case WinVista:          name = "Windows Vista";     break;
        case WinXP:             name = "Windows XP";        break;
        case Win2000:           name = "Windows 2000";      break;
        default:                jassertfalse; break; // !! new type of OS?
    }

    return name;
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
        const MMRESULT res = timeBeginPeriod (1);
        (void) res;
        jassert (res == TIMERR_NOERROR);

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
   #if JUCE_USE_INTRINSICS
    // MS intrinsics version...
    return (int64) __rdtsc();

   #elif JUCE_GCC
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
    TCHAR text [MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
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

    if (getUserDefaultUILanguage != nullptr)
        return getLocaleValue (MAKELCID (getUserDefaultUILanguage(), SORT_DEFAULT), LOCALE_SISO639LANGNAME, "en");

    return "en";
}
