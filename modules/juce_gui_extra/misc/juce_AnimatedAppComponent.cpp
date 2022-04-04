/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AnimatedAppComponent::AnimatedAppComponent()
    : lastUpdateTime (Time::getCurrentTime()), totalUpdates (0)
{
    setOpaque (true);
}

void AnimatedAppComponent::setFramesPerSecond (int framesPerSecond)
{
    jassert (framesPerSecond > 0 && framesPerSecond < 1000);
    startTimerHz (framesPerSecond);
}

int AnimatedAppComponent::getMillisecondsSinceLastUpdate() const noexcept
{
    return (int) (Time::getCurrentTime() - lastUpdateTime).inMilliseconds();
}

void AnimatedAppComponent::timerCallback()
{
    ++totalUpdates;
    update();
    repaint();
    lastUpdateTime = Time::getCurrentTime();
}

} // namespace juce
