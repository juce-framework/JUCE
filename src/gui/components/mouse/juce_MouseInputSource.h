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

#ifndef __JUCE_MOUSEEVENT_JUCEHEADER__x
#define __JUCE_MOUSEEVENT_JUCEHEADER__x

class Component;
class ComponentPeer;
class MouseInputSourceInternal;
#include "../keyboard/juce_ModifierKeys.h"
#include "../../../core/juce_Time.h"
#include "../../../containers/juce_ScopedPointer.h"
#include "../../graphics/geometry/juce_Point.h"
#include "../juce_Desktop.h"


//==============================================================================
/**
*/
class JUCE_API  MouseInputSource
{
public:
    //==============================================================================
    MouseInputSource (int index, bool isMouseDevice);
    ~MouseInputSource();

    //==============================================================================
    bool isMouse() const;
    bool isTouch() const;
    bool canHover() const;
    bool hasMouseWheel() const;

    int getIndex() const;

    bool isDragging() const;
    const Point<int> getScreenPosition() const;
    const ModifierKeys getCurrentModifiers() const;
    Component* getComponentUnderMouse() const;
    void triggerFakeMove() const;
    int getNumberOfMultipleClicks() const throw();
    const Time getLastMouseDownTime() const throw();
    const Point<int> getLastMouseDownPosition() const throw();
    bool hasMouseMovedSignificantlySincePressed() const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

    void handleEvent (ComponentPeer* peer, const Point<int>& positionWithinPeer, int64 time, const ModifierKeys& mods);
    void handleWheel (ComponentPeer* peer, const Point<int>& positionWithinPeer, int64 time, float x, float y);
    
private:
    friend class MouseInputSourceInternal;
    ScopedPointer<MouseInputSourceInternal> pimpl;

    friend class Desktop;
    friend class ComponentPeer;

    MouseInputSource (const MouseInputSource&);
    MouseInputSource& operator= (const MouseInputSource&);
};


#endif   // __JUCE_MOUSEEVENT_JUCEHEADER__
