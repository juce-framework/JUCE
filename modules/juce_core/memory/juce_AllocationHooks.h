/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JUCE_ENABLE_ALLOCATION_HOOKS

namespace juce
{

class AllocationHooks
{
public:
    struct Listener
    {
        virtual ~Listener() noexcept = default;
        virtual void newOrDeleteCalled() noexcept = 0;
    };

    void addListener    (Listener* l)          { listenerList.add (l); }
    void removeListener (Listener* l) noexcept { listenerList.remove (l); }

private:
    friend void notifyAllocationHooksForThread();
    ListenerList<Listener> listenerList;
};

//==============================================================================
/** Scoped checker which will cause a unit test failure if any new/delete calls
    are made during the lifetime of the UnitTestAllocationChecker.
*/
class UnitTestAllocationChecker  : private AllocationHooks::Listener
{
public:
    /** Create a checker which will log a failure to the passed test if
        any calls to new/delete are made.

        Remember to call `UnitTest::beginTest` before constructing this checker!
    */
    explicit UnitTestAllocationChecker (UnitTest& test);

    /** Will add a failure to the test if the number of new/delete calls during
        this object's lifetime was greater than zero.
    */
    ~UnitTestAllocationChecker() noexcept override;

private:
    void newOrDeleteCalled() noexcept override;

    UnitTest& unitTest;
    size_t calls = 0;
};

}

#endif
