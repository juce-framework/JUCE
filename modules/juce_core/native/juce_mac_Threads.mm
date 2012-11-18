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

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
bool Process::isForegroundProcess()
{
   #if JUCE_MAC
    return [NSApp isActive];
   #else
    return true; // xxx change this if more than one app is ever possible on iOS!
   #endif
}

void Process::makeForegroundProcess()
{
   #if JUCE_MAC
    [NSApp activateIgnoringOtherApps: YES];
   #endif
}

void Process::raisePrivilege()
{
    jassertfalse;
}

void Process::lowerPrivilege()
{
    jassertfalse;
}

void Process::terminate()
{
    exit (0);
}

void Process::setPriority (ProcessPriority)
{
    // xxx
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger()
{
    static char testResult = 0;

    if (testResult == 0)
    {
        struct kinfo_proc info;
        int m[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
        size_t sz = sizeof (info);
        sysctl (m, 4, &info, &sz, 0, 0);
        testResult = ((info.kp_proc.p_flag & P_TRACED) != 0) ? 1 : -1;
    }

    return testResult > 0;
}

JUCE_API bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
{
    return juce_isRunningUnderDebugger();
}
