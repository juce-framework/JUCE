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

/** Some helper methods for checking a callable object before invoking with
    the specified arguments.

    If the object is a std::function it will check for nullptr before
    calling. For a callable object it will invoke the function call operator.

    @tags{Core}
*/
struct NullCheckedInvocation
{
    template <typename... Signature, typename... Args>
    static void invoke (std::function<Signature...>&& fn, Args&&... args)
    {
        if (fn != nullptr)
            fn (std::forward<Args> (args)...);
    }

    template <typename Callable, typename... Args>
    static void invoke (Callable&& fn, Args&&... args)
    {
        fn (std::forward<Args> (args)...);
    }

    template <typename... Args>
    static void invoke (std::nullptr_t, Args&&...) {}
};

} // namespace juce
