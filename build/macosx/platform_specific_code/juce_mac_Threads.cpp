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
#include <sys/types.h>
#include <sys/sysctl.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/threads/juce_CriticalSection.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_Thread.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/io/files/juce_File.h"

//==============================================================================
/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

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

//==============================================================================
bool JUCE_CALLTYPE juce_isRunningUnderDebugger() throw()
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

bool JUCE_CALLTYPE Process::isRunningUnderDebugger() throw()
{
    return juce_isRunningUnderDebugger();
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


END_JUCE_NAMESPACE
