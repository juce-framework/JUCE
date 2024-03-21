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

namespace juce::detail
{

template <typename... Ts>
constexpr bool isValueOrLvalueReferenceToConst()
{
    return ((   (! std::is_reference_v<Ts>)
             || (std::is_lvalue_reference_v<Ts> && std::is_const_v<std::remove_reference_t<Ts>>)) && ...);
}

template <typename... Args>
class CallbackListenerList
{
public:
    static_assert (isValueOrLvalueReferenceToConst<Args...>(),
                   "CallbackListenerList can only forward values or const lvalue references");

    using Callback = std::function<void (Args...)>;

    ErasedScopeGuard addListener (Callback callback)
    {
        jassert (callback != nullptr);

        const auto it = callbacks.insert (callbacks.end(), std::move (callback));
        listeners.add (&*it);

        return ErasedScopeGuard { [this, it]
        {
            listeners.remove (&*it);
            callbacks.erase (it);
        } };
    }

    void call (Args... args)
    {
        listeners.call ([&] (auto& l) { l (std::forward<Args> (args)...); });
    }

private:
    std::list<Callback> callbacks;
    ListenerList<Callback> listeners;
};

} // namespace juce::detail
