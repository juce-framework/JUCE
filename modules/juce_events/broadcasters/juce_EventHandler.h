/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

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

//==============================================================================
/**
    Helper class for dispatching callbacks to a lambda function.

    This class probably isn't something many users will use in their own code, but
    some juce classes use it as a helper to allow lambdas to be assigned to callback
    hooks - e.g. see its use in Button::onClick
*/
template <typename OwnerClass>
struct EventHandler
{
    EventHandler() {}
    ~EventHandler() {}

    /** Assigns a void std::function to this callback.
        Note that this will replace any existing function that was previously assigned.
    */
    void operator= (std::function<void()> callbackToAttach)
    {
        callback = [=](OwnerClass&) { callbackToAttach(); };
    }

    /** Assigns a std::function to this callback which takes a reference to the
        source object that's making the callback.
        In the example of this class being used inside, e.g. a Button, then the
        callback parameter would be the Button that is doing the calling.
        Note that this will replace any existing function that was previously assigned.
    */
    void operator= (std::function<void(OwnerClass&)> callbackToAttach)
    {
        callback = static_cast<std::function<void(OwnerClass&)>&&> (callbackToAttach);
    }

    /** Removes any existing function that was previously assigned to the callback. */
    void reset() noexcept
    {
        callback = {};
    }

    /** @internal */
    void invoke (OwnerClass& owner)
    {
        if (callback != nullptr)
            callback (owner);
    }

private:
    std::function<void(OwnerClass&)> callback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventHandler)
};

} // namespace juce
