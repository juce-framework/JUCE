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
    This object watches for mouse-events happening within a component, and if
    the mouse remains still for long enough, triggers an event to indicate that
    it has become inactive.

    You'd use this for situations where e.g. you want to hide the mouse-cursor
    when the user's not actively using the mouse.

    After creating an instance of this, use addListener to get callbacks when
    the activity status changes.

    @tags{GUI}
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
    ~MouseInactivityDetector() override;

    /** Sets the time for which the mouse must be still before the callback
        is triggered.
    */
    void setDelay (int newDelayMilliseconds) noexcept;

    /** Sets the number of pixels by which the cursor is allowed to drift before it is
        considered to be actively moved.
    */
    void setMouseMoveTolerance (int pixelsNeededToTrigger) noexcept;

    //==============================================================================
    /** Classes should implement this to receive callbacks from a MouseInactivityDetector
        when the mouse becomes active or inactive.
    */
    class Listener
    {
    public:
        virtual ~Listener() = default;

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
    int delayMs = 1500, toleranceDistance = 15;
    bool isActive = true;

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

} // namespace juce
