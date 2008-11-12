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

#include "linuxincludes.h"
#include <dlfcn.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/threads/juce_CriticalSection.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_core/io/files/juce_File.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"

//==============================================================================
/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

#ifndef CPU_ISSET
  #undef SUPPORT_AFFINITIES
#endif

//==============================================================================
void JUCE_API juce_threadEntryPoint (void*);

void* threadEntryProc (void* value) throw()
{
    // New threads start off as root when running suid
    Process::lowerPrivilege();

    juce_threadEntryPoint (value);
    return 0;
}

void* juce_createThread (void* userData) throw()
{
    pthread_t handle = 0;

    if (pthread_create (&handle, 0, threadEntryProc, userData) == 0)
    {
        pthread_detach (handle);
        return (void*)handle;
    }

    return 0;
}

void juce_killThread (void* handle) throw()
{
    if (handle != 0)
        pthread_cancel ((pthread_t)handle);
}

void juce_setCurrentThreadName (const String& /*name*/) throw()
{
}

int64 Thread::getCurrentThreadId() throw()
{
    return pthread_self();
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
void juce_setThreadPriority (void* handle, int priority) throw()
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

        pthread_setschedparam ((pthread_t) handle, policy, &param);
    }
}

void Thread::setCurrentThreadAffinityMask (const uint32 affinityMask) throw()
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

void Thread::yield() throw()
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

bool JUCE_CALLTYPE juce_isRunningUnderDebugger() throw()
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

bool JUCE_CALLTYPE Process::isRunningUnderDebugger() throw()
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

END_JUCE_NAMESPACE
