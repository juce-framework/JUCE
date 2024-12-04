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

namespace juce::detail
{

class MouseInputSourceList  : public Timer
{
public:
    MouseInputSourceList()
    {
      #if JUCE_ANDROID || JUCE_IOS
        auto mainMouseInputType = MouseInputSource::InputSourceType::touch;
      #else
        auto mainMouseInputType = MouseInputSource::InputSourceType::mouse;
      #endif

        addSource (0, mainMouseInputType);
    }

    MouseInputSource* addSource (int index, MouseInputSource::InputSourceType type)
    {
        auto* s = new MouseInputSourceImpl (index, type);
        sources.add (s);
        sourceArray.add (MouseInputSource (s));

        return &sourceArray.getReference (sourceArray.size() - 1);
    }

    MouseInputSource* getMouseSource (int index) noexcept
    {
        return isPositiveAndBelow (index, sourceArray.size()) ? &sourceArray.getReference (index)
                                                              : nullptr;
    }

    MouseInputSource* getOrCreateMouseInputSource (MouseInputSource::InputSourceType type, int touchIndex = 0)
    {
        if (type == MouseInputSource::InputSourceType::mouse
            || type == MouseInputSource::InputSourceType::pen)
        {
            for (auto& m : sourceArray)
                if (type == m.getType())
                    return &m;

            addSource (0, type);
        }
        else if (type == MouseInputSource::InputSourceType::touch)
        {
            jassert (0 <= touchIndex && touchIndex < 100); // sanity-check on number of fingers

            for (auto& m : sourceArray)
                if (type == m.getType() && touchIndex == m.getIndex())
                    return &m;

            if (canUseTouch())
                return addSource (touchIndex, type);
        }

        return nullptr;
    }

    int getNumDraggingMouseSources() const noexcept
    {
        int num = 0;

        for (auto* s : sources)
            if (s->isDragging())
                ++num;

        return num;
    }

    MouseInputSource* getDraggingMouseSource (int index) noexcept
    {
        int num = 0;

        for (auto& s : sourceArray)
        {
            if (s.isDragging())
            {
                if (index == num)
                    return &s;

                ++num;
            }
        }

        return nullptr;
    }

    void beginDragAutoRepeat (int interval)
    {
        if (interval > 0)
        {
            if (getTimerInterval() != interval)
                startTimer (interval);
        }
        else
        {
            stopTimer();
        }
    }

    void timerCallback() override
    {
        bool anyDragging = false;

        for (auto* s : sources)
        {
            // NB: when doing auto-repeat, we need to force an update of the current position and button state,
            // because on some OSes the queue can get overloaded with messages so that mouse-events don't get through..
            if (s->isDragging() && ComponentPeer::getCurrentModifiersRealtime().isAnyMouseButtonDown())
            {
                s->lastPointerState.position = s->getRawScreenPosition();
                s->triggerFakeMove();
                anyDragging = true;
            }
        }

        if (! anyDragging)
            stopTimer();
    }

    OwnedArray<MouseInputSourceImpl> sources;
    Array<MouseInputSource> sourceArray;

private:
    bool addSource();
    bool canUseTouch() const;
};

} // namespace juce::detail
