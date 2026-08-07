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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

WaitableEvent::WaitableEvent (bool manualReset) noexcept
    : useManualReset (manualReset)
{
}

void WaitableEvent::wait() const
{
    std::unique_lock<std::mutex> lock (mutex);

    if (! triggered)
        condition.wait (lock, [this] { return triggered == true; });

    if (! useManualReset)
        reset();
}

bool WaitableEvent::wait (Seconds timeOut) const
{
    // Unlike wait (double), a negative timeout is not supported. To wait
    // indefinitely, call wait() with no arguments.
    jassert (timeOut >= Seconds { 0.0 });

    std::unique_lock<std::mutex> lock (mutex);

    if (! triggered && ! condition.wait_for (lock, timeOut, [this] { return triggered == true; }))
        return false;

    if (! useManualReset)
        reset();

    return true;
}

bool WaitableEvent::wait (double timeOutMilliseconds) const
{
    if (timeOutMilliseconds >= 0.0)
        return wait (Milliseconds { timeOutMilliseconds });

    wait();
    return true;
}

void WaitableEvent::signal() const
{
    std::lock_guard<std::mutex> lock (mutex);

    triggered = true;
    condition.notify_all();
}

void WaitableEvent::reset() const
{
    triggered = false;
}

} // namespace juce
