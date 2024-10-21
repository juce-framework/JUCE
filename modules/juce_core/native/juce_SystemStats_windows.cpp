/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

void Logger::outputDebugString (const String& text)
{
    OutputDebugString ((text + "\n").toWideCharPointer());
}

//==============================================================================
#ifdef JUCE_DLL_BUILD
 JUCE_API void* juceDLL_malloc (size_t sz)    { return std::malloc (sz); }
 JUCE_API void  juceDLL_free (void* block)    { std::free (block); }
#endif

static int findNumberOfPhysicalCores() noexcept
{
    DWORD bufferSize = 0;
    GetLogicalProcessorInformation (nullptr, &bufferSize);

    const auto numBuffers = (size_t) (bufferSize / sizeof (SYSTEM_LOGICAL_PROCESSOR_INFORMATION));

    if (numBuffers == 0)
    {
        jassertfalse;
        return 0;
    };

    HeapBlock<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer (numBuffers);

    if (! GetLogicalProcessorInformation (buffer, &bufferSize))
    {
        jassertfalse;
        return 0;
    }

    return (int) std::count_if (buffer.get(), buffer.get() + numBuffers, [] (const auto& info)
    {
        return info.Relationship == RelationProcessorCore;
    });
}

//==============================================================================
#if JUCE_INTEL
 #if JUCE_MSVC && ! defined (__INTEL_COMPILER)
  #pragma intrinsic (__cpuid)
  #pragma intrinsic (__rdtsc)
 #endif

 #if JUCE_CLANG
static void callCPUID (int result[4], uint32 type)
{
  uint32 la = (uint32) result[0], lb = (uint32) result[1],
         lc = (uint32) result[2], ld = (uint32) result[3];

  asm ("mov %%ebx, %%esi \n\t"
       "cpuid \n\t"
       "xchg %%esi, %%ebx"
       : "=a" (la), "=S" (lb), "=c" (lc), "=d" (ld) : "a" (type)
        #if JUCE_64BIT
     , "b" (lb), "c" (lc), "d" (ld)
        #endif
       );

  result[0] = (int) la; result[1] = (int) lb;
  result[2] = (int) lc; result[3] = (int) ld;
}
 #else
static void callCPUID (int result[4], int infoType)
{
    __cpuid (result, infoType);
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

String SystemStats::getCpuModel()
{
    char name[65] = { 0 };
    int info[4] = { 0 };

    callCPUID (info, 0x80000000);

    const int numExtIDs = info[0];

    if ((unsigned) numExtIDs < 0x80000004)  // if brand string is unsupported
        return {};

    callCPUID (info, 0x80000002);
    memcpy (name, info, sizeof (info));

    callCPUID (info, 0x80000003);
    memcpy (name + 16, info, sizeof (info));

    callCPUID (info, 0x80000004);
    memcpy (name + 32, info, sizeof (info));

    return String (name).trim();
}

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
    hasFMA3  = (info[2] & (1 << 12)) != 0;
    hasSSSE3 = (info[2] & (1 <<  9)) != 0;
    hasSSE41 = (info[2] & (1 << 19)) != 0;
    hasSSE42 = (info[2] & (1 << 20)) != 0;

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshift-sign-overflow")
    has3DNow = (info[1] & (1 << 31)) != 0;
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    callCPUID (info, 0x80000001);
    hasFMA4  = (info[2] & (1 << 16)) != 0;

    callCPUID (info, 7);

    hasAVX2            = ((unsigned int) info[1] & (1 << 5))   != 0;
    hasAVX512F         = ((unsigned int) info[1] & (1u << 16)) != 0;
    hasAVX512DQ        = ((unsigned int) info[1] & (1u << 17)) != 0;
    hasAVX512IFMA      = ((unsigned int) info[1] & (1u << 21)) != 0;
    hasAVX512PF        = ((unsigned int) info[1] & (1u << 26)) != 0;
    hasAVX512ER        = ((unsigned int) info[1] & (1u << 27)) != 0;
    hasAVX512CD        = ((unsigned int) info[1] & (1u << 28)) != 0;
    hasAVX512BW        = ((unsigned int) info[1] & (1u << 30)) != 0;
    hasAVX512VL        = ((unsigned int) info[1] & (1u << 31)) != 0;
    hasAVX512VBMI      = ((unsigned int) info[2] & (1u <<  1)) != 0;
    hasAVX512VPOPCNTDQ = ((unsigned int) info[2] & (1u << 14)) != 0;

    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo (&systemInfo);
    numLogicalCPUs  = (int) systemInfo.dwNumberOfProcessors;
    numPhysicalCPUs = findNumberOfPhysicalCores();

    if (numPhysicalCPUs <= 0)
        numPhysicalCPUs = numLogicalCPUs;
}
#elif JUCE_ARM
String SystemStats::getCpuVendor()
{
    static const auto cpuVendor = []
    {
        static constexpr auto* path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\\VendorIdentifier";
        auto vendor = RegistryKeyWrapper::getValue (path, {}, 0).trim();

        return vendor.isEmpty() ? String ("Unknown Vendor") : vendor;
    }();

    return cpuVendor;
}

String SystemStats::getCpuModel()
{
    static const auto cpuModel = []
    {
        static constexpr auto* path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\\ProcessorNameString";
        auto model = RegistryKeyWrapper::getValue (path, {}, 0).trim();

        return model.isEmpty() ? String ("Unknown Model") : model;
    }();

    return cpuModel;
}

void CPUInformation::initialise() noexcept
{
    // Windows for arm requires at least armv7 which has neon support
    hasNeon = true;

    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo (&systemInfo);
    numLogicalCPUs  = (int) systemInfo.dwNumberOfProcessors;
    numPhysicalCPUs = findNumberOfPhysicalCores();

    if (numPhysicalCPUs <= 0)
        numPhysicalCPUs = numLogicalCPUs;
}
#else
 #error Unknown CPU architecture type
#endif

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
RTL_OSVERSIONINFOW getWindowsVersionInfo();
RTL_OSVERSIONINFOW getWindowsVersionInfo()
{
    RTL_OSVERSIONINFOW versionInfo = {};

    if (auto* moduleHandle = ::GetModuleHandleW (L"ntdll.dll"))
    {
        using RtlGetVersion = LONG (WINAPI*) (PRTL_OSVERSIONINFOW);

        if (auto* rtlGetVersion = (RtlGetVersion) ::GetProcAddress (moduleHandle, "RtlGetVersion"))
        {
            versionInfo.dwOSVersionInfoSize = sizeof (versionInfo);
            LONG STATUS_SUCCESS = 0;

            if (rtlGetVersion (&versionInfo) != STATUS_SUCCESS)
                versionInfo = {};
        }
    }

    return versionInfo;
}

SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    const auto versionInfo = getWindowsVersionInfo();
    const auto major = versionInfo.dwMajorVersion;
    const auto minor = versionInfo.dwMinorVersion;
    const auto build = versionInfo.dwBuildNumber;

    jassert (major <= 10); // need to add support for new version!

    if (major == 10 && build >= 22000) return Windows11;
    if (major == 10)                   return Windows10;
    if (major == 6 && minor == 3)      return Windows8_1;
    if (major == 6 && minor == 2)      return Windows8_0;
    if (major == 6 && minor == 1)      return Windows7;
    if (major == 6 && minor == 0)      return WinVista;
    if (major == 5 && minor == 1)      return WinXP;
    if (major == 5 && minor == 0)      return Win2000;

    jassertfalse;
    return Windows;
}

String SystemStats::getOperatingSystemName()
{
    const auto type = getOperatingSystemType();

    if (type == Windows11)      return "Windows 11";
    if (type == Windows10)      return "Windows 10";
    if (type == Windows8_1)     return "Windows 8.1";
    if (type == Windows8_0)     return "Windows 8.0";
    if (type == Windows7)       return "Windows 7";
    if (type == WinVista)       return "Windows Vista";
    if (type == WinXP)          return "Windows XP";
    if (type == Win2000)        return "Windows 2000";

    jassertfalse;
    return "Unknown OS";
}

String SystemStats::getDeviceDescription()
{
   #if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    return "Windows (Desktop)";
   #elif WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
    return "Windows (Store)";
   #elif WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
    return "Windows (Phone)";
   #elif WINAPI_FAMILY == WINAPI_FAMILY_SYSTEM
    return "Windows (System)";
   #elif WINAPI_FAMILY == WINAPI_FAMILY_SERVER
    return "Windows (Server)";
   #else
    return "Windows";
   #endif
}

String SystemStats::getDeviceManufacturer()
{
    return {};
}

bool SystemStats::isOperatingSystem64Bit()
{
   #if JUCE_64BIT
    return true;
   #else
    typedef BOOL (WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    const auto moduleHandle = GetModuleHandleA ("kernel32");

    if (moduleHandle == nullptr)
    {
        jassertfalse;
        return false;
    }

    LPFN_ISWOW64PROCESS fnIsWow64Process
        = (LPFN_ISWOW64PROCESS) GetProcAddress (moduleHandle, "IsWow64Process");

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
    auto len = GetEnvironmentVariableW (name.toWideCharPointer(), nullptr, 0);

    if (len == 0)
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
        [[maybe_unused]] auto res = timeBeginPeriod (JUCE_WIN32_TIMER_PERIOD);
        jassert (res == TIMERR_NOERROR);
       #endif

        LARGE_INTEGER f;
        QueryPerformanceFrequency (&f);
        hiResTicksPerSecond = f.QuadPart;
        hiResTicksScaleFactor = 1000.0 / (double) hiResTicksPerSecond;
    }

    inline int64 getHighResolutionTicks() noexcept
    {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter (&ticks);
        return ticks.QuadPart + hiResTicksOffset;
    }

    inline double getMillisecondCounterHiRes() noexcept
    {
        return (double) getHighResolutionTicks() * hiResTicksScaleFactor;
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
 #if JUCE_MSVC
  #if JUCE_INTEL
    // MS intrinsics version...
    return (int64) __rdtsc();
  #elif JUCE_ARM
   #if defined (_M_ARM)
    return __rdpmccntr64();
   #elif defined (_M_ARM64)
    return _ReadStatusReg (ARM64_PMCCNTR_EL0);
   #else
    #error Unknown arm architecture
   #endif
  #endif
 #elif JUCE_GCC || JUCE_CLANG
  #if JUCE_INTEL
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
  #elif JUCE_ARM
    int64 retval;

    __asm__ __volatile__ ("mrs %0, cntvct_el0" : "=r"(retval));
    return retval;
  #endif
 #else
  #error "unknown compiler?"
 #endif
}

int SystemStats::getCpuSpeedInMegahertz()
{
    auto cycles = juce_getClockCycleCounter();
    auto millis = Time::getMillisecondCounter();
    int lastResult = 0;

    for (;;)
    {
        int n = 1000000;
        while (--n > 0) {}

        auto millisElapsed = Time::getMillisecondCounter() - millis;
        auto cyclesNow = juce_getClockCycleCounter();

        if (millisElapsed > 80)
        {
            auto newResult = (int) (((cyclesNow - cycles) / millisElapsed) / 1000);

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
    // NB: the local variable is here to avoid analysers warning about having
    // two identical sub-expressions in the return statement
    auto firstCallToSetTimezone = SetLocalTime (&st) != 0;
    return firstCallToSetTimezone && SetLocalTime (&st) != 0;
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
    auto len = (DWORD) numElementsInArray (text) - 1;
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
    auto len = (DWORD) numElementsInArray (text) - 1;
    GetComputerNameEx (ComputerNamePhysicalDnsHostname, text, &len);
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
    JUCE_LOAD_WINAPI_FUNCTION (dll,
                               GetUserPreferredUILanguages,
                               getUserPreferredUILanguages,
                               BOOL,
                               (DWORD, PULONG, PZZWSTR, PULONG))

    constexpr auto defaultResult = "en";

    if (getUserPreferredUILanguages == nullptr)
        return defaultResult;

    ULONG numLanguages = 0;
    ULONG numCharsInLanguagesBuffer = 0;

    // Retrieving the necessary buffer size for storing the list of languages
    if (! getUserPreferredUILanguages (MUI_LANGUAGE_NAME, &numLanguages, nullptr, &numCharsInLanguagesBuffer))
        return defaultResult;

    std::vector<WCHAR> languagesBuffer (numCharsInLanguagesBuffer);
    const auto success = getUserPreferredUILanguages (MUI_LANGUAGE_NAME,
                                                      &numLanguages,
                                                      languagesBuffer.data(),
                                                      &numCharsInLanguagesBuffer);

    if (! success || numLanguages == 0)
        return defaultResult;

    // The buffer contains a zero delimited list of languages, the first being
    // the currently displayed language.
    return languagesBuffer.data();
}

static constexpr DWORD generateProviderID (const char* string)
{
    return (DWORD) string[0] << 0x18
         | (DWORD) string[1] << 0x10
         | (DWORD) string[2] << 0x08
         | (DWORD) string[3] << 0x00;
}

static std::optional<std::vector<std::byte>> readSMBIOSData()
{
    const auto sig = generateProviderID ("RSMB");
    const auto  id = generateProviderID ("RSDT");

    if (const auto bufLen = GetSystemFirmwareTable (sig, id, nullptr, 0); bufLen > 0)
    {
        std::vector<std::byte> buffer;

        buffer.resize (bufLen);

        if (GetSystemFirmwareTable (sig, id, buffer.data(), bufLen) == buffer.size())
            return std::make_optional (std::move (buffer));
    }

    return {};
}

String getLegacyUniqueDeviceID()
{
    if (const auto dump = readSMBIOSData())
    {
        uint64_t hash = 0;
        const auto start = dump->data();
        const auto end   = start + jmin (1024, (int) dump->size());

        for (auto dataPtr = start; dataPtr != end; ++dataPtr)
            hash = hash * (uint64_t) 101 + (uint8_t) *dataPtr;

        return String (hash);
    }

    return {};
}

String SystemStats::getUniqueDeviceID()
{
    if (const auto smbiosBuffer = readSMBIOSData())
    {
        #pragma pack (push, 1)
        struct RawSMBIOSData
        {
            uint8_t unused[4];
            uint32_t length;
        };

        struct SMBIOSHeader
        {
            uint8_t  id;
            uint8_t  length;
            uint16_t handle;
        };
        #pragma pack (pop)

        if (smbiosBuffer->size() < sizeof (RawSMBIOSData))
        {
            // Malformed buffer; not enough room for RawSMBIOSData instance
            jassertfalse;
            return {};
        }

        String uuid;
        const auto* asRawSMBIOSData = unalignedPointerCast<const RawSMBIOSData*> (smbiosBuffer->data());

        if (smbiosBuffer->size() < sizeof (RawSMBIOSData) + static_cast<size_t> (asRawSMBIOSData->length))
        {
            // Malformed buffer; declared length is longer than the buffer we were given
            jassertfalse;
            return {};
        }

        Span<const std::byte> content (smbiosBuffer->data() + sizeof (RawSMBIOSData), asRawSMBIOSData->length);

        while (! content.empty())
        {
            if (content.size() < sizeof (SMBIOSHeader))
            {
                // Malformed buffer; not enough room for header
                jassertfalse;
                break;
            }

            const auto* header = unalignedPointerCast<const SMBIOSHeader*> (content.data());

            if (content.size() < header->length)
            {
                // Malformed buffer; declared length is longer than the buffer we were given
                jassertfalse;
                break;
            }

            std::vector<std::string_view> strings;

            // Each table comprises a struct and a varying number of null terminated
            // strings. The string section is delimited by a pair of null terminators.
            // Some fields in the header are indices into the string table.

            const auto endOfStringTable = [&header, &strings, &content]
            {
                const auto* dataTable = unalignedPointerCast<const char*> (content.data());
                size_t stringOffset = header->length;

                while (stringOffset < content.size())
                {
                    const auto* str = dataTable + stringOffset;
                    const auto maxLength = content.size() - stringOffset;
                    const auto n = strnlen (str, maxLength);

                    if (n == 0)
                        break;

                    strings.emplace_back (str, n);
                    stringOffset += std::min (n + 1, maxLength);
                }

                const auto lengthAfterHeader = jmax ((size_t) header->length + 2, stringOffset + 1);
                return jmin (lengthAfterHeader, content.size());
            }();

            const auto stringFromOffset = [&content, &strings] (size_t byteOffset) -> String
            {
                if (! isPositiveAndBelow (byteOffset, content.size()))
                    return std::string{};

                const auto index = std::to_integer<size_t> (content[byteOffset]);

                if (index <= 0 || strings.size() < index)
                    return std::string{};

                const auto view = strings[index - 1];
                return std::string { view };
            };

            enum
            {
                systemManufacturer      = 0x04,
                systemProductName       = 0x05,
                systemSerialNumber      = 0x07,
                systemUUID              = 0x08, // 16byte UUID. Can be all 0xFF or all 0x00. Might be user changeable.
                systemSKU               = 0x19,
                systemFamily            = 0x1a,

                baseboardManufacturer   = 0x04,
                baseboardProduct        = 0x05,
                baseboardVersion        = 0x06,
                baseboardSerialNumber   = 0x07,
                baseboardAssetTag       = 0x08,

                processorManufacturer   = 0x07,
                processorVersion        = 0x10,
                processorAssetTag       = 0x21,
                processorPartNumber     = 0x22
            };

            switch (header->id)
            {
                case 1: // System
                {
                    uuid += stringFromOffset (systemManufacturer);
                    uuid += "\n";
                    uuid += stringFromOffset (systemProductName);
                    uuid += "\n";

                    char hexBuf[(16 * 2) + 1]{};

                    if (systemUUID + 16 < content.size())
                    {
                        const auto* src = content.data() + systemUUID;

                        for (auto i = 0; i != 16; ++i)
                            snprintf (hexBuf + 2 * i, 3, "%02hhX", std::to_integer<uint8_t> (src[i]));
                    }

                    uuid += hexBuf;
                    uuid += "\n";
                    break;
                }

                case 2: // Baseboard
                    uuid += stringFromOffset (baseboardManufacturer);
                    uuid += "\n";
                    uuid += stringFromOffset (baseboardProduct);
                    uuid += "\n";
                    uuid += stringFromOffset (baseboardVersion);
                    uuid += "\n";
                    uuid += stringFromOffset (baseboardSerialNumber);
                    uuid += "\n";
                    uuid += stringFromOffset (baseboardAssetTag);
                    uuid += "\n";
                    break;

                case 4: // Processor
                    uuid += stringFromOffset (processorManufacturer);
                    uuid += "\n";
                    uuid += stringFromOffset (processorVersion);
                    uuid += "\n";
                    uuid += stringFromOffset (processorAssetTag);
                    uuid += "\n";
                    uuid += stringFromOffset (processorPartNumber);
                    uuid += "\n";
                    break;
            }

            content = Span (content.data() + endOfStringTable, content.size() - endOfStringTable);
        }

        return String (uuid.hashCode64());
    }

    // Please tell someone at JUCE if this occurs
    jassertfalse;
    return {};
}

} // namespace juce
