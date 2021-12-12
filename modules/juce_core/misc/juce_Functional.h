/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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
    struct EqualityComparableToNullptr
        : std::false_type {};

    template <typename T>
    struct EqualityComparableToNullptr<T, Void<decltype (std::declval<T>() != nullptr)>>
        : std::true_type {};
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
    template <typename Callable, typename... Args,
              std::enable_if_t<detail::EqualityComparableToNullptr<Callable>::value, int> = 0>
    static void invoke (Callable&& fn, Args&&... args)
    {
        if (fn != nullptr)
            fn (std::forward<Args> (args)...);
    }

    template <typename Callable, typename... Args,
              std::enable_if_t<! detail::EqualityComparableToNullptr<Callable>::value, int> = 0>
    static void invoke (Callable&& fn, Args&&... args)
    {
        fn (std::forward<Args> (args)...);
    }

    template <typename... Args>
    static void invoke (std::nullptr_t, Args&&...) {}
};

} // namespace juce
