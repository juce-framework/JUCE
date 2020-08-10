/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#ifndef DOXYGEN

#if JUCE_MAC

//==============================================================================
// Helper class to workaround windows not getting mouse-moves...
class FakeMouseMoveGenerator  : private Timer
{
public:
    FakeMouseMoveGenerator()
    {
        startTimer (1000 / 30);
    }

    static bool componentContainsAudioProcessorEditor (Component* comp) noexcept
    {
        if (dynamic_cast<AudioProcessorEditor*> (comp) != nullptr)
            return true;

        for (auto* child : comp->getChildren())
            if (componentContainsAudioProcessorEditor (child))
                return true;

        return false;
    }

    void timerCallback() override
    {
        // Workaround for windows not getting mouse-moves...
        auto screenPos = Desktop::getInstance().getMainMouseSource().getScreenPosition();

        if (screenPos != lastScreenPos)
        {
            lastScreenPos = screenPos;
            auto mods = ModifierKeys::currentModifiers;

            if (! mods.isAnyMouseButtonDown())
            {
                if (auto* comp = Desktop::getInstance().findComponentAt (screenPos.roundToInt()))
                {
                    if (componentContainsAudioProcessorEditor (comp->getTopLevelComponent()))
                    {
                        safeOldComponent = comp;

                        if (auto* peer = comp->getPeer())
                        {
                            if (! peer->isFocused())
                            {
                                peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse,
                                                        peer->globalToLocal (Desktop::getInstance().getMainMouseSource().getRawScreenPosition()),
                                                        mods,
                                                        MouseInputSource::invalidPressure,
                                                        MouseInputSource::invalidOrientation,
                                                        Time::currentTimeMillis());
                            }
                        }

                        return;
                    }
                }

                if (safeOldComponent != nullptr)
                {
                    if (auto* peer = safeOldComponent->getPeer())
                    {
                        peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse,
                                                MouseInputSource::offscreenMousePos,
                                                mods,
                                                MouseInputSource::invalidPressure,
                                                MouseInputSource::invalidOrientation,
                                                Time::currentTimeMillis());
                    }
                }

                safeOldComponent = nullptr;
            }
        }
    }

private:
    Point<float> lastScreenPos;
    WeakReference<Component> safeOldComponent;
};

#else
struct FakeMouseMoveGenerator {};
#endif

#endif

} // namespace juce
