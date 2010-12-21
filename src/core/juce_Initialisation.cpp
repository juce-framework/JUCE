/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../memory/juce_Atomic.h"
#include "../maths/juce_Random.h"
#include "juce_PlatformUtilities.h"
#include "juce_SystemStats.h"
#include "../text/juce_LocalisedStrings.h"
#include "../io/streams/juce_MemoryOutputStream.h"
#include "../io/streams/juce_MemoryInputStream.h"
#include "../threads/juce_Thread.h"

#if ! JUCE_ONLY_BUILD_CORE_LIBRARY
 #include "../events/juce_MessageManager.h"
 #include "../gui/components/buttons/juce_TextButton.h"
 #include "../gui/components/lookandfeel/juce_LookAndFeel.h"
#endif

#if JUCE_WINDOWS
 extern void juce_shutdownWin32Sockets();  // (defined in the sockets code)
#endif

#if JUCE_DEBUG
 extern void juce_CheckForDanglingStreams();  // (in juce_OutputStream.cpp)
#endif


//==============================================================================
static bool juceInitialisedNonGUI = false;

JUCE_API void JUCE_CALLTYPE initialiseJuce_NonGUI()
{
    if (! juceInitialisedNonGUI)
    {
        juceInitialisedNonGUI = true;

        JUCE_AUTORELEASEPOOL

        DBG (SystemStats::getJUCEVersion());
        SystemStats::initialiseStats();
        Random::getSystemRandom().setSeedRandomly(); // (mustn't call this before initialiseStats() because it relies on the time being set up)
    }

    // Some basic tests, to keep an eye on things and make sure these types work ok
    // on all platforms. Let me know if any of these assertions fail on your system!
    static_jassert (sizeof (pointer_sized_int) == sizeof (void*));
    static_jassert (sizeof (int8) == 1);
    static_jassert (sizeof (uint8) == 1);
    static_jassert (sizeof (int16) == 2);
    static_jassert (sizeof (uint16) == 2);
    static_jassert (sizeof (int32) == 4);
    static_jassert (sizeof (uint32) == 4);
    static_jassert (sizeof (int64) == 8);
    static_jassert (sizeof (uint64) == 8);
}

JUCE_API void JUCE_CALLTYPE shutdownJuce_NonGUI()
{
    if (juceInitialisedNonGUI)
    {
        juceInitialisedNonGUI = false;

        JUCE_AUTORELEASEPOOL

        LocalisedStrings::setCurrentMappings (0);
        Thread::stopAllThreads (3000);

      #if JUCE_WINDOWS
        juce_shutdownWin32Sockets();
      #endif

      #if JUCE_DEBUG
        juce_CheckForDanglingStreams();
      #endif
    }
}

//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

static bool juceInitialisedGUI = false;

JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI()
{
    if (! juceInitialisedGUI)
    {
        juceInitialisedGUI = true;

        JUCE_AUTORELEASEPOOL
        initialiseJuce_NonGUI();

        MessageManager::getInstance();
        LookAndFeel::setDefaultLookAndFeel (0);

      #if JUCE_DEBUG
        try  // This section is just a safety-net for catching builds without RTTI enabled..
        {
            MemoryOutputStream mo;
            OutputStream* o = &mo;

            // Got an exception here? Then TURN ON RTTI in your compiler settings!!
            o = dynamic_cast <MemoryOutputStream*> (o);
            jassert (o != 0);
        }
        catch (...)
        {
            // Ended up here? If so, TURN ON RTTI in your compiler settings!!
            jassertfalse;
        }
      #endif
    }
}

JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI()
{
    if (juceInitialisedGUI)
    {
        juceInitialisedGUI = false;

        JUCE_AUTORELEASEPOOL
        DeletedAtShutdown::deleteAll();
        LookAndFeel::clearDefaultLookAndFeel();
        delete MessageManager::getInstance();

        shutdownJuce_NonGUI();
    }
}

#endif


//==============================================================================
#if JUCE_UNIT_TESTS

#include "../utilities/juce_UnitTest.h"

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

        beginTest ("Atomic types");
        AtomicTester <int>::testInteger (*this);
        AtomicTester <unsigned int>::testInteger (*this);
        AtomicTester <int32>::testInteger (*this);
        AtomicTester <uint32>::testInteger (*this);
        AtomicTester <long>::testInteger (*this);
        AtomicTester <void*>::testInteger (*this);
        AtomicTester <int*>::testInteger (*this);
        AtomicTester <float>::testFloat (*this);
      #if ! JUCE_64BIT_ATOMICS_UNAVAILABLE  // 64-bit intrinsics aren't available on some old platforms
        AtomicTester <int64>::testInteger (*this);
        AtomicTester <uint64>::testInteger (*this);
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
            a += (Type) 15;
            a.memoryBarrier();
            a -= (Type) 5;
            ++a; ++a; --a;
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
