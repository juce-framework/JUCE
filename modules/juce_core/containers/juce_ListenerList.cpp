/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_UNIT_TESTS

class ListenerListTests final : public UnitTest
{
public:
    //==============================================================================
    class TestListener
    {
    public:
        explicit TestListener (std::function<void()> cb) : callback (std::move (cb)) {}

        void doCallback()
        {
            ++numCalls;
            callback();
        }

        int getNumCalls() const { return numCalls; }

    private:
        int numCalls = 0;
        std::function<void()> callback;
    };

    class TestObject
    {
    public:
        void addListener (std::function<void()> cb)
        {
            listeners.push_back (std::make_unique<TestListener> (std::move (cb)));
            listenerList.add (listeners.back().get());
        }

        void removeListener (int i) { listenerList.remove (listeners[(size_t) i].get()); }

        void callListeners()
        {
            ++callLevel;
            listenerList.call ([] (auto& l) { l.doCallback(); });
            --callLevel;
        }

        int getNumListeners() const { return (int) listeners.size(); }

        auto& getListener (int i) { return *listeners[(size_t) i]; }

        int getCallLevel() const
        {
            return callLevel;
        }

        bool wereAllNonRemovedListenersCalled (int numCalls) const
        {
            return std::all_of (std::begin (listeners),
                                std::end (listeners),
                                [&] (auto& listener)
                                {
                                    return (! listenerList.contains (listener.get())) || listener->getNumCalls() == numCalls;
                                });
        }

    private:
        std::vector<std::unique_ptr<TestListener>> listeners;
        ListenerList<TestListener> listenerList;
        int callLevel = 0;
    };

    //==============================================================================
    ListenerListTests() : UnitTest ("ListenerList", UnitTestCategories::containers) {}

    void runTest() override
    {
        // This is a test that the pre-iterator adjustment implementation should pass too
        beginTest ("All non-removed listeners should be called - removing an already called listener");
        {
            TestObject test;

            for (int i = 0; i < 20; ++i)
            {
                test.addListener ([i, &test]
                                  {
                                      if (i == 5)
                                          test.removeListener (6);
                                  });
            }

            test.callListeners();
            expect (test.wereAllNonRemovedListenersCalled (1));
        }

        // Iterator adjustment is necessary for passing this
        beginTest ("All non-removed listeners should be called - removing a yet uncalled listener");
        {
            TestObject test;

            for (int i = 0; i < 20; ++i)
            {
                test.addListener ([i, &test]
                                  {
                                      if (i == 5)
                                          test.removeListener (4);
                                  });
            }

            test.callListeners();
            expect (test.wereAllNonRemovedListenersCalled (1));
        }

        // This test case demonstrates why we have to call --it.index instead of it.next()
        beginTest ("All non-removed listeners should be called - one callback removes multiple listeners");
        {
            TestObject test;

            for (int i = 0; i < 20; ++i)
            {
                test.addListener ([i, &test]
                                  {
                                      if (i == 19)
                                      {
                                          test.removeListener (19);
                                          test.removeListener (0);
                                      }
                                  });
            }

            test.callListeners();
            expect (test.wereAllNonRemovedListenersCalled (1));
        }

        beginTest ("All non-removed listeners should be called - removing listeners randomly");
        {
            auto random = getRandom();

            for (auto run = 0; run < 10; ++run)
            {
                const auto numListeners = random.nextInt ({ 10, 100 });
                const auto listenersThatRemoveListeners = chooseUnique (random,
                                                                        numListeners,
                                                                        random.nextInt ({ 0, numListeners / 2 }));

                // The listener in position [key] should remove listeners in [value]
                std::map<int, std::set<int>> removals;

                for (auto i : listenersThatRemoveListeners)
                {
                    // Random::nextInt ({1, 1}); triggers an assertion
                    removals[i] = chooseUnique (random,
                                                numListeners,
                                                random.nextInt ({ 1, std::max (2, numListeners / 10) }));
                }

                TestObject test;

                for (int i = 0; i < numListeners; ++i)
                {
                    test.addListener ([i, &removals, &test]
                                      {
                                          const auto iter = removals.find (i);

                                          if (iter == removals.end())
                                              return;

                                          for (auto j : iter->second)
                                          {
                                              test.removeListener (j);
                                          }
                                      });
                }

                test.callListeners();
                expect (test.wereAllNonRemovedListenersCalled (1));
            }
        }

        // Iterator adjustment is not necessary for passing this
        beginTest ("All non-removed listeners should be called - add listener during iteration");
        {
            TestObject test;
            const auto numStartingListeners = 20;

            for (int i = 0; i < numStartingListeners; ++i)
            {
                test.addListener ([i, &test]
                                  {
                                      if (i == 5 || i == 6)
                                          test.addListener ([] {});
                                  });
            }

            test.callListeners();

            // Only the Listeners added before the test can be expected to have been called
            bool success = true;

            for (int i = 0; i < numStartingListeners; ++i)
                success = success && test.getListener (i).getNumCalls() == 1;

            // Listeners added during the iteration must not be called in that iteration
            for (int i = numStartingListeners; i < test.getNumListeners(); ++i)
                success = success && test.getListener (i).getNumCalls() == 0;

            expect (success);
        }

        beginTest ("All non-removed listeners should be called - nested ListenerList::call()");
        {
            TestObject test;

            for (int i = 0; i < 20; ++i)
            {
                test.addListener ([i, &test]
                                  {
                                      const auto callLevel = test.getCallLevel();

                                      if (i == 6 && callLevel == 1)
                                      {
                                          test.callListeners();
                                      }

                                      if (i == 5)
                                      {
                                          if (callLevel == 1)
                                              test.removeListener (4);
                                          else if (callLevel == 2)
                                              test.removeListener (6);
                                      }
                                  });
            }

            test.callListeners();
            expect (test.wereAllNonRemovedListenersCalled (2));
        }

        beginTest ("All non-removed listeners should be called - random ListenerList::call()");
        {
            const auto numListeners = 20;
            auto random = getRandom();

            for (int run = 0; run < 10; ++run)
            {
                TestObject test;
                auto numCalls = 0;

                auto listenersToRemove = chooseUnique (random, numListeners, numListeners / 2);

                for (int i = 0; i < numListeners; ++i)
                {
                    // Capturing numListeners is a warning on MacOS, not capturing it is an error on Windows
                    test.addListener ([&]
                                      {
                                          const auto callLevel = test.getCallLevel();

                                          if (callLevel < 4 && random.nextFloat() < 0.05f)
                                          {
                                              ++numCalls;
                                              test.callListeners();
                                          }

                                          if (random.nextFloat() < 0.5f)
                                          {
                                              const auto listenerToRemove = random.nextInt ({ 0, numListeners });

                                              if (listenersToRemove.erase (listenerToRemove) > 0)
                                                  test.removeListener (listenerToRemove);
                                          }
                                      });
                }

                while (listenersToRemove.size() > 0)
                {
                    test.callListeners();
                    ++numCalls;
                }

                expect (test.wereAllNonRemovedListenersCalled (numCalls));
            }
        }

        beginTest ("Deleting the listener list from a callback");
        {
            struct Listener
            {
                std::function<void()> onCallback;
                void notify() { onCallback(); }
            };

            auto listeners = std::make_unique<juce::ListenerList<Listener>>();

            const auto callback = [&]
            {
                expect (listeners != nullptr);
                listeners.reset();
            };

            Listener listener1 { callback };
            Listener listener2 { callback };

            listeners->add (&listener1);
            listeners->add (&listener2);

            listeners->call (&Listener::notify);

            expect (listeners == nullptr);
        }

        beginTest ("Using a BailOutChecker");
        {
            struct Listener
            {
                std::function<void()> onCallback;
                void notify() { onCallback(); }
            };

            ListenerList<Listener> listeners;

            bool listener1Called = false;
            bool listener2Called = false;
            bool listener3Called = false;

            Listener listener1 { [&]{ listener1Called = true; } };
            Listener listener2 { [&]{ listener2Called = true; } };
            Listener listener3 { [&]{ listener3Called = true; } };

            listeners.add (&listener1);
            listeners.add (&listener2);
            listeners.add (&listener3);

            struct BailOutChecker
            {
                bool& bailOutBool;
                bool shouldBailOut() const { return bailOutBool; }
            };

            BailOutChecker bailOutChecker { listener2Called };
            listeners.callChecked (bailOutChecker, &Listener::notify);

            expect (  listener1Called);
            expect (  listener2Called);
            expect (! listener3Called);
        }

        beginTest ("Using a critical section");
        {
            struct Listener
            {
                std::function<void()> onCallback;
                void notify() { onCallback(); }
            };

            struct TestCriticalSection
            {
                TestCriticalSection()  { isAlive() = true; }
                ~TestCriticalSection() { isAlive() = false; }

                static void enter() noexcept { numOutOfScopeCalls() += isAlive() ? 0 : 1; }
                static void exit() noexcept  { numOutOfScopeCalls() += isAlive() ? 0 : 1; }

                static bool tryEnter() noexcept
                {
                    numOutOfScopeCalls() += isAlive() ? 0 : 1;
                    return true;
                }

                using ScopedLockType = GenericScopedLock<TestCriticalSection>;

                static bool& isAlive()
                {
                    static bool inScope = false;
                    return inScope;
                }

                static int& numOutOfScopeCalls()
                {
                    static int numOutOfScopeCalls = 0;
                    return numOutOfScopeCalls;
                }
            };

            auto listeners = std::make_unique<juce::ListenerList<Listener, Array<Listener*, TestCriticalSection>>>();

            const auto callback = [&]{ listeners.reset(); };

            Listener listener { callback };

            listeners->add (&listener);
            listeners->call (&Listener::notify);

            expect (listeners == nullptr);
            expect (TestCriticalSection::numOutOfScopeCalls() == 0);
        }

        beginTest ("Adding a listener during a callback when one has already been removed");
        {
            struct Listener{};

            ListenerList<Listener> listeners;
            expect (listeners.size() == 0);

            Listener listener;
            listeners.add (&listener);
            expect (listeners.size() == 1);

            bool listenerCalled = false;

            listeners.call ([&] (auto& l)
            {
                listeners.remove (&l);
                expect (listeners.size() == 0);

                listeners.add (&l);
                expect (listeners.size() == 1);

                listenerCalled = true;
            });

            expect (listenerCalled);
            expect (listeners.size() == 1);
        }
    }

private:
    static std::set<int> chooseUnique (Random& random, int max, int numChosen)
    {
        std::set<int> result;

        while ((int) result.size() < numChosen)
            result.insert (random.nextInt ({ 0, max }));

        return result;
    }
};

static ListenerListTests listenerListTests;

#endif

} // namespace juce
