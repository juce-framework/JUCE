/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace detail
{
namespace adlSwap
{
using std::swap;

template <typename T>
constexpr auto isNothrowSwappable = noexcept (swap (std::declval<T&>(), std::declval<T&>()));
} // namespace adlSwap
} // namespace detail

/** A type representing the null state of an Optional.
    Similar to std::nullopt_t.
*/
struct Nullopt
{
    explicit constexpr Nullopt (int) {}
};

/** An object that can be used when constructing and comparing Optional instances.
    Similar to std::nullopt.
*/
constexpr Nullopt nullopt { 0 };

// Without this, our tests can emit "unreachable code" warnings during
// link time code generation.
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4702)

/**
    A simple optional type.

    Has similar (not necessarily identical!) semantics to std::optional.

    This is intended to stand-in for std::optional while JUCE's minimum
    supported language standard is lower than C++17. When the minimum language
    standard moves to C++17, this class will probably be deprecated, in much
    the same way that juce::ScopedPointer was deprecated in favour of
    std::unique_ptr after C++11.

    This isn't really intended to be used by JUCE clients. Instead, it's to be
    used internally in JUCE code, with an API close-enough to std::optional
    that the types can be swapped with fairly minor disruption at some point in
    the future, but *without breaking any public APIs*.

    @tags{Core}
*/
template <typename Value>
class Optional
{
    template <typename T, typename U>
    struct NotConstructibleFromSimilarType
    {
        static constexpr auto value = ! std::is_constructible<T, Optional<U>&>::value
                                   && ! std::is_constructible<T, const Optional<U>&>::value
                                   && ! std::is_constructible<T, Optional<U>&&>::value
                                   && ! std::is_constructible<T, const Optional<U>&&>::value
                                   && ! std::is_convertible<Optional<U>&, T>::value
                                   && ! std::is_convertible<const Optional<U>&, T>::value
                                   && ! std::is_convertible<Optional<U>&&, T>::value
                                   && ! std::is_convertible<const Optional<U>&&, T>::value;
    };

    template <typename T, typename U>
    using OptionalCopyConstructorEnabled = std::enable_if_t<std::is_constructible<T, const U&>::value && NotConstructibleFromSimilarType<T, U>::value>;

    template <typename T, typename U>
    using OptionalMoveConstructorEnabled = std::enable_if_t<std::is_constructible<T, U&&>::value && NotConstructibleFromSimilarType<T, U>::value>;

    template <typename T, typename U>
    static auto notAssignableFromSimilarType = NotConstructibleFromSimilarType<T, U>::value
                                               && ! std::is_assignable<T&, Optional<U>&>::value
                                               && ! std::is_assignable<T&, const Optional<U>&>::value
                                               && ! std::is_assignable<T&, Optional<U>&&>::value
                                               && ! std::is_assignable<T&, const Optional<U>&&>::value;

    template <typename T, typename U>
    using OptionalCopyAssignmentEnabled = std::enable_if_t<std::is_constructible<T, const U&>::value
                                                           && std::is_assignable<T&, const U&>::value
                                                           && NotConstructibleFromSimilarType<T, U>::value>;

    template <typename T, typename U>
    using OptionalMoveAssignmentEnabled = std::enable_if_t<std::is_constructible<T, U>::value
                                                           && std::is_nothrow_assignable<T&, U>::value
                                                           && NotConstructibleFromSimilarType<T, U>::value>;

public:
    Optional() : placeholder() {}

    Optional (Nullopt) noexcept : placeholder() {}

    template <typename U = Value,
              typename = std::enable_if_t<std::is_constructible<Value, U&&>::value
                                          && ! std::is_same<std::decay_t<U>, Optional>::value>>
    Optional (U&& value) noexcept (noexcept (Value (std::forward<U> (value))))
        : storage (std::forward<U> (value)), valid (true)
    {
    }

    Optional (Optional&& other) noexcept (noexcept (std::declval<Optional>().constructFrom (other)))
        : placeholder()
    {
        constructFrom (other);
    }

    Optional (const Optional& other)
        : placeholder(), valid (other.valid)
    {
        if (valid)
            new (&storage) Value (*other);
    }

    template <typename Other, typename = OptionalMoveConstructorEnabled<Value, Other>>
    Optional (Optional<Other>&& other) noexcept (noexcept (std::declval<Optional>().constructFrom (other)))
        : placeholder()
    {
        constructFrom (other);
    }

    template <typename Other, typename = OptionalCopyConstructorEnabled<Value, Other>>
    Optional (const Optional<Other>& other)
        : placeholder(), valid (other.hasValue())
    {
        if (valid)
            new (&storage) Value (*other);
    }

    Optional& operator= (Nullopt) noexcept
    {
        reset();
        return *this;
    }

    template <typename U = Value,
              typename = std::enable_if_t<std::is_nothrow_move_constructible<U>::value
                                          && std::is_nothrow_move_assignable<U>::value>>
    Optional& operator= (Optional&& other) noexcept (noexcept (std::declval<Optional>().assign (std::declval<Optional&>())))
    {
        assign (other);
        return *this;
    }

    template <typename U = Value,
              typename = std::enable_if_t<! std::is_same<std::decay_t<U>, Optional>::value
                                          && std::is_constructible<Value, U>::value
                                          && std::is_assignable<Value&, U>::value
                                          && (! std::is_scalar<Value>::value || ! std::is_same<std::decay_t<U>, Value>::value)>>
    Optional& operator= (U&& value)
    {
        if (valid)
            **this = std::forward<U> (value);
        else
            new (&storage) Value (std::forward<U> (value));

        valid = true;
        return *this;
    }

    /** Maintains the strong exception safety guarantee. */
    Optional& operator= (const Optional& other)
    {
        auto copy = other;
        assign (copy);
        return *this;
    }

    template <typename Other, typename = OptionalMoveAssignmentEnabled<Value, Other>>
    Optional& operator= (Optional<Other>&& other) noexcept (noexcept (std::declval<Optional>().assign (other)))
    {
        assign (other);
        return *this;
    }

    /** Maintains the strong exception safety guarantee. */
    template <typename Other, typename = OptionalCopyAssignmentEnabled<Value, Other>>
    Optional& operator= (const Optional<Other>& other)
    {
        auto copy = other;
        assign (copy);
        return *this;
    }

    ~Optional() noexcept
    {
        reset();
    }

          Value* operator->()       noexcept { return reinterpret_cast<      Value*> (&storage); }
    const Value* operator->() const noexcept { return reinterpret_cast<const Value*> (&storage); }

          Value& operator*()       noexcept { return *operator->(); }
    const Value& operator*() const noexcept { return *operator->(); }

    explicit operator bool() const noexcept { return valid; }
    bool hasValue() const noexcept { return valid; }

    void reset()
    {
        if (std::exchange (valid, false))
            operator*().~Value();
    }

    /** Like std::optional::value_or */
    template <typename U>
    Value orFallback (U&& fallback) const { return *this ? **this : std::forward<U> (fallback); }

    template <typename... Args>
    Value& emplace (Args&&... args)
    {
        reset();
        new (&storage) Value (std::forward<Args> (args)...);
        valid = true;
        return **this;
    }

    void swap (Optional& other) noexcept (std::is_nothrow_move_constructible<Value>::value
                                          && detail::adlSwap::isNothrowSwappable<Value>)
    {
        if (hasValue() && other.hasValue())
        {
            using std::swap;
            swap (**this, *other);
        }
        else if (hasValue() || other.hasValue())
        {
            (hasValue() ? other : *this).constructFrom (hasValue() ? *this : other);
        }
    }

private:
    template <typename Other>
    void constructFrom (Optional<Other>& other) noexcept (noexcept (Value (std::move (*other))))
    {
        if (! other.hasValue())
            return;

        new (&storage) Value (std::move (*other));
        valid = true;
        other.reset();
    }

    template <typename Other>
    void assign (Optional<Other>& other) noexcept (noexcept (std::declval<Value&>() = std::move (*other)) && noexcept (std::declval<Optional>().constructFrom (other)))
    {
        if (valid)
        {
            if (other.hasValue())
            {
                **this = std::move (*other);
                other.reset();
            }
            else
            {
                reset();
            }
        }
        else
        {
            constructFrom (other);
        }
    }

    union
    {
        char placeholder;
        Value storage;
    };
    bool valid = false;
};

JUCE_END_IGNORE_WARNINGS_MSVC

template <typename Value>
Optional<std::decay_t<Value>> makeOptional (Value&& v)
{
    return std::forward<Value> (v);
}

template <class T, class U>
bool operator== (const Optional<T>& lhs, const Optional<U>& rhs)
{
    if (lhs.hasValue() != rhs.hasValue()) return false;
    if (! lhs.hasValue()) return true;
    return *lhs == *rhs;
}

template <class T, class U>
bool operator!= (const Optional<T>& lhs, const Optional<U>& rhs)
{
    if (lhs.hasValue() != rhs.hasValue()) return true;
    if (! lhs.hasValue()) return false;
    return *lhs != *rhs;
}

template <class T, class U>
bool operator< (const Optional<T>& lhs, const Optional<U>& rhs)
{
    if (! rhs.hasValue()) return false;
    if (! lhs.hasValue()) return true;
    return *lhs < *rhs;
}

template <class T, class U>
bool operator<= (const Optional<T>& lhs, const Optional<U>& rhs)
{
    if (! lhs.hasValue()) return true;
    if (! rhs.hasValue()) return false;
    return *lhs <= *rhs;
}

template <class T, class U>
bool operator> (const Optional<T>& lhs, const Optional<U>& rhs)
{
    if (! lhs.hasValue()) return false;
    if (! rhs.hasValue()) return true;
    return *lhs > *rhs;
}

template <class T, class U>
bool operator>= (const Optional<T>& lhs, const Optional<U>& rhs)
{
    if (! rhs.hasValue()) return true;
    if (! lhs.hasValue()) return false;
    return *lhs >= *rhs;
}

template <class T>
bool operator== (const Optional<T>& opt, Nullopt) noexcept { return ! opt.hasValue(); }
template <class T>
bool operator== (Nullopt, const Optional<T>& opt) noexcept { return ! opt.hasValue(); }
template <class T>
bool operator!= (const Optional<T>& opt, Nullopt) noexcept { return opt.hasValue(); }
template <class T>
bool operator!= (Nullopt, const Optional<T>& opt) noexcept { return opt.hasValue(); }
template <class T>
bool operator< (const Optional<T>&, Nullopt) noexcept { return false; }
template <class T>
bool operator< (Nullopt, const Optional<T>& opt) noexcept { return opt.hasValue(); }
template <class T>
bool operator<= (const Optional<T>& opt, Nullopt) noexcept { return ! opt.hasValue(); }
template <class T>
bool operator<= (Nullopt, const Optional<T>&) noexcept { return true; }
template <class T>
bool operator> (const Optional<T>& opt, Nullopt) noexcept { return opt.hasValue(); }
template <class T>
bool operator> (Nullopt, const Optional<T>&) noexcept { return false; }
template <class T>
bool operator>= (const Optional<T>&, Nullopt) noexcept { return true; }
template <class T>
bool operator>= (Nullopt, const Optional<T>& opt) noexcept { return ! opt.hasValue(); }

template <class T, class U>
bool operator== (const Optional<T>& opt, const U& value) { return opt.hasValue() ? *opt == value : false; }
template <class T, class U>
bool operator== (const T& value, const Optional<U>& opt) { return opt.hasValue() ? value == *opt : false; }
template <class T, class U>
bool operator!= (const Optional<T>& opt, const U& value) { return opt.hasValue() ? *opt != value : true; }
template <class T, class U>
bool operator!= (const T& value, const Optional<U>& opt) { return opt.hasValue() ? value != *opt : true; }
template <class T, class U>
bool operator< (const Optional<T>& opt, const U& value) { return opt.hasValue() ? *opt < value : true; }
template <class T, class U>
bool operator< (const T& value, const Optional<U>& opt) { return opt.hasValue() ? value < *opt : false; }
template <class T, class U>
bool operator<= (const Optional<T>& opt, const U& value) { return opt.hasValue() ? *opt <= value : true; }
template <class T, class U>
bool operator<= (const T& value, const Optional<U>& opt) { return opt.hasValue() ? value <= *opt : false; }
template <class T, class U>
bool operator> (const Optional<T>& opt, const U& value) { return opt.hasValue() ? *opt > value : false; }
template <class T, class U>
bool operator> (const T& value, const Optional<U>& opt) { return opt.hasValue() ? value > *opt : true; }
template <class T, class U>
bool operator>= (const Optional<T>& opt, const U& value) { return opt.hasValue() ? *opt >= value : false; }
template <class T, class U>
bool operator>= (const T& value, const Optional<U>& opt) { return opt.hasValue() ? value >= *opt : true; }

} // namespace juce
