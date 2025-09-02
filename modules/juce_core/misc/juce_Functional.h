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
namespace detail
{
    template <typename...>
    using Void = void;

    template <typename, typename = void>
    constexpr auto equalityComparableToNullptr = false;

    template <typename T>
    constexpr auto equalityComparableToNullptr<T, Void<decltype (std::declval<T>() != nullptr)>> = true;
} // namespace detail
/** @endcond */

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
[[nodiscard]] Object withMember (Object copy, Member OtherObject::* member, Other&& value)
{
    copy.*member = std::forward<Other> (value);
    return copy;
}

/** @cond */
namespace detail
{
template <typename Functor, typename Return, typename... Args>
static constexpr auto toFnPtr (Functor functor, Return (Functor::*) (Args...) const)
{
    return static_cast<Return (*) (Args...)> (functor);
}
} // namespace detail
/** @endcond */

/** Converts a captureless lambda to its equivalent function pointer type. */
template <typename Functor>
static constexpr auto toFnPtr (Functor functor) { return detail::toFnPtr (functor, &Functor::operator()); }

} // namespace juce
