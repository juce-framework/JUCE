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

#include "../../../src/juce_core/basics/juce_StandardHeader.h"
#include <pthread.h>
#include <sched.h>
#include <sys/file.h>
#include <Carbon/Carbon.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/threads/juce_CriticalSection.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_core/threads/juce_InterProcessLock.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/io/files/juce_File.h"


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
    EventStruct* const es = (EventStruct*) internal;

    pthread_cond_destroy (&es->condition);
    pthread_mutex_destroy (&es->mutex);

    delete es;
}

bool WaitableEvent::wait (const int timeOutMillisecs) const throw()
{
    EventStruct* const es = (EventStruct*) internal;

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
            time.tv_sec = timeOutMillisecs / 1000;
            time.tv_nsec = (timeOutMillisecs % 1000) * 1000000;
            pthread_cond_timedwait_relative_np (&es->condition, &es->mutex, &time);
        }

        ok = es->triggered;
    }

    es->triggered = false;

    pthread_mutex_unlock (&es->mutex);
    return ok;
}

void WaitableEvent::signal() const throw()
{
    EventStruct* const es = (EventStruct*) internal;

    pthread_mutex_lock (&es->mutex);
    es->triggered = true;
    pthread_cond_signal (&es->condition);
    pthread_mutex_unlock (&es->mutex);
}

void WaitableEvent::reset() const throw()
{
    EventStruct* const es = (EventStruct*) internal;

    pthread_mutex_lock (&es->mutex);
    es->triggered = false;
    pthread_mutex_unlock (&es->mutex);
}

//==============================================================================
void JUCE_API juce_threadEntryPoint (void*);

void* threadEntryProc (void* userData) throw()
{
    juce_threadEntryPoint (userData);
    return 0;
}

void* juce_createThread (void* userData) throw()
{
    pthread_t handle = 0;

    if (pthread_create (&handle, 0, threadEntryProc, userData) == 0)
    {
        pthread_detach (handle);
        return (void*) handle;
    }

    return 0;
}

void juce_killThread (void* handle) throw()
{
    if (handle != 0)
        pthread_cancel ((pthread_t) handle);
}

void juce_setCurrentThreadName (const String& /*name*/) throw()
{
}

int Thread::getCurrentThreadId() throw()
{
    return (int) pthread_self();
}

void juce_setThreadPriority (void* handle, int priority) throw()
{
    if (handle == 0)
        handle = (void*) pthread_self();

    struct sched_param param;
    int policy;
    pthread_getschedparam ((pthread_t) handle, &policy, &param);
    param.sched_priority = jlimit (1, 127, 1 + (priority * 126) / 11);
    pthread_setschedparam ((pthread_t) handle, policy, &param);
}

void Thread::yield() throw()
{
    sched_yield();
}

void Thread::setCurrentThreadAffinityMask (const uint32 affinityMask) throw()
{
    // xxx
    jassertfalse
}

void JUCE_CALLTYPE Thread::sleep (int millisecs) throw()
{
    struct timespec time;
    time.tv_sec = millisecs / 1000;
    time.tv_nsec = (millisecs % 1000) * 1000000;
    nanosleep (&time, 0);
}


//==============================================================================
void Process::raisePrivilege()
{
    jassertfalse
}

void Process::lowerPrivilege()
{
    jassertfalse
}

void Process::terminate()
{
    ExitToShell();
}

void Process::setPriority (ProcessPriority p)
{
    // xxx
}

void* Process::loadDynamicLibrary (const String& name)
{
    // xxx needs to use bundles

    FSSpec fs;
    if (PlatformUtilities::makeFSSpecFromPath (&fs, name))
    {
        CFragConnectionID connID;
        Ptr mainPtr;
        Str255 errorMessage;
        Str63 nm;
        PlatformUtilities::copyToStr63 (nm, name);

        const OSErr err = GetDiskFragment (&fs, 0, kCFragGoesToEOF, nm, kReferenceCFrag, &connID, &mainPtr, errorMessage);
        if (err == noErr)
            return (void*)connID;
    }

    return 0;
}

void Process::freeDynamicLibrary (void* handle)
{
    if (handle != 0)
        CloseConnection ((CFragConnectionID*)&handle);
}

void* Process::getProcedureEntryPoint (void* h, const String& procedureName)
{
    if (h != 0)
    {
        CFragSymbolClass cl;
        Ptr ptr;
        Str255 name;
        PlatformUtilities::copyToStr255 (name, procedureName);

        if (FindSymbol ((CFragConnectionID) h, name, &ptr, &cl) == noErr)
        {
            return ptr;
        }
    }

    return 0;
}

//==============================================================================
InterProcessLock::InterProcessLock (const String& name_) throw()
    : internal (0),
      name (name_),
      reentrancyLevel (0)
{
    const File tempDir (File::getSpecialLocation (File::tempDirectory));
    const File temp (tempDir.getChildFile (name));
    temp.create();

    internal = (void*) open (temp.getFullPathName().toUTF8(), O_NONBLOCK | O_RDONLY);
}

InterProcessLock::~InterProcessLock() throw()
{
    while (reentrancyLevel > 0)
        this->exit();

    close ((int) internal);
}

bool InterProcessLock::enter (const int timeOutMillisecs) throw()
{
    if (internal == 0)
        return false;

    if (reentrancyLevel != 0)
        return true;

    if (timeOutMillisecs <= 0)
    {
        if (flock ((int) internal,
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
            if (flock ((int) internal, LOCK_EX | LOCK_NB) == 0)
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

        const int result = flock ((int) internal, LOCK_UN);
        (void) result;
        jassert (result == 0);
    }
}

END_JUCE_NAMESPACE
