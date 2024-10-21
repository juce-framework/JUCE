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
