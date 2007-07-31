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
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/threads/juce_CriticalSection.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_InterProcessLock.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_core/io/files/juce_File.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"

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

int Thread::getCurrentThreadId() throw()
{
    return (int) pthread_self();
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

void JUCE_CALLTYPE Thread::sleep (int millisecs) throw()
{
    struct timespec time;
    time.tv_sec = millisecs / 1000;
    time.tv_nsec = (millisecs % 1000) * 1000000;
    nanosleep (&time, 0);
}

//==============================================================================
CriticalSection::CriticalSection() throw()
{
    pthread_mutexattr_t atts;
    pthread_mutexattr_init (&atts);
    pthread_mutexattr_settype (&atts, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init (&internal, &atts);
}

CriticalSection::~CriticalSection() throw()
{
    pthread_mutex_destroy (&internal);
}

void CriticalSection::enter() const throw()
{
    pthread_mutex_lock (&internal);
}

bool CriticalSection::tryEnter() const throw()
{
    return pthread_mutex_trylock (&internal) == 0;
}

void CriticalSection::exit() const throw()
{
    pthread_mutex_unlock (&internal);
}

//==============================================================================
struct EventStruct
{
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    bool triggered;
};

WaitableEvent::WaitableEvent() throw()
{
    EventStruct* const es = new EventStruct();
    es->triggered = false;

    pthread_cond_init (&es->condition, 0);
    pthread_mutex_init (&es->mutex, 0);

    internal = es;
}

WaitableEvent::~WaitableEvent() throw()
{
    EventStruct* const es = (EventStruct*)internal;

    pthread_cond_destroy (&es->condition);
    pthread_mutex_destroy (&es->mutex);

    delete es;
}

bool WaitableEvent::wait (const int timeOutMillisecs) const throw()
{
    EventStruct* const es = (EventStruct*)internal;

    bool ok = true;
    pthread_mutex_lock (&es->mutex);

    if (! es->triggered)
    {
        if (timeOutMillisecs < 0)
        {
            pthread_cond_wait (&es->condition, &es->mutex);
        }
        else
        {
            struct timespec time;
            struct timeval t;
            int timeout = 0;

            gettimeofday (&t, 0);

            time.tv_sec  = t.tv_sec  + (timeOutMillisecs / 1000);
            time.tv_nsec = (t.tv_usec + ((timeOutMillisecs % 1000) * 1000)) * 1000;

            while (time.tv_nsec >= 1000000000)
            {
                time.tv_nsec -= 1000000000;
                time.tv_sec++;
            }

            while (! timeout)
            {
                timeout = pthread_cond_timedwait (&es->condition, &es->mutex, &time);

                if (! timeout)
                    // Success
                    break;

                if (timeout == EINTR)
                    // Go round again
                    timeout = 0;
            }
        }

        ok = es->triggered;
    }

    es->triggered = false;

    pthread_mutex_unlock (&es->mutex);
    return ok;
}

void WaitableEvent::signal() const throw()
{
    EventStruct* const es = (EventStruct*)internal;

    pthread_mutex_lock (&es->mutex);
    es->triggered = true;
    pthread_cond_signal (&es->condition);
    pthread_mutex_unlock (&es->mutex);
}

void WaitableEvent::reset() const throw()
{
    EventStruct* const es = (EventStruct*)internal;

    pthread_mutex_lock (&es->mutex);
    es->triggered = false;
    pthread_mutex_unlock (&es->mutex);
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

#if JUCE_BUILD_GUI_CLASSES
void* Process::loadDynamicLibrary (const String& name)
{
    return dlopen ((const char*) name.toUTF8(), RTLD_LOCAL | RTLD_NOW);
}

void Process::freeDynamicLibrary (void* handle)
{
    dlclose(handle);
}

void* Process::getProcedureEntryPoint (void* libraryHandle, const String& procedureName)
{
    return dlsym (libraryHandle, (const char*) procedureName);
}
#endif


//==============================================================================
InterProcessLock::InterProcessLock (const String& name_) throw()
    : internal (0),
      name (name_),
      reentrancyLevel (0)
{
    const File tempDir (File::getSpecialLocation (File::tempDirectory));
    const File temp (tempDir.getChildFile (name));
    temp.create();

    internal = (void*) open (temp.getFullPathName().toUTF8(), 'a');
}

InterProcessLock::~InterProcessLock() throw()
{
    while (reentrancyLevel > 0)
        this->exit();

#if JUCE_64BIT
    close ((long long) internal);
#else
    close ((int) internal);
#endif
}

bool InterProcessLock::enter (const int timeOutMillisecs) throw()
{
    if (internal == 0)
        return false;

    if (reentrancyLevel != 0)
        return true;

    if (timeOutMillisecs <= 0)
    {
        if (flock ((long) internal,
                   timeOutMillisecs < 0 ? LOCK_EX
                                        : (LOCK_EX | LOCK_NB)) == 0)
        {
            ++reentrancyLevel;
            return true;
        }
    }
    else
    {
        const int64 endTime = Time::currentTimeMillis() + timeOutMillisecs;

        for (;;)
        {
            if (flock ((long) internal, LOCK_EX | LOCK_NB) == 0)
            {
                ++reentrancyLevel;
                return true;
            }

            if (Time::currentTimeMillis() >= endTime)
                break;

            Thread::sleep (10);
        }
    }

    return false;
}

void InterProcessLock::exit() throw()
{
    if (reentrancyLevel > 0 && internal != 0)
    {
        --reentrancyLevel;

        const int result = flock ((long) internal, LOCK_UN);
        (void) result;
        jassert (result == 0);
    }
}


END_JUCE_NAMESPACE
