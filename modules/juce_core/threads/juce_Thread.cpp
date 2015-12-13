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

Thread::Thread (const String& threadName_)
    : threadName (threadName_),
      threadHandle (nullptr),
      threadId (0),
      threadPriority (5),
      affinityMask (0),
      shouldExit (false)
{
}

Thread::~Thread()
{
    /* If your thread class's destructor has been called without first stopping the thread, that
       means that this partially destructed object is still performing some work - and that's
       probably a Bad Thing!

       To avoid this type of nastiness, always make sure you call stopThread() before or during
       your subclass's destructor.
    */
    jassert (! isThreadRunning());

    stopThread (-1);
}

//==============================================================================
// Use a ref-counted object to hold this shared data, so that it can outlive its static
// shared pointer when threads are still running during static shutdown.
struct CurrentThreadHolder   : public ReferenceCountedObject
{
    CurrentThreadHolder() noexcept {}

    typedef ReferenceCountedObjectPtr<CurrentThreadHolder> Ptr;
    ThreadLocalValue<Thread*> value;

    JUCE_DECLARE_NON_COPYABLE (CurrentThreadHolder)
};

static char currentThreadHolderLock [sizeof (SpinLock)]; // (statically initialised to zeros).

static SpinLock* castToSpinLockWithoutAliasingWarning (void* s)
{
    return static_cast<SpinLock*> (s);
}

static CurrentThreadHolder::Ptr getCurrentThreadHolder()
{
    static CurrentThreadHolder::Ptr currentThreadHolder;
    SpinLock::ScopedLockType lock (*castToSpinLockWithoutAliasingWarning (currentThreadHolderLock));

    if (currentThreadHolder == nullptr)
        currentThreadHolder = new CurrentThreadHolder();

    return currentThreadHolder;
}

void Thread::threadEntryPoint()
{
    const CurrentThreadHolder::Ptr currentThreadHolder (getCurrentThreadHolder());
    currentThreadHolder->value = this;

    JUCE_TRY
    {
        if (threadName.isNotEmpty())
            setCurrentThreadName (threadName);

        if (startSuspensionEvent.wait (10000))
        {
            jassert (getCurrentThreadId() == threadId);

            if (affinityMask != 0)
                setCurrentThreadAffinityMask (affinityMask);

            run();
        }
    }
    JUCE_CATCH_ALL_ASSERT

    currentThreadHolder->value.releaseCurrentThreadStorage();
    closeThreadHandle();
}

// used to wrap the incoming call from the platform-specific code
void JUCE_API juce_threadEntryPoint (void* userData)
{
    static_cast<Thread*> (userData)->threadEntryPoint();
}

//==============================================================================
void Thread::startThread()
{
    const ScopedLock sl (startStopLock);

    shouldExit = false;

    if (threadHandle == nullptr)
    {
        launchThread();
        setThreadPriority (threadHandle, threadPriority);
        startSuspensionEvent.signal();
    }
}

void Thread::startThread (const int priority)
{
    const ScopedLock sl (startStopLock);

    if (threadHandle == nullptr)
    {
        threadPriority = priority;
        startThread();
    }
    else
    {
        setPriority (priority);
    }
}

bool Thread::isThreadRunning() const
{
    return threadHandle != nullptr;
}

Thread* JUCE_CALLTYPE Thread::getCurrentThread()
{
    return getCurrentThreadHolder()->value.get();
}

//==============================================================================
void Thread::signalThreadShouldExit()
{
    shouldExit = true;
}

bool Thread::waitForThreadToExit (const int timeOutMilliseconds) const
{
    // Doh! So how exactly do you expect this thread to wait for itself to stop??
    jassert (getThreadId() != getCurrentThreadId() || getCurrentThreadId() == 0);

    const uint32 timeoutEnd = Time::getMillisecondCounter() + (uint32) timeOutMilliseconds;

    while (isThreadRunning())
    {
        if (timeOutMilliseconds >= 0 && Time::getMillisecondCounter() > timeoutEnd)
            return false;

        sleep (2);
    }

    return true;
}

bool Thread::stopThread (const int timeOutMilliseconds)
{
    // agh! You can't stop the thread that's calling this method! How on earth
    // would that work??
    jassert (getCurrentThreadId() != getThreadId());

    const ScopedLock sl (startStopLock);

    if (isThreadRunning())
    {
        signalThreadShouldExit();
        notify();

        if (timeOutMilliseconds != 0)
            waitForThreadToExit (timeOutMilliseconds);

        if (isThreadRunning())
        {
            // very bad karma if this point is reached, as there are bound to be
            // locks and events left in silly states when a thread is killed by force..
            jassertfalse;
            Logger::writeToLog ("!! killing thread by force !!");

            killThread();

            threadHandle = nullptr;
            threadId = 0;
            return false;
        }
    }

    return true;
}

//==============================================================================
bool Thread::setPriority (const int newPriority)
{
    // NB: deadlock possible if you try to set the thread prio from the thread itself,
    // so using setCurrentThreadPriority instead in that case.
    if (getCurrentThreadId() == getThreadId())
        return setCurrentThreadPriority (newPriority);

    const ScopedLock sl (startStopLock);

    if ((! isThreadRunning()) || setThreadPriority (threadHandle, newPriority))
    {
        threadPriority = newPriority;
        return true;
    }

    return false;
}

bool Thread::setCurrentThreadPriority (const int newPriority)
{
    return setThreadPriority (0, newPriority);
}

void Thread::setAffinityMask (const uint32 newAffinityMask)
{
    affinityMask = newAffinityMask;
}

//==============================================================================
bool Thread::wait (const int timeOutMilliseconds) const
{
    return defaultEvent.wait (timeOutMilliseconds);
}

void Thread::notify() const
{
    defaultEvent.signal();
}

//==============================================================================
void SpinLock::enter() const noexcept
{
    if (! tryEnter())
    {
        for (int i = 20; --i >= 0;)
            if (tryEnter())
                return;

        while (! tryEnter())
            Thread::yield();
    }
}

//==============================================================================
#if JUCE_UNIT_TESTS

class AtomicTests  : public UnitTest
{
public:
    AtomicTests() : UnitTest ("Atomics") {}

    void runTest() override
    {
        beginTest ("Misc");

        char a1[7];
        expect (numElementsInArray(a1) == 7);
        int a2[3];
        expect (numElementsInArray(a2) == 3);

        expect (ByteOrder::swap ((uint16) 0x1122) == 0x2211);
        expect (ByteOrder::swap ((uint32) 0x11223344) == 0x44332211);
        expect (ByteOrder::swap ((uint64) 0x1122334455667788ULL) == 0x8877665544332211LL);

        beginTest ("Atomic int");
        AtomicTester <int>::testInteger (*this);
        beginTest ("Atomic unsigned int");
        AtomicTester <unsigned int>::testInteger (*this);
        beginTest ("Atomic int32");
        AtomicTester <int32>::testInteger (*this);
        beginTest ("Atomic uint32");
        AtomicTester <uint32>::testInteger (*this);
        beginTest ("Atomic long");
        AtomicTester <long>::testInteger (*this);
        beginTest ("Atomic void*");
        AtomicTester <void*>::testInteger (*this);
        beginTest ("Atomic int*");
        AtomicTester <int*>::testInteger (*this);
        beginTest ("Atomic float");
        AtomicTester <float>::testFloat (*this);
      #if ! JUCE_64BIT_ATOMICS_UNAVAILABLE  // 64-bit intrinsics aren't available on some old platforms
        beginTest ("Atomic int64");
        AtomicTester <int64>::testInteger (*this);
        beginTest ("Atomic uint64");
        AtomicTester <uint64>::testInteger (*this);
        beginTest ("Atomic double");
        AtomicTester <double>::testFloat (*this);
      #endif
    }

    template <typename Type>
    class AtomicTester
    {
    public:
        AtomicTester() {}

        static void testInteger (UnitTest& test)
        {
            Atomic<Type> a, b;
            a.set ((Type) 10);
            test.expect (a.value == (Type) 10);
            test.expect (a.get() == (Type) 10);
            a += (Type) 15;
            test.expect (a.get() == (Type) 25);
            a.memoryBarrier();
            a -= (Type) 5;
            test.expect (a.get() == (Type) 20);
            test.expect (++a == (Type) 21);
            ++a;
            test.expect (--a == (Type) 21);
            test.expect (a.get() == (Type) 21);
            a.memoryBarrier();

            testFloat (test);
        }

        static void testFloat (UnitTest& test)
        {
            Atomic<Type> a, b;
            a = (Type) 21;
            a.memoryBarrier();

            /*  These are some simple test cases to check the atomics - let me know
                if any of these assertions fail on your system!
            */
            test.expect (a.get() == (Type) 21);
            test.expect (a.compareAndSetValue ((Type) 100, (Type) 50) == (Type) 21);
            test.expect (a.get() == (Type) 21);
            test.expect (a.compareAndSetValue ((Type) 101, a.get()) == (Type) 21);
            test.expect (a.get() == (Type) 101);
            test.expect (! a.compareAndSetBool ((Type) 300, (Type) 200));
            test.expect (a.get() == (Type) 101);
            test.expect (a.compareAndSetBool ((Type) 200, a.get()));
            test.expect (a.get() == (Type) 200);

            test.expect (a.exchange ((Type) 300) == (Type) 200);
            test.expect (a.get() == (Type) 300);

            b = a;
            test.expect (b.get() == a.get());
        }
    };
};

static AtomicTests atomicUnitTests;

#endif
