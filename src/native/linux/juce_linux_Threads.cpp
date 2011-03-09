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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
// sets the process to 0=low priority, 1=normal, 2=high, 3=realtime
void Process::setPriority (ProcessPriority prior)
{
    struct sched_param param;
    int policy, maxp, minp;

    const int p = (int) prior;

    if (p <= 1)
        policy = SCHED_OTHER;
    else
        policy = SCHED_RR;

    minp = sched_get_priority_min (policy);
    maxp = sched_get_priority_max (policy);

    if (p < 2)
        param.sched_priority = 0;
    else if (p == 2 )
        // Set to middle of lower realtime priority range
        param.sched_priority = minp + (maxp - minp) / 4;
    else
        // Set to middle of higher realtime priority range
        param.sched_priority = minp + (3 * (maxp - minp) / 4);

    pthread_setschedparam (pthread_self(), policy, &param);
}

void Process::terminate()
{
    exit (0);
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger()
{
    static char testResult = 0;

    if (testResult == 0)
    {
        testResult = (char) ptrace (PT_TRACE_ME, 0, 0, 0);

        if (testResult >= 0)
        {
            ptrace (PT_DETACH, 0, (caddr_t) 1, 0);
            testResult = 1;
        }
    }

    return testResult < 0;
}

JUCE_API bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
{
    return juce_isRunningUnderDebugger();
}

void Process::raisePrivilege()
{
    // If running suid root, change effective user
    // to root
    if (geteuid() != 0 && getuid() == 0)
    {
        setreuid (geteuid(), getuid());
        setregid (getegid(), getgid());
    }
}

void Process::lowerPrivilege()
{
    // If runing suid root, change effective user
    // back to real user
    if (geteuid() == 0 && getuid() != 0)
    {
        setreuid (geteuid(), getuid());
        setregid (getegid(), getgid());
    }
}

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

void* PlatformUtilities::loadDynamicLibrary (const String& name)
{
    return dlopen (name.toUTF8(), RTLD_LOCAL | RTLD_NOW);
}

void PlatformUtilities::freeDynamicLibrary (void* handle)
{
    dlclose(handle);
}

void* PlatformUtilities::getProcedureEntryPoint (void* libraryHandle, const String& procedureName)
{
    return dlsym (libraryHandle, procedureName.toUTF8());
}

#endif

#endif
