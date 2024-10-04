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
/**
    A builder class that can be used to construct an Animator wrapping a ValueAnimator
    implementation.

    Every ValueAnimatorBuilder object is immutable, and every "with..." function returns a new
    object. Each object can be used independently and as many times as required to build an Animator
    object.

    Calling build() multiple times will return an independent Animator object referencing a new
    instance of the underlying implementation. Such Animator objects don't affect each other's
    lifetime. The copy of an Animator object however shares ownership with the object that it was
    copied from.

    You can treat ValueAnimatorBuilder instances as disposable objects that are only needed until
    you call build() on them. You can then store only the returned Animator object, which can be
    started and completed multiple times as needed.

    All functions beginning with "with..." are optional and can be used to affect the created
    Animator's behaviour by overriding defaults.

    @tags{Animations}
*/
class JUCE_API  ValueAnimatorBuilder
{
public:
    /** The type of the value change callback. The float parameter is related to the time parameter
        passed to Animator::update(). The update function is typically called by an AnimatorUpdater.
        The parameter will have a value of 0.0 during the first call of the value change callback,
        and it will reach 1.0 when the time passed equals the duration of the Animator. This however
        can be changed if an EasingFn is also specified. Correctly written easing functions should
        preserve the 0.0 and 1.0 start and end values, but intermittent values can fall outside the
        range of [0.0, 1.0].

        After a call with a progress value of 1.0, the on complete callback will be called and the
        updates will stop, unless the Animator is infinitely running, in which case the progress
        will go beyond 1.0 and on complete will not be called until after Animator::complete() has
        been called.

        A value change callback is optional. If you want to use one, you need to create it inside
        your on start callback that you must pass to withOnStartCallback().

        @see withOnStartCallback, withDurationMs, withEasing, runningInfinitely
    */
    using ValueChangedCallback = std::function<void (float)>;

    /** The type of the on start callback. It can be used to do any initialisation necessary at the
        start of an animation, then it must return a ValueChangedCallback.

        The ValueChangedCallback is called during every update of the Animator after the on start
        callback and before the on complete callback.

        The on start callback is called during the first update that the Animator receives after
        Animator::start() has been called. Immediately after the on start callback the first call to
        the value changed callback is made.

        @see ValueChangedCallback
    */
    using OnStartReturningValueChangedCallback = std::function<ValueChangedCallback()>;

    /** The type of an optional easing function that can be passed to the withEasing() builder
        function.

        If this function is specified it will be called with the progress value going from 0.0 to
        1.0 (and beyond in case of an infinitely running Animator), that would otherwise be passed
        directly to the value change callback. The return value is then passed to the on value
        change callback instead.

        This function can be used to change the linear progression of the animation and create
        effects like rubber band like motion.

        @see ValueChangedCallback
    */
    using EasingFn            = std::function<float (float)>;

    /** Use this function to specify an optional on start callback.

        Alternatively you can use the withOnStartReturningValueChangedCallback function that allows
        you to return the ValueChangedCallback from inside your on start callback.

        You can only use either withOnStartCallback() or withOnStartReturningValueChangedCallback().
    */
    [[nodiscard]] ValueAnimatorBuilder withOnStartCallback (std::function<void()> onStartCallback) const
    {
        return with (&ValueAnimatorBuilder::onStartReturningValueChanged,
                     [start = std::move (onStartCallback), previous = onStartReturningValueChanged]
                     {
                         NullCheckedInvocation::invoke (start);
                         return previous != nullptr ? previous() : nullptr;
                     });
    }

    /** Use this function to specify an optional on change callback.

        Alternatively you can use the withOnStartReturningValueChangedCallback function that allows
        you to return the ValueChangedCallback from inside your on start callback.

        You can only use either withValueChangedCallback or withOnStartReturningValueChangedCallback.

        @see OnStartReturningValueChangedCallback
    */
    [[nodiscard]] ValueAnimatorBuilder withValueChangedCallback (ValueChangedCallback valueChangedCallback) const
    {
        return with (&ValueAnimatorBuilder::onStartReturningValueChanged,
                     [changed = std::move (valueChangedCallback), previous = onStartReturningValueChanged]
                     {
                         return [changed, previousChanged = previous != nullptr ? previous() : nullptr] (float x)
                         {
                             NullCheckedInvocation::invoke (previousChanged, x);
                             NullCheckedInvocation::invoke (changed, x);
                         };
                     });
    }

    /** Use this function to specify an optional on start callback.

        The return value of the provided function is a ValueChangedCallback. This allows you to
        construct a new ValueChangedCallback on every on start event, capturing state that is also
        constructed at the time of starting.

        If you don't need to return a new ValueChangedCallback on every animation start, you can use
        the simpler variants withOnStartCallback and withValueChangedCallback. However you cannot
        use those functions together with this one.

        @see OnStartReturningValueChangedCallback, withOnStartCallback, withValueChangedCallback
    */
    [[nodiscard]] ValueAnimatorBuilder withOnStartReturningValueChangedCallback (OnStartReturningValueChangedCallback value) const
    {
        return with (&ValueAnimatorBuilder::onStartReturningValueChanged, std::move (value));
    }

    /** Use this function to optionally specify an on complete callback. This function will be
        called after the Animator reached a progress value >= 1.0, or in the case of an
        infinitely running animation, if Animator::complete() has been called.
    */
    [[nodiscard]] ValueAnimatorBuilder withOnCompleteCallback (std::function<void()> value) const
    {
        return with (&ValueAnimatorBuilder::onComplete, std::move (value));
    }

    /** Use this function to specify the time it takes for the Animator to reach a progress of 1.0.

        The default value is 300 ms.

        A progress of 1.0 will be reached after this time elapses even if the Animator is infinitely
        running.
    */
    [[nodiscard]] ValueAnimatorBuilder withDurationMs (double durationMsIn) const
    {
        return with (&ValueAnimatorBuilder::durationMs, durationMsIn);
    }

    /** Supply a function that transforms the linear progression of time.

        @see EasingFn
    */
    [[nodiscard]] ValueAnimatorBuilder withEasing (EasingFn fn) const
    {
        return with (&ValueAnimatorBuilder::easing, std::move (fn));
    }

    /** This function specifies that the Animator will keep running even after its progress > 1.0
        and its on complete function will not be called until Animator::complete() is called.
    */
    [[nodiscard]] ValueAnimatorBuilder runningInfinitely() const
    {
        return with (&ValueAnimatorBuilder::infinitelyRunning, true);
    }

    //==============================================================================
    /** Getter function used by the corresponding Animator implementation.
     */
    auto& getOnComplete() const
    {
        return onComplete;
    }

    /** Getter function used by the corresponding Animator implementation.
    */
    auto& getOnStartWithValueChanged() const
    {
        return onStartReturningValueChanged;
    }

    /** Getter function used by the corresponding Animator implementation.
     */
    auto getDurationMs() const
    {
        return durationMs;
    }

    /** Getter function used by the corresponding Animator implementation.
     */
    auto isInfinitelyRunning() const
    {
        return infinitelyRunning;
    }

    /** Getter function used by the corresponding Animator implementation.
     */
    auto& getEasing() const
    {
        return easing;
    }

    /** The build() function will instantiate a new underlying implementation with the specified
        parameters and return an Animator object referencing it. Calling build() multiple times
        will return unrelated Animator objects, that reference separate underlying implementation
        instances.
    */
    Animator build() const&;

    /** The build() function will instantiate a new underlying implementation with the specified
        parameters and return an Animator object referencing it. Calling build() multiple times
        will return unrelated Animator objects, that reference separate underlying implementation
        instances.

        This overload will be called on rvalue handles.
    */
    Animator build() &&;

private:
    //==============================================================================
    template <typename Member, typename Value>
    ValueAnimatorBuilder with (Member&& member, Value&& value) const noexcept
    {
        auto copy = *this;
        copy.*member = std::forward<Value> (value);
        return copy;
    }

    OnStartReturningValueChangedCallback onStartReturningValueChanged;
    std::function<void()> onComplete;
    double durationMs = 300.0;
    bool infinitelyRunning = false;
    EasingFn easing = Easings::createEase();
};

} // namespace juce
