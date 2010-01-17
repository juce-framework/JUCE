/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_MOUSEHOVERDETECTOR_JUCEHEADER__
#define __JUCE_MOUSEHOVERDETECTOR_JUCEHEADER__

#include "juce_MouseListener.h"
#include "../../../events/juce_Timer.h"


//==============================================================================
/**
    Monitors a component for mouse activity, and triggers a callback
    when the mouse hovers in one place for a specified length of time.

    To use a hover-detector, just create one and call its setHoverComponent()
    method to start it watching a component. You can call setHoverComponent (0)
    to make it inactive.

    (Be careful not to delete a component that's being monitored without first
    stopping or deleting the hover detector).
*/
class JUCE_API  MouseHoverDetector
{
public:
    //==============================================================================
    /** Creates a hover detector.

        Initially the object is inactive, and you need to tell it which component
        to monitor, using the setHoverComponent() method.

        @param hoverTimeMillisecs   the number of milliseconds for which the mouse
                                    needs to stay still before the mouseHovered() method
                                    is invoked. You can change this setting later with
                                    the setHoverTimeMillisecs() method
    */
    MouseHoverDetector (const int hoverTimeMillisecs = 400);

    /** Destructor. */
    virtual ~MouseHoverDetector();

    //==============================================================================
    /** Changes the time for which the mouse has to stay still before it's considered
        to be hovering.
    */
    void setHoverTimeMillisecs (const int newTimeInMillisecs);

    /** Changes the component that's being monitored for hovering.

        Be careful not to delete a component that's being monitored without first
        stopping or deleting the hover detector.
    */
    void setHoverComponent (Component* const newSourceComponent);


protected:
    //==============================================================================
    /** Called back when the mouse hovers.

        After the mouse has stayed still over the component for the length of time
        specified by setHoverTimeMillisecs(), this method will be invoked.

        When the mouse is first moved after this callback has occurred, the
        mouseMovedAfterHover() method will be called.

        @param mouseX   the mouse's X position relative to the component being monitored
        @param mouseY   the mouse's Y position relative to the component being monitored
    */
    virtual void mouseHovered (int mouseX,
                               int mouseY) = 0;

    /** Called when the mouse is moved away after just having hovered. */
    virtual void mouseMovedAfterHover() = 0;



private:
    //==============================================================================
    class JUCE_API  HoverDetectorInternal  : public MouseListener,
                                             public Timer
    {
    public:
        MouseHoverDetector* owner;
        int lastX, lastY;

        void timerCallback();
        void mouseEnter (const MouseEvent&);
        void mouseExit (const MouseEvent&);
        void mouseDown (const MouseEvent&);
        void mouseUp (const MouseEvent&);
        void mouseMove (const MouseEvent&);
        void mouseWheelMove (const MouseEvent&, float, float);

    } internalTimer;

    friend class HoverDetectorInternal;

    Component* source;
    int hoverTimeMillisecs;
    bool hasJustHovered;

    void hoverTimerCallback();
    void checkJustHoveredCallback();

    MouseHoverDetector (const MouseHoverDetector&);
    const MouseHoverDetector& operator= (const MouseHoverDetector&);
};

#endif   // __JUCE_MOUSEHOVERDETECTOR_JUCEHEADER__
