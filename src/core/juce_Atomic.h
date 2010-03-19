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

    /** This atomically sets *value1 to be value2, and returns the previous value of *value1. */
    static void* swapPointers (void* volatile* value1, void* value2);
};


//==============================================================================
#if (JUCE_MAC || JUCE_IPHONE)           //  Mac and iPhone...

inline void  Atomic::increment (int32& variable)                { OSAtomicIncrement32 ((int32_t*) &variable); }
inline int32 Atomic::incrementAndReturn (int32& variable)       { return OSAtomicIncrement32 ((int32_t*) &variable); }
inline void  Atomic::decrement (int32& variable)                { OSAtomicDecrement32 ((int32_t*) &variable); }
inline int32 Atomic::decrementAndReturn (int32& variable)       { return OSAtomicDecrement32 ((int32_t*) &variable); }
inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
                                                                { return OSAtomicCompareAndSwap32Barrier (oldValue, newValue, (int32_t*) &destination); }
inline void* Atomic::swapPointers (void* volatile* value1, void* value2)
{
    void* currentVal = *value1;
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5 && ! JUCE_64BIT
    while (! OSAtomicCompareAndSwap32 ((int32_t) currentVal, (int32_t) value2, (int32_t*) value1)) { currentVal = *value1; }
#else
    while (! OSAtomicCompareAndSwapPtr (currentVal, value2, value1)) { currentVal = *value1; }
#endif
    return currentVal;
}

#elif JUCE_LINUX                        // Linux...

//==============================================================================
inline void  Atomic::increment (int32& variable)                { __sync_add_and_fetch (&variable, 1); }
inline int32 Atomic::incrementAndReturn (int32& variable)       { return __sync_add_and_fetch (&variable, 1); }
inline void  Atomic::decrement (int32& variable)                { __sync_add_and_fetch (&variable, -1); }
inline int32 Atomic::decrementAndReturn (int32& variable)       { return __sync_add_and_fetch (&variable, -1); }
inline int32 Atomic::compareAndExchange (int32& destination, int32 newValue, int32 oldValue)
                                                                { return __sync_val_compare_and_swap (&destination, oldValue, newValue); }
inline void* Atomic::swapPointers (void* volatile* value1, void* value2)
{
    void* currentVal = *value1;
    while (! __sync_bool_compare_and_swap (value1, currentVal, value2)) { currentVal = *value1; }
    return currentVal;
}

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
