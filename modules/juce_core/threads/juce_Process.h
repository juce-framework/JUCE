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
    static bool isForegroundProcess();

    /** Attempts to make the current process the active one.
        (This is not possible on some platforms).
    */
    static void makeForegroundProcess();

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
    /** Returns true if this process is being hosted by a debugger. */
    static bool JUCE_CALLTYPE isRunningUnderDebugger();


    //==============================================================================
    /** Tries to launch the OS's default reader application for a given file or URL. */
    static bool openDocument (const String& documentURL, const String& parameters);

    /** Tries to launch the OS's default email application to let the user create a message. */
    static bool openEmailWithAttachments (const String& targetEmailAddress,
                                          const String& emailSubject,
                                          const String& bodyText,
                                          const StringArray& filesToAttach);

   #if JUCE_WINDOWS || DOXYGEN
    //==============================================================================
    /** WINDOWS ONLY - This returns the HINSTANCE of the current module.

        The return type is a void* to avoid being dependent on windows.h - just cast
        it to a HINSTANCE to use it.

        In a normal JUCE application, this will be automatically set to the module
        handle of the executable.

        If you've built a DLL and plan to use any JUCE messaging or windowing classes,
        you'll need to make sure you call the setCurrentModuleInstanceHandle()
        to provide the correct module handle in your DllMain() function, because
        the system relies on the correct instance handle when opening windows.
    */
    static void* JUCE_CALLTYPE getCurrentModuleInstanceHandle() noexcept;

    /** WINDOWS ONLY - Sets a new module handle to be used by the library.

        The parameter type is a void* to avoid being dependent on windows.h, but it actually
        expects a HINSTANCE value.

        @see getCurrentModuleInstanceHandle()
    */
    static void JUCE_CALLTYPE setCurrentModuleInstanceHandle (void* newHandle) noexcept;
   #endif

   #if JUCE_MAC || DOXYGEN
    //==============================================================================
    /** OSX ONLY - Shows or hides the OSX dock icon for this app. */
    static void setDockIconVisible (bool isVisible);
   #endif

private:
    Process();
    JUCE_DECLARE_NON_COPYABLE (Process)
};


#endif   // __JUCE_PROCESS_JUCEHEADER__
