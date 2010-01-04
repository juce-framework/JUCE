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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
void JUCE_API juce_threadEntryPoint (void*);

void* threadEntryProc (void* userData)
{
    const ScopedAutoReleasePool pool;
    juce_threadEntryPoint (userData);
    return 0;
}

void* juce_createThread (void* userData)
{
    pthread_t handle = 0;

    if (pthread_create (&handle, 0, threadEntryProc, userData) == 0)
    {
        pthread_detach (handle);
        return (void*) handle;
    }

    return 0;
}

void juce_killThread (void* handle)
{
    if (handle != 0)
        pthread_cancel ((pthread_t) handle);
}

void juce_setCurrentThreadName (const String& /*name*/)
{
}

Thread::ThreadID Thread::getCurrentThreadId()
{
    return (ThreadID) pthread_self();
}

bool juce_setThreadPriority (void* handle, int priority)
{
    if (handle == 0)
        handle = (void*) pthread_self();

    struct sched_param param;
    int policy;
    pthread_getschedparam ((pthread_t) handle, &policy, &param);
    param.sched_priority = jlimit (1, 127, 1 + (priority * 126) / 11);
    return pthread_setschedparam ((pthread_t) handle, policy, &param) == 0;
}

void Thread::yield()
{
    sched_yield();
}

void Thread::setCurrentThreadAffinityMask (const uint32 affinityMask)
{
    // xxx
    jassertfalse
}

//==============================================================================
bool Process::isForegroundProcess()
{
#if JUCE_MAC
    return [NSApp isActive];
#else
    return true; // xxx change this if more than one app is ever possible on the iPhone!
#endif
}

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
    exit (0);
}

void Process::setPriority (ProcessPriority p)
{
    // xxx
}

#endif
