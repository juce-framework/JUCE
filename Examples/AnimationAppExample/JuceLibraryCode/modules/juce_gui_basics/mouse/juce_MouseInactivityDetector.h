/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_MOUSEINACTIVITYDETECTOR_H_INCLUDED
#define JUCE_MOUSEINACTIVITYDETECTOR_H_INCLUDED


//==============================================================================
/**
    This object watches for mouse-events happening within a component, and if
    the mouse remains still for long enough, triggers an event to indicate that
    it has become inactive.

    You'd use this for situations where e.g. you want to hide the mouse-cursor
    when the user's not actively using the mouse.

    After creating an instance of this, use addListener to get callbacks when
    the activity status changes.
*/
class JUCE_API  MouseInactivityDetector  : private Timer,
                                           private MouseListener
{
public:
    /** Creates an inactivity watcher, attached to the given component.
        The target component must not be deleted while this - it will be monitored
        for any mouse events in it or its child components.
    */
    MouseInactivityDetector (Component& target);

    /** Destructor. */
    ~MouseInactivityDetector();

    /** Sets the time for which the mouse must be still before the callback
        is triggered.
    */
    void setDelay (int newDelayMilliseconds);

    //==============================================================================
    /** Classes should implement this to receive callbacks from a MouseInactivityDetector
        when the mouse becomes active or inactive.
    */
    class Listener
    {
    public:
        virtual ~Listener() {}

        /** Called when the mouse is moved or clicked for the first time
            after a period of inactivity. */
        virtual void mouseBecameActive() = 0;

        /** Called when the mouse hasn't been moved for the timeout period. */
        virtual void mouseBecameInactive() = 0;
    };

    /** Registers a listener. */
    void addListener (Listener* listener);

    /** Removes a previously-registered listener. */
    void removeListener (Listener* listener);

private:
    //==============================================================================
    Component& targetComp;
    ListenerList<Listener> listenerList;
    Point<int> lastMousePos;
    int delayMs;
    bool isActive;

    void timerCallback() override;
    void wakeUp (const MouseEvent&, bool alwaysWake);
    void setActive (bool);

    void mouseMove  (const MouseEvent& e) override   { wakeUp (e, false); }
    void mouseEnter (const MouseEvent& e) override   { wakeUp (e, false); }
    void mouseExit  (const MouseEvent& e) override   { wakeUp (e, false); }
    void mouseDown  (const MouseEvent& e) override   { wakeUp (e, true); }
    void mouseDrag  (const MouseEvent& e) override   { wakeUp (e, true); }
    void mouseUp    (const MouseEvent& e) override   { wakeUp (e, true); }
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails&) override  { wakeUp (e, true); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseInactivityDetector)
};


#endif   // JUCE_MOUSEINACTIVITYDETECTOR_H_INCLUDED
