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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

Random::Random (int64 seedValue) noexcept  : seed (seedValue)
{
}

Random::Random()  : seed (1)
{
    setSeedRandomly();
}

void Random::setSeed (const int64 newSeed) noexcept
{
    // Resetting the system Random risks messing up JUCE's internal state.
    // If you need a predictable stream of random numbers you should use a
    // local Random object.
    jassert (! isSystemRandom);

    seed = newSeed;
}

void Random::combineSeed (const int64 seedValue) noexcept
{
    // Resetting the system Random risks messing up JUCE's internal state.
    // Consider using a local Random object instead.
    jassert (! isSystemRandom);

    seed ^= nextInt64() ^ seedValue;
}

void Random::setSeedRandomly()
{
    // Resetting the system Random risks messing up JUCE's internal state.
    // Consider using a local Random object instead.
    jassert (! isSystemRandom);

    static std::atomic<int64> globalSeed { 0 };

    combineSeed (globalSeed ^ (int64) (pointer_sized_int) this);
    combineSeed (Time::getMillisecondCounter());
    combineSeed (Time::getHighResolutionTicks());
    combineSeed (Time::getHighResolutionTicksPerSecond());
    combineSeed (Time::currentTimeMillis());
    globalSeed ^= seed;
}

Random& Random::getSystemRandom() noexcept
{
    thread_local Random sysRand = std::invoke ([]
    {
        Random r;
       #if JUCE_ASSERTIONS_ENABLED_OR_LOGGED
        r.isSystemRandom = true;
       #endif
        return r;
    });

    return sysRand;
}

//==============================================================================
int Random::nextInt() noexcept
{
    // If you encounter this assertion you've likely stored a reference to the
    // system random object and are accessing it from a thread other than the
    // one it was first created on. This may lead to race conditions on the
    // random object. To avoid this assertion call Random::getSystemRandom()
    // directly instead of storing a reference.
    jassert (! isSystemRandom || this == &getSystemRandom());

    seed = (int64) (((((uint64) seed) * 0x5deece66dLL) + 11) & 0xffffffffffffLL);

    return (int) (seed >> 16);
}

int Random::nextInt (const int maxValue) noexcept
{
    jassert (maxValue > 0);
    return (int) ((((unsigned int) nextInt()) * (uint64) maxValue) >> 32);
}

int Random::nextInt (Range<int> range) noexcept
{
    return range.getStart() + nextInt (range.getLength());
}

int64 Random::nextInt64() noexcept
{
    return (int64) ((((uint64) (unsigned int) nextInt()) << 32) | (uint64) (unsigned int) nextInt());
}

bool Random::nextBool() noexcept
{
    return (nextInt() & 0x40000000) != 0;
}

float Random::nextFloat() noexcept
{
    auto result = static_cast<float> (static_cast<uint32> (nextInt()))
                  / (static_cast<float> (std::numeric_limits<uint32>::max()) + 1.0f);
    return jmin (result, 1.0f - std::numeric_limits<float>::epsilon());
}

double Random::nextDouble() noexcept
{
    return static_cast<uint32> (nextInt()) / (std::numeric_limits<uint32>::max() + 1.0);
}

BigInteger Random::nextLargeNumber (const BigInteger& maximumValue)
{
    BigInteger n;

    do
    {
        fillBitsRandomly (n, 0, maximumValue.getHighestBit() + 1);
    }
    while (n >= maximumValue);

    return n;
}

void Random::fillBitsRandomly (void* const buffer, size_t bytes)
{
    int* d = static_cast<int*> (buffer);

    for (; bytes >= sizeof (int); bytes -= sizeof (int))
        *d++ = nextInt();

    if (bytes > 0)
    {
        const int lastBytes = nextInt();
        memcpy (d, &lastBytes, bytes);
    }
}

void Random::fillBitsRandomly (BigInteger& arrayToChange, int startBit, int numBits)
{
    arrayToChange.setBit (startBit + numBits - 1, true);  // to force the array to pre-allocate space

    while ((startBit & 31) != 0 && numBits > 0)
    {
        arrayToChange.setBit (startBit++, nextBool());
        --numBits;
    }

    while (numBits >= 32)
    {
        arrayToChange.setBitRangeAsInt (startBit, 32, (unsigned int) nextInt());
        startBit += 32;
        numBits -= 32;
    }

    while (--numBits >= 0)
        arrayToChange.setBit (startBit + numBits, nextBool());
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class RandomTests final : public UnitTest
{
public:
    RandomTests()
        : UnitTest ("Random", UnitTestCategories::maths)
    {}

    void runTest() override
    {
        beginTest ("Random");
        {
            Random r = getRandom();

            for (int i = 2000; --i >= 0;)
            {
                expect (r.nextDouble() >= 0.0 && r.nextDouble() < 1.0);
                expect (r.nextFloat() >= 0.0f && r.nextFloat() < 1.0f);
                expect (r.nextInt (5) >= 0 && r.nextInt (5) < 5);
                expect (r.nextInt (1) == 0);

                int n = r.nextInt (50) + 1;
                expect (r.nextInt (n) >= 0 && r.nextInt (n) < n);

                n = r.nextInt (0x7ffffffe) + 1;
                expect (r.nextInt (n) >= 0 && r.nextInt (n) < n);
            }
        }

        beginTest ("System random stress test");
        {
            // Run this with thread-sanitizer to detect race conditions
            runOnMultipleThreadsConcurrently ([] { Random::getSystemRandom().nextInt(); });
        }
    }

private:
    static void runOnMultipleThreadsConcurrently (std::function<void()> functionToInvoke,
                                                  int numberOfInvocationsPerThread = 10'000,
                                                  int numberOfThreads = 100)
    {
        class FastWaitableEvent
        {
        public:
            void notify() { notified = true; }
            void wait() const { while (! notified){} }

        private:
            std::atomic<bool> notified = false;
        };

        class InvokerThread final : private Thread
        {
        public:
            InvokerThread (std::function<void()> fn, FastWaitableEvent& notificationEvent, int numInvocationsToTrigger)
                : Thread ("InvokerThread"),
                  invokable (fn),
                  notified (&notificationEvent),
                  numInvocations (numInvocationsToTrigger)
            {
                startThread();
            }

            ~InvokerThread() { stopThread (-1); }

            void waitUntilReady() const { ready.wait(); }

        private:
            void run() final
            {
                ready.notify();
                notified->wait();

                for (int i = numInvocations; --i >= 0;)
                    invokable();
            }

            std::function<void()> invokable;
            FastWaitableEvent* notified;
            FastWaitableEvent ready;
            int numInvocations;
        };

        std::vector<std::unique_ptr<InvokerThread>> threads;
        threads.reserve ((size_t) numberOfThreads);
        FastWaitableEvent start;

        for (int i = numberOfThreads; --i >= 0;)
            threads.push_back (std::make_unique<InvokerThread> (functionToInvoke, start, numberOfInvocationsPerThread));

        for (auto& thread : threads)
            thread->waitUntilReady();

        // just to increase the odds that all the threads are now at the same point
        // ready to be notified
        Thread::sleep (1);
        start.notify();
    }
};

static RandomTests randomTests;

#endif

} // namespace juce
