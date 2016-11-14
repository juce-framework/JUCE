/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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

#if JUCE_UNIT_TESTS

struct ListenerBase
{
    ListenerBase (int& counter) : c (counter) {}
    virtual ~ListenerBase () {}

    virtual void f () = 0;
    virtual void f (void*) = 0;
    virtual void f (void*, void*) = 0;
    virtual void f (void*, void*, void*) = 0;
    virtual void f (void*, void*, void*, void*) = 0;
    virtual void f (void*, void*, void*, void*, void*) = 0;
    virtual void f (void*, void*, void*, void*, void*, void*) = 0;

    int& c;
};

struct Listener1 : public ListenerBase
{
    Listener1 (int& counter) : ListenerBase (counter) {}

    void f () override                                         { c += 1; }
    void f (void*) override                                    { c += 2; }
    void f (void*, void*) override                             { c += 3; }
    void f (void*, void*, void*) override                      { c += 4; }
    void f (void*, void*, void*, void*) override               { c += 5; }
    void f (void*, void*, void*, void*, void*) override        { c += 6; }
    void f (void*, void*, void*, void*, void*, void*) override { c += 7; }
};

struct Listener2 : public ListenerBase
{
    Listener2 (int& counter) : ListenerBase (counter) {}

    void f () override                                         { c -= 2; }
    void f (void*) override                                    { c -= 4; }
    void f (void*, void*) override                             { c -= 6; }
    void f (void*, void*, void*) override                      { c -= 8; }
    void f (void*, void*, void*, void*) override               { c -= 10; }
    void f (void*, void*, void*, void*, void*) override        { c -= 12; }
    void f (void*, void*, void*, void*, void*, void*) override { c -= 14; }
};

class ListenerListTests : public UnitTest
{
public:
    ListenerListTests() : UnitTest ("ListenerList") {}

    template <typename T>
    void callHelper (std::vector<int>& expectedCounterValues, T v)
    {
        counter = 0;
        listeners.call (&ListenerBase::f, v);
        expect (counter == expectedCounterValues[1]);

        counter = 0;
        listeners.call (&ListenerBase::f);
        expect (counter == expectedCounterValues[0]);

        ListenerList<ListenerBase>::DummyBailOutChecker boc;

        counter = 0;
        listeners.callChecked (boc, &ListenerBase::f, v);
        expect (counter == expectedCounterValues[1]);

        counter = 0;
        listeners.callChecked (boc, &ListenerBase::f);
        expect (counter == expectedCounterValues[0]);
    }

    template<typename T, typename... Args>
    void callHelper (std::vector<int>& expectedCounterValues, T first, Args... args)
    {
        const int expected = expectedCounterValues[sizeof... (args) + 1];

        counter = 0;
        listeners.call (&ListenerBase::f, first, args...);
        expect (counter == expected);

        ListenerList<ListenerBase>::DummyBailOutChecker boc;
        counter = 0;
        listeners.callChecked (boc, &ListenerBase::f, first, args...);
        expect (counter == expected);

        callHelper (expectedCounterValues, args...);
    }

    template <typename T>
    void callExcludingHelper (ListenerBase& listenerToExclude,
                              std::vector<int>& expectedCounterValues, T v)
    {
        counter = 0;
        listeners.callExcluding (listenerToExclude, &ListenerBase::f, v);
        expect (counter == expectedCounterValues[1]);

        counter = 0;
        listeners.callExcluding (listenerToExclude, &ListenerBase::f);
        expect (counter == expectedCounterValues[0]);

        ListenerList<ListenerBase>::DummyBailOutChecker boc;

        counter = 0;
        listeners.callCheckedExcluding (listenerToExclude, boc, &ListenerBase::f, v);
        expect (counter == expectedCounterValues[1]);

        counter = 0;
        listeners.callCheckedExcluding (listenerToExclude, boc, &ListenerBase::f);
        expect (counter == expectedCounterValues[0]);
    }

    template<typename T, typename... Args>
    void callExcludingHelper (ListenerBase& listenerToExclude,
                              std::vector<int>& expectedCounterValues, T first, Args... args)
    {
        const int expected = expectedCounterValues[sizeof... (args) + 1];

        counter = 0;
        listeners.callExcluding (listenerToExclude, &ListenerBase::f, first, args...);
        expect (counter == expected);

        ListenerList<ListenerBase>::DummyBailOutChecker boc;
        counter = 0;
        listeners.callCheckedExcluding (listenerToExclude, boc, &ListenerBase::f, first, args...);
        expect (counter == expected);

        callExcludingHelper (listenerToExclude, expectedCounterValues, args...);
    }

    void runTest() override
    {
        beginTest ("Call single listener");
        listeners.add (&listener1);
        std::vector<int> expectedCounterValues = { 1, 2, 3, 4, 5, 6, 7 };
        callHelper (expectedCounterValues, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

        beginTest ("Call multiple listeners");
        listeners.add (&listener2);
        expectedCounterValues = { -1, -2, -3, -4, -5, -6, -7 };
        callHelper (expectedCounterValues, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

        beginTest ("Call listeners excluding");
        expectedCounterValues = { 1, 2, 3, 4, 5, 6, 7 };
        callExcludingHelper (listener2, expectedCounterValues, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    }

    int counter = 0;
    ListenerList<ListenerBase> listeners;
    Listener1 listener1 {counter};
    Listener2 listener2 {counter};
};

static ListenerListTests listenerListTests;

#endif
