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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

extern void juce_initialiseThreadEvents();


//==============================================================================
void Logger::outputDebugString (const String& text) throw()
{
    OutputDebugString (text + T("\n"));
}

void Logger::outputDebugPrintf (const tchar* format, ...) throw()
{
    String text;
    va_list args;
    va_start (args, format);
    text.vprintf(format, args);
    outputDebugString (text);
}

//==============================================================================
static int64 hiResTicksPerSecond;
static double hiResTicksScaleFactor;


//==============================================================================
#if JUCE_USE_INTRINSICS

// CPU info functions using intrinsics...

#pragma intrinsic (__cpuid)
#pragma intrinsic (__rdtsc)

const String SystemStats::getCpuVendor() throw()
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
    int vendor[4];
    zeromem (vendor, 16);

#ifdef JUCE_64BIT
#else
  #ifndef __MINGW32__
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
  #ifndef __MINGW32__
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        *v = 0;
    }
  #endif
#endif

    memcpy (v, vendor, 16);
}

const String SystemStats::getCpuVendor() throw()
{
    char v [16];
    juce_getCpuVendor (v);
    return String (v, 16);
}
#endif


//==============================================================================
struct CPUFlags
{
    bool hasMMX : 1;
    bool hasSSE : 1;
    bool hasSSE2 : 1;
    bool has3DNow : 1;
};

static CPUFlags cpuFlags;

bool SystemStats::hasMMX() throw()
{
    return cpuFlags.hasMMX;
}

bool SystemStats::hasSSE() throw()
{
    return cpuFlags.hasSSE;
}

bool SystemStats::hasSSE2() throw()
{
    return cpuFlags.hasSSE2;
}

bool SystemStats::has3DNow() throw()
{
    return cpuFlags.has3DNow;
}

void SystemStats::initialiseStats() throw()
{
    juce_initialiseThreadEvents();

    cpuFlags.hasMMX   = IsProcessorFeaturePresent (PF_MMX_INSTRUCTIONS_AVAILABLE) != 0;
    cpuFlags.hasSSE   = IsProcessorFeaturePresent (PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0;
    cpuFlags.hasSSE2  = IsProcessorFeaturePresent (PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0;
#ifdef PF_AMD3D_INSTRUCTIONS_AVAILABLE
    cpuFlags.has3DNow = IsProcessorFeaturePresent (PF_AMD3D_INSTRUCTIONS_AVAILABLE) != 0;
#else
    cpuFlags.has3DNow = IsProcessorFeaturePresent (PF_3DNOW_INSTRUCTIONS_AVAILABLE) != 0;
#endif

    LARGE_INTEGER f;
    QueryPerformanceFrequency (&f);
    hiResTicksPerSecond = f.QuadPart;
    hiResTicksScaleFactor = 1000.0 / hiResTicksPerSecond;

    String s (SystemStats::getJUCEVersion());

#ifdef JUCE_DEBUG
    const MMRESULT res = timeBeginPeriod (1);
    jassert (res == TIMERR_NOERROR);
#else
    timeBeginPeriod (1);
#endif

#if defined (JUCE_DEBUG) && JUCE_MSVC && JUCE_CHECK_MEMORY_LEAKS
    _CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType() throw()
{
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof (info);
    GetVersionEx (&info);

    if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        switch (info.dwMajorVersion)
        {
            case 5:     return (info.dwMinorVersion == 0) ? Win2000 : WinXP;
            case 6:     return (info.dwMinorVersion == 0) ? WinVista : Windows7;
            default:    jassertfalse; break; // !! not a supported OS!
        }
    }
    else if (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        jassert (info.dwMinorVersion != 0); // !! still running on Windows 95??
        return Win98;
    }

    return UnknownOS;
}

const String SystemStats::getOperatingSystemName() throw()
{
    const char* name = "Unknown OS";

    switch (getOperatingSystemType())
    {
        case Windows7:          name = "Windows 7";         break;
        case WinVista:          name = "Windows Vista";     break;
        case WinXP:             name = "Windows XP";        break;
        case Win2000:           name = "Windows 2000";      break;
        case Win98:             name = "Windows 98";        break;
        default:                jassertfalse; break; // !! new type of OS?
    }

    return name;
}

bool SystemStats::isOperatingSystem64Bit() throw()
{
#ifdef _WIN64
    return true;
#else
    typedef BOOL (WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress (GetModuleHandle (L"kernel32"), "IsWow64Process");

    BOOL isWow64 = FALSE;

    return (fnIsWow64Process != 0)
            && fnIsWow64Process (GetCurrentProcess(), &isWow64)
            && (isWow64 != FALSE);
#endif
}


//==============================================================================
int SystemStats::getMemorySizeInMegabytes() throw()
{
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof (mem);
    GlobalMemoryStatusEx (&mem);
    return (int) (mem.ullTotalPhys / (1024 * 1024)) + 1;
}

int SystemStats::getNumCpus() throw()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo (&systemInfo);

    return systemInfo.dwNumberOfProcessors;
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() throw()
{
    return (uint32) GetTickCount();
}

int64 Time::getHighResolutionTicks() throw()
{
    LARGE_INTEGER ticks;
    QueryPerformanceCounter (&ticks);

    const int64 mainCounterAsHiResTicks = (GetTickCount() * hiResTicksPerSecond) / 1000;
    const int64 newOffset = mainCounterAsHiResTicks - ticks.QuadPart;

    // fix for a very obscure PCI hardware bug that can make the counter
    // sometimes jump forwards by a few seconds..
    static int64 hiResTicksOffset = 0;
    const int64 offsetDrift = abs64 (newOffset - hiResTicksOffset);

    if (offsetDrift > (hiResTicksPerSecond >> 1))
        hiResTicksOffset = newOffset;

    return ticks.QuadPart + hiResTicksOffset;
}

double Time::getMillisecondCounterHiRes() throw()
{
    return getHighResolutionTicks() * hiResTicksScaleFactor;
}

int64 Time::getHighResolutionTicksPerSecond() throw()
{
    return hiResTicksPerSecond;
}

int64 SystemStats::getClockCycleCounter() throw()
{
#if JUCE_USE_INTRINSICS
    // MS intrinsics version...
    return __rdtsc();

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

int SystemStats::getCpuSpeedInMegaherz() throw()
{
    const int64 cycles = SystemStats::getClockCycleCounter();
    const uint32 millis = Time::getMillisecondCounter();
    int lastResult = 0;

    for (;;)
    {
        int n = 1000000;
        while (--n > 0) {}

        const uint32 millisElapsed = Time::getMillisecondCounter() - millis;
        const int64 cyclesNow = SystemStats::getClockCycleCounter();

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
bool Time::setSystemTimeToThisTime() const throw()
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

int SystemStats::getPageSize() throw()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo (&systemInfo);

    return systemInfo.dwPageSize;
}

#endif
