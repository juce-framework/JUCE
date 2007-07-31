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

#ifndef __JUCE_PROCESS_JUCEHEADER__
#define __JUCE_PROCESS_JUCEHEADER__

#include "../text/juce_String.h"


//==============================================================================
/** Represents the current executable's process.

    This contains methods for controlling the current application at the
    process-level.

    @see Thread, JUCEApplication
*/
class JUCE_API  Process
{
public:
    //==============================================================================
    enum ProcessPriority
    {
        LowPriority         = 0,
        NormalPriority      = 1,
        HighPriority        = 2,
        RealtimePriority    = 3
    };

    /** Changes the current process's priority.

        @param priority     the process priority, where
                            0=low, 1=normal, 2=high, 3=realtime
    */
    static void setPriority (const ProcessPriority priority);

    /** Kills the current process immediately.

        This is an emergency process terminator that kills the application
        immediately - it's intended only for use only when something goes
        horribly wrong.

        @see JUCEApplication::quit
    */
    static void terminate();

    //==============================================================================
    /** Returns true if this application process is the one that the user is
        currently using.
    */
    static bool isForegroundProcess() throw();

    //==============================================================================
    /** Raises the current process's privilege level.

        Does nothing if this isn't supported by the current OS, or if process
        privilege level is fixed.
    */
    static void raisePrivilege();

    /** Lowers the current process's privilege level.

        Does nothing if this isn't supported by the current OS, or if process
        privilege level is fixed.
    */
    static void lowerPrivilege();

    //==============================================================================
    /** Loads a dynamically-linked library into the process's address space.

        @param pathOrFilename   the platform-dependent name and search path
        @returns                a handle which can be used by getProcedureEntryPoint(), or
                                zero if it fails.
        @see freeDynamicLibrary, getProcedureEntryPoint
    */
    static void* loadDynamicLibrary (const String& pathOrFilename);

    /** Frees a dynamically-linked library.

        @param libraryHandle   a handle created by loadDynamicLibrary
        @see loadDynamicLibrary, getProcedureEntryPoint
    */
    static void freeDynamicLibrary (void* libraryHandle);

    /** Finds a procedure call in a dynamically-linked library.

        @param libraryHandle    a library handle returned by loadDynamicLibrary
        @param procedureName    the name of the procedure call to try to load
        @returns                a pointer to the function if found, or 0 if it fails
        @see loadDynamicLibrary
    */
    static void* getProcedureEntryPoint (void* libraryHandle,
                                         const String& procedureName);

};


#endif   // __JUCE_PROCESS_JUCEHEADER__
