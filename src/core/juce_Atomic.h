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
    static void increment (int& variable);

    /** Increments an integer in a thread-safe way and returns its new value. */
    static int incrementAndReturn (int& variable);

    /** Decrements an integer in a thread-safe way. */
    static void decrement (int& variable);

    /** Decrements an integer in a thread-safe way and returns its new value. */
    static int decrementAndReturn (int& variable);
};


//==============================================================================
#if (JUCE_MAC || JUCE_IPHONE)           //  Mac and iPhone...

#include <libkern/OSAtomic.h>
inline void Atomic::increment (int& variable)               { OSAtomicIncrement32 ((int32_t*) &variable); }
inline int  Atomic::incrementAndReturn (int& variable)      { return OSAtomicIncrement32 ((int32_t*) &variable); }
inline void Atomic::decrement (int& variable)               { OSAtomicDecrement32 ((int32_t*) &variable); }
inline int  Atomic::decrementAndReturn (int& variable)      { return OSAtomicDecrement32 ((int32_t*) &variable); }

#elif JUCE_GCC

//==============================================================================
#if JUCE_USE_GCC_ATOMIC_INTRINSICS      //  Linux with intrinsics...

inline void Atomic::increment (int& variable)               { __sync_add_and_fetch (&variable, 1); }
inline int  Atomic::incrementAndReturn (int& variable)      { return __sync_add_and_fetch (&variable, 1); }
inline void Atomic::decrement (int& variable)               { __sync_add_and_fetch (&variable, -1); }
inline int  Atomic::decrementAndReturn (int& variable)      { return __sync_add_and_fetch (&variable, -1); }

//==============================================================================
#else                                   //  Linux without intrinsics...

inline void Atomic::increment (int& variable)
{
    __asm__ __volatile__ (
    #if JUCE_64BIT
        "lock incl (%%rax)"
        :
        : "a" (&variable)
        : "cc", "memory");
    #else
        "lock incl %0"
        : "=m" (variable)
        : "m" (variable));
    #endif
}

inline int Atomic::incrementAndReturn (int& variable)
{
    int result;

    __asm__ __volatile__ (
    #if JUCE_64BIT
        "lock xaddl %%ebx, (%%rax) \n\
         incl %%ebx"
        : "=b" (result)
        : "a" (&variable), "b" (1)
        : "cc", "memory");
    #else
        "lock xaddl %%eax, (%%ecx) \n\
         incl %%eax"
        : "=a" (result)
        : "c" (&variable), "a" (1)
        : "memory");
    #endif

    return result;
}

inline void Atomic::decrement (int& variable)
{
    __asm__ __volatile__ (
    #if JUCE_64BIT
        "lock decl (%%rax)"
        :
        : "a" (&variable)
        : "cc", "memory");
    #else
        "lock decl %0"
        : "=m" (variable)
        : "m" (variable));
    #endif
}

inline int Atomic::decrementAndReturn (int& variable)
{
    int result;

    __asm__ __volatile__ (
    #if JUCE_64BIT
        "lock xaddl %%ebx, (%%rax) \n\
         decl %%ebx"
        : "=b" (result)
        : "a" (&variable), "b" (-1)
        : "cc", "memory");
    #else
        "lock xaddl %%eax, (%%ecx) \n\
         decl %%eax"
        : "=a" (result)
        : "c" (&variable), "a" (-1)
        : "memory");
    #endif
    return result;
}
#endif

//==============================================================================
#elif JUCE_USE_INTRINSICS           // Windows with intrinsics...

#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)

inline void Atomic::increment (int& variable)               { _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable)); }
inline int  Atomic::incrementAndReturn (int& variable)      { return _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable)); }
inline void Atomic::decrement (int& variable)               { _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable)); }
inline int  Atomic::decrementAndReturn (int& variable)      { return _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable)); }

//==============================================================================
#else                               // Windows without intrinsics...

inline void Atomic::increment (int& variable)
{
    __asm {
        mov ecx, dword ptr [variable]
        lock inc dword ptr [ecx]
    }
}

inline int Atomic::incrementAndReturn (int& variable)
{
    int result;

    __asm {
        mov ecx, dword ptr [variable]
        mov eax, 1
        lock xadd dword ptr [ecx], eax
        inc eax
        mov result, eax
    }

    return result;
}

inline void Atomic::decrement (int& variable)
{
    __asm {
        mov ecx, dword ptr [variable]
        lock dec dword ptr [ecx]
    }
}

inline int Atomic::decrementAndReturn (int& variable)
{
    int result;

    __asm {
        mov ecx, dword ptr [variable]
        mov eax, -1
        lock xadd dword ptr [ecx], eax
        dec eax
        mov result, eax
    }

    return result;
}

#endif

#endif   // __JUCE_ATOMIC_JUCEHEADER__
