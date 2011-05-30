/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_PlatformUtilities.h"
#include "../utilities/juce_DeletedAtShutdown.h"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #include "../events/juce_MessageManager.h"
#endif

//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

static bool juceInitialisedGUI = false;

JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI()
{
    if (! juceInitialisedGUI)
    {
        juceInitialisedGUI = true;

        JUCE_AUTORELEASEPOOL
        MessageManager::getInstance();
    }
}

JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI()
{
    if (juceInitialisedGUI)
    {
        juceInitialisedGUI = false;

        JUCE_AUTORELEASEPOOL
        DeletedAtShutdown::deleteAll();
        delete MessageManager::getInstance();
    }
}

#endif

//==============================================================================
#if JUCE_UNIT_TESTS

#include "../utilities/juce_UnitTest.h"
#include "../memory/juce_Atomic.h"

class AtomicTests  : public UnitTest
{
public:
    AtomicTests() : UnitTest ("Atomics") {}

    void runTest()
    {
        beginTest ("Misc");

        char a1[7];
        expect (numElementsInArray(a1) == 7);
        int a2[3];
        expect (numElementsInArray(a2) == 3);

        expect (ByteOrder::swap ((uint16) 0x1122) == 0x2211);
        expect (ByteOrder::swap ((uint32) 0x11223344) == 0x44332211);
        expect (ByteOrder::swap ((uint64) literal64bit (0x1122334455667788)) == literal64bit (0x8877665544332211));

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

END_JUCE_NAMESPACE
