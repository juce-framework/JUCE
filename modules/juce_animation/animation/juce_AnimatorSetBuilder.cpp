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
class DelayAnimator : public Animator::Impl
{
public:
    double getDurationMs() const override { return delayMs; }

    static Animator build (double delayMsIn, std::function<void()> callbackIn = nullptr)
    {
        return Animator { rawToUniquePtr (new DelayAnimator (delayMsIn, std::move (callbackIn))) };
    }

private:
    DelayAnimator (double delayMsIn, std::function<void()> callbackIn)
        : onCompletion (std::move (callbackIn)),
          delayMs (delayMsIn)
    {
    }

    Animator::Status internalUpdate (double timestampMs) override
    {
        if (timestampMs - startedAtMs >= delayMs)
            return Animator::Status::finished;

        return Animator::Status::inProgress;
    }

    void onStart (double timeMs) override { startedAtMs = timeMs; }

    void onComplete() override { NullCheckedInvocation::invoke (onCompletion); }

    std::function<void()> onCompletion;
    double startedAtMs = 0.0, delayMs = 0.0;
};

//==============================================================================
struct AnimatorSetData
{
    explicit AnimatorSetData (Animator root)
        : roots { root }, entries { { root, {} } } {}

    struct Entry
    {
        std::optional<Animator> parent; // If no parent, this is the root node
        std::set<Animator, Animator::Compare> children;
    };

    auto getRoots() const { return roots; }

    auto getChildren (const Animator& a) const
    {
        if (const auto iter = entries.find (a); iter != entries.end())
            return iter->second.children;

        return std::set<Animator, Animator::Compare>();
    }

    std::set<Animator, Animator::Compare> roots;
    std::map<Animator, Entry, Animator::Compare> entries;
    std::function<double (double)> timeTransform;
};

class AnimatorSet  : public Animator::Impl
{
public:
    explicit AnimatorSet (AnimatorSetData dataIn) : data (std::move (dataIn)) {}

    double getDurationMs() const override
    {
        const auto roots = data.getRoots();
        return getMaxDuration (roots.begin(), roots.end());
    }

private:
    void onStart (double timestampMs) override
    {
        startedAtMs = timestampMs;
        active = data.getRoots();

        for (const auto& i : active)
            i.start();
    }

    void onComplete() override {}

    Animator::Status internalUpdate (double timestampMs) override
    {
        const auto internalTimestampMs = [&]
        {
            if (data.timeTransform == nullptr)
                return timestampMs;

            return data.timeTransform (timestampMs - startedAtMs);
        }();

        if (isComplete())
        {
            for (auto i : active)
                i.complete();

            while (updateAnimatorSet (internalTimestampMs) != Animator::Status::finished)
            {
            }

            return Animator::Status::finished;
        }

        return updateAnimatorSet (internalTimestampMs);
    }

    Animator::Status updateAnimatorSet (double timestampMs)
    {
        std::set<Animator, Animator::Compare> animatorsToRemove;
        const auto currentlyActive = active;

        for (const auto& animator : currentlyActive)
        {
            const auto status = animator.update (timestampMs);

            if (status == Animator::Status::finished)
            {
                animatorsToRemove.insert (animator);

                for (const auto& j : data.getChildren (animator))
                {
                    j.start();

                    if (isComplete())
                        j.complete();

                    active.insert (j);
                }
            }
        }

        for (auto i : animatorsToRemove)
            active.erase (i);

        return active.empty() ? Animator::Status::finished : Animator::Status::inProgress;
    }

    template <typename It>
    double getMaxDuration (It begin, It end) const
    {
        return std::accumulate (begin, end, 0.0, [this] (const auto acc, const auto& anim)
        {
            const auto children = data.getChildren (anim);
            return std::max (acc, anim.getDurationMs() + getMaxDuration (children.begin(), children.end()));
        });
    }

    const AnimatorSetData data;
    std::set<Animator, Animator::Compare> active;
    double startedAtMs = 0.0;
};

//==============================================================================
struct AnimatorSetBuilder::AnimatorSetBuilderState
{
    explicit AnimatorSetBuilderState (Animator animator)
        : data (std::move (animator)) {}

    bool valid = true;
    AnimatorSetData data;
};

AnimatorSetBuilder::AnimatorSetBuilder (Animator startingAnimator)
    : AnimatorSetBuilder (startingAnimator,
                          std::make_shared<AnimatorSetBuilderState> (startingAnimator))
{}

AnimatorSetBuilder::AnimatorSetBuilder (double delayMs)
    : AnimatorSetBuilder (DelayAnimator::build (delayMs, nullptr))
{}

AnimatorSetBuilder::AnimatorSetBuilder (std::function<void()> cb)
    : AnimatorSetBuilder (DelayAnimator::build (0.0, std::move (cb)))
{}

AnimatorSetBuilder::AnimatorSetBuilder (std::shared_ptr<AnimatorSetBuilderState> dataIn)
    : cursor (*dataIn->data.getRoots().begin()), state (std::move (dataIn))
{}

AnimatorSetBuilder::AnimatorSetBuilder (Animator cursorIn, std::shared_ptr<AnimatorSetBuilderState> dataIn)
    : cursor (cursorIn), state (std::move (dataIn))
{}

AnimatorSetBuilder AnimatorSetBuilder::togetherWith (Animator animator)
{
    add (state->data.entries.at (cursor).parent, animator);
    return { animator, state };
}

AnimatorSetBuilder AnimatorSetBuilder::followedBy (Animator animator)
{
    add (cursor, animator);
    return { animator, state };
}

void AnimatorSetBuilder::add (std::optional<Animator> parent, Animator child)
{
    state->data.entries.emplace (child, AnimatorSetData::Entry { parent, {} });

    if (parent.has_value())
        state->data.entries.at (*parent).children.insert (child);
    else
        state->data.roots.insert (child);
}

Animator AnimatorSetBuilder::build()
{
    if (state == nullptr)
    {
        /*  If you're hitting this assertion, you've already used this AnimatorSetBuilder to build an
            AnimatorSet.

            To create another AnimatorSet you need to use another AnimatorSetBuilder independently
            created with the AnimatorSetBuilder (Animator) constructor.
        */
        jassertfalse;
        return ValueAnimatorBuilder{}.build();
    }

    auto animator = Animator { std::make_unique<AnimatorSet> (std::move (state->data)) };
    state = nullptr;
    return animator;
}

AnimatorSetBuilder AnimatorSetBuilder::followedBy (double delayMs)
{
    return followedBy (DelayAnimator::build (delayMs, nullptr));
}

AnimatorSetBuilder AnimatorSetBuilder::followedBy (std::function<void()> cb)
{
    return followedBy (DelayAnimator::build (0.0, std::move (cb)));
}

AnimatorSetBuilder AnimatorSetBuilder::togetherWith (double delayMs)
{
    return togetherWith (DelayAnimator::build (delayMs, nullptr));
}

AnimatorSetBuilder AnimatorSetBuilder::togetherWith (std::function<void()> cb)
{
    return togetherWith (DelayAnimator::build (0.0, std::move (cb)));
}

AnimatorSetBuilder AnimatorSetBuilder::withTimeTransform (std::function<double (double)> timeTransformIn)
{
    state->data.timeTransform = std::move (timeTransformIn);
    return *this;
}

} // namespace juce
