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

class PlatformTimer final : private HighResolutionTimerThread::Impl
{
public:
    explicit PlatformTimer (PlatformTimerListener& ptl)
        : listener { ptl } {}

    void startTimer (int newIntervalMs)
    {
        jassert (newIntervalMs > 0);
        jassert (timer == nullptr);

        if (runLoop == nullptr)
            return;

        const auto intervalSeconds = static_cast<double> (newIntervalMs) / 1'000.0;

        CFRunLoopTimerContext context{};
        context.info = &listener;

        const auto callback = [] (CFRunLoopTimerRef, void* ctx)
        {
            static_cast<PlatformTimerListener*> (ctx)->onTimerExpired (1);
        };

        timer.reset (CFRunLoopTimerCreate (kCFAllocatorDefault,
                                           intervalSeconds + CFAbsoluteTimeGetCurrent(),
                                           intervalSeconds,
                                           0,
                                           0,
                                           callback,
                                           &context));

        CFRunLoopAddTimer (runLoop, timer.get(), kCFRunLoopDefaultMode);
    }

    void cancelTimer()
    {
        jassert (runLoop != nullptr);
        jassert (timer != nullptr);

        CFRunLoopRemoveTimer (runLoop, timer.get(), kCFRunLoopDefaultMode);
        timer.reset();
    }

    int getIntervalMs() const
    {
        return timer != nullptr ? (int) (CFRunLoopTimerGetInterval (timer.get()) * 1'000.0) : 0;
    }

private:
    static void preventRunLoopFromEarlyExits()
    {
        CFRunLoopSourceContext context{};
        CFRunLoopAddSource (CFRunLoopGetCurrent(),
                            CFRunLoopSourceCreate (kCFAllocatorDefault, 0, &context),
                            kCFRunLoopDefaultMode);
    }

    void setRunLoop()
    {
        CFRunLoopPerformBlock (CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, ^()
        {
            runLoopPromise.set_value (CFRunLoopGetCurrent());
        });
    }

    void runThread() override
    {
        preventRunLoopFromEarlyExits();
        setRunLoop();
        CFRunLoopRun();
    }

    void signalThreadShouldExit() override
    {
        if (runLoop != nullptr)
            CFRunLoopStop (runLoop);
    }

    PlatformTimerListener& listener;
    CFUniquePtr<CFRunLoopTimerRef> timer;
    std::promise<CFRunLoopRef> runLoopPromise;
    HighResolutionTimerThread thread { *this };
    CFRunLoopRef runLoop = [&]() -> CFRunLoopRef
    {
       return thread.isRunning() ? runLoopPromise.get_future().get() : nullptr;
    }();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTimer)
    JUCE_DECLARE_NON_MOVEABLE (PlatformTimer)
};

} // namespace juce
