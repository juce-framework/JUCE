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

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

bool Thread::createNativeThread (Priority)
{
    PosixThreadAttribute attr { threadStackSize };
    PosixSchedulerPriority::getNativeSchedulerAndPriority (realtimeOptions, {}).apply (attr);

    threadId = threadHandle = makeThreadHandle (attr, this, [] (void* userData) -> void*
    {
        auto* myself = static_cast<Thread*> (userData);

        juce_threadEntryPoint (myself);

        return nullptr;
    });

    return threadId != nullptr;
}

void Thread::killThread()
{
    if (threadHandle != nullptr)
        pthread_cancel ((pthread_t) threadHandle.load());
}

// Until we implement Nice awareness, these don't do anything on Linux.
Thread::Priority Thread::getPriority() const
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    return priority;
}

bool Thread::setPriority (Priority newPriority)
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    // Return true to make it compatible with other platforms.
    priority = newPriority;
    return true;
}

//==============================================================================
JUCE_API void JUCE_CALLTYPE Process::setPriority (ProcessPriority) {}

static bool swapUserAndEffectiveUser()
{
    auto result1 = setreuid (geteuid(), getuid());
    auto result2 = setregid (getegid(), getgid());
    return result1 == 0 && result2 == 0;
}

JUCE_API void JUCE_CALLTYPE Process::raisePrivilege()  { if (geteuid() != 0 && getuid() == 0) swapUserAndEffectiveUser(); }
JUCE_API void JUCE_CALLTYPE Process::lowerPrivilege()  { if (geteuid() == 0 && getuid() != 0) swapUserAndEffectiveUser(); }

} // namespace juce
