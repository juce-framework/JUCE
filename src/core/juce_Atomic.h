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

#ifndef __JUCE_ATOMIC_JUCEHEADER__
#define __JUCE_ATOMIC_JUCEHEADER__


//==============================================================================
/**
    Simple class to hold a primitive value and perform atomic operations on it.

    The type used must be a 32 or 64 bit primitive, like an int, pointer, etc.
    There are methods to perform most of the basic atomic operations.
*/
template <typename Type>
class Atomic
{
public:
    /** Creates a new value, initialised to zero. */
    inline Atomic() throw()
        : value (0)
    {
    }

    /** Creates a new value, with a given initial value. */
    inline Atomic (const Type initialValue) throw()
        : value (initialValue)
    {
    }

    /** Copies another value (atomically). */
    inline Atomic (const Atomic& other) throw()
        : value (other.get())
    {
    }

    /** Copies another value onto this one (atomically). */
    inline Atomic& operator= (const Atomic& other) throw()
    {
        set (other.get());
    }

    /** Destructor. */
    inline ~Atomic() throw()
    {
        // This class can only be used for types which are 32 or 64 bits in size.
        static_jassert (sizeof (Type) == 4 || sizeof (Type) == 8);
    }

    /** Atomically reads and returns the current value. */
    Type get() const throw();

    /** Atomically sets the current value. */
    void set (Type newValue) throw();

    /** Atomically sets the current value, returning the value that was replaced. */
    Type exchange (Type value) throw();

    /** Atomically adds a number to this value, returning the new value. */
    Type operator+= (Type amountToAdd) throw();

    /** Atomically subtracts a number from this value, returning the new value. */
    Type operator-= (Type amountToSubtract) throw();

    /** Atomically increments this value, returning the new value. */
    Type operator++() throw();

    /** Atomically decrements this value, returning the new value. */
    Type operator--() throw();

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        bool compareAndSetBool (Type newValue, Type valueToCompare) throw();
        {
            if (get() == valueToCompare)
            {
                set (newValue);
                return true;
            }

            return false;
        }
        @endcode

        @returns true if the comparison was true and the value was replaced; false if
                 the comparison failed and the value was left unchanged.
        @see compareAndSetValue
    */
    bool compareAndSetBool (Type newValue, Type valueToCompare) throw();

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        Type compareAndSetValue (Type newValue, Type valueToCompare) throw();
        {
            Type oldValue = get();
            if (oldValue == valueToCompare)
                set (newValue);

            return oldValue;
        }
        @endcode

        @returns the old value before it was changed.
        @see compareAndSetBool
    */
    Type compareAndSetValue (Type newValue, Type valueToCompare) throw();

    /** Performs a memory write barrier. */
    static void memoryBarrier() throw();

    //==============================================================================
    #if JUCE_MSVC
      __declspec (align (8))
    #else
      __attribute__ ((aligned (8)))
    #endif

    /** The raw value that this class operates on.
        This is exposed publically in case you need to manipulate it directly
        for performance reasons.
    */
    Type value;
};


//==============================================================================
/*
    The following code allows the atomics to be performed as inline functions where possible...
*/
#if (JUCE_IPHONE && (__IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_3_2 || ! defined (__IPHONE_3_2))) \
      || (JUCE_MAC && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 2)))
  #define JUCE_ATOMICS_MAC 1        // Older OSX builds using gcc4.1 or earlier
//==============================================================================
#elif JUCE_GCC
  #define JUCE_ATOMICS_GCC 1        // GCC with intrinsics
//==============================================================================
#else
  #define JUCE_ATOMICS_WINDOWS 1    // Windows with intrinsics

  #if JUCE_USE_INTRINSICS
    #pragma intrinsic (_InterlockedExchange, _InterlockedIncrement, _InterlockedDecrement, _InterlockedCompareExchange, \
                       _InterlockedCompareExchange64, _InterlockedExchangeAdd, _ReadWriteBarrier)
    #define juce_InterlockedExchange(a, b)              _InterlockedExchange(a, b)
    #define juce_InterlockedIncrement(a)                _InterlockedIncrement(a)
    #define juce_InterlockedDecrement(a)                _InterlockedDecrement(a)
    #define juce_InterlockedExchangeAdd(a, b)           _InterlockedExchangeAdd(a, b)
    #define juce_InterlockedCompareExchange(a, b, c)    _InterlockedCompareExchange(a, b, c)
    #define juce_InterlockedCompareExchange64(a, b, c)  _InterlockedCompareExchange64(a, b, c)
    #define juce_MemoryBarrier MemoryBarrier
  #else
    // (these are defined in juce_win32_Threads.cpp)
    long juce_InterlockedExchange (volatile long* a, long b) throw();
    long juce_InterlockedIncrement (volatile long* a) throw();
    long juce_InterlockedDecrement (volatile long* a) throw();
    long juce_InterlockedExchangeAdd (volatile long* a, long b) throw();
    long juce_InterlockedCompareExchange (volatile long* a, long b, long c) throw();
    __int64 juce_InterlockedCompareExchange64 (volatile __int64* a, __int64 b, __int64 c) throw();
    static void juce_MemoryBarrier() throw()   { long x = 0; juce_InterlockedIncrement (&x); }
  #endif

  #if JUCE_64BIT
    #pragma intrinsic (_InterlockedExchangeAdd64, _InterlockedExchange64, _InterlockedIncrement64, _InterlockedDecrement64)
    #define juce_InterlockedExchangeAdd64(a, b)     _InterlockedExchangeAdd64(a, b)
    #define juce_InterlockedExchange64(a, b)        _InterlockedExchange64(a, b)
    #define juce_InterlockedIncrement64(a)          _InterlockedIncrement64(a)
    #define juce_InterlockedDecrement64(a)          _InterlockedDecrement64(a)
  #else
    // None of these atomics are available in a 32-bit Windows build!!
    static __int64 juce_InterlockedExchangeAdd64 (volatile __int64* a, __int64 b) throw()   { jassertfalse; __int64 old = *a; *a += b; return old; }
    static __int64 juce_InterlockedExchange64 (volatile __int64* a, __int64 b) throw()      { jassertfalse; __int64 old = *a; *a = b; return old; }
    static __int64 juce_InterlockedIncrement64 (volatile __int64* a) throw()                { jassertfalse; return ++*a; }
    static __int64 juce_InterlockedDecrement64 (volatile __int64* a) throw()                { jassertfalse; return --*a; }
  #endif
#endif

//==============================================================================
template <typename Type>
inline Type Atomic<Type>::get() const throw()
{
    return const_cast <Atomic<Type>*> (this)->operator+= (0);
}

template <typename Type>
inline void Atomic<Type>::set (const Type newValue) throw()
{
    exchange (newValue);
}

template <typename Type>
Type Atomic<Type>::exchange (const Type newValue) throw()
{
  #if JUCE_ATOMICS_MAC || JUCE_ATOMICS_GCC
    Type currentVal = value;
    while (! compareAndSetBool (newValue, currentVal)) { currentVal = value; }
    return currentVal;
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedExchange ((volatile long*) &value, (long) newValue)
                              : (Type) juce_InterlockedExchange64 ((volatile __int64*) &value, (__int64) newValue);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator+= (const Type amountToAdd) throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? (Type) OSAtomicAdd32 ((int32_t) amountToAdd, (int32_t*) &value)
                              : (Type) OSAtomicAdd64 ((int64_t) amountToAdd, (int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) (juce_InterlockedExchangeAdd ((volatile long*) &value, (long) amountToAdd) + (long) amountToAdd)
                              : (Type) (juce_InterlockedExchangeAdd64 ((volatile __int64*) &value, (__int64) amountToAdd) + (__int64) amountToAdd);
  #elif JUCE_ATOMICS_GCC
    return (Type) __sync_add_and_fetch (&value, amountToAdd);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator-= (const Type amountToSubtract) throw()
{
    return operator+= (sizeof (Type) == 4 ? (Type) (-(int32) amountToSubtract)
                                          : (Type) (-(int64) amountToSubtract));
}

template <typename Type>
inline Type Atomic<Type>::operator++() throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? (Type) OSAtomicIncrement32 ((int32_t*) &value)
                              : (Type) OSAtomicIncrement64 ((int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedIncrement ((volatile long*) &value)
                              : (Type) juce_InterlockedIncrement64 ((volatile __int64*) &value);
  #elif JUCE_ATOMICS_GCC
    return (Type) __sync_add_and_fetch (&value, 1);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator--() throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? (Type) OSAtomicDecrement32 ((int32_t*) &value)
                              : (Type) OSAtomicDecrement64 ((int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedDecrement ((volatile long*) &value)
                              : (Type) juce_InterlockedDecrement64 ((volatile __int64*) &value);
  #elif JUCE_ATOMICS_GCC
    return (Type) __sync_add_and_fetch (&value, -1);
  #endif
}

template <typename Type>
inline bool Atomic<Type>::compareAndSetBool (const Type newValue, const Type valueToCompare) throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? (Type) OSAtomicCompareAndSwap32Barrier ((int32_t) valueToCompare, (int32_t) newValue, (int32_t*) &value)
                              : (Type) OSAtomicCompareAndSwap64Barrier ((int64_t) valueToCompare, (int64_t) newValue, (int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return compareAndSetValue (newValue, valueToCompare) == valueToCompare;
  #elif JUCE_ATOMICS_GCC
    return __sync_bool_compare_and_swap (&value, valueToCompare, newValue);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::compareAndSetValue (const Type newValue, const Type valueToCompare) throw()
{
  #if JUCE_ATOMICS_MAC
    for (;;) // Annoying workaround for OSX only having a bool CAS operation..
    {
        if (compareAndSetBool (newValue, valueToCompare))
            return valueToCompare;

        const Type result = value;
        if (result != valueToCompare)
            return result;
    }

  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedCompareExchange ((volatile long*) &value, (long) newValue, (long) valueToCompare)
                              : (Type) juce_InterlockedCompareExchange64 ((volatile __int64*) &value, (__int64) newValue, (__int64) valueToCompare);
  #elif JUCE_ATOMICS_GCC
    return __sync_val_compare_and_swap (&value, valueToCompare, newValue);
  #endif
}

template <typename Type>
inline void Atomic<Type>::memoryBarrier() throw()
{
  #if JUCE_ATOMICS_MAC
    OSMemoryBarrier();
  #elif JUCE_ATOMICS_GCC
    __sync_synchronize();
  #elif JUCE_ATOMICS_WINDOWS
    juce_MemoryBarrier();
  #endif
}


#endif   // __JUCE_ATOMIC_JUCEHEADER__
