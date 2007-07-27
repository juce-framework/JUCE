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

#include "win32_headers.h"
#include "../../../src/juce_core/basics/juce_StandardHeader.h"


//==============================================================================
// Auto-link the other win32 libs that are needed by library calls..
#if defined (JUCE_DLL_BUILD) && JUCE_MSVC

  #pragma comment(lib, "kernel32.lib")
  #pragma comment(lib, "user32.lib")
  #pragma comment(lib, "shell32.lib")
  #pragma comment(lib, "gdi32.lib")
  #pragma comment(lib, "vfw32.lib")
  #pragma comment(lib, "comdlg32.lib")
  #pragma comment(lib, "winmm.lib")
  #pragma comment(lib, "wininet.lib")
  #pragma comment(lib, "ole32.lib")
  #pragma comment(lib, "advapi32.lib")
  #pragma comment(lib, "ws2_32.lib")

  #if JUCE_OPENGL
    #pragma comment(lib, "OpenGL32.Lib")
    #pragma comment(lib, "GlU32.Lib")
  #endif
#endif


//==============================================================================
BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/io/files/juce_File.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"

extern void juce_updateMultiMonitorInfo() throw();
extern void juce_initialiseThreadEvents() throw();

#if JUCE_ENABLE_WIN98_COMPATIBILITY
 extern void juce_initialiseUnicodeFileFunctions() throw();
#endif


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

static SYSTEM_INFO systemInfo;


//==============================================================================
#if JUCE_USE_INTRINSICS

// CPU info functions using intrinsics...

#pragma intrinsic (__cpuid)
#pragma intrinsic (__rdtsc)

/*static unsigned int getCPUIDWord (int* familyModel = 0, int* extFeatures = 0) throw()
{
    int info [4];
    __cpuid (info, 1);

    if (familyModel != 0)
        *familyModel = info [0];

    if (extFeatures != 0)
        *extFeatures = info[1];

    return info[3];
}*/

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

/*static juce_noinline unsigned int getCPUIDWord (int* familyModel = 0, int* extFeatures = 0)
{
    unsigned int cpu = 0;
    unsigned int ext = 0;
    unsigned int family = 0;

  #if JUCE_GCC
    unsigned int dummy = 0;
  #endif

  #ifndef __MINGW32__
    __try
  #endif
    {
  #if JUCE_GCC
        __asm__ ("cpuid" : "=a" (family), "=b" (ext), "=c" (dummy),"=d" (cpu) : "a" (1));
  #else
        __asm
        {
            mov eax, 1
            cpuid
            mov cpu, edx
            mov family, eax
            mov ext, ebx
        }

  #endif
    }
  #ifndef __MINGW32__
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return 0;
    }
  #endif

    if (familyModel != 0)
        *familyModel = family;

    if (extFeatures != 0)
        *extFeatures = ext;

    return cpu;
}*/

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
#if JUCE_ENABLE_WIN98_COMPATIBILITY
    juce_initialiseUnicodeFileFunctions();
#endif

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

    GetSystemInfo (&systemInfo);

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
        case 5:
            return (info.dwMinorVersion == 0) ? Win2000 : WinXP;

        case 6:
            return WinVista;

        default:
            jassertfalse // !! not a supported OS!
            break;
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
    case WinVista:
        name = "Windows Vista";
        break;

    case WinXP:
        name = "Windows XP";
        break;

    case Win2000:
        name = "Windows 2000";
        break;

    case Win98:
        name = "Windows 98";
        break;

    default:
        jassertfalse // !! new type of OS?
        break;
    }

    return name;
}

//==============================================================================
int SystemStats::getMemorySizeInMegabytes() throw()
{
    MEMORYSTATUS mem;
    GlobalMemoryStatus (&mem);
    return (int) (mem.dwTotalPhys / (1024 * 1024)) + 1;
}

int SystemStats::getNumCpus() throw()
{
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
    return systemInfo.dwPageSize;
}

END_JUCE_NAMESPACE
