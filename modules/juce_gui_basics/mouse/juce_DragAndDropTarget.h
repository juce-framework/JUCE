/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
#define __JUCE_DRAGANDDROPTARGET_JUCEHEADER__

#include "../components/juce_Component.h"


//==============================================================================
/**
    Components derived from this class can have things dropped onto them by a DragAndDropContainer.

    To create a component that can receive things drag-and-dropped by a DragAndDropContainer,
    derive your component from this class, and make sure that it is somewhere inside a
    DragAndDropContainer component.

    Note: If all that you need to do is to respond to files being drag-and-dropped from
    the operating system onto your component, you don't need any of these classes: instead
    see the FileDragAndDropTarget class.

    @see DragAndDropContainer, FileDragAndDropTarget
*/
class JUCE_API  DragAndDropTarget
{
public:
    /** Destructor. */
    virtual ~DragAndDropTarget()  {}

    //==============================================================================
    /** Contains details about the source of a drag-and-drop operation.
        The contents of this
    */
    class JUCE_API  SourceDetails
    {
    public:
        /** Creates a SourceDetails object from its various settings. */
        SourceDetails (const var& description,
                       Component* sourceComponent,
                       const Point<int>& localPosition) noexcept;

        /** A descriptor for the drag - this is set DragAndDropContainer::startDragging(). */
        var description;

        /** The component from the drag operation was started. */
        WeakReference<Component> sourceComponent;

        /** The local position of the mouse, relative to the target component.
            Note that for calls such as isInterestedInDragSource(), this may be a null position.
        */
        Point<int> localPosition;
    };

    //==============================================================================
    /** Callback to check whether this target is interested in the type of object being
        dragged.

        @param dragSourceDetails    contains information about the source of the drag operation.
        @returns                    true if this component wants to receive the other callbacks regarging this
                                    type of object; if it returns false, no other callbacks will be made.
    */
    virtual bool isInterestedInDragSource (const SourceDetails& dragSourceDetails) = 0;

    /** Callback to indicate that something is being dragged over this component.

        This gets called when the user moves the mouse into this component while dragging
        something.

        Use this callback as a trigger to make your component repaint itself to give the
        user feedback about whether the item can be dropped here or not.

        @param dragSourceDetails    contains information about the source of the drag operation.
        @see itemDragExit
    */
    virtual void itemDragEnter (const SourceDetails& dragSourceDetails);

    /** Callback to indicate that the user is dragging something over this component.

        This gets called when the user moves the mouse over this component while dragging
        something. Normally overriding itemDragEnter() and itemDragExit() are enough, but
        this lets you know what happens in-between.

        @param dragSourceDetails    contains information about the source of the drag operation.
    */
    virtual void itemDragMove (const SourceDetails& dragSourceDetails);

    /** Callback to indicate that something has been dragged off the edge of this component.

        This gets called when the user moves the mouse out of this component while dragging
        something.

        If you've used itemDragEnter() to repaint your component and give feedback, use this
        as a signal to repaint it in its normal state.

        @param dragSourceDetails    contains information about the source of the drag operation.
        @see itemDragEnter
    */
    virtual void itemDragExit (const SourceDetails& dragSourceDetails);

    /** Callback to indicate that the user has dropped something onto this component.

        When the user drops an item this get called, and you can use the description to
        work out whether your object wants to deal with it or not.

        Note that after this is called, the itemDragExit method may not be called, so you should
        clean up in here if there's anything you need to do when the drag finishes.

        @param dragSourceDetails    contains information about the source of the drag operation.
    */
    virtual void itemDropped (const SourceDetails& dragSourceDetails) = 0;

    /** Overriding this allows the target to tell the drag container whether to
        draw the drag image while the cursor is over it.

        By default it returns true, but if you return false, then the normal drag
        image will not be shown when the cursor is over this target.
    */
    virtual bool shouldDrawDragImageWhenOver();


    //==============================================================================
private:
   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // The parameters for these methods have changed - please update your code!
    virtual void isInterestedInDragSource (const String&, Component*) {}
    virtual int itemDragEnter (const String&, Component*, int, int) { return 0; }
    virtual int itemDragMove (const String&, Component*, int, int) { return 0; }
    virtual int itemDragExit (const String&, Component*) { return 0; }
    virtual int itemDropped (const String&, Component*, int, int) { return 0; }
   #endif
};

#endif   // __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
