/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
//==============================================================================
Thread::Thread (const String& name, size_t stackSize) : threadName (name),
                                                        threadStackSize (stackSize)
{
}

Thread::~Thread()
{
    if (deleteOnThreadEnd)
        return;

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
struct CurrentThreadHolder final : public ReferenceCountedObject
{
    CurrentThreadHolder() noexcept {}

    using Ptr = ReferenceCountedObjectPtr<CurrentThreadHolder>;
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

    if (threadName.isNotEmpty())
        setCurrentThreadName (threadName);

    // This 'startSuspensionEvent' protects 'threadId' which is initialised after the platform's native 'CreateThread' method.
    // This ensures it has been initialised correctly before it reaches this point.
    if (startSuspensionEvent.wait (10000))
    {
        jassert (getCurrentThreadId() == threadId);

        if (affinityMask != 0)
            setCurrentThreadAffinityMask (affinityMask);

        JUCE_AUTORELEASEPOOL
        {
            try
            {
                run();
            }
            catch (...)
            {
                jassertfalse; // Your run() method mustn't throw any exceptions!
            }
        }
    }

    currentThreadHolder->value.releaseCurrentThreadStorage();

    // Once closeThreadHandle is called this class may be deleted by a different
    // thread, so we need to store deleteOnThreadEnd in a local variable.
    auto shouldDeleteThis = deleteOnThreadEnd;

    // On Windows, CloseHandle must not race with another thread's
    // WaitForSingleObject, as used in isThreadRunning(). deleteOnThreadEnd
    // means no other thread holds this object, so it is safe to close here.
   #if JUCE_WINDOWS
    if (shouldDeleteThis)
   #endif
        closeThreadHandle();

    if (shouldDeleteThis)
        delete this;
}

// used to wrap the incoming call from the platform-specific code
void JUCE_API juce_threadEntryPoint (void* userData)
{
    static_cast<Thread*> (userData)->threadEntryPoint();
}

//==============================================================================
bool Thread::startThreadInternal (Priority threadPriority)
{
    shouldExit = false;

    // 'priority' is essentially useless on Linux as only realtime
    // has any options but we need to set this here to satisfy
    // later queries, otherwise we get inconsistent results across
    // platforms.
   #if JUCE_ANDROID || JUCE_LINUX || JUCE_BSD
    priority = threadPriority;
   #endif

    if (! createNativeThread (threadPriority))
        return false;

    startSuspensionEvent.signal();
    return true;
}

bool Thread::startThread()
{
    return startThread (Priority::normal);
}

bool Thread::startThread (Priority threadPriority)
{
    const ScopedLock sl (startStopLock);

    if (isThreadRunning())
        return false;

    realtimeOptions.reset();
    return startThreadInternal (threadPriority);
}

bool Thread::startRealtimeThread (const RealtimeOptions& options)
{
    const ScopedLock sl (startStopLock);

    if (isThreadRunning())
        return false;

    realtimeOptions = std::make_optional (options);

    if (startThreadInternal (Priority::normal))
        return true;

    realtimeOptions.reset();
    return false;
}

bool Thread::isThreadRunning() const
{
   #if JUCE_WINDOWS
    if (threadHandle == nullptr)
        return false;

    // If this is the thread itself it must still be running. Avoid taking
    // startStopLock here to avoid a deadlock while trying to stop the thread.
    if (const auto id = getThreadId(); id != ThreadID() && id == getCurrentThreadId())
        return true;

    const ScopedLock sl (startStopLock);

    // In the event WaitForSingleObject returns an error it's safest to assume
    // the thread is still running.
    return threadHandle != nullptr
        && WaitForSingleObject (threadHandle, 0) != WAIT_OBJECT_0;
   #else
    return threadHandle != nullptr;
   #endif
}

Thread* JUCE_CALLTYPE Thread::getCurrentThread()
{
    return getCurrentThreadHolder()->value.get();
}

Thread::ThreadID Thread::getThreadId() const noexcept
{
    return threadId;
}

//==============================================================================
void Thread::signalThreadShouldExit()
{
    shouldExit = true;
    listeners.call ([] (Listener& l) { l.exitSignalSent(); });
}

bool Thread::threadShouldExit() const
{
    return shouldExit;
}

bool Thread::currentThreadShouldExit()
{
    if (auto* currentThread = getCurrentThread())
        return currentThread->threadShouldExit();

    return false;
}

bool Thread::waitForThreadToExit (int timeOutMilliseconds) const
{
    if (timeOutMilliseconds >= 0)
        return waitForThreadToExit (Milliseconds { (double) timeOutMilliseconds });

    waitForThreadToExit();
    return true;
}

bool Thread::waitForThreadToExit (Seconds timeOut) const
{
    // It doesn't make sense to wait a negative amount of time for a thread to
    // exit.
    jassert (timeOut >= Seconds { 0 });

    // A thread can't wait for itself to stop. This function must only ever be
    // called from another thread.
    jassert (getThreadId() != getCurrentThreadId() || getCurrentThreadId() == ThreadID());

    const auto timeoutEnd = std::chrono::steady_clock::now() + timeOut;

    while (isThreadRunning())
    {
        if (std::chrono::steady_clock::now() > timeoutEnd)
            return false;

        sleep (2);
    }

    return true;
}

void Thread::waitForThreadToExit() const
{
    // A thread can't wait for itself to stop!
    jassert (getThreadId() != getCurrentThreadId() || getCurrentThreadId() == ThreadID());

    while (isThreadRunning())
        sleep (2);
}

bool Thread::stopThread (int timeOut)
{
    if (timeOut >= 0)
        return stopThread (Milliseconds { (double) timeOut });

    stopThread();
    return true;
}

bool Thread::stopThread (Seconds timeOut)
{
    // Unlike stopThread (int), only positive timeout values are supported.
    // To wait indefinitely, call stopThread() with no arguments.
    // If you're trying to wait for 0 seconds, this will almost definitely
    // result in the thread being killed by force, potentially leaving members
    // in an unexpected state.
    jassert (timeOut > Seconds { 0.0 });

    // A thread can't stop itself, another thread must stop this thread.
    jassert (getCurrentThreadId() != getThreadId());

    const ScopedLock sl (startStopLock);

    if (isThreadRunning())
    {
        signalThreadShouldExit();
        notify();

        if (! waitForThreadToExit (timeOut))
        {
            // very bad karma if this point is reached, as there are bound to be
            // locks and events left in silly states when a thread is killed by force
            jassertfalse;
            Logger::writeToLog ("!! killing thread by force !!");

            killThread();
            closeThreadHandle();
            return false;
        }
    }

    if (threadHandle != nullptr)
        closeThreadHandle();

    return true;
}

void Thread::stopThread()
{
    // A thread can't stop itself, another thread must stop this thread.
    jassert (getCurrentThreadId() != getThreadId());

    const ScopedLock sl (startStopLock);

    if (isThreadRunning())
    {
        signalThreadShouldExit();
        notify();
        waitForThreadToExit();
    }

    if (threadHandle != nullptr)
        closeThreadHandle();
}

void Thread::addListener (Listener* listener)
{
    listeners.add (listener);
}

void Thread::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

bool Thread::isRealtime() const
{
    return realtimeOptions.has_value();
}

void Thread::setAffinityMask (const uint32 newAffinityMask)
{
    affinityMask = newAffinityMask;
}

//==============================================================================
bool Thread::wait (double timeOutMilliseconds) const
{
    return defaultEvent.wait (timeOutMilliseconds);
}

bool Thread::wait (Seconds timeOut) const
{
    return defaultEvent.wait (timeOut);
}

void Thread::wait() const
{
    defaultEvent.wait();
}

void Thread::notify() const
{
    defaultEvent.signal();
}

//==============================================================================
struct LambdaThread final : public Thread
{
    LambdaThread (std::function<void()>&& f) : Thread (SystemStats::getJUCEVersion() + ": anonymous"), fn (std::move (f)) {}

    void run() override
    {
        fn();
        fn = nullptr; // free any objects that the lambda might contain while the thread is still active
    }

    std::function<void()> fn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LambdaThread)
};

bool Thread::launch (std::function<void()> functionToRun)
{
    return launch (Priority::normal, std::move (functionToRun));
}

bool Thread::launch (Priority priority, std::function<void()> functionToRun)
{
    auto anon = std::make_unique<LambdaThread> (std::move (functionToRun));
    anon->deleteOnThreadEnd = true;

    if (anon->startThread (priority))
    {
        anon.release();
        return true;
    }

    return false;
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
bool JUCE_CALLTYPE Process::isRunningUnderDebugger() noexcept
{
    return juce_isRunningUnderDebugger();
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class ThreadTests final : public UnitTest
{
public:
    ThreadTests()
        : UnitTest ("Thread", UnitTestCategories::threads)
    {}

    void runTest() final
    {
        static constexpr Seconds maximumTimeout { 30.0 };

        beginTest ("Start and stop a thread");
        {
            struct TestThread final : public Thread
            {
                TestThread() : Thread ("TestThread") {}

                void run() final
                {
                    runMethodCalled.signal();
                    wait (maximumTimeout);
                }

                WaitableEvent runMethodCalled;
            };

            TestThread thread;
            expect (! thread.isThreadRunning());

            expect (thread.startThread());
            expect (thread.isThreadRunning());
            expect (thread.runMethodCalled.wait (maximumTimeout));

            expect (thread.isThreadRunning());
            expect (thread.stopThread (maximumTimeout));
            expect (! thread.isThreadRunning());
        }

        beginTest ("Notify a thread");
        {
            struct TestThread final : public Thread
            {
                TestThread() : Thread ("TestThread") {}
                void run() final { wait (maximumTimeout); }
            };

            TestThread thread;
            expect (thread.startThread());

            thread.notify();
            expect (thread.waitForThreadToExit (maximumTimeout));
        }

        beginTest ("Lambda thread");
        {
            WaitableEvent threadLaunched;

            LambdaThread thread ([&] { threadLaunched.signal(); });
            expect (! thread.isThreadRunning());

            expect (thread.startThread());
            expect (thread.isThreadRunning());
            expect (threadLaunched.wait (maximumTimeout));

            expect (thread.stopThread (maximumTimeout));
            expect (! thread.isThreadRunning());
        }

        beginTest ("Launch a thread");
        {
            WaitableEvent threadLaunched;

            Thread::launch ([&] { threadLaunched.signal(); });
            expect (threadLaunched.wait (maximumTimeout));
        }

        beginTest ("A thread is not running once finished, without calling stopThread()");
        {
            struct TestThread final : public Thread
            {
                TestThread() : Thread ("TestThread") {}
                void run() final { wait (maximumTimeout); }
            };

            TestThread thread;
            expect (thread.startThread());

            // We check this twice to check state isn't changing by observing
            // the result
            expect (thread.isThreadRunning());
            expect (thread.isThreadRunning());

            thread.notify();
            expect (thread.waitForThreadToExit (maximumTimeout));

            // We check this twice to check state isn't changing by observing
            // the result
            expect (! thread.isThreadRunning());
            expect (! thread.isThreadRunning());
        }

        beginTest ("A thread can be restarted after finishing without stopThread()");
        {
            struct TestThread final : public Thread
            {
                TestThread() : Thread ("TestThread") {}

                void run() final
                {
                    runMethodCalled.signal();
                    wait (maximumTimeout);
                }

                WaitableEvent runMethodCalled;
            };

            TestThread thread;
            expect (thread.startThread());
            expect (thread.runMethodCalled.wait (maximumTimeout));

            thread.notify();
            expect (thread.waitForThreadToExit (maximumTimeout));
            expect (! thread.isThreadRunning());

            expect (thread.startThread());
            expect (thread.isThreadRunning());
            expect (thread.runMethodCalled.wait (maximumTimeout));
            expect (thread.stopThread (maximumTimeout));
            expect (! thread.isThreadRunning());
        }
    }
};

static ThreadTests threadTests;

//==============================================================================
class AtomicTests final : public UnitTest
{
public:
    AtomicTests()
        : UnitTest ("Atomics", UnitTestCategories::threads)
    {}

    void runTest() override
    {
        beginTest ("Misc");

        char a1[7];
        expect (numElementsInArray (a1) == 7);
        int a2[3];
        expect (numElementsInArray (a2) == 3);

        expect (ByteOrder::swap ((uint16) 0x1122) == 0x2211);
        expect (ByteOrder::swap ((uint32) 0x11223344) == 0x44332211);
        expect (ByteOrder::swap ((uint64) 0x1122334455667788ULL) == (uint64) 0x8877665544332211LL);

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
        beginTest ("Atomic pointer increment/decrement");
        Atomic<int*> a (a2); int* b (a2);
        expect (++a == ++b);

        {
            beginTest ("Atomic void*");
            Atomic<void*> atomic;
            void* c;

            atomic.set ((void*) 10);
            c = (void*) 10;

            expect (atomic.value == c);
            expect (atomic.get() == c);
        }
    }

    template <typename Type>
    class AtomicTester
    {
    public:
        AtomicTester() = default;

        static void testInteger (UnitTest& test)
        {
            Atomic<Type> a, b;
            Type c;

            a.set ((Type) 10);
            c = (Type) 10;

            test.expect (a.value == c);
            test.expect (a.get() == c);

            a += 15;
            c += 15;
            test.expect (a.get() == c);
            a.memoryBarrier();

            a -= 5;
            c -= 5;
            test.expect (a.get() == c);

            test.expect (++a == ++c);
            ++a;
            ++c;
            test.expect (--a == --c);
            test.expect (a.get() == c);
            a.memoryBarrier();

            testFloat (test);
        }



        static void testFloat (UnitTest& test)
        {
            Atomic<Type> a, b;
            a = (Type) 101;
            a.memoryBarrier();

            /*  These are some simple test cases to check the atomics - let me know
                if any of these assertions fail on your system!
            */
            test.expect (exactlyEqual (a.get(), (Type) 101));
            test.expect (! a.compareAndSetBool ((Type) 300, (Type) 200));
            test.expect (exactlyEqual (a.get(), (Type) 101));
            test.expect (a.compareAndSetBool ((Type) 200, a.get()));
            test.expect (exactlyEqual (a.get(), (Type) 200));

            test.expect (exactlyEqual (a.exchange ((Type) 300), (Type) 200));
            test.expect (exactlyEqual (a.get(), (Type) 300));

            b = a;
            test.expect (exactlyEqual (b.get(), a.get()));
        }
    };
};

static AtomicTests atomicUnitTests;

//==============================================================================
class ThreadLocalValueUnitTest final : public UnitTest,
                                       private Thread
{
public:
    ThreadLocalValueUnitTest()
        : UnitTest ("ThreadLocalValue", UnitTestCategories::threads),
          Thread (SystemStats::getJUCEVersion() + ": ThreadLocalValue Thread")
    {}

    void runTest() override
    {
        beginTest ("values are thread local");

        {
            ThreadLocalValue<int> threadLocal;

            sharedThreadLocal = &threadLocal;

            sharedThreadLocal.get()->get() = 1;

            startThread();
            signalThreadShouldExit();
            waitForThreadToExit (-1);

            mainThreadResult = sharedThreadLocal.get()->get();

            expectEquals (mainThreadResult.get(), 1);
            expectEquals (auxThreadResult.get(), 2);
        }

        beginTest ("values are per-instance");

        {
            ThreadLocalValue<int> a, b;

            a.get() = 1;
            b.get() = 2;

            expectEquals (a.get(), 1);
            expectEquals (b.get(), 2);
        }
    }

private:
    Atomic<int> mainThreadResult, auxThreadResult;
    Atomic<ThreadLocalValue<int>*> sharedThreadLocal;

    void run() override
    {
        sharedThreadLocal.get()->get() = 2;
        auxThreadResult = sharedThreadLocal.get()->get();
    }
};

ThreadLocalValueUnitTest threadLocalValueUnitTest;

#endif

} // namespace juce
