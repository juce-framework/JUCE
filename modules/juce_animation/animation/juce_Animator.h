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
    Wrapper class for managing the lifetime of all the different animator kinds created through the
    builder classes.

    It uses reference counting. If you copy an Animator the resulting object will refer to the same
    underlying instance, and the underlying instance is guaranteed to remain valid for as long as
    you have an Animator object referencing it.

    An Animator object can be registered with the AnimatorUpdater, which only stores a weak
    reference to the underlying instance. If an AnimatorUpdater references the underlying instance
    and it becomes deleted due to all Animator objects being deleted, the updater will automatically
    remove it from its queue, so manually removing it is not required.

    @see ValueAnimatorBuilder, AnimatorSetBuilder, AnimatorUpdater, VBlankAnimatorUpdater

    @tags{Animations}
*/
class JUCE_API  Animator
{
public:
    class Impl;

    /** The state of an Animator that determines how AnimatorUpdaters and other Animators will
        interact with it.
    */
    enum class Status
    {
        /** The Animator is idle and its state is not progressing even if it is attached to an
            AnimatorUpdater.
        */
        idle,

        /** The Animator is active and its state is progressing whenever its update function is
            called.
        */
        inProgress,

        /** The Animator finished its run and its onCompletion callback may be called. It requires
            no further calls to its update function.
        */
        finished
    };

    /** @internal

        Constructor. Used by the builder classes.

        @see ValueAnimatorBuilder, AnimatorSetBuilder
    */
    explicit Animator (std::shared_ptr<Impl>);

    /** Returns the total animation duration in milliseconds. */
    double getDurationMs() const;

    /** Marks the Animator ready for starting. You must call this function to allow the Animator to
        move out of the idle state.

        After calling this function the Animator's on start callback will be executed at the next
        update immediately followed by the first call to it's update function.

        You can call this function before or after adding the Animator to an AnimatorUpdater. Until
        start() is called the Animator will just sit idly in the updater's queue.
    */
    void start() const;

    /** Marks the Animator ready to be completed. ValueAnimators will be completed automatically
        when they reach a progress >= 1.0 unless they are infinitely running. AnimatorSets will also
        complete on their own when all of their constituent Animators complete.

        Using this function you can fast track the completion of an Animator. After calling this
        function isComplete will return true, and it's guaranteed that you will receive an update
        callback with a progress value of 1.0. After this the onComplete callback will be executed.
    */
    void complete() const;

    /** Called periodically for active Animators by the AnimatorUpdater classes. The passed in
        timestamp must be monotonically increasing. This allows the underlying Animator
        to follow its progression towards completion.

        While you can call this function in special circumstances, you will generally want an
        AnimatorUpdater to do it. Using the VBlankAnimatorUpdater ensures that update is called in
        sync with the monitor's vertical refresh resulting in smooth animations.

        @see AnimatorUpdater, VBlankAnimatorUpdater
    */
    Status update (double timestampMs) const;

    /** Returns true if the Animator has reached the point of completion either because complete()
        has been called on it, or in case of the ValueAnimator, if it reached a progress of >= 1.0.

        You typically don't need to call this function, because in any case a completed Animator
        will receive an update callback with a progress value of 1.0 and following that the
        on complete callback will be called.
    */
    bool isComplete() const;

    /** Comparison function used by the implementation to store Animators in ordered collections.
        It can also be used to determine equality of Animator objects based on whether they
        reference the same underlying implementation.
    */
    struct JUCE_API  Compare
    {
        /** Comparison function. */
        bool operator() (const Animator& a, const Animator& b) const { return a.ptr < b.ptr; }
    };

    /** @internal

        Stores a weak reference to the Animator's underlying implementation. Animator objects store
        a strong reference to the implementation, so it won't be deleted as long as an Animator
        object references it. Instead of copying the Animator, you can use makeWeak() to create a
        weak reference, which will not prevent deletion of the underlying implementation, but allows
        you to create a strong reference using the lock function for as long as the underlying
        object is alive.

        This class is used by the AnimatorUpdater, and it's unlikely you will need to use it
        directly.

        @see AnimatorUpdater, VBlankAnimatorUpdater
    */
    class JUCE_API  Weak
    {
    public:
        /** Constructor used by the Animator implementation. To obtain a weak reference use
            Animator::makeWeak().
        */
        Weak() = default;

        /** Constructor used by the Animator implementation. To obtain a weak reference use
            Animator::makeWeak().
        */
        explicit Weak (std::shared_ptr<Impl> p) : ptr (p), originalPtr (p.get()) {}

        /** If the referenced Animator implementation object still exists it returns an Animator
            object storing a strong reference to it.

            If the implementation object was deleted it returns a nullopt.
        */
        std::optional<Animator> lock() const
        {
            if (const auto l = ptr.lock())
                return Animator { l };

            return {};
        }

        /** @internal

            Used internally for storing the reference in a std::map.
        */
        void* getKey() const
        {
            return originalPtr;
        }

    private:
        std::weak_ptr<Impl> ptr;
        Impl* originalPtr{};
    };

    /** @internal

        Returns a weak reference to the underlying implementation. You can use Weak::lock() to
        obtain a strong reference as long as the underlying object has not been deleted.
    */
    Weak makeWeak() const { return Weak { ptr }; }

private:
    std::shared_ptr<Impl> ptr;
};

} // namespace juce
