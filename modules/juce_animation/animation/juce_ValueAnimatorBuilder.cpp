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

//==============================================================================
class ValueAnimator : public Animator::Impl
{
public:
    explicit ValueAnimator (ValueAnimatorBuilder optionsIn) : options (std::move (optionsIn)) {}

    auto getValue() const
    {
        using namespace detail::ArrayAndTupleOps;

        return options.getEasing() == nullptr ? getProgress() : options.getEasing() (getProgress());
    }

    float getProgress() const
    {
        if (isComplete())
            return 1.0f;

        return timeBasedProgress;
    }

    /** Returns the time in milliseconds that it takes for the progress to go from 0.0 to 1.0.

        This is the value returned even if the Animator is infinitely running.
    */
    double getDurationMs() const override
    {
        return options.getDurationMs();
    }

    bool isComplete() const override
    {
        return Animator::Impl::isComplete()
               || (! options.isInfinitelyRunning() && timeBasedProgress >= 1.0f);
    }

private:
    Animator::Status internalUpdate (double timestampMs) override
    {
        timeBasedProgress = (float) ((timestampMs - startedAtMs) / options.getDurationMs());

        NullCheckedInvocation::invoke (onValueChanged, getValue());

        if (! options.isInfinitelyRunning())
            return getProgress() >= 1.0 ? Animator::Status::finished : Animator::Status::inProgress;

        return Animator::Status::inProgress;
    }

    void onStart (double timeMs) override
    {
        startedAtMs = timeMs;
        timeBasedProgress = 0.0f;

        if (auto fn = options.getOnStartWithValueChanged())
            onValueChanged = fn();
    }

    void onComplete() override
    {
        NullCheckedInvocation::invoke (options.getOnComplete());
    }

    double startedAtMs = 0.0;
    float timeBasedProgress = 0.0f;

    const ValueAnimatorBuilder options;
    ValueAnimatorBuilder::ValueChangedCallback onValueChanged;
};

//==============================================================================
Animator ValueAnimatorBuilder::build() const&  { return Animator { std::make_unique<ValueAnimator> (*this) }; }
Animator ValueAnimatorBuilder::build() &&      { return Animator { std::make_unique<ValueAnimator> (std::move (*this)) }; }

} // namespace juce
