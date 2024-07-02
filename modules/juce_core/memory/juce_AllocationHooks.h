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
    LightweightListenerList<Listener> listenerList;
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
