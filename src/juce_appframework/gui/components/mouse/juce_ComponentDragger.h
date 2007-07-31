/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_COMPONENTDRAGGER_JUCEHEADER__
#define __JUCE_COMPONENTDRAGGER_JUCEHEADER__

#include "juce_MouseEvent.h"
#include "../layout/juce_ComponentBoundsConstrainer.h"


//==============================================================================
/**
    An object to take care of the logic for dragging components around with the mouse.

    Very easy to use - in your mouseDown() callback, call startDraggingComponent(),
    then in your mouseDrag() callback, call dragComponent().

    When starting a drag, you can give it a ComponentBoundsConstrainer to use
    to limit the component's position and keep it on-screen.

    e.g. @code
    class MyDraggableComp
    {
        ComponentDragger myDragger;

        void mouseDown (const MouseEvent& e)
        {
            myDragger.startDraggingComponent (this, 0);
        }

        void mouseDrag (const MouseEvent& e)
        {
            myDragger.dragComponent (this, e);
        }
    };
    @endcode
*/
class JUCE_API  ComponentDragger
{
public:
    //==============================================================================
    /** Creates a ComponentDragger. */
    ComponentDragger();

    /** Destructor. */
    virtual ~ComponentDragger();

    //==============================================================================
    /** Call this from your component's mouseDown() method, to prepare for dragging.

        @param componentToDrag      the component that you want to drag
        @param constrainer          a constrainer object to use to keep the component
                                    from going offscreen
        @see dragComponent
    */
    void startDraggingComponent (Component* const componentToDrag,
                                 ComponentBoundsConstrainer* constrainer);

    /** Call this from your mouseDrag() callback to move the component.

        This will move the component, but will first check the validity of the
        component's new position using the checkPosition() method, which you
        can override if you need to enforce special positioning limits on the
        component.

        @param componentToDrag      the component that you want to drag
        @param e                    the current mouse-drag event
        @see dragComponent
    */
    void dragComponent (Component* const componentToDrag,
                        const MouseEvent& e);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ComponentBoundsConstrainer* constrainer;
    int originalX, originalY;
};

#endif   // __JUCE_COMPONENTDRAGGER_JUCEHEADER__
