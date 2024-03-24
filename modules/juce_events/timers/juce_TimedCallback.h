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
