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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 extern HWND juce_messageWindowHandle;
#endif


//==============================================================================
CriticalSection::CriticalSection() throw()
{
    // (just to check the MS haven't changed this structure and broken things...)
#if _MSC_VER >= 1400
    static_jassert (sizeof (CRITICAL_SECTION) <= sizeof (internal));
#else
    static_jassert (sizeof (CRITICAL_SECTION) <= 24);
#endif

    InitializeCriticalSection ((CRITICAL_SECTION*) internal);
}

CriticalSection::~CriticalSection() throw()
{
    DeleteCriticalSection ((CRITICAL_SECTION*) internal);
}

void CriticalSection::enter() const throw()
{
    EnterCriticalSection ((CRITICAL_SECTION*) internal);
}

bool CriticalSection::tryEnter() const throw()
{
    return TryEnterCriticalSection ((CRITICAL_SECTION*) internal) != FALSE;
}

void CriticalSection::exit() const throw()
{
    LeaveCriticalSection ((CRITICAL_SECTION*) internal);
}

//==============================================================================
WaitableEvent::WaitableEvent() throw()
    : internal (CreateEvent (0, FALSE, FALSE, 0))
{
}

WaitableEvent::~WaitableEvent() throw()
{
    CloseHandle (internal);
}

bool WaitableEvent::wait (const int timeOutMillisecs) const throw()
{
    return WaitForSingleObject (internal, timeOutMillisecs) == WAIT_OBJECT_0;
}

void WaitableEvent::signal() const throw()
{
    SetEvent (internal);
}

void WaitableEvent::reset() const throw()
{
    ResetEvent (internal);
}

//==============================================================================
void JUCE_API juce_threadEntryPoint (void*);

static unsigned int __stdcall threadEntryProc (void* userData)
{
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
    AttachThreadInput (GetWindowThreadProcessId (juce_messageWindowHandle, 0),
                       GetCurrentThreadId(), TRUE);
#endif

    juce_threadEntryPoint (userData);

    _endthreadex (0);
    return 0;
}

void juce_CloseThreadHandle (void* handle)
{
    CloseHandle ((HANDLE) handle);
}

void* juce_createThread (void* userData)
{
    unsigned int threadId;

    return (void*) _beginthreadex (0, 0,
                                   &threadEntryProc,
                                   userData,
                                   0, &threadId);
}

void juce_killThread (void* handle)
{
    if (handle != 0)
    {
#ifdef JUCE_DEBUG
        OutputDebugString (_T("** Warning - Forced thread termination **\n"));
#endif
        TerminateThread (handle, 0);
    }
}

void juce_setCurrentThreadName (const String& name)
{
#if defined (JUCE_DEBUG) && JUCE_MSVC
    struct
    {
        DWORD dwType;
        LPCSTR szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    } info;

    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;

    #define MS_VC_EXCEPTION 0x406d1388

    __try
    {
        RaiseException (MS_VC_EXCEPTION, 0, sizeof (info) / sizeof (ULONG_PTR), (ULONG_PTR*) &info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {}
#else
    (void) name;
#endif
}

Thread::ThreadID Thread::getCurrentThreadId()
{
    return (ThreadID) (pointer_sized_int) GetCurrentThreadId();
}

// priority 1 to 10 where 5=normal, 1=low
bool juce_setThreadPriority (void* threadHandle, int priority)
{
    int pri = THREAD_PRIORITY_TIME_CRITICAL;

    if (priority < 1)
        pri = THREAD_PRIORITY_IDLE;
    else if (priority < 2)
        pri = THREAD_PRIORITY_LOWEST;
    else if (priority < 5)
        pri = THREAD_PRIORITY_BELOW_NORMAL;
    else if (priority < 7)
        pri = THREAD_PRIORITY_NORMAL;
    else if (priority < 9)
        pri = THREAD_PRIORITY_ABOVE_NORMAL;
    else if (priority < 10)
        pri = THREAD_PRIORITY_HIGHEST;

    if (threadHandle == 0)
        threadHandle = GetCurrentThread();

    return SetThreadPriority (threadHandle, pri) != FALSE;
}

void Thread::setCurrentThreadAffinityMask (const uint32 affinityMask)
{
    SetThreadAffinityMask (GetCurrentThread(), affinityMask);
}

static HANDLE sleepEvent = 0;

void juce_initialiseThreadEvents()
{
    if (sleepEvent == 0)
#ifdef JUCE_DEBUG
        sleepEvent = CreateEvent (0, 0, 0, _T("Juce Sleep Event"));
#else
        sleepEvent = CreateEvent (0, 0, 0, 0);
#endif
}

void Thread::yield()
{
    Sleep (0);
}

void JUCE_CALLTYPE Thread::sleep (const int millisecs)
{
    if (millisecs >= 10)
    {
        Sleep (millisecs);
    }
    else
    {
        jassert (sleepEvent != 0);

        // unlike Sleep() this is guaranteed to return to the current thread after
        // the time expires, so we'll use this for short waits, which are more likely
        // to need to be accurate
        WaitForSingleObject (sleepEvent, millisecs);
    }
}

//==============================================================================
static int lastProcessPriority = -1;

// called by WindowDriver because Windows does wierd things to process priority
// when you swap apps, and this forces an update when the app is brought to the front.
void juce_repeatLastProcessPriority()
{
    if (lastProcessPriority >= 0) // (avoid changing this if it's not been explicitly set by the app..)
    {
        DWORD p;

        switch (lastProcessPriority)
        {
        case Process::LowPriority:
            p = IDLE_PRIORITY_CLASS;
            break;

        case Process::NormalPriority:
            p = NORMAL_PRIORITY_CLASS;
            break;

        case Process::HighPriority:
            p = HIGH_PRIORITY_CLASS;
            break;

        case Process::RealtimePriority:
            p = REALTIME_PRIORITY_CLASS;
            break;

        default:
            jassertfalse // bad priority value
            return;
        }

        SetPriorityClass (GetCurrentProcess(), p);
    }
}

void Process::setPriority (ProcessPriority prior)
{
    if (lastProcessPriority != (int) prior)
    {
        lastProcessPriority = (int) prior;
        juce_repeatLastProcessPriority();
    }
}

bool JUCE_PUBLIC_FUNCTION juce_isRunningUnderDebugger()
{
    return IsDebuggerPresent() != FALSE;
}

bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
{
    return juce_isRunningUnderDebugger();
}


//==============================================================================
void Process::raisePrivilege()
{
    jassertfalse // xxx not implemented
}

void Process::lowerPrivilege()
{
    jassertfalse // xxx not implemented
}

void Process::terminate()
{
#if defined (JUCE_DEBUG) && JUCE_MSVC && JUCE_CHECK_MEMORY_LEAKS
    _CrtDumpMemoryLeaks();
#endif

    // bullet in the head in case there's a problem shutting down..
    ExitProcess (0);
}

//==============================================================================
void* PlatformUtilities::loadDynamicLibrary (const String& name)
{
    void* result = 0;

    JUCE_TRY
    {
        result = (void*) LoadLibrary (name);
    }
    JUCE_CATCH_ALL

    return result;
}

void PlatformUtilities::freeDynamicLibrary (void* h)
{
    JUCE_TRY
    {
        if (h != 0)
            FreeLibrary ((HMODULE) h);
    }
    JUCE_CATCH_ALL
}

void* PlatformUtilities::getProcedureEntryPoint (void* h, const String& name)
{
    return (h != 0) ? (void*) GetProcAddress ((HMODULE) h, name)
                    : 0;
}


//==============================================================================
InterProcessLock::InterProcessLock (const String& name_)
    : internal (0),
      name (name_),
      reentrancyLevel (0)
{
}

InterProcessLock::~InterProcessLock()
{
    exit();
}

bool InterProcessLock::enter (const int timeOutMillisecs)
{
    if (reentrancyLevel++ == 0)
    {
        internal = CreateMutex (0, TRUE, "Global\\" + name);

        if (internal != 0 && GetLastError() == ERROR_ALREADY_EXISTS)
        {
            if (timeOutMillisecs == 0
                 || WaitForSingleObject (internal, (timeOutMillisecs < 0) ? INFINITE : timeOutMillisecs)
                       == WAIT_TIMEOUT)
            {
                ReleaseMutex (internal);
                CloseHandle (internal);
                internal = 0;
            }
        }
    }

    return (internal != 0);
}

void InterProcessLock::exit()
{
    if (--reentrancyLevel == 0 && internal != 0)
    {
        ReleaseMutex (internal);
        CloseHandle (internal);
        internal = 0;
    }
}


#endif
