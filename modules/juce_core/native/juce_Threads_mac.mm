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

#if JUCE_IOS
 bool isIOSAppActive = true;
#endif

API_AVAILABLE (macos (10.10))
static auto getNativeQOS (Thread::Priority priority)
{
    switch (priority)
    {
        case Thread::Priority::highest:    return QOS_CLASS_USER_INTERACTIVE;
        case Thread::Priority::high:       return QOS_CLASS_USER_INITIATED;
        case Thread::Priority::low:        return QOS_CLASS_UTILITY;
        case Thread::Priority::background: return QOS_CLASS_BACKGROUND;
        case Thread::Priority::normal:     break;
    }

    return QOS_CLASS_DEFAULT;
}

API_AVAILABLE (macos (10.10))
static auto getJucePriority (qos_class_t qos)
{
    switch (qos)
    {
        case QOS_CLASS_USER_INTERACTIVE:    return Thread::Priority::highest;
        case QOS_CLASS_USER_INITIATED:      return Thread::Priority::high;
        case QOS_CLASS_UTILITY:             return Thread::Priority::low;
        case QOS_CLASS_BACKGROUND:          return Thread::Priority::background;

        case QOS_CLASS_UNSPECIFIED:
        case QOS_CLASS_DEFAULT:             break;
    }

    return Thread::Priority::normal;
}

template<typename Type>
static std::optional<Type> firstOptionalWithValue (const std::initializer_list<std::optional<Type>>& optionals)
{
    for (const auto& optional : optionals)
        if (optional.has_value())
            return optional;

    return {};
}

static bool tryToUpgradeCurrentThreadToRealtime (const Thread::RealtimeOptions& options)
{
    const auto periodMs = options.getPeriodMs().value_or (0.0);

    const auto processingTimeMs = firstOptionalWithValue (
    {
        options.getProcessingTimeMs(),
        options.getMaximumProcessingTimeMs(),
        options.getPeriodMs()
    }).value_or (10.0);

    const auto maxProcessingTimeMs = options.getMaximumProcessingTimeMs()
                                            .value_or (processingTimeMs);

    // The processing time can not exceed the maximum processing time!
    jassert (maxProcessingTimeMs >= processingTimeMs);

    thread_time_constraint_policy_data_t policy;
    policy.period = (uint32_t) Time::secondsToHighResolutionTicks (periodMs / 1'000.0);
    policy.computation = (uint32_t) Time::secondsToHighResolutionTicks (processingTimeMs / 1'000.0);
    policy.constraint = (uint32_t) Time::secondsToHighResolutionTicks (maxProcessingTimeMs / 1'000.0);
    policy.preemptible = true;

    const auto result = thread_policy_set (pthread_mach_thread_np (pthread_self()),
                                           THREAD_TIME_CONSTRAINT_POLICY,
                                           (thread_policy_t) &policy,
                                           THREAD_TIME_CONSTRAINT_POLICY_COUNT);

    if (result == KERN_SUCCESS)
        return true;

    // testing has shown that passing a computation value > 50ms can
    // lead to thread_policy_set returning an error indicating that an
    // invalid argument was passed. If that happens this code tries to
    // limit that value in the hope of resolving the issue.

    if (result == KERN_INVALID_ARGUMENT && options.getProcessingTimeMs() > 50.0)
        return tryToUpgradeCurrentThreadToRealtime (options.withProcessingTimeMs (50.0));

    return false;
}

bool Thread::createNativeThread (Priority priority)
{
    PosixThreadAttribute attribute { threadStackSize };

    if (@available (macos 10.10, *))
        pthread_attr_set_qos_class_np (attribute.get(), getNativeQOS (priority), 0);
    else
        PosixSchedulerPriority::getNativeSchedulerAndPriority (realtimeOptions, priority).apply (attribute);

    struct ThreadData
    {
        Thread& thread;
        std::promise<bool> started{};
    };

    ThreadData threadData { *this, {} };

    threadId = threadHandle = makeThreadHandle (attribute, &threadData, [] (void* userData) -> void*
    {
        auto& data { *static_cast<ThreadData*> (userData) };
        auto& thread = data.thread;

        if (thread.isRealtime()
            && ! tryToUpgradeCurrentThreadToRealtime (*thread.realtimeOptions))
        {
            data.started.set_value (false);
            return nullptr;
        }

        data.started.set_value (true);

        JUCE_AUTORELEASEPOOL
        {
            juce_threadEntryPoint (&thread);
        }

        return nullptr;
    });

    return threadId != nullptr
        && threadData.started.get_future().get();
}

void Thread::killThread()
{
    if (threadHandle != nullptr)
        pthread_cancel ((pthread_t) threadHandle.load());
}

Thread::Priority Thread::getPriority() const
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    if (! isRealtime())
    {
        if (@available (macOS 10.10, *))
            return getJucePriority (qos_class_self());

        // fallback for older versions of macOS
        const auto min = jmax (0, sched_get_priority_min (SCHED_OTHER));
        const auto max = jmax (0, sched_get_priority_max (SCHED_OTHER));

        if (min != 0 && max != 0)
        {
            const auto native = PosixSchedulerPriority::findCurrentSchedulerAndPriority().getPriority();
            const auto mapped = jmap (native, min, max, 0, 4);
            return ThreadPriorities::getJucePriority (mapped);
        }
    }

    return {};
}

bool Thread::setPriority (Priority priority)
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    if (@available (macOS 10.10, *))
        return pthread_set_qos_class_self_np (getNativeQOS (priority), 0) == 0;

   #if JUCE_ARM
    // M1 platforms should never reach this code!!!!!!
    jassertfalse;
   #endif

    // Just in case older versions of macOS support SCHED_OTHER priorities.
    const auto psp = PosixSchedulerPriority::getNativeSchedulerAndPriority ({}, priority);

    struct sched_param param;
    param.sched_priority = psp.getPriority();
    return pthread_setschedparam (pthread_self(), psp.getScheduler(), &param) == 0;
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()
{
   if (SystemStats::isRunningInAppExtensionSandbox())
       return true;

   #if JUCE_MAC
    return [NSApp isActive];
   #else
    return isIOSAppActive;
   #endif
}

JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess()
{
   #if JUCE_MAC
    if (! SystemStats::isRunningInAppExtensionSandbox())
        [NSApp activateIgnoringOtherApps: YES];
   #endif
}

JUCE_API void JUCE_CALLTYPE Process::hide()
{
    if (! SystemStats::isRunningInAppExtensionSandbox())
    {
       #if JUCE_MAC
        [NSApp hide: nil];
       #elif JUCE_IOS
        [[UIApplication sharedApplication] performSelector: @selector (suspend)];
       #endif
    }
}

JUCE_API void JUCE_CALLTYPE Process::raisePrivilege() {}
JUCE_API void JUCE_CALLTYPE Process::lowerPrivilege() {}

JUCE_API void JUCE_CALLTYPE Process::setPriority (ProcessPriority) {}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger() noexcept
{
    struct kinfo_proc info;
    int m[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    size_t sz = sizeof (info);
    sysctl (m, 4, &info, &sz, nullptr, 0);
    return (info.kp_proc.p_flag & P_TRACED) != 0;
}

} // namespace juce
