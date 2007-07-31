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

#ifndef __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
#define __JUCE_DRAGANDDROPTARGET_JUCEHEADER__

#include "../juce_Component.h"


//==============================================================================
/**
    Components derived from this class can have things dropped onto them by a DragAndDropContainer.

    To create a component that can receive things drag-and-dropped by a DragAndDropContainer,
    derive your component from this class, and make sure that it is somewhere inside a
    DragAndDropContainer component.

    Note: If all that you need to do is to respond to files being drag-and-dropped from
    the operating system onto your component, you don't need any of these classes: you can do this
    simply by overriding Component::filesDropped().

    @see DragAndDropContainer
*/
class JUCE_API  DragAndDropTarget
{
public:
    /** Destructor. */
    virtual ~DragAndDropTarget()  {}

    /** Callback to check whether this target is interested in the type of object being
        dragged.

        @param sourceDescription    the description string passed into DragAndDropContainer::startDragging()
        @returns                    true if this component wants to receive the other callbacks regarging this
                                    type of object; if it returns false, no other callbacks will be made.
    */
    virtual bool isInterestedInDragSource (const String& sourceDescription) = 0;

    /** Callback to indicate that something is being dragged over this component.

        This gets called when the user moves the mouse into this component while dragging
        something.

        Use this callback as a trigger to make your component repaint itself to give the
        user feedback about whether the item can be dropped here or not.

        @param sourceDescription    the description string passed into DragAndDropContainer::startDragging()
        @param sourceComponent      the component passed into DragAndDropContainer::startDragging()
        @param x                    the mouse x position, relative to this component
        @param y                    the mouse y position, relative to this component
        @see itemDragExit
    */
    virtual void itemDragEnter (const String& sourceDescription,
                                Component* sourceComponent,
                                int x,
                                int y);

    /** Callback to indicate that the user is dragging something over this component.

        This gets called when the user moves the mouse over this component while dragging
        something. Normally overriding itemDragEnter() and itemDragExit() are enough, but
        this lets you know what happens in-between.

        @param sourceDescription    the description string passed into DragAndDropContainer::startDragging()
        @param sourceComponent      the component passed into DragAndDropContainer::startDragging()
        @param x                    the mouse x position, relative to this component
        @param y                    the mouse y position, relative to this component
    */
    virtual void itemDragMove (const String& sourceDescription,
                               Component* sourceComponent,
                               int x,
                               int y);

    /** Callback to indicate that something has been dragged off the edge of this component.

        This gets called when the user moves the mouse out of this component while dragging
        something.

        If you've used itemDragEnter() to repaint your component and give feedback, use this
        as a signal to repaint it in its normal state.

        @param sourceDescription    the description string passed into DragAndDropContainer::startDragging()
        @param sourceComponent      the component passed into DragAndDropContainer::startDragging()
        @see itemDragEnter
    */
    virtual void itemDragExit (const String& sourceDescription,
                               Component* sourceComponent);

    /** Callback to indicate that the user has dropped something onto this component.

        When the user drops an item this get called, and you can use the description to
        work out whether your object wants to deal with it or not.

        @param sourceDescription    the description string passed into DragAndDropContainer::startDragging()
        @param sourceComponent      the component passed into DragAndDropContainer::startDragging()
        @param x                    the mouse x position, relative to this component
        @param y                    the mouse y position, relative to this component
    */
    virtual void itemDropped (const String& sourceDescription,
                              Component* sourceComponent,
                              int x,
                              int y) = 0;

    /** Overriding this allows the target to tell the drag container whether to
        draw the drag image while the cursor is over it.

        By default it returns true, but if you return false, then the normal drag
        image will not be shown when the cursor is over this target.
    */
    virtual bool shouldDrawDragImageWhenOver();
};


#endif   // __JUCE_DRAGANDDROPTARGET_JUCEHEADER__
