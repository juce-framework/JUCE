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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

//==============================================================================
/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
void JUCE_API juce_threadEntryPoint (void*);

void* threadEntryProc (void* userData) throw()
{
    const ScopedAutoReleasePool pool;
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

Thread::ThreadID Thread::getCurrentThreadId() throw()
{
    return (ThreadID) pthread_self();
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

//==============================================================================
bool Process::isForegroundProcess() throw()
{
    return [NSApp isActive];
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
