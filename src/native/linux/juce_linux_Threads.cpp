/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
void JUCE_API juce_threadEntryPoint (void*);

void* threadEntryProc (void* value)
{
    // New threads start off as root when running suid
    Process::lowerPrivilege();

    juce_threadEntryPoint (value);
    return 0;
}

void* juce_createThread (void* userData)
{
    pthread_t handle = 0;

    if (pthread_create (&handle, 0, threadEntryProc, userData) == 0)
    {
        pthread_detach (handle);
        return (void*)handle;
    }

    return 0;
}

void juce_killThread (void* handle)
{
    if (handle != 0)
        pthread_cancel ((pthread_t)handle);
}

void juce_setCurrentThreadName (const String& /*name*/)
{
}

Thread::ThreadID Thread::getCurrentThreadId()
{
    return (ThreadID) pthread_self();
}

/*
 * This is all a bit non-ideal... the trouble is that on Linux you
 * need to call setpriority to affect the dynamic priority for
 * non-realtime processes, but this requires the pid, which is not
 * accessible from the pthread_t.  We could get it by calling getpid
 * once each thread has started, but then we would need a list of
 * running threads etc etc.
 * Also there is no such thing as IDLE priority on Linux.
 * For the moment, map idle, low and normal process priorities to
 * SCHED_OTHER, with the thread priority ignored for these classes.
 * Map high priority processes to the lower half of the SCHED_RR
 * range, and realtime to the upper half
 */

// priority 1 to 10 where 5=normal, 1=low. If the handle is 0, sets the
// priority of the current thread
bool juce_setThreadPriority (void* handle, int priority)
{
    struct sched_param param;
    int policy, maxp, minp, pri;

    if (handle == 0)
        handle = (void*) pthread_self();

    if (pthread_getschedparam ((pthread_t) handle, &policy, &param) == 0
         && policy != SCHED_OTHER)
    {
        minp = sched_get_priority_min(policy);
        maxp = sched_get_priority_max(policy);

        pri = ((maxp - minp) / 2) * (priority - 1) / 9;

        if (param.__sched_priority >= (minp + (maxp - minp) / 2))
            // Realtime process priority
            param.__sched_priority = minp + ((maxp - minp) / 2) + pri;
        else
            // High process priority
            param.__sched_priority = minp + pri;

        param.sched_priority = jlimit (1, 127, 1 + (priority * 126) / 11);

        return pthread_setschedparam ((pthread_t) handle, policy, &param) == 0;
    }

    return false;
}

void Thread::setCurrentThreadAffinityMask (const uint32 affinityMask)
{
#if SUPPORT_AFFINITIES
    cpu_set_t affinity;
    CPU_ZERO (&affinity);

    for (int i = 0; i < 32; ++i)
        if ((affinityMask & (1 << i)) != 0)
            CPU_SET (i, &affinity);

    /*
       N.B. If this line causes a compile error, then you've probably not got the latest
       version of glibc installed.

       If you don't want to update your copy of glibc and don't care about cpu affinities,
       then you can just disable all this stuff by removing the SUPPORT_AFFINITIES macro
       from the linuxincludes.h file.
    */
    sched_setaffinity (getpid(), sizeof (cpu_set_t), &affinity);
    sched_yield();

#else
    /* affinities aren't supported because either the appropriate header files weren't found,
       or the SUPPORT_AFFINITIES macro was turned off in linuxheaders.h
    */
    jassertfalse
#endif
}

void Thread::yield()
{
    sched_yield();
}


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
        param.__sched_priority = 0;
    else if (p == 2 )
        // Set to middle of lower realtime priority range
        param.__sched_priority = minp + (maxp - minp) / 4;
    else
        // Set to middle of higher realtime priority range
        param.__sched_priority = minp + (3 * (maxp - minp) / 4);

    pthread_setschedparam (pthread_self(), policy, &param);
}

void Process::terminate()
{
    exit (0);
}

bool JUCE_PUBLIC_FUNCTION juce_isRunningUnderDebugger()
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

bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
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
    return dlopen ((const char*) name.toUTF8(), RTLD_LOCAL | RTLD_NOW);
}

void PlatformUtilities::freeDynamicLibrary (void* handle)
{
    dlclose(handle);
}

void* PlatformUtilities::getProcedureEntryPoint (void* libraryHandle, const String& procedureName)
{
    return dlsym (libraryHandle, (const char*) procedureName);
}

#endif

#endif
