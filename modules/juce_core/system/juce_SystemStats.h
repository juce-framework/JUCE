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

#ifndef JUCE_SYSTEMSTATS_H_INCLUDED
#define JUCE_SYSTEMSTATS_H_INCLUDED


//==============================================================================
/**
    Contains methods for finding out about the current hardware and OS configuration.
*/
class JUCE_API  SystemStats
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

        MacOSX_10_4     = MacOSX | 4,
        MacOSX_10_5     = MacOSX | 5,
        MacOSX_10_6     = MacOSX | 6,
        MacOSX_10_7     = MacOSX | 7,
        MacOSX_10_8     = MacOSX | 8,
        MacOSX_10_9     = MacOSX | 9,
        MacOSX_10_10    = MacOSX | 10,
        MacOSX_10_11    = MacOSX | 11,
        MacOSX_10_12    = MacOSX | 12,

        Win2000         = Windows | 1,
        WinXP           = Windows | 2,
        WinVista        = Windows | 3,
        Windows7        = Windows | 4,
        Windows8_0      = Windows | 5,
        Windows8_1      = Windows | 6,
        Windows10       = Windows | 7
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

    //==============================================================================
    // CPU and memory information..

    /** Returns the number of CPU cores. */
    static int getNumCpus() noexcept;

    /** Returns the approximate CPU speed.
        @returns    the speed in megahertz, e.g. 1500, 2500, 32000 (depending on
                    what year you're reading this...)
    */
    static int getCpuSpeedInMegaherz();

    /** Returns a string to indicate the CPU vendor.
        Might not be known on some systems.
    */
    static String getCpuVendor();

    static bool hasMMX() noexcept;    /**< Returns true if Intel MMX instructions are available. */
    static bool has3DNow() noexcept;  /**< Returns true if AMD 3DNOW instructions are available. */
    static bool hasSSE() noexcept;    /**< Returns true if Intel SSE instructions are available. */
    static bool hasSSE2() noexcept;   /**< Returns true if Intel SSE2 instructions are available. */
    static bool hasSSE3() noexcept;   /**< Returns true if Intel SSE3 instructions are available. */
    static bool hasSSSE3() noexcept;  /**< Returns true if Intel SSSE3 instructions are available. */
    static bool hasSSE41() noexcept;  /**< Returns true if Intel SSE4.1 instructions are available. */
    static bool hasSSE42() noexcept;  /**< Returns true if Intel SSE4.2 instructions are available. */
    static bool hasAVX() noexcept;    /**< Returns true if Intel AVX instructions are available. */
    static bool hasAVX2() noexcept;   /**< Returns true if Intel AVX2 instructions are available. */

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

    /** A void() function type, used by setApplicationCrashHandler(). */
    typedef void (*CrashHandlerFunction)();

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

private:
    //==============================================================================
    SystemStats();

    JUCE_DECLARE_NON_COPYABLE (SystemStats)
};


#endif   // JUCE_SYSTEMSTATS_H_INCLUDED
