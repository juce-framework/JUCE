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

#ifndef __JUCE_SYSTEMSTATS_JUCEHEADER__
#define __JUCE_SYSTEMSTATS_JUCEHEADER__


//==============================================================================
/**
    Contains methods for finding out about the current hardware and OS configuration.
*/
class JUCE_API  SystemStats
{
public:
    //==============================================================================
    /** Returns the current version of JUCE,

        (just in case you didn't already know at compile-time.)

        See also the JUCE_VERSION, JUCE_MAJOR_VERSION and JUCE_MINOR_VERSION macros.
    */
    static const String getJUCEVersion() throw();

    //==============================================================================
    /** The set of possible results of the getOperatingSystemType() method.
    */
    enum OperatingSystemType
    {
        UnknownOS   = 0,

        MacOSX      = 0x1000,
        Linux       = 0x2000,

        Win95       = 0x4001,
        Win98       = 0x4002,
        WinNT351    = 0x4103,
        WinNT40     = 0x4104,
        Win2000     = 0x4105,
        WinXP       = 0x4106,
        WinVista    = 0x4107,

        Windows     = 0x4000,   /**< To test whether any version of Windows is running,
                                     you can use the expression ((getOperatingSystemType() & Windows) != 0). */
        WindowsNT   = 0x0100,   /**< To test whether the platform is Windows NT or later (i.e. not Win95 or 98),
                                     you can use the expression ((getOperatingSystemType() & WindowsNT) != 0). */
    };

    /** Returns the type of operating system we're running on.

        @returns one of the values from the OSType enum.
        @see getOperatingSystemName
    */
    static OperatingSystemType getOperatingSystemType() throw();

    /** Returns the name of the type of operating system we're running on.

        @returns a string describing the OS type.
        @see getOperatingSystemType
    */
    static const String getOperatingSystemName() throw();

    //==============================================================================
    // CPU and memory information..

    /** Returns the approximate CPU speed.

        @returns    the speed in megahertz, e.g. 1500, 2500, 32000 (depending on
                    what year you're reading this...)
    */
    static int getCpuSpeedInMegaherz() throw();

    /** Returns a string to indicate the CPU vendor.

        Might not be known on some systems.
    */
    static const String getCpuVendor() throw();

    /** Checks whether Intel MMX instructions are available. */
    static bool hasMMX() throw();

    /** Checks whether Intel SSE instructions are available. */
    static bool hasSSE() throw();

    /** Checks whether Intel SSE2 instructions are available. */
    static bool hasSSE2() throw();

    /** Checks whether AMD 3DNOW instructions are available. */
    static bool has3DNow() throw();

    /** Returns the number of CPUs.
    */
    static int getNumCpus() throw();

    /** Returns a clock-cycle tick counter, if available.

        If the machine can do it, this will return a tick-count
        where each tick is one cpu clock cycle - used for profiling
        code.

        @returns    the tick count, or zero if not available.
    */
    static int64 getClockCycleCounter() throw();

    //==============================================================================
    /** Finds out how much RAM is in the machine.

        @returns    the approximate number of megabytes of memory, or zero if
                    something goes wrong when finding out.
    */
    static int getMemorySizeInMegabytes() throw();

    /** Returns the system page-size.

        This is only used by programmers with beards.
    */
    static int getPageSize() throw();

    //==============================================================================
    /** Returns a list of MAC addresses found on this machine.

        @param  addresses   an array into which the MAC addresses should be copied
        @param  maxNum      the number of elements in this array
        @returns            the number of MAC addresses that were found
    */
    static int getMACAddresses (int64* addresses, int maxNum) throw();


    //==============================================================================
    // not-for-public-use platform-specific method gets called at startup to initialise things.
    static void initialiseStats() throw();
};


#endif   // __JUCE_SYSTEMSTATS_JUCEHEADER__
