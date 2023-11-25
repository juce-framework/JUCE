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
