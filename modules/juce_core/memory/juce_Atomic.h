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

namespace juce
{

/** @cond */
namespace AtomicHelpers
{
    template <typename T> struct DiffTypeHelper     { using Type = T; };
    template <typename T> struct DiffTypeHelper<T*> { using Type = std::ptrdiff_t; };
}
/** @endcond */

//==============================================================================
/**
    A simple wrapper around std::atomic.

    @tags{Core}
*/
template <typename Type>
struct Atomic  final
{
    using DiffType = typename AtomicHelpers::DiffTypeHelper<Type>::Type;

    /** Creates a new value, initialised to zero. */
    Atomic() noexcept  : value (Type()) {}

    /** Creates a new value, with a given initial value. */
    Atomic (Type initialValue) noexcept  : value (initialValue) {}

    /** Copies another value (atomically). */
    Atomic (const Atomic& other) noexcept  : value (other.get()) {}

    /** Destructor. */
    ~Atomic() noexcept
    {
       #if __cpp_lib_atomic_is_always_lock_free
        static_assert (std::atomic<Type>::is_always_lock_free,
                       "This class can only be used for lock-free types");
       #endif
    }

    /** Atomically reads and returns the current value. */
    Type get() const noexcept               { return value.load(); }

    /** Atomically sets the current value. */
    void set (Type newValue) noexcept       { value = newValue; }

    /** Atomically sets the current value, returning the value that was replaced. */
    Type exchange (Type newValue) noexcept  { return value.exchange (newValue); }

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

        Internally, this method calls std::atomic::compare_exchange_strong with
        memory_order_seq_cst (the strictest std::memory_order).

        @returns true if the comparison was true and the value was replaced; false if
                 the comparison failed and the value was left unchanged.
        @see compareAndSetValue
    */
    bool compareAndSetBool (Type newValue, Type valueToCompare) noexcept
    {
        return value.compare_exchange_strong (valueToCompare, newValue);
    }

    /** Copies another value into this one (atomically). */
    Atomic<Type>& operator= (const Atomic& other) noexcept
    {
        value = other.value.load();
        return *this;
    }

    /** Copies another value into this one (atomically). */
    Atomic<Type>& operator= (Type newValue) noexcept
    {
        value = newValue;
        return *this;
    }

    /** Atomically adds a number to this value, returning the new value. */
    Type operator+= (DiffType amountToAdd) noexcept { return value += amountToAdd; }

    /** Atomically subtracts a number from this value, returning the new value. */
    Type operator-= (DiffType amountToSubtract) noexcept { return value -= amountToSubtract; }

    /** Atomically increments this value, returning the new value. */
    Type operator++() noexcept { return ++value; }

    /** Atomically decrements this value, returning the new value. */
    Type operator--() noexcept { return --value; }

    /** Implements a memory read/write barrier.

        Internally this calls std::atomic_thread_fence with
        memory_order_seq_cst (the strictest std::memory_order).
     */
    void memoryBarrier() noexcept          { atomic_thread_fence (std::memory_order_seq_cst); }

    /** The std::atomic object that this class operates on. */
    std::atomic<Type> value;

    //==============================================================================
    /** @cond */
    [[deprecated ("This method has been deprecated as there is no equivalent method in "
                 "std::atomic. Use compareAndSetBool instead.")]]
    Type compareAndSetValue (Type, Type) noexcept;
    /** @endcond */
};

} // namespace juce
