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

#ifndef __JUCE_SYSTEMSTATS_JUCEHEADER__
#define __JUCE_SYSTEMSTATS_JUCEHEADER__

#include "../text/juce_StringArray.h"


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
    /** The set of possible results of the getOperatingSystemType() method.
    */
    enum OperatingSystemType
    {
        UnknownOS   = 0,

        MacOSX      = 0x1000,
        Linux       = 0x2000,
        Android     = 0x3000,

        Win95       = 0x4001,
        Win98       = 0x4002,
        WinNT351    = 0x4103,
        WinNT40     = 0x4104,
        Win2000     = 0x4105,
        WinXP       = 0x4106,
        WinVista    = 0x4107,
        Windows7    = 0x4108,

        Windows     = 0x4000,   /**< To test whether any version of Windows is running,
                                     you can use the expression ((getOperatingSystemType() & Windows) != 0). */
        WindowsNT   = 0x0100,   /**< To test whether the platform is Windows NT or later (i.e. not Win95 or 98),
                                     you can use the expression ((getOperatingSystemType() & WindowsNT) != 0). */
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

    /** Returns true if the OS is 64-bit, or false for a 32-bit OS.
    */
    static bool isOperatingSystem64Bit();

   #if JUCE_MAC || DOXYGEN
    /** OSX ONLY - Returns the current OS version number.
        E.g. if it's running on 10.4, this will be 4, 10.5 will return 5, etc.
    */
    static int getOSXMinorVersionNumber();
   #endif

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

    /** Returns the user's language.
        The return value is a 2 or 3 letter language code (ISO 639-1 or ISO 639-2)
    */
    static String getUserLanguage();

    /** Returns the user's region.
        The return value is a 2 letter country code (ISO 3166-1 alpha-2).
    */
    static String getUserRegion();

    //==============================================================================
    // CPU and memory information..

    /** Returns the number of CPUs. */
    static int getNumCpus() noexcept            { return getCPUFlags().numCpus; }

    /** Returns the approximate CPU speed.

        @returns    the speed in megahertz, e.g. 1500, 2500, 32000 (depending on
                    what year you're reading this...)
    */
    static int getCpuSpeedInMegaherz();

    /** Returns a string to indicate the CPU vendor.

        Might not be known on some systems.
    */
    static String getCpuVendor();

    /** Checks whether Intel MMX instructions are available. */
    static bool hasMMX() noexcept               { return getCPUFlags().hasMMX; }

    /** Checks whether Intel SSE instructions are available. */
    static bool hasSSE() noexcept               { return getCPUFlags().hasSSE; }

    /** Checks whether Intel SSE2 instructions are available. */
    static bool hasSSE2() noexcept              { return getCPUFlags().hasSSE2; }

    /** Checks whether AMD 3DNOW instructions are available. */
    static bool has3DNow() noexcept             { return getCPUFlags().has3DNow; }

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


private:
    //==============================================================================
    struct CPUFlags
    {
        CPUFlags();

        int numCpus;
        bool hasMMX : 1;
        bool hasSSE : 1;
        bool hasSSE2 : 1;
        bool has3DNow : 1;
    };

    SystemStats();
    static const CPUFlags& getCPUFlags();

    JUCE_DECLARE_NON_COPYABLE (SystemStats);
};


#endif   // __JUCE_SYSTEMSTATS_JUCEHEADER__
