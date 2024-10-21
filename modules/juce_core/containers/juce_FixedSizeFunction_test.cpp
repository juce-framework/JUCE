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

#if JUCE_ENABLE_ALLOCATION_HOOKS
#define JUCE_FAIL_ON_ALLOCATION_IN_SCOPE const UnitTestAllocationChecker checker (*this)
#else
#define JUCE_FAIL_ON_ALLOCATION_IN_SCOPE
#endif

namespace juce
{
namespace
{

class ConstructCounts
{
    auto tie() const noexcept { return std::tie (constructions, copies, moves, calls, destructions); }

public:
    int constructions = 0;
    int copies        = 0;
    int moves         = 0;
    int calls         = 0;
    int destructions  = 0;

    ConstructCounts withConstructions (int i) const noexcept { auto c = *this; c.constructions = i; return c; }
    ConstructCounts withCopies        (int i) const noexcept { auto c = *this; c.copies        = i; return c; }
    ConstructCounts withMoves         (int i) const noexcept { auto c = *this; c.moves         = i; return c; }
    ConstructCounts withCalls         (int i) const noexcept { auto c = *this; c.calls         = i; return c; }
    ConstructCounts withDestructions  (int i) const noexcept { auto c = *this; c.destructions  = i; return c; }

    bool operator== (const ConstructCounts& other) const noexcept { return tie() == other.tie(); }
    bool operator!= (const ConstructCounts& other) const noexcept { return tie() != other.tie(); }
};

String& operator<< (String& str, const ConstructCounts& c)
{
    return str << "{ constructions: " << c.constructions
               << ", copies: " << c.copies
               << ", moves: " << c.moves
               << ", calls: " << c.calls
               << ", destructions: " << c.destructions
               << " }";
}

class FixedSizeFunctionTest final : public UnitTest
{
    static void toggleBool (bool& b) { b = ! b; }

    struct ConstructCounter
    {
        explicit ConstructCounter (ConstructCounts& countsIn)
            : counts (countsIn) {}

        ConstructCounter (const ConstructCounter& c)
            : counts (c.counts)
        {
            counts.copies += 1;
        }

        ConstructCounter (ConstructCounter&& c) noexcept
            : counts (c.counts)
        {
            counts.moves += 1;
        }

        ~ConstructCounter() noexcept { counts.destructions += 1; }

        void operator()() const noexcept { counts.calls += 1; }

        ConstructCounts& counts;
    };

public:
    FixedSizeFunctionTest()
        : UnitTest ("Fixed Size Function", UnitTestCategories::containers)
    {}

    void runTest() override
    {
        beginTest ("Can be constructed and called from a lambda");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            const auto result = 5;
            bool wasCalled = false;
            const auto lambda = [&] { wasCalled = true; return result; };

            const FixedSizeFunction<sizeof (lambda), int()> fn (lambda);
            const auto out = fn();

            expect (wasCalled);
            expectEquals (result, out);
        }

        beginTest ("void fn can be constructed from function with return value");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            bool wasCalled = false;
            const auto lambda = [&] { wasCalled = true; return 5; };
            const FixedSizeFunction<sizeof (lambda), void()> fn (lambda);

            fn();
            expect (wasCalled);
        }

        beginTest ("Can be constructed and called from a function pointer");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            bool state = false;

            const FixedSizeFunction<sizeof (void*), void (bool&)> fn (toggleBool);

            fn (state);
            expect (state);

            fn (state);
            expect (! state);

            fn (state);
            expect (state);
        }

        beginTest ("Default constructed functions throw if called");
        {
            const auto a = FixedSizeFunction<8, void()>();
            expectThrowsType (a(), std::bad_function_call)

            const auto b = FixedSizeFunction<8, void()> (nullptr);
            expectThrowsType (b(), std::bad_function_call)
        }

        beginTest ("Functions can be moved");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            ConstructCounts counts;

            auto a = FixedSizeFunction<sizeof (ConstructCounter), void()> (ConstructCounter { counts });
            expectEquals (counts, ConstructCounts().withMoves (1).withDestructions (1)); // The temporary gets destroyed

            a();
            expectEquals (counts, ConstructCounts().withMoves (1).withDestructions (1).withCalls (1));

            const auto b = std::move (a);
            expectEquals (counts, ConstructCounts().withMoves (2).withDestructions (1).withCalls (1));

            b();
            expectEquals (counts, ConstructCounts().withMoves (2).withDestructions (1).withCalls (2));

            b();
            expectEquals (counts, ConstructCounts().withMoves (2).withDestructions (1).withCalls (3));
        }

        beginTest ("Functions are destructed properly");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            ConstructCounts counts;
            const ConstructCounter toCopy { counts };

            {
                auto a = FixedSizeFunction<sizeof (ConstructCounter), void()> (toCopy);
                expectEquals (counts, ConstructCounts().withCopies (1));
            }

            expectEquals (counts, ConstructCounts().withCopies (1).withDestructions (1));
        }

        beginTest ("Avoid destructing functions that fail to construct");
        {
            struct BadConstructor
            {
                explicit BadConstructor (ConstructCounts& c)
                    : counts (c)
                {
                    counts.constructions += 1;
                    throw std::runtime_error { "this was meant to happen" };
                }

                BadConstructor (const BadConstructor&) = default;
                BadConstructor& operator= (const BadConstructor&) = delete;

                ~BadConstructor() noexcept { counts.destructions += 1; }

                void operator()() const noexcept { counts.calls += 1; }

                ConstructCounts& counts;
            };

            ConstructCounts counts;

            expectThrowsType ((FixedSizeFunction<sizeof (BadConstructor), void()> (BadConstructor { counts })),
                              std::runtime_error)

            expectEquals (counts, ConstructCounts().withConstructions (1));
        }

        beginTest ("Equality checks work");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            FixedSizeFunction<8, void()> a;
            expect (! bool (a));
            expect (a == nullptr);
            expect (nullptr == a);
            expect (! (a != nullptr));
            expect (! (nullptr != a));

            FixedSizeFunction<8, void()> b ([] {});
            expect (bool (b));
            expect (b != nullptr);
            expect (nullptr != b);
            expect (! (b == nullptr));
            expect (! (nullptr == b));
        }

        beginTest ("Functions can be cleared");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            FixedSizeFunction<8, void()> fn ([] {});
            expect (bool (fn));

            fn = nullptr;
            expect (! bool (fn));
        }

        beginTest ("Functions can be assigned");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            using Fn = FixedSizeFunction<8, void()>;

            int numCallsA = 0;
            int numCallsB = 0;

            Fn x;
            Fn y;
            expect (! bool (x));
            expect (! bool (y));

            x = [&] { numCallsA += 1; };
            y = [&] { numCallsB += 1; };
            expect (bool (x));
            expect (bool (y));

            x();
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 0);

            y();
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 1);

            x = std::move (y);
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 1);

            x();
            expectEquals (numCallsA, 1);
            expectEquals (numCallsB, 2);
        }

        beginTest ("Functions may mutate internal state");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            using Fn = FixedSizeFunction<64, void()>;

            Fn x;
            expect (! bool (x));

            int numCalls = 0;
            x = [&numCalls, counter = 0]() mutable { counter += 1; numCalls = counter; };
            expect (bool (x));

            expectEquals (numCalls, 0);

            x();
            expectEquals (numCalls, 1);

            x();
            expectEquals (numCalls, 2);
        }

        beginTest ("Functions can sink move-only parameters");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            using FnA = FixedSizeFunction<64, int (std::unique_ptr<int>)>;

            auto value = 5;
            auto ptr = std::make_unique<int> (value);

            FnA fnA = [] (std::unique_ptr<int> p) { return *p; };

            expect (value == fnA (std::move (ptr)));

            using FnB = FixedSizeFunction<64, void (std::unique_ptr<int>&&)>;

            FnB fnB = [&value] (std::unique_ptr<int>&& p)
            {
                auto x = std::move (p);
                value = *x;
            };

            const auto newValue = 10;
            fnB (std::make_unique<int> (newValue));
            expect (value == newValue);
        }

        beginTest ("Functions be converted from smaller functions");
        {
            JUCE_FAIL_ON_ALLOCATION_IN_SCOPE;

            using SmallFn = FixedSizeFunction<20, void()>;
            using LargeFn = FixedSizeFunction<21, void()>;

            bool smallCalled = false;
            bool largeCalled = false;

            SmallFn small = [&smallCalled, a = std::array<char, 8>{}] { smallCalled = true; ignoreUnused (a); };
            LargeFn large = [&largeCalled, a = std::array<char, 8>{}] { largeCalled = true; ignoreUnused (a); };

            large = std::move (small);

            large();

            expect (smallCalled);
            expect (! largeCalled);
        }
    }
};

FixedSizeFunctionTest fixedSizedFunctionTest;

}
} // namespace juce
#undef JUCE_FAIL_ON_ALLOCATION_IN_SCOPE
