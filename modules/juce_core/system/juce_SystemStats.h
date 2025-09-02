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

//==============================================================================
/**
    Contains methods for finding out about the current hardware and OS configuration.

    @tags{Core}
*/
class JUCE_API  SystemStats  final
{
public:
    //==============================================================================
    /** Returns the current version of JUCE,
        See also the JUCE_VERSION, JUCE_MAJOR_VERSION and JUCE_MINOR_VERSION macros.
    */
    static String getJUCEVersion();

    //==============================================================================
    /** The set of possible results of the getOperatingSystemType() method. */
    enum OperatingSystemType
    {
        UnknownOS       = 0,

        MacOSX          = 0x0100,   /**< To test whether any version of OSX is running,
                                         you can use the expression ((getOperatingSystemType() & MacOSX) != 0). */
        Windows         = 0x0200,   /**< To test whether any version of Windows is running,
                                         you can use the expression ((getOperatingSystemType() & Windows) != 0). */
        Linux           = 0x0400,
        Android         = 0x0800,
        iOS             = 0x1000,
        WASM            = 0x2000,

        MacOSX_10_7     = MacOSX | 7,
        MacOSX_10_8     = MacOSX | 8,
        MacOSX_10_9     = MacOSX | 9,
        MacOSX_10_10    = MacOSX | 10,
        MacOSX_10_11    = MacOSX | 11,
        MacOSX_10_12    = MacOSX | 12,
        MacOSX_10_13    = MacOSX | 13,
        MacOSX_10_14    = MacOSX | 14,
        MacOSX_10_15    = MacOSX | 15,
        MacOS_11        = MacOSX | 16,
        MacOS_12        = MacOSX | 17,
        MacOS_13        = MacOSX | 18,
        MacOS_14        = MacOSX | 19,
        MacOS_15        = MacOSX | 20,
        MacOS_26        = MacOSX | 21,

        Win2000         = Windows | 1,
        WinXP           = Windows | 2,
        WinVista        = Windows | 3,
        Windows7        = Windows | 4,
        Windows8_0      = Windows | 5,
        Windows8_1      = Windows | 6,
        Windows10       = Windows | 7,
        Windows11       = Windows | 8
    };

    /** Returns the type of operating system we're running on.

        @returns one of the values from the OperatingSystemType enum.
        @see getOperatingSystemName
    */
    static OperatingSystemType getOperatingSystemType();

    /** Returns the name of the type of operating system we're running on.

        @returns a string describing the OS type.
        @see getOperatingSystemType
    */
    static String getOperatingSystemName();

    /** Returns true if the OS is 64-bit, or false for a 32-bit OS. */
    static bool isOperatingSystem64Bit();

    /** Returns an environment variable.
        If the named value isn't set, this will return the defaultValue string instead.
    */
    static String getEnvironmentVariable (const String& name, const String& defaultValue);

    //==============================================================================
    /** Returns the current user's name, if available.
        @see getFullUserName()
    */
    static String getLogonName();

    /** Returns the current user's full name, if available.
        On some OSes, this may just return the same value as getLogonName().
        @see getLogonName()
    */
    static String getFullUserName();

    /** Returns the host-name of the computer. */
    static String getComputerName();

    /** Returns the language of the user's locale.
        The return value is a 2 or 3 letter language code (ISO 639-1 or ISO 639-2)
    */
    static String getUserLanguage();

    /** Returns the region of the user's locale.
        The return value is a 2 letter country code (ISO 3166-1 alpha-2).
    */
    static String getUserRegion();

    /** Returns the user's display language.
        The return value is a 2 or 3 letter language code (ISO 639-1 or ISO 639-2).
        Note that depending on the OS and region, this may also be followed by a dash
        and a sub-region code, e.g "en-GB"
    */
    static String getDisplayLanguage();

    /** This will attempt to return some kind of string describing the device.
        If no description is available, it'll just return an empty string. You may
        want to use this for things like determining the type of phone/iPad, etc.
    */
    static String getDeviceDescription();

    /** This will attempt to return the manufacturer of the device.
        If no description is available, it'll just return an empty string.
    */
    static String getDeviceManufacturer();

    /** This method calculates some IDs to uniquely identify the device.

        The first choice for an ID is a filesystem ID for the user's home folder or
        windows directory. If that fails then this function returns the MAC addresses.
    */
    [[deprecated ("The identifiers produced by this function are not reliable. Use getUniqueDeviceID() instead.")]]
    static StringArray getDeviceIdentifiers();

    /** This method returns a machine unique ID unaffected by storage or peripheral
        changes.

        This ID will be invalidated by changes to the motherboard and CPU on non-mobile
        platforms, or performing a system restore on an Android device.

        There are some extra caveats on iOS: The returned ID is unique to the vendor part of
        your  'Bundle Identifier' and is stable for all associated apps. The key is invalidated
        once all associated apps are uninstalled. This function can return an empty string
        under certain conditions, for example, If the device has not been unlocked since a
        restart.
    */
    static String getUniqueDeviceID();

    /** Kinds of identifier that are passed to getMachineIdentifiers(). */
    enum class MachineIdFlags
    {
        macAddresses    = 1 << 0, ///< All Mac addresses of the machine.
        fileSystemId    = 1 << 1, ///< The filesystem id of the user's home directory (or system directory on Windows).
        legacyUniqueId  = 1 << 2, ///< Only implemented on Windows. A hash of the full smbios table, may be unstable on certain machines.
        uniqueId        = 1 << 3, ///< The most stable kind of machine identifier. A good default to use.
    };

    /** Returns a list of strings that can be used to uniquely identify a machine.

        To get multiple kinds of identifier at once, you can combine flags using
        bitwise-or, e.g. `uniqueId | legacyUniqueId`.

        If a particular kind of identifier isn't available, it will be omitted from
        the StringArray of results, so passing `uniqueId | legacyUniqueId`
        may return 0, 1, or 2 results, depending on the platform and whether any
        errors are encountered.

        If you've previously generated a machine ID and just want to check it against
        all possible identifiers, you can enable all of the flags and check whether
        the stored identifier matches any of the results.
    */
    static StringArray getMachineIdentifiers (MachineIdFlags flags);

    //==============================================================================
    // CPU and memory information..

    /** Returns the number of logical CPU cores. */
    static int getNumCpus() noexcept;

    /** Returns the number of physical CPU cores. */
    static int getNumPhysicalCpus() noexcept;

    /** Returns the approximate CPU speed.
        @returns    the speed in megahertz, e.g. 1500, 2500, 32000 (depending on
                    what year you're reading this...)
    */
    static int getCpuSpeedInMegahertz();

    /** Returns a string to indicate the CPU vendor.
        Might not be known on some systems.
    */
    static String getCpuVendor();

    /** Attempts to return a string describing the CPU model.
        May not be available on some systems.
    */
    static String getCpuModel();

    static bool hasMMX() noexcept;             /**< Returns true if Intel MMX instructions are available. */
    static bool has3DNow() noexcept;           /**< Returns true if AMD 3DNOW instructions are available. */
    static bool hasFMA3() noexcept;            /**< Returns true if AMD FMA3 instructions are available. */
    static bool hasFMA4() noexcept;            /**< Returns true if AMD FMA4 instructions are available. */
    static bool hasSSE() noexcept;             /**< Returns true if Intel SSE instructions are available. */
    static bool hasSSE2() noexcept;            /**< Returns true if Intel SSE2 instructions are available. */
    static bool hasSSE3() noexcept;            /**< Returns true if Intel SSE3 instructions are available. */
    static bool hasSSSE3() noexcept;           /**< Returns true if Intel SSSE3 instructions are available. */
    static bool hasSSE41() noexcept;           /**< Returns true if Intel SSE4.1 instructions are available. */
    static bool hasSSE42() noexcept;           /**< Returns true if Intel SSE4.2 instructions are available. */
    static bool hasAVX() noexcept;             /**< Returns true if Intel AVX instructions are available. */
    static bool hasAVX2() noexcept;            /**< Returns true if Intel AVX2 instructions are available. */
    static bool hasAVX512F() noexcept;         /**< Returns true if Intel AVX-512 Foundation instructions are available. */
    static bool hasAVX512BW() noexcept;        /**< Returns true if Intel AVX-512 Byte and Word instructions are available. */
    static bool hasAVX512CD() noexcept;        /**< Returns true if Intel AVX-512 Conflict Detection instructions are available. */
    static bool hasAVX512DQ() noexcept;        /**< Returns true if Intel AVX-512 Doubleword and Quadword instructions are available. */
    static bool hasAVX512ER() noexcept;        /**< Returns true if Intel AVX-512 Exponential and Reciprocal instructions are available. */
    static bool hasAVX512IFMA() noexcept;      /**< Returns true if Intel AVX-512 Integer Fused Multiply-Add instructions are available. */
    static bool hasAVX512PF() noexcept;        /**< Returns true if Intel AVX-512 Prefetch instructions are available. */
    static bool hasAVX512VBMI() noexcept;      /**< Returns true if Intel AVX-512 Vector Bit Manipulation instructions are available. */
    static bool hasAVX512VL() noexcept;        /**< Returns true if Intel AVX-512 Vector Length instructions are available. */
    static bool hasAVX512VPOPCNTDQ() noexcept; /**< Returns true if Intel AVX-512 Vector Population Count Double and Quad-word instructions are available. */
    static bool hasNeon() noexcept;            /**< Returns true if ARM NEON instructions are available. */

    //==============================================================================
    /** Finds out how much RAM is in the machine.
        @returns    the approximate number of megabytes of memory, or zero if
                    something goes wrong when finding out.
    */
    static int getMemorySizeInMegabytes();

    /** Returns the system page-size.
        This is only used by programmers with beards.
    */
    static int getPageSize();

    //==============================================================================
    /** Returns a backtrace of the current call-stack.
        The usefulness of the result will depend on the level of debug symbols
        that are available in the executable.
    */
    static String getStackBacktrace();

    /** A function type for use in setApplicationCrashHandler().
        When called, its void* argument will contain platform-specific data about the crash.
    */
    using CrashHandlerFunction = void (*) (void*);

    /** Sets up a global callback function that will be called if the application
        executes some kind of illegal instruction.

        You may want to call getStackBacktrace() in your handler function, to find out
        where the problem happened and log it, etc.
    */
    static void setApplicationCrashHandler (CrashHandlerFunction);

    /** Returns true if this code is running inside an app extension sandbox.
        This function will always return false on windows, linux and android.
    */
    static bool isRunningInAppExtensionSandbox() noexcept;

   #if JUCE_MAC
    static bool isAppSandboxEnabled();
   #endif

    //==============================================================================
    /** @cond */
    [[deprecated ("This method was spelt wrong! Please change your code to use getCpuSpeedInMegahertz instead.")]]
    static int getCpuSpeedInMegaherz() { return getCpuSpeedInMegahertz(); }
    /** @endcond */

private:
    SystemStats() = delete; // uses only static methods
    JUCE_DECLARE_NON_COPYABLE (SystemStats)
};

JUCE_DECLARE_SCOPED_ENUM_BITWISE_OPERATORS (SystemStats::MachineIdFlags)

} // namespace juce
