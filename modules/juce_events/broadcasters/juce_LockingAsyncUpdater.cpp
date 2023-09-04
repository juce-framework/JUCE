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

class LockingAsyncUpdater::Impl : public CallbackMessage
{
public:
    explicit Impl (std::function<void()> cb)
        : callback (std::move (cb)) {}

    void clear()
    {
        const ScopedLock lock (mutex);
        deliver = false;
        callback = nullptr;
    }

    void trigger()
    {
        {
            const ScopedLock lock (mutex);

            if (deliver)
                return;

            deliver = true;
        }

        if (! post())
            cancel();
    }

    void cancel()
    {
        const ScopedLock lock (mutex);
        deliver = false;
    }

    bool isPending()
    {
        const ScopedLock lock (mutex);
        return deliver;
    }

    void messageCallback() override
    {
        const ScopedLock lock (mutex);

        if (std::exchange (deliver, false))
            NullCheckedInvocation::invoke (callback);
    }

private:
    CriticalSection mutex;
    std::function<void()> callback;
    bool deliver = false;
};

//==============================================================================
LockingAsyncUpdater::LockingAsyncUpdater (std::function<void()> callbackToUse)
    : impl (new Impl (std::move (callbackToUse))) {}

LockingAsyncUpdater::LockingAsyncUpdater (LockingAsyncUpdater&& other) noexcept
    : impl (std::exchange (other.impl, nullptr)) {}

LockingAsyncUpdater& LockingAsyncUpdater::operator= (LockingAsyncUpdater&& other) noexcept
{
    LockingAsyncUpdater temp { std::move (other) };
    std::swap (temp.impl, impl);
    return *this;
}

LockingAsyncUpdater::~LockingAsyncUpdater()
{
    if (impl != nullptr)
        impl->clear();
}

void LockingAsyncUpdater::triggerAsyncUpdate()
{
    if (impl != nullptr)
        impl->trigger();
    else
        jassertfalse; // moved-from!
}

void LockingAsyncUpdater::cancelPendingUpdate() noexcept
{
    if (impl != nullptr)
        impl->cancel();
    else
        jassertfalse; // moved-from!
}

void LockingAsyncUpdater::handleUpdateNowIfNeeded()
{
    if (impl != nullptr)
        impl->messageCallback();
    else
        jassertfalse; // moved-from!
}

bool LockingAsyncUpdater::isUpdatePending() const noexcept
{
    if (impl != nullptr)
        return impl->isPending();

    jassertfalse; // moved-from!
    return false;
}

} // namespace juce
