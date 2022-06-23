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
            myDragger.startDraggingComponent (this, e);
        }

        void mouseDrag (const MouseEvent& e)
        {
            myDragger.dragComponent (this, e, nullptr);
        }
    };
    @endcode

    @tags{GUI}
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
        @param e                    the mouse event that is triggering the drag
        @see dragComponent
    */
    void startDraggingComponent (Component* componentToDrag,
                                 const MouseEvent& e);

    /** Call this from your mouseDrag() callback to move the component.

        This will move the component, using the given constrainer object to check
        the new position.

        @param componentToDrag      the component that you want to drag
        @param e                    the current mouse-drag event
        @param constrainer          an optional constrainer object that should be used
                                    to apply limits to the component's position. Pass
                                    null if you don't want to constrain the movement.
        @see startDraggingComponent
    */
    void dragComponent (Component* componentToDrag,
                        const MouseEvent& e,
                        ComponentBoundsConstrainer* constrainer);

private:
    //==============================================================================
    Point<int> mouseDownWithinTarget;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentDragger)
};

} // namespace juce
