/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#ifndef DOXYGEN

namespace detail
{
    template <typename Ret, typename... Args>
    struct Vtable
    {
        using Storage = void*;

        using Move  = void (*) (Storage, Storage);
        using Call  = Ret  (*) (Storage, Args...);
        using Clear = void (*) (Storage);

        constexpr Vtable (Move moveIn, Call callIn, Clear clearIn) noexcept
                : move (moveIn), call (callIn), clear (clearIn) {}

        Move  move  = nullptr;
        Call  call  = nullptr;
        Clear clear = nullptr;
    };

    template <typename Fn>
    void move (void* from, void* to)
    {
        new (to) Fn (std::move (*reinterpret_cast<Fn*> (from)));
    }

    template <typename Fn, typename Ret, typename... Args>
    std::enable_if_t<std::is_same_v<Ret, void>, Ret> call (void* s, Args... args)
    {
        (*reinterpret_cast<Fn*> (s)) (std::forward<Args> (args)...);
    }

    template <typename Fn, typename Ret, typename... Args>
    std::enable_if_t<! std::is_same_v<Ret, void>, Ret> call (void* s, Args... args)
    {
        return (*reinterpret_cast<Fn*> (s)) (std::forward<Args> (args)...);
    }

    template <typename Fn>
    void clear (void* s)
    {
        // I know this looks insane, for some reason MSVC 14 sometimes thinks fn is unreferenced
        [[maybe_unused]] auto& fn = *reinterpret_cast<Fn*> (s);
        fn.~Fn();
    }

    template <typename Fn, typename Ret, typename... Args>
    constexpr Vtable<Ret, Args...> makeVtable()
    {
        return { move <Fn>, call <Fn, Ret, Args...>, clear<Fn> };
    }
} // namespace detail

template <size_t len, typename T>
class FixedSizeFunction;

#endif

/**
    A type similar to `std::function` that holds a callable object.

    Unlike `std::function`, the callable object will always be stored in
    a buffer of size `len` that is internal to the FixedSizeFunction instance.
    This in turn means that creating a FixedSizeFunction instance will never allocate,
    making FixedSizeFunctions suitable for use in realtime contexts.

    @tags{DSP}
*/
template <size_t len, typename Ret, typename... Args>
class FixedSizeFunction<len, Ret (Args...)>
{
private:
    using Storage = std::aligned_storage_t<len>;

    template <typename Item>
    using Decay = std::decay_t<Item>;

    template <typename Item, typename Fn = Decay<Item>>
    using IntIfValidConversion = std::enable_if_t<sizeof (Fn) <= len
                                                      && alignof (Fn) <= alignof (Storage)
                                                      && ! std::is_same_v<FixedSizeFunction, Fn>,
                                                  int>;

public:
    /** Create an empty function. */
    FixedSizeFunction() noexcept = default;

    /** Create an empty function. */
    FixedSizeFunction (std::nullptr_t) noexcept
        : FixedSizeFunction() {}

    FixedSizeFunction (const FixedSizeFunction&) = delete;

    /** Forwards the passed Callable into the internal storage buffer. */
    template <typename Callable,
              typename Fn = Decay<Callable>,
              IntIfValidConversion<Callable> = 0>
    FixedSizeFunction (Callable&& callable)
    {
        static_assert (sizeof (Fn) <= len,
                       "The requested function cannot fit in this FixedSizeFunction");
        static_assert (alignof (Fn) <= alignof (Storage),
                       "FixedSizeFunction cannot accommodate the requested alignment requirements");

        static constexpr auto vtableForCallable = detail::makeVtable<Fn, Ret, Args...>();
        vtable = &vtableForCallable;

        [[maybe_unused]] auto* ptr = new (&storage) Fn (std::forward<Callable> (callable));
        jassert ((void*) ptr == (void*) &storage);
    }

    /** Move constructor. */
    FixedSizeFunction (FixedSizeFunction&& other) noexcept
        : vtable (other.vtable)
    {
        move (std::move (other));
    }

    /** Converting constructor from smaller FixedSizeFunctions. */
    template <size_t otherLen, std::enable_if_t<(otherLen < len), int> = 0>
    FixedSizeFunction (FixedSizeFunction<otherLen, Ret (Args...)>&& other) noexcept
        : vtable (other.vtable)
    {
        move (std::move (other));
    }

    /** Nulls this instance. */
    FixedSizeFunction& operator= (std::nullptr_t) noexcept
    {
        return *this = FixedSizeFunction();
    }

    FixedSizeFunction& operator= (const FixedSizeFunction&) = delete;

    /** Assigns a new callable to this instance. */
    template <typename Callable, IntIfValidConversion<Callable> = 0>
    FixedSizeFunction& operator= (Callable&& callable)
    {
        return *this = FixedSizeFunction (std::forward<Callable> (callable));
    }

    /** Move assignment from smaller FixedSizeFunctions. */
    template <size_t otherLen, std::enable_if_t<(otherLen < len), int> = 0>
    FixedSizeFunction& operator= (FixedSizeFunction<otherLen, Ret (Args...)>&& other) noexcept
    {
        return *this = FixedSizeFunction (std::move (other));
    }

    /** Move assignment operator. */
    FixedSizeFunction& operator= (FixedSizeFunction&& other) noexcept
    {
        clear();
        vtable = other.vtable;
        move (std::move (other));
        return *this;
    }

    /** Destructor. */
    ~FixedSizeFunction() noexcept { clear(); }

    /** If this instance is currently storing a callable object, calls that object,
        otherwise throws `std::bad_function_call`.
    */
    Ret operator() (Args... args) const
    {
        if (vtable != nullptr)
            return vtable->call (&storage, std::forward<Args> (args)...);

        throw std::bad_function_call();
    }

    /** Returns true if this instance currently holds a callable. */
    explicit operator bool() const noexcept { return vtable != nullptr; }

private:
    template <size_t, typename>
    friend class FixedSizeFunction;

    void clear() noexcept
    {
        if (vtable != nullptr)
            vtable->clear (&storage);
    }

    template <size_t otherLen, typename T>
    void move (FixedSizeFunction<otherLen, T>&& other) noexcept
    {
        if (vtable != nullptr)
            vtable->move (&other.storage, &storage);
    }

    const detail::Vtable<Ret, Args...>* vtable = nullptr;
    mutable Storage storage;
};

template <size_t len, typename T>
bool operator!= (const FixedSizeFunction<len, T>& fn, std::nullptr_t) { return bool (fn); }

template <size_t len, typename T>
bool operator!= (std::nullptr_t, const FixedSizeFunction<len, T>& fn) { return bool (fn); }

template <size_t len, typename T>
bool operator== (const FixedSizeFunction<len, T>& fn, std::nullptr_t) { return ! (fn != nullptr); }

template <size_t len, typename T>
bool operator== (std::nullptr_t, const FixedSizeFunction<len, T>& fn) { return ! (fn != nullptr); }

} // namespace juce
