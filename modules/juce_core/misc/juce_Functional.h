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

#ifndef DOXYGEN
namespace detail
{
    template <typename...>
    using Void = void;

    template <typename, typename = void>
    constexpr auto equalityComparableToNullptr = false;

    template <typename T>
    constexpr auto equalityComparableToNullptr<T, Void<decltype (std::declval<T>() != nullptr)>> = true;
} // namespace detail
#endif

//==============================================================================
/** Some helper methods for checking a callable object before invoking with
    the specified arguments.

    If the object provides a comparison operator for nullptr it will check before
    calling. For other objects it will just invoke the function call operator.

    @tags{Core}
*/
struct NullCheckedInvocation
{
    template <typename Callable, typename... Args>
    static void invoke (Callable&& fn, Args&&... args)
    {
        if constexpr (detail::equalityComparableToNullptr<Callable>)
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Waddress")

            if (fn != nullptr)
                fn (std::forward<Args> (args)...);

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
        else
        {
            fn (std::forward<Args> (args)...);
        }
    }

    template <typename... Args>
    static void invoke (std::nullptr_t, Args&&...) {}
};

/** Can be used to disable template constructors that would otherwise cause ambiguity with
    compiler-generated copy and move constructors.

    Adapted from https://ericniebler.com/2013/08/07/universal-references-and-the-copy-constructo/
*/
template <typename A, typename B>
using DisableIfSameOrDerived = std::enable_if_t<! std::is_base_of_v<A, std::remove_reference_t<B>>>;

/** Copies an object, sets one of the copy's members to the specified value, and then returns the copy. */
template <typename Object, typename OtherObject, typename Member, typename Other>
Object withMember (Object copy, Member OtherObject::* member, Other&& value)
{
    copy.*member = std::forward<Other> (value);
    return copy;
}

/** An easy way to ensure that a function is called at the end of the current
    scope.

    Usage:
    @code
    {
        if (flag == true)
            return;

        // While this code executes, flag is true e.g. to prevent reentrancy
        flag = true;
        // When we exit this scope, flag must be false
        const ScopeGuard scope { [&] { flag = false; } };

        if (checkInitialCondition())
            return; // Scope's lambda will fire here...

        if (checkCriticalCondition())
            throw std::runtime_error{}; // ...or here...

        doWorkHavingEstablishedPreconditions();
    } // ...or here!
    @endcode
*/
template <typename Fn> struct ScopeGuard : Fn { ~ScopeGuard() { Fn::operator()(); } };
template <typename Fn> ScopeGuard (Fn) -> ScopeGuard<Fn>;

#ifndef DOXYGEN
namespace detail
{
template <typename Functor, typename Return, typename... Args>
static constexpr auto toFnPtr (Functor functor, Return (Functor::*) (Args...) const)
{
    return static_cast<Return (*) (Args...)> (functor);
}
} // namespace detail
#endif

/** Converts a captureless lambda to its equivalent function pointer type. */
template <typename Functor>
static constexpr auto toFnPtr (Functor functor) { return detail::toFnPtr (functor, &Functor::operator()); }

} // namespace juce
