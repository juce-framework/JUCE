/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

MouseInactivityDetector::MouseInactivityDetector (Component& c)  : targetComp (c)
{
    targetComp.addMouseListener (this, true);
}

MouseInactivityDetector::~MouseInactivityDetector()
{
    targetComp.removeMouseListener (this);
}

void MouseInactivityDetector::setDelay (int newDelay) noexcept                  { delayMs = newDelay; }
void MouseInactivityDetector::setMouseMoveTolerance (int newDistance) noexcept  { toleranceDistance = newDistance; }

void MouseInactivityDetector::addListener    (Listener* l)   { listenerList.add (l); }
void MouseInactivityDetector::removeListener (Listener* l)   { listenerList.remove (l); }

void MouseInactivityDetector::timerCallback()
{
    setActive (false);
}

void MouseInactivityDetector::wakeUp (const MouseEvent& e, bool alwaysWake)
{
    auto newPos = e.getEventRelativeTo (&targetComp).getPosition();

    if ((! isActive) && (alwaysWake || e.source.isTouch() || newPos.getDistanceFrom (lastMousePos) > toleranceDistance))
        setActive (true);

    if (lastMousePos != newPos)
    {
        lastMousePos = newPos;
        startTimer (delayMs);
    }
}

void MouseInactivityDetector::setActive (bool b)
{
    if (isActive != b)
    {
        isActive = b;

        if (isActive)
            listenerList.call ([] (Listener& l) { l.mouseBecameActive(); });
        else
            listenerList.call ([] (Listener& l) { l.mouseBecameInactive(); });
    }
}

} // namespace juce
