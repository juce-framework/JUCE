/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_ATOMIC_JUCEHEADER__
#define __JUCE_ATOMIC_JUCEHEADER__


//==============================================================================
/** Contains static functions for thread-safe atomic operations.
*/
class JUCE_API  Atomic
{
public:
    /** Increments an integer in a thread-safe way. */
    static void increment (int32& variable);

    /** Increments an integer in a thread-safe way and returns its new value. */
    static int32 incrementAndReturn (int32& variable);

    /** Decrements an integer in a thread-safe way. */
    static void decrement (int32& variable);

    /** Decrements an integer in a thread-safe way and returns its new value. */
    static int32 decrementAndReturn (int32& variable);

    /** If the current value of destination is equal to requiredCurrentValue, this
        will set it to newValue; otherwise, it will leave it unchanged.
        @returns the new value of destination
    */
    static int32 compareAndExchange (int32& destination, int32 newValue, int32 requiredCurrentValue);
};


//==============================================================================
#if (JUCE_MAC || JUCE_IPHONE)           //  Mac and iPhone...

inline void Atomic::increment (int32& variable)                 { OSAtomicIncrement32 ((volatile int32_t*) &variable); }
inline int32  Atomic::incrementAndReturn (int32& variable)      { return OSAtomicIncrement32 ((volatile int32_t*) &variable); }
inline void Atomic::decrement (int32& variable)                 { OSAtomicDecrement32 ((volatile int32_t*) &variable); }
inline int32  Atomic::decrementAndReturn (int32& variable)      { return OSAtomicDecrement32 ((volatile int32_t*) &variable); }
inline int32  Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
                                                                { return OSAtomicCompareAndSwap32Barrier (oldValue, newValue, (volatile int32_t*) &destination); }

#elif JUCE_GCC                          // Linux...

//==============================================================================
inline void  Atomic::increment (int32& variable)                { __sync_add_and_fetch (&variable, 1); }
inline int32 Atomic::incrementAndReturn (int32& variable)       { return __sync_add_and_fetch (&variable, 1); }
inline void  Atomic::decrement (int32& variable)                { __sync_add_and_fetch (&variable, -1); }
inline int32 Atomic::decrementAndReturn (int32& variable)       { return __sync_add_and_fetch (&variable, -1); }
inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
                                                                { return __sync_val_compare_and_swap (&destination, oldValue, newValue); }

//==============================================================================
#elif JUCE_USE_INTRINSICS               // Windows...

// (If JUCE_USE_INTRINSICS isn't enabled, a fallback version of these methods is
// declared in juce_win32_Threads.cpp)
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedCompareExchange)

inline void  Atomic::increment (int32& variable)                { _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable)); }
inline int32 Atomic::incrementAndReturn (int32& variable)       { return _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable)); }
inline void  Atomic::decrement (int32& variable)                { _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable)); }
inline int32 Atomic::decrementAndReturn (int32& variable)       { return _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable)); }
inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
                                                                { return _InterlockedCompareExchange (reinterpret_cast <volatile long*> (&destination), newValue, oldValue); }

#endif

#endif   // __JUCE_ATOMIC_JUCEHEADER__
