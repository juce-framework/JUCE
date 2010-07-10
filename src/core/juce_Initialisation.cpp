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

#include "juce_Atomic.h"
#include "juce_Random.h"
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
#if JUCE_DEBUG

namespace SimpleUnitTests
{
    template <typename Type>
    class AtomicTester
    {
    public:
        AtomicTester() {}

        static void testInteger()
        {
            Atomic<Type> a, b;
            a.set ((Type) 10);
            a += (Type) 15;
            a.memoryBarrier();
            a -= (Type) 5;
            ++a; ++a; --a;
            a.memoryBarrier();

            testFloat();
        }

        static void testFloat()
        {
            Atomic<Type> a, b;
            a = (Type) 21;
            a.memoryBarrier();

            /*  These are some simple test cases to check the atomics - let me know
                if any of these assertions fail on your system!
            */
            jassert (a.get() == (Type) 21);
            jassert (a.compareAndSetValue ((Type) 100, (Type) 50) == (Type) 21);
            jassert (a.get() == (Type) 21);
            jassert (a.compareAndSetValue ((Type) 101, a.get()) == (Type) 21);
            jassert (a.get() == (Type) 101);
            jassert (! a.compareAndSetBool ((Type) 300, (Type) 200));
            jassert (a.get() == (Type) 101);
            jassert (a.compareAndSetBool ((Type) 200, a.get()));
            jassert (a.get() == (Type) 200);

            jassert (a.exchange ((Type) 300) == (Type) 200);
            jassert (a.get() == (Type) 300);

            b = a;
            jassert (b.get() == a.get());
        }
    };

    static void runBasicTests()
    {
        // Some simple test code, to keep an eye on things and make sure these functions
        // work ok on all platforms. Let me know if any of these assertions fail on your system!

        static_jassert (sizeof (pointer_sized_int) == sizeof (void*));
        static_jassert (sizeof (int8) == 1);
        static_jassert (sizeof (uint8) == 1);
        static_jassert (sizeof (int16) == 2);
        static_jassert (sizeof (uint16) == 2);
        static_jassert (sizeof (int32) == 4);
        static_jassert (sizeof (uint32) == 4);
        static_jassert (sizeof (int64) == 8);
        static_jassert (sizeof (uint64) == 8);

        char a1[7];
        jassert (numElementsInArray(a1) == 7);
        int a2[3];
        jassert (numElementsInArray(a2) == 3);

        jassert (ByteOrder::swap ((uint16) 0x1122) == 0x2211);
        jassert (ByteOrder::swap ((uint32) 0x11223344) == 0x44332211);
        jassert (ByteOrder::swap ((uint64) literal64bit (0x1122334455667788)) == literal64bit (0x8877665544332211));

        // Some quick stream tests..
        int randomInt = Random::getSystemRandom().nextInt();
        int64 randomInt64 = Random::getSystemRandom().nextInt64();
        double randomDouble = Random::getSystemRandom().nextDouble();
        String randomString;
        for (int i = 50; --i >= 0;)
            randomString << (juce_wchar) (Random::getSystemRandom().nextInt() & 0xffff);

        MemoryOutputStream mo;
        mo.writeInt (randomInt);
        mo.writeIntBigEndian (randomInt);
        mo.writeCompressedInt (randomInt);
        mo.writeString (randomString);
        mo.writeInt64 (randomInt64);
        mo.writeInt64BigEndian (randomInt64);
        mo.writeDouble (randomDouble);
        mo.writeDoubleBigEndian (randomDouble);

        MemoryInputStream mi (mo.getData(), mo.getDataSize(), false);
        jassert (mi.readInt() == randomInt);
        jassert (mi.readIntBigEndian() == randomInt);
        jassert (mi.readCompressedInt() == randomInt);
        jassert (mi.readString() == randomString);
        jassert (mi.readInt64() == randomInt64);
        jassert (mi.readInt64BigEndian() == randomInt64);
        jassert (mi.readDouble() == randomDouble);
        jassert (mi.readDoubleBigEndian() == randomDouble);

        AtomicTester <int>::testInteger();
        AtomicTester <unsigned int>::testInteger();
        AtomicTester <int32>::testInteger();
        AtomicTester <uint32>::testInteger();
        AtomicTester <long>::testInteger();
        AtomicTester <void*>::testInteger();
        AtomicTester <int*>::testInteger();
        AtomicTester <float>::testFloat();
      #if ! JUCE_64BIT_ATOMICS_UNAVAILABLE  // 64-bit intrinsics aren't available on some old platforms
        AtomicTester <int64>::testInteger();
        AtomicTester <uint64>::testInteger();
        AtomicTester <double>::testFloat();
      #endif
    }
}
#endif

//==============================================================================
static bool juceInitialisedNonGUI = false;

void JUCE_PUBLIC_FUNCTION initialiseJuce_NonGUI()
{
    if (! juceInitialisedNonGUI)
    {
        juceInitialisedNonGUI = true;

        JUCE_AUTORELEASEPOOL

      #if JUCE_DEBUG
        SimpleUnitTests::runBasicTests();
      #endif

        DBG (SystemStats::getJUCEVersion());
        SystemStats::initialiseStats();
        Random::getSystemRandom().setSeedRandomly(); // (mustn't call this before initialiseStats() because it relies on the time being set up)
    }
}

void JUCE_PUBLIC_FUNCTION shutdownJuce_NonGUI()
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

void juce_setCurrentThreadName (const String& name);

static bool juceInitialisedGUI = false;

void JUCE_PUBLIC_FUNCTION initialiseJuce_GUI()
{
    if (! juceInitialisedGUI)
    {
        juceInitialisedGUI = true;

        JUCE_AUTORELEASEPOOL
        initialiseJuce_NonGUI();

        MessageManager::getInstance();
        LookAndFeel::setDefaultLookAndFeel (0);
        juce_setCurrentThreadName ("Juce Message Thread");

      #if JUCE_DEBUG
        // This section is just for catching people who mess up their project settings and
        // turn RTTI off..
        try
        {
            TextButton tb (String::empty);
            Component* c = &tb;

            // Got an exception here? Then TURN ON RTTI in your compiler settings!!
            c = dynamic_cast <Button*> (c);
        }
        catch (...)
        {
            // Ended up here? If so, TURN ON RTTI in your compiler settings!!
            jassertfalse;
        }
      #endif
    }
}

void JUCE_PUBLIC_FUNCTION shutdownJuce_GUI()
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

END_JUCE_NAMESPACE
