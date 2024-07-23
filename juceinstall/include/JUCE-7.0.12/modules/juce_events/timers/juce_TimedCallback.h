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

/** Utility class wrapping a single non-null callback called by a Timer.

    You can use the usual Timer functions to start and stop the TimedCallback. Deleting the
    TimedCallback will automatically stop the underlying Timer.

    With this class you can use the Timer facility without inheritance.

    @see Timer
    @tags{Events}
*/
class TimedCallback final : private Timer
{
public:
    /** Constructor. The passed in callback must be non-null. */
    explicit TimedCallback (std::function<void()> callbackIn)
        : callback (std::move (callbackIn))
    {
        jassert (callback);
    }

    /** Destructor. */
    ~TimedCallback() noexcept override { stopTimer(); }

    using Timer::startTimer;
    using Timer::startTimerHz;
    using Timer::stopTimer;
    using Timer::isTimerRunning;
    using Timer::getTimerInterval;

private:
    void timerCallback() override { callback(); }

    std::function<void()> callback;
};

} // namespace juce
