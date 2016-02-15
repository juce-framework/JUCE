/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

HWND juce_messageWindowHandle = 0;  // (this is used by other parts of the codebase)

void* getUser32Function (const char* functionName)
{
    HMODULE module = GetModuleHandleA ("user32.dll");
    jassert (module != 0);
    return (void*) GetProcAddress (module, functionName);
}

//==============================================================================
#if ! JUCE_USE_MSVC_INTRINSICS
// In newer compilers, the inline versions of these are used (in juce_Atomic.h), but in
// older ones we have to actually call the ops as win32 functions..
long juce_InterlockedExchange (volatile long* a, long b) noexcept                { return InterlockedExchange (a, b); }
long juce_InterlockedIncrement (volatile long* a) noexcept                       { return InterlockedIncrement (a); }
long juce_InterlockedDecrement (volatile long* a) noexcept                       { return InterlockedDecrement (a); }
long juce_InterlockedExchangeAdd (volatile long* a, long b) noexcept             { return InterlockedExchangeAdd (a, b); }
long juce_InterlockedCompareExchange (volatile long* a, long b, long c) noexcept { return InterlockedCompareExchange (a, b, c); }

__int64 juce_InterlockedCompareExchange64 (volatile __int64* value, __int64 newValue, __int64 valueToCompare) noexcept
{
    jassertfalse; // This operation isn't available in old MS compiler versions!

    __int64 oldValue = *value;
    if (oldValue == valueToCompare)
        *value = newValue;

    return oldValue;
}

#endif

//==============================================================================
CriticalSection::CriticalSection() noexcept
{
    // (just to check the MS haven't changed this structure and broken things...)
   #if JUCE_VC7_OR_EARLIER
    static_jassert (sizeof (CRITICAL_SECTION) <= 24);
   #else
    static_jassert (sizeof (CRITICAL_SECTION) <= sizeof (lock));
   #endif

    InitializeCriticalSection ((CRITICAL_SECTION*) lock);
}

CriticalSection::~CriticalSection() noexcept        { DeleteCriticalSection ((CRITICAL_SECTION*) lock); }
void CriticalSection::enter() const noexcept        { EnterCriticalSection ((CRITICAL_SECTION*) lock); }
bool CriticalSection::tryEnter() const noexcept     { return TryEnterCriticalSection ((CRITICAL_SECTION*) lock) != FALSE; }
void CriticalSection::exit() const noexcept         { LeaveCriticalSection ((CRITICAL_SECTION*) lock); }


//==============================================================================
WaitableEvent::WaitableEvent (const bool manualReset) noexcept
    : handle (CreateEvent (0, manualReset ? TRUE : FALSE, FALSE, 0)) {}

WaitableEvent::~WaitableEvent() noexcept        { CloseHandle (handle); }

void WaitableEvent::signal() const noexcept     { SetEvent (handle); }
void WaitableEvent::reset() const noexcept      { ResetEvent (handle); }

bool WaitableEvent::wait (const int timeOutMs) const noexcept
{
    return WaitForSingleObject (handle, (DWORD) timeOutMs) == WAIT_OBJECT_0;
}

//==============================================================================
void JUCE_API juce_threadEntryPoint (void*);

static unsigned int __stdcall threadEntryProc (void* userData)
{
    if (juce_messageWindowHandle != 0)
        AttachThreadInput (GetWindowThreadProcessId (juce_messageWindowHandle, 0),
                           GetCurrentThreadId(), TRUE);

    juce_threadEntryPoint (userData);

    _endthreadex (0);
    return 0;
}

void Thread::launchThread()
{
    unsigned int newThreadId;
    threadHandle = (void*) _beginthreadex (0, threadStackSize, &threadEntryProc, this, 0, &newThreadId);
    threadId = (ThreadID) (pointer_sized_int) newThreadId;
}

void Thread::closeThreadHandle()
{
    CloseHandle ((HANDLE) threadHandle);
    threadId = 0;
    threadHandle = 0;
}

void Thread::killThread()
{
    if (threadHandle != 0)
    {
       #if JUCE_DEBUG
        OutputDebugStringA ("** Warning - Forced thread termination **\n");
       #endif
        TerminateThread (threadHandle, 0);
    }
}

void JUCE_CALLTYPE Thread::setCurrentThreadName (const String& name)
{
   #if JUCE_DEBUG && JUCE_MSVC
    struct
    {
        DWORD dwType;
        LPCSTR szName;
        DWORD dwThreadID;
        DWORD dwFlags;
    } info;

    info.dwType = 0x1000;
    info.szName = name.toUTF8();
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;

    __try
    {
        RaiseException (0x406d1388 /*MS_VC_EXCEPTION*/, 0, sizeof (info) / sizeof (ULONG_PTR), (ULONG_PTR*) &info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {}
   #else
    ignoreUnused (name);
   #endif
}

Thread::ThreadID JUCE_CALLTYPE Thread::getCurrentThreadId()
{
    return (ThreadID) (pointer_sized_int) GetCurrentThreadId();
}

bool Thread::setThreadPriority (void* handle, int priority)
{
    int pri = THREAD_PRIORITY_TIME_CRITICAL;

    if (priority < 1)       pri = THREAD_PRIORITY_IDLE;
    else if (priority < 2)  pri = THREAD_PRIORITY_LOWEST;
    else if (priority < 5)  pri = THREAD_PRIORITY_BELOW_NORMAL;
    else if (priority < 7)  pri = THREAD_PRIORITY_NORMAL;
    else if (priority < 9)  pri = THREAD_PRIORITY_ABOVE_NORMAL;
    else if (priority < 10) pri = THREAD_PRIORITY_HIGHEST;

    if (handle == 0)
        handle = GetCurrentThread();

    return SetThreadPriority (handle, pri) != FALSE;
}

void JUCE_CALLTYPE Thread::setCurrentThreadAffinityMask (const uint32 affinityMask)
{
    SetThreadAffinityMask (GetCurrentThread(), affinityMask);
}

//==============================================================================
struct SleepEvent
{
    SleepEvent() noexcept
        : handle (CreateEvent (nullptr, FALSE, FALSE,
                              #if JUCE_DEBUG
                               _T("JUCE Sleep Event")))
                              #else
                               nullptr))
                              #endif
    {}

    ~SleepEvent() noexcept
    {
        CloseHandle (handle);
        handle = 0;
    }

    HANDLE handle;
};

static SleepEvent sleepEvent;

void JUCE_CALLTYPE Thread::sleep (const int millisecs)
{
    jassert (millisecs >= 0);

    if (millisecs >= 10 || sleepEvent.handle == 0)
        Sleep ((DWORD) millisecs);
    else
        // unlike Sleep() this is guaranteed to return to the current thread after
        // the time expires, so we'll use this for short waits, which are more likely
        // to need to be accurate
        WaitForSingleObject (sleepEvent.handle, (DWORD) millisecs);
}

void Thread::yield()
{
    Sleep (0);
}

//==============================================================================
static int lastProcessPriority = -1;

// called when the app gains focus because Windows does weird things to process priority
// when you swap apps, and this forces an update when the app is brought to the front.
void juce_repeatLastProcessPriority()
{
    if (lastProcessPriority >= 0) // (avoid changing this if it's not been explicitly set by the app..)
    {
        DWORD p;

        switch (lastProcessPriority)
        {
            case Process::LowPriority:          p = IDLE_PRIORITY_CLASS; break;
            case Process::NormalPriority:       p = NORMAL_PRIORITY_CLASS; break;
            case Process::HighPriority:         p = HIGH_PRIORITY_CLASS; break;
            case Process::RealtimePriority:     p = REALTIME_PRIORITY_CLASS; break;
            default:                            jassertfalse; return; // bad priority value
        }

        SetPriorityClass (GetCurrentProcess(), p);
    }
}

void JUCE_CALLTYPE Process::setPriority (ProcessPriority prior)
{
    if (lastProcessPriority != (int) prior)
    {
        lastProcessPriority = (int) prior;
        juce_repeatLastProcessPriority();
    }
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger() noexcept
{
    return IsDebuggerPresent() != FALSE;
}

static void* currentModuleHandle = nullptr;

void* JUCE_CALLTYPE Process::getCurrentModuleInstanceHandle() noexcept
{
    if (currentModuleHandle == nullptr)
        currentModuleHandle = GetModuleHandleA (nullptr);

    return currentModuleHandle;
}

void JUCE_CALLTYPE Process::setCurrentModuleInstanceHandle (void* const newHandle) noexcept
{
    currentModuleHandle = newHandle;
}

void JUCE_CALLTYPE Process::raisePrivilege()
{
    jassertfalse; // xxx not implemented
}

void JUCE_CALLTYPE Process::lowerPrivilege()
{
    jassertfalse; // xxx not implemented
}

void JUCE_CALLTYPE Process::terminate()
{
   #if JUCE_MSVC && JUCE_CHECK_MEMORY_LEAKS
    _CrtDumpMemoryLeaks();
   #endif

    // bullet in the head in case there's a problem shutting down..
    ExitProcess (1);
}

bool juce_isRunningInWine()
{
    HMODULE ntdll = GetModuleHandleA ("ntdll");
    return ntdll != 0 && GetProcAddress (ntdll, "wine_get_version") != nullptr;
}

//==============================================================================
bool DynamicLibrary::open (const String& name)
{
    close();
    handle = LoadLibrary (name.toWideCharPointer());
    return handle != nullptr;
}

void DynamicLibrary::close()
{
    if (handle != nullptr)
    {
        FreeLibrary ((HMODULE) handle);
        handle = nullptr;
    }
}

void* DynamicLibrary::getFunction (const String& functionName) noexcept
{
    return handle != nullptr ? (void*) GetProcAddress ((HMODULE) handle, functionName.toUTF8()) // (void* cast is required for mingw)
                             : nullptr;
}


//==============================================================================
class InterProcessLock::Pimpl
{
public:
    Pimpl (String name, const int timeOutMillisecs)
        : handle (0), refCount (1)
    {
        name = name.replaceCharacter ('\\', '/');
        handle = CreateMutexW (0, TRUE, ("Global\\" + name).toWideCharPointer());

        // Not 100% sure why a global mutex sometimes can't be allocated, but if it fails, fall back to
        // a local one. (A local one also sometimes fails on other machines so neither type appears to be
        // universally reliable)
        if (handle == 0)
            handle = CreateMutexW (0, TRUE, ("Local\\" + name).toWideCharPointer());

        if (handle != 0 && GetLastError() == ERROR_ALREADY_EXISTS)
        {
            if (timeOutMillisecs == 0)
            {
                close();
                return;
            }

            switch (WaitForSingleObject (handle, timeOutMillisecs < 0 ? INFINITE : timeOutMillisecs))
            {
                case WAIT_OBJECT_0:
                case WAIT_ABANDONED:
                    break;

                case WAIT_TIMEOUT:
                default:
                    close();
                    break;
            }
        }
    }

    ~Pimpl()
    {
        close();
    }

    void close()
    {
        if (handle != 0)
        {
            ReleaseMutex (handle);
            CloseHandle (handle);
            handle = 0;
        }
    }

    HANDLE handle;
    int refCount;
};

InterProcessLock::InterProcessLock (const String& name_)
    : name (name_)
{
}

InterProcessLock::~InterProcessLock()
{
}

bool InterProcessLock::enter (const int timeOutMillisecs)
{
    const ScopedLock sl (lock);

    if (pimpl == nullptr)
    {
        pimpl = new Pimpl (name, timeOutMillisecs);

        if (pimpl->handle == 0)
            pimpl = nullptr;
    }
    else
    {
        pimpl->refCount++;
    }

    return pimpl != nullptr;
}

void InterProcessLock::exit()
{
    const ScopedLock sl (lock);

    // Trying to release the lock too many times!
    jassert (pimpl != nullptr);

    if (pimpl != nullptr && --(pimpl->refCount) == 0)
        pimpl = nullptr;
}

//==============================================================================
class ChildProcess::ActiveProcess
{
public:
    ActiveProcess (const String& command, int streamFlags)
        : ok (false), readPipe (0), writePipe (0)
    {
        SECURITY_ATTRIBUTES securityAtts = { 0 };
        securityAtts.nLength = sizeof (securityAtts);
        securityAtts.bInheritHandle = TRUE;

        if (CreatePipe (&readPipe, &writePipe, &securityAtts, 0)
             && SetHandleInformation (readPipe, HANDLE_FLAG_INHERIT, 0))
        {
            STARTUPINFOW startupInfo = { 0 };
            startupInfo.cb = sizeof (startupInfo);

            startupInfo.hStdOutput = (streamFlags & wantStdOut) != 0 ? writePipe : 0;
            startupInfo.hStdError  = (streamFlags & wantStdErr) != 0 ? writePipe : 0;
            startupInfo.dwFlags = STARTF_USESTDHANDLES;

            ok = CreateProcess (nullptr, const_cast<LPWSTR> (command.toWideCharPointer()),
                                nullptr, nullptr, TRUE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
                                nullptr, nullptr, &startupInfo, &processInfo) != FALSE;
        }
    }

    ~ActiveProcess()
    {
        if (ok)
        {
            CloseHandle (processInfo.hThread);
            CloseHandle (processInfo.hProcess);
        }

        if (readPipe != 0)
            CloseHandle (readPipe);

        if (writePipe != 0)
            CloseHandle (writePipe);
    }

    bool isRunning() const noexcept
    {
        return WaitForSingleObject (processInfo.hProcess, 0) != WAIT_OBJECT_0;
    }

    int read (void* dest, int numNeeded) const noexcept
    {
        int total = 0;

        while (ok && numNeeded > 0)
        {
            DWORD available = 0;

            if (! PeekNamedPipe ((HANDLE) readPipe, nullptr, 0, nullptr, &available, nullptr))
                break;

            const int numToDo = jmin ((int) available, numNeeded);

            if (available == 0)
            {
                if (! isRunning())
                    break;

                Thread::yield();
            }
            else
            {
                DWORD numRead = 0;
                if (! ReadFile ((HANDLE) readPipe, dest, numToDo, &numRead, nullptr))
                    break;

                total += numRead;
                dest = addBytesToPointer (dest, numRead);
                numNeeded -= numRead;
            }
        }

        return total;
    }

    bool killProcess() const noexcept
    {
        return TerminateProcess (processInfo.hProcess, 0) != FALSE;
    }

    uint32 getExitCode() const noexcept
    {
        DWORD exitCode = 0;
        GetExitCodeProcess (processInfo.hProcess, &exitCode);
        return (uint32) exitCode;
    }

    bool ok;

private:
    HANDLE readPipe, writePipe;
    PROCESS_INFORMATION processInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActiveProcess)
};

bool ChildProcess::start (const String& command, int streamFlags)
{
    activeProcess = new ActiveProcess (command, streamFlags);

    if (! activeProcess->ok)
        activeProcess = nullptr;

    return activeProcess != nullptr;
}

bool ChildProcess::start (const StringArray& args, int streamFlags)
{
    String escaped;

    for (int i = 0; i < args.size(); ++i)
    {
        String arg (args[i]);

        // If there are spaces, surround it with quotes. If there are quotes,
        // replace them with \" so that CommandLineToArgv will correctly parse them.
        if (arg.containsAnyOf ("\" "))
            arg = arg.replace ("\"", "\\\"").quoted();

        escaped << arg << ' ';
    }

    return start (escaped.trim(), streamFlags);
}

//==============================================================================
struct HighResolutionTimer::Pimpl
{
    Pimpl (HighResolutionTimer& t) noexcept  : owner (t), periodMs (0)
    {
    }

    ~Pimpl()
    {
        jassert (periodMs == 0);
    }

    void start (int newPeriod)
    {
        if (newPeriod != periodMs)
        {
            stop();
            periodMs = newPeriod;

            TIMECAPS tc;
            if (timeGetDevCaps (&tc, sizeof (tc)) == TIMERR_NOERROR)
            {
                const int actualPeriod = jlimit ((int) tc.wPeriodMin, (int) tc.wPeriodMax, newPeriod);

                timerID = timeSetEvent (actualPeriod, tc.wPeriodMin, callbackFunction, (DWORD_PTR) this,
                                        TIME_PERIODIC | TIME_CALLBACK_FUNCTION | 0x100 /*TIME_KILL_SYNCHRONOUS*/);
            }
        }
    }

    void stop()
    {
        periodMs = 0;
        timeKillEvent (timerID);
    }

    HighResolutionTimer& owner;
    int periodMs;

private:
    unsigned int timerID;

    static void __stdcall callbackFunction (UINT, UINT, DWORD_PTR userInfo, DWORD_PTR, DWORD_PTR)
    {
        if (Pimpl* const timer = reinterpret_cast<Pimpl*> (userInfo))
            if (timer->periodMs != 0)
                timer->owner.hiResTimerCallback();
    }

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};
