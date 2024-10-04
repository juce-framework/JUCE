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
void AnimatorUpdater::addAnimator (const Animator& animator)
{
    addAnimator (animator, nullptr);
}

void AnimatorUpdater::addAnimator (const Animator& animator, std::function<void()> onComplete)
{
    Entry entry { animator.makeWeak(), std::move (onComplete) };
    animators[entry.animator.getKey()] = std::move (entry);
}

void AnimatorUpdater::removeAnimator (const Animator& animator)
{
    if (auto it = animators.find (animator.makeWeak().getKey()); it != animators.end())
    {
        if (it == currentIterator)
        {
            ++currentIterator;
            iteratorServiced = false;
        }

        animators.erase (it);
    }
}

void AnimatorUpdater::update()
{
    update (Time::getMillisecondCounterHiRes());
}

void AnimatorUpdater::update (double timestampMs)
{
    if (reentrancyGuard)
    {
        // If this is hit, one of the animators is trying to update itself
        // recursively. This is a bad idea! Inspect the callstack to find the
        // cause of the problem.
        jassertfalse;
        return;
    }

    const ScopedValueSetter setter { reentrancyGuard, true };

    for (currentIterator = animators.begin(); currentIterator != animators.end();)
    {
        auto& current = *currentIterator;

        if (const auto locked = current.second.animator.lock())
        {
            iteratorServiced = true;

            if (locked->update (timestampMs) == Animator::Status::finished)
                NullCheckedInvocation::invoke (current.second.onComplete);

            if (iteratorServiced && currentIterator != animators.end())
                ++currentIterator;
        }
        else
        {
            currentIterator = animators.erase (currentIterator);
        }
    }
}

} // namespace juce
