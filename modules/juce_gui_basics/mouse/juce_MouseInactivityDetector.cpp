/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

MouseInactivityDetector::MouseInactivityDetector (Component& c)
    : targetComp (c), delayMs (1500), toleranceDistance (15), isActive (true)
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
    const Point<int> newPos (e.getEventRelativeTo (&targetComp).getPosition());

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

        listenerList.call (b ? &Listener::mouseBecameActive
                             : &Listener::mouseBecameInactive);
    }
}
