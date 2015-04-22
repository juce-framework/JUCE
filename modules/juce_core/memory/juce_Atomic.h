/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_ATOMIC_H_INCLUDED
#define JUCE_ATOMIC_H_INCLUDED


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
    inline Atomic() noexcept
        : value (0)
    {
    }

    /** Creates a new value, with a given initial value. */
    inline explicit Atomic (const Type initialValue) noexcept
        : value (initialValue)
    {
    }

    /** Copies another value (atomically). */
    inline Atomic (const Atomic& other) noexcept
        : value (other.get())
    {
    }

    /** Destructor. */
    inline ~Atomic() noexcept
    {
        // This class can only be used for types which are 32 or 64 bits in size.
        static_jassert (sizeof (Type) == 4 || sizeof (Type) == 8);
    }

    /** Atomically reads and returns the current value. */
    Type get() const noexcept;

    /** Copies another value onto this one (atomically). */
    inline Atomic& operator= (const Atomic& other) noexcept         { exchange (other.get()); return *this; }

    /** Copies another value onto this one (atomically). */
    inline Atomic& operator= (const Type newValue) noexcept         { exchange (newValue); return *this; }

    /** Atomically sets the current value. */
    void set (Type newValue) noexcept                               { exchange (newValue); }

    /** Atomically sets the current value, returning the value that was replaced. */
    Type exchange (Type value) noexcept;

    /** Atomically adds a number to this value, returning the new value. */
    Type operator+= (Type amountToAdd) noexcept;

    /** Atomically subtracts a number from this value, returning the new value. */
    Type operator-= (Type amountToSubtract) noexcept;

    /** Atomically increments this value, returning the new value. */
    Type operator++() noexcept;

    /** Atomically decrements this value, returning the new value. */
    Type operator--() noexcept;

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        bool compareAndSetBool (Type newValue, Type valueToCompare)
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
    bool compareAndSetBool (Type newValue, Type valueToCompare) noexcept;

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        Type compareAndSetValue (Type newValue, Type valueToCompare)
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
    Type compareAndSetValue (Type newValue, Type valueToCompare) noexcept;

    /** Implements a memory read/write barrier. */
    static void memoryBarrier() noexcept;

    //==============================================================================
   #if JUCE_64BIT
    JUCE_ALIGN (8)
   #else
    JUCE_ALIGN (4)
   #endif

    /** The raw value that this class operates on.
        This is exposed publically in case you need to manipulate it directly
        for performance reasons.
    */
    volatile Type value;

private:
    template <typename Dest, typename Source>
    static inline Dest castTo (Source value) noexcept         { union { Dest d; Source s; } u; u.s = value; return u.d; }

    static inline Type castFrom32Bit (int32 value) noexcept   { return castTo <Type, int32> (value); }
    static inline Type castFrom64Bit (int64 value) noexcept   { return castTo <Type, int64> (value); }
    static inline int32 castTo32Bit (Type value) noexcept     { return castTo <int32, Type> (value); }
    static inline int64 castTo64Bit (Type value) noexcept     { return castTo <int64, Type> (value); }

    Type operator++ (int); // better to just use pre-increment with atomics..
    Type operator-- (int);

    /** This templated negate function will negate pointers as well as integers */
    template <typename ValueType>
    inline ValueType negateValue (ValueType n) noexcept
    {
        return sizeof (ValueType) == 1 ? (ValueType) -(signed char) n
            : (sizeof (ValueType) == 2 ? (ValueType) -(short) n
            : (sizeof (ValueType) == 4 ? (ValueType) -(int) n
            : ((ValueType) -(int64) n)));
    }

    /** This templated negate function will negate pointers as well as integers */
    template <typename PointerType>
    inline PointerType* negateValue (PointerType* n) noexcept
    {
        return reinterpret_cast <PointerType*> (-reinterpret_cast <pointer_sized_int> (n));
    }
};


//==============================================================================
/*
    The following code is in the header so that the atomics can be inlined where possible...
*/
#if JUCE_MAC && (JUCE_PPC || __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 2))
  #define JUCE_ATOMICS_MAC_LEGACY 1      // Older OSX builds using gcc4.1 or earlier

  #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    #define JUCE_MAC_ATOMICS_VOLATILE
  #else
    #define JUCE_MAC_ATOMICS_VOLATILE volatile
  #endif

  #if JUCE_PPC
    // None of these atomics are available for PPC or for iOS 3.1 or earlier!!
    template <typename Type> static Type OSAtomicAdd64Barrier (Type b, JUCE_MAC_ATOMICS_VOLATILE Type* a) noexcept  { jassertfalse; return *a += b; }
    template <typename Type> static Type OSAtomicIncrement64Barrier (JUCE_MAC_ATOMICS_VOLATILE Type* a) noexcept    { jassertfalse; return ++*a; }
    template <typename Type> static Type OSAtomicDecrement64Barrier (JUCE_MAC_ATOMICS_VOLATILE Type* a) noexcept    { jassertfalse; return --*a; }
    template <typename Type> static bool OSAtomicCompareAndSwap64Barrier (Type old, Type newValue, JUCE_MAC_ATOMICS_VOLATILE Type* value) noexcept
        { jassertfalse; if (old == *value) { *value = newValue; return true; } return false; }
    #define JUCE_64BIT_ATOMICS_UNAVAILABLE 1
  #endif

//==============================================================================
#elif JUCE_GCC || JUCE_CLANG
  #define JUCE_ATOMICS_GCC 1        // GCC with intrinsics

  #if JUCE_IOS || JUCE_ANDROID // (64-bit ops will compile but not link on these mobile OSes)
    #define JUCE_64BIT_ATOMICS_UNAVAILABLE 1
  #endif

//==============================================================================
#else
  #define JUCE_ATOMICS_WINDOWS 1    // Windows with intrinsics

  #if JUCE_USE_MSVC_INTRINSICS
    #ifndef __INTEL_COMPILER
     #pragma intrinsic (_InterlockedExchange, _InterlockedIncrement, _InterlockedDecrement, _InterlockedCompareExchange, \
                        _InterlockedCompareExchange64, _InterlockedExchangeAdd, _ReadWriteBarrier)
    #endif
    #define juce_InterlockedExchange(a, b)              _InterlockedExchange(a, b)
    #define juce_InterlockedIncrement(a)                _InterlockedIncrement(a)
    #define juce_InterlockedDecrement(a)                _InterlockedDecrement(a)
    #define juce_InterlockedExchangeAdd(a, b)           _InterlockedExchangeAdd(a, b)
    #define juce_InterlockedCompareExchange(a, b, c)    _InterlockedCompareExchange(a, b, c)
    #define juce_InterlockedCompareExchange64(a, b, c)  _InterlockedCompareExchange64(a, b, c)
    #define juce_MemoryBarrier _ReadWriteBarrier
  #else
    long juce_InterlockedExchange (volatile long* a, long b) noexcept;
    long juce_InterlockedIncrement (volatile long* a) noexcept;
    long juce_InterlockedDecrement (volatile long* a) noexcept;
    long juce_InterlockedExchangeAdd (volatile long* a, long b) noexcept;
    long juce_InterlockedCompareExchange (volatile long* a, long b, long c) noexcept;
    __int64 juce_InterlockedCompareExchange64 (volatile __int64* a, __int64 b, __int64 c) noexcept;
    inline void juce_MemoryBarrier() noexcept  { long x = 0; juce_InterlockedIncrement (&x); }
  #endif

  #if JUCE_64BIT
    #ifndef __INTEL_COMPILER
     #pragma intrinsic (_InterlockedExchangeAdd64, _InterlockedExchange64, _InterlockedIncrement64, _InterlockedDecrement64)
    #endif
    #define juce_InterlockedExchangeAdd64(a, b)     _InterlockedExchangeAdd64(a, b)
    #define juce_InterlockedExchange64(a, b)        _InterlockedExchange64(a, b)
    #define juce_InterlockedIncrement64(a)          _InterlockedIncrement64(a)
    #define juce_InterlockedDecrement64(a)          _InterlockedDecrement64(a)
  #else
    // None of these atomics are available in a 32-bit Windows build!!
    template <typename Type> static Type juce_InterlockedExchangeAdd64 (volatile Type* a, Type b) noexcept  { jassertfalse; Type old = *a; *a += b; return old; }
    template <typename Type> static Type juce_InterlockedExchange64 (volatile Type* a, Type b) noexcept     { jassertfalse; Type old = *a; *a = b; return old; }
    template <typename Type> static Type juce_InterlockedIncrement64 (volatile Type* a) noexcept            { jassertfalse; return ++*a; }
    template <typename Type> static Type juce_InterlockedDecrement64 (volatile Type* a) noexcept            { jassertfalse; return --*a; }
    #define JUCE_64BIT_ATOMICS_UNAVAILABLE 1
  #endif
#endif


#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4311)  // (truncation warning)
#endif

//==============================================================================
template <typename Type>
inline Type Atomic<Type>::get() const noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) OSAtomicAdd32Barrier ((int32_t) 0, (JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value))
                              : castFrom64Bit ((int64) OSAtomicAdd64Barrier ((int64_t) 0, (JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value));
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) juce_InterlockedExchangeAdd ((volatile long*) &value, (long) 0))
                              : castFrom64Bit ((int64) juce_InterlockedExchangeAdd64 ((volatile __int64*) &value, (__int64) 0));
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) __sync_add_and_fetch ((volatile int32*) &value, 0))
                              : castFrom64Bit ((int64) __sync_add_and_fetch ((volatile int64*) &value, 0));
  #endif
}

template <typename Type>
inline Type Atomic<Type>::exchange (const Type newValue) noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY || JUCE_ATOMICS_GCC
    Type currentVal = value;
    while (! compareAndSetBool (newValue, currentVal)) { currentVal = value; }
    return currentVal;
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) juce_InterlockedExchange ((volatile long*) &value, (long) castTo32Bit (newValue)))
                              : castFrom64Bit ((int64) juce_InterlockedExchange64 ((volatile __int64*) &value, (__int64) castTo64Bit (newValue)));
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator+= (const Type amountToAdd) noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY
    return sizeof (Type) == 4 ? (Type) OSAtomicAdd32Barrier ((int32_t) castTo32Bit (amountToAdd), (JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : (Type) OSAtomicAdd64Barrier ((int64_t) amountToAdd, (JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) (juce_InterlockedExchangeAdd ((volatile long*) &value, (long) amountToAdd) + (long) amountToAdd)
                              : (Type) (juce_InterlockedExchangeAdd64 ((volatile __int64*) &value, (__int64) amountToAdd) + (__int64) amountToAdd);
  #elif JUCE_ATOMICS_GCC
    return (Type) __sync_add_and_fetch (&value, amountToAdd);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator-= (const Type amountToSubtract) noexcept
{
    return operator+= (negateValue (amountToSubtract));
}

template <typename Type>
inline Type Atomic<Type>::operator++() noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY
    return sizeof (Type) == 4 ? (Type) OSAtomicIncrement32Barrier ((JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : (Type) OSAtomicIncrement64Barrier ((JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedIncrement ((volatile long*) &value)
                              : (Type) juce_InterlockedIncrement64 ((volatile __int64*) &value);
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? (Type) __sync_add_and_fetch (&value, (Type) 1)
                              : (Type) __sync_add_and_fetch ((int64_t*) &value, 1);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator--() noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY
    return sizeof (Type) == 4 ? (Type) OSAtomicDecrement32Barrier ((JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : (Type) OSAtomicDecrement64Barrier ((JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedDecrement ((volatile long*) &value)
                              : (Type) juce_InterlockedDecrement64 ((volatile __int64*) &value);
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? (Type) __sync_add_and_fetch (&value, (Type) -1)
                              : (Type) __sync_add_and_fetch ((int64_t*) &value, -1);
  #endif
}

template <typename Type>
inline bool Atomic<Type>::compareAndSetBool (const Type newValue, const Type valueToCompare) noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY
    return sizeof (Type) == 4 ? OSAtomicCompareAndSwap32Barrier ((int32_t) castTo32Bit (valueToCompare), (int32_t) castTo32Bit (newValue), (JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : OSAtomicCompareAndSwap64Barrier ((int64_t) castTo64Bit (valueToCompare), (int64_t) castTo64Bit (newValue), (JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return compareAndSetValue (newValue, valueToCompare) == valueToCompare;
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? __sync_bool_compare_and_swap ((volatile int32*) &value, castTo32Bit (valueToCompare), castTo32Bit (newValue))
                              : __sync_bool_compare_and_swap ((volatile int64*) &value, castTo64Bit (valueToCompare), castTo64Bit (newValue));
  #endif
}

template <typename Type>
inline Type Atomic<Type>::compareAndSetValue (const Type newValue, const Type valueToCompare) noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY
    for (;;) // Annoying workaround for only having a bool CAS operation..
    {
        if (compareAndSetBool (newValue, valueToCompare))
            return valueToCompare;

        const Type result = value;
        if (result != valueToCompare)
            return result;
    }

  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) juce_InterlockedCompareExchange ((volatile long*) &value, (long) castTo32Bit (newValue), (long) castTo32Bit (valueToCompare)))
                              : castFrom64Bit ((int64) juce_InterlockedCompareExchange64 ((volatile __int64*) &value, (__int64) castTo64Bit (newValue), (__int64) castTo64Bit (valueToCompare)));
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) __sync_val_compare_and_swap ((volatile int32*) &value, castTo32Bit (valueToCompare), castTo32Bit (newValue)))
                              : castFrom64Bit ((int64) __sync_val_compare_and_swap ((volatile int64*) &value, castTo64Bit (valueToCompare), castTo64Bit (newValue)));
  #endif
}

template <typename Type>
inline void Atomic<Type>::memoryBarrier() noexcept
{
  #if JUCE_ATOMICS_MAC_LEGACY
    OSMemoryBarrier();
  #elif JUCE_ATOMICS_GCC
    __sync_synchronize();
  #elif JUCE_ATOMICS_WINDOWS
    juce_MemoryBarrier();
  #endif
}

#if JUCE_MSVC
  #pragma warning (pop)
#endif

#endif   // JUCE_ATOMIC_H_INCLUDED
