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
// Atomic increment/decrement operations..


//==============================================================================
#if (JUCE_MAC || JUCE_IPHONE) && ! DOXYGEN

    //==============================================================================
    #include <libkern/OSAtomic.h>
    static forcedinline void atomicIncrement (int& variable) throw()           { OSAtomicIncrement32 ((int32_t*) &variable); }
    static forcedinline int atomicIncrementAndReturn (int& variable) throw()   { return OSAtomicIncrement32 ((int32_t*) &variable); }
    static forcedinline void atomicDecrement (int& variable) throw()           { OSAtomicDecrement32 ((int32_t*) &variable); }
    static forcedinline int atomicDecrementAndReturn (int& variable) throw()   { return OSAtomicDecrement32 ((int32_t*) &variable); }

#elif JUCE_GCC
    //==============================================================================
  #if JUCE_USE_GCC_ATOMIC_INTRINSICS
    forcedinline void atomicIncrement (int& variable) throw()           { __sync_add_and_fetch (&variable, 1); }
    forcedinline int atomicIncrementAndReturn (int& variable) throw()   { return __sync_add_and_fetch (&variable, 1); }
    forcedinline void atomicDecrement (int& variable) throw()           { __sync_add_and_fetch (&variable, -1); }
    forcedinline int atomicDecrementAndReturn (int& variable) throw()   { return __sync_add_and_fetch (&variable, -1); }
  #else
    //==============================================================================
    /** Increments an integer in a thread-safe way. */
    forcedinline void atomicIncrement (int& variable) throw()
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

    /** Increments an integer in a thread-safe way and returns the incremented value. */
    forcedinline int atomicIncrementAndReturn (int& variable) throw()
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

    /** Decrememts an integer in a thread-safe way. */
    forcedinline void atomicDecrement (int& variable) throw()
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

    /** Decrememts an integer in a thread-safe way and returns the incremented value. */
    forcedinline int atomicDecrementAndReturn (int& variable) throw()
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

#elif JUCE_USE_INTRINSICS
    //==============================================================================
    #pragma intrinsic (_InterlockedIncrement)
    #pragma intrinsic (_InterlockedDecrement)

    /** Increments an integer in a thread-safe way. */
    forcedinline void __fastcall atomicIncrement (int& variable) throw()
    {
        _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable));
    }

    /** Increments an integer in a thread-safe way and returns the incremented value. */
    forcedinline int __fastcall atomicIncrementAndReturn (int& variable) throw()
    {
        return _InterlockedIncrement (reinterpret_cast <volatile long*> (&variable));
    }

    /** Decrememts an integer in a thread-safe way. */
    forcedinline void __fastcall atomicDecrement (int& variable) throw()
    {
        _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable));
    }

    /** Decrememts an integer in a thread-safe way and returns the incremented value. */
    forcedinline int __fastcall atomicDecrementAndReturn (int& variable) throw()
    {
        return _InterlockedDecrement (reinterpret_cast <volatile long*> (&variable));
    }
#else
    //==============================================================================
    /** Increments an integer in a thread-safe way. */
    forcedinline void __fastcall atomicIncrement (int& variable) throw()
    {
        __asm {
            mov ecx, dword ptr [variable]
            lock inc dword ptr [ecx]
        }
    }

    /** Increments an integer in a thread-safe way and returns the incremented value. */
    forcedinline int __fastcall atomicIncrementAndReturn (int& variable) throw()
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

    /** Decrememts an integer in a thread-safe way. */
    forcedinline void __fastcall atomicDecrement (int& variable) throw()
    {
        __asm {
            mov ecx, dword ptr [variable]
            lock dec dword ptr [ecx]
        }
    }

    /** Decrememts an integer in a thread-safe way and returns the incremented value. */
    forcedinline int __fastcall atomicDecrementAndReturn (int& variable) throw()
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
