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

struct ThreadPriorities
{
    struct Entry
    {
        Thread::Priority priority;
        int native;
    };

   #if JUCE_ANDROID
    enum AndroidThreadPriority
    {
        THREAD_PRIORITY_AUDIO          = -16,
        THREAD_PRIORITY_FOREGROUND     = -2,
        THREAD_PRIORITY_MORE_FAVORABLE = -1,
        THREAD_PRIORITY_DEFAULT        = 0,
        THREAD_PRIORITY_LESS_FAVORABLE = 1,
        THREAD_PRIORITY_BACKGROUND     = 10,
        THREAD_PRIORITY_LOWEST         = 19
    };
   #endif

    inline static constexpr Entry table[]
    {
       #if JUCE_ANDROID
        { Thread::Priority::highest,    AndroidThreadPriority::THREAD_PRIORITY_AUDIO },
        { Thread::Priority::high,       AndroidThreadPriority::THREAD_PRIORITY_FOREGROUND },
        { Thread::Priority::normal,     AndroidThreadPriority::THREAD_PRIORITY_DEFAULT },
        { Thread::Priority::low,        AndroidThreadPriority::THREAD_PRIORITY_BACKGROUND - 5 },
        { Thread::Priority::background, AndroidThreadPriority::THREAD_PRIORITY_BACKGROUND },
       #endif

       #if JUCE_LINUX || JUCE_BSD
        { Thread::Priority::highest,    0 },
        { Thread::Priority::high,       0 },
        { Thread::Priority::normal,     0 },
        { Thread::Priority::low,        0 },
        { Thread::Priority::background, 0 },
       #endif

       #if JUCE_MAC || JUCE_IOS
        { Thread::Priority::highest,    4 },
        { Thread::Priority::high,       3 },
        { Thread::Priority::normal,     2 },
        { Thread::Priority::low,        1 },
        { Thread::Priority::background, 0 },
       #endif

       #if JUCE_WINDOWS
        { Thread::Priority::highest,    THREAD_PRIORITY_TIME_CRITICAL },
        { Thread::Priority::high,       THREAD_PRIORITY_HIGHEST },
        { Thread::Priority::normal,     THREAD_PRIORITY_NORMAL },
        { Thread::Priority::low,        THREAD_PRIORITY_LOWEST },
        { Thread::Priority::background, THREAD_PRIORITY_IDLE },
       #endif
    };

    static_assert (std::size (table) == 5,
                   "The platform may be unsupported or there may be a priority entry missing.");

    static Thread::Priority getJucePriority (const int value)
    {
        const auto iter = std::min_element (std::begin (table),
                                            std::end   (table),
                                            [value] (const auto& a, const auto& b)
                                            {
                                                return std::abs (a.native - value) < std::abs (b.native - value);
                                            });

        jassert (iter != std::end (table));
        return iter != std::end (table) ? iter->priority : Thread::Priority{};
    }

    static int getNativePriority (const Thread::Priority value)
    {
        const auto iter = std::find_if (std::begin (table),
                                        std::end   (table),
                                        [value] (const auto& entry) { return entry.priority == value; });

        jassert (iter != std::end (table));
        return iter != std::end (table) ? iter->native : 0;
    }
};

} // namespace juce
