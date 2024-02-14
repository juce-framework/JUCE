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
