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

#ifndef __JUCE_MOUSECURSOR_JUCEHEADER__
#define __JUCE_MOUSECURSOR_JUCEHEADER__

#include "../../../containers/juce_ReferenceCountedObject.h"
class Image;
class SharedMouseCursorInternal;
class ComponentPeer;
class Component;


//==============================================================================
/**
    Represents a mouse cursor image.

    This object can either be used to represent one of the standard mouse
    cursor shapes, or a custom one generated from an image.
*/
class JUCE_API  MouseCursor
{
public:
    //==============================================================================
    /** The set of available standard mouse cursors. */
    enum StandardCursorType
    {
        NoCursor = 0,                   /**< An invisible cursor. */
        NormalCursor,                   /**< The stardard arrow cursor. */

        WaitCursor,                     /**< The normal hourglass or spinning-beachball 'busy' cursor. */
        IBeamCursor,                    /**< A vertical I-beam for positioning within text. */
        CrosshairCursor,                /**< A pair of crosshairs. */
        CopyingCursor,                  /**< The normal arrow cursor, but with a "+" on it to indicate
                                             that you're dragging a copy of something. */

        PointingHandCursor,             /**< A hand with a pointing finger, for clicking on web-links. */
        DraggingHandCursor,             /**< An open flat hand for dragging heavy objects around. */

        LeftRightResizeCursor,          /**< An arrow pointing left and right. */
        UpDownResizeCursor,             /**< an arrow pointing up and down. */
        UpDownLeftRightResizeCursor,    /**< An arrow pointing up, down, left and right. */

        TopEdgeResizeCursor,            /**< A platform-specific cursor for resizing the top-edge of a window. */
        BottomEdgeResizeCursor,         /**< A platform-specific cursor for resizing the bottom-edge of a window. */
        LeftEdgeResizeCursor,           /**< A platform-specific cursor for resizing the left-edge of a window. */
        RightEdgeResizeCursor,          /**< A platform-specific cursor for resizing the right-edge of a window. */
        TopLeftCornerResizeCursor,      /**< A platform-specific cursor for resizing the top-left-corner of a window. */
        TopRightCornerResizeCursor,     /**< A platform-specific cursor for resizing the top-right-corner of a window. */
        BottomLeftCornerResizeCursor,   /**< A platform-specific cursor for resizing the bottom-left-corner of a window. */
        BottomRightCornerResizeCursor   /**< A platform-specific cursor for resizing the bottom-right-corner of a window. */
    };

    //==============================================================================
    /** Creates the standard arrow cursor. */
    MouseCursor() throw();

    /** Creates one of the standard mouse cursor */
    MouseCursor (const StandardCursorType type) throw();

    /** Creates a custom cursor from an image.

        @param image    the image to use for the cursor - if this is bigger than the
                        system can manage, it might get scaled down first, and might
                        also have to be turned to black-and-white if it can't do colour
                        cursors.
        @param hotSpotX the x position of the cursor's hotspot within the image
        @param hotSpotY the y position of the cursor's hotspot within the image
    */
    MouseCursor (const Image& image,
                 const int hotSpotX,
                 const int hotSpotY) throw();

    //==============================================================================
    /** Creates a copy of another cursor object. */
    MouseCursor (const MouseCursor& other) throw();

    /** Copies this cursor from another object. */
    const MouseCursor& operator= (const MouseCursor& other) throw();

    /** Destructor. */
    ~MouseCursor() throw();

    /** Checks whether two mouse cursors are the same.

        For custom cursors, two cursors created from the same image won't be
        recognised as the same, only MouseCursor objects that have been
        copied from the same object.
    */
    bool operator== (const MouseCursor& other) const throw();

    /** Checks whether two mouse cursors are the same.

        For custom cursors, two cursors created from the same image won't be
        recognised as the same, only MouseCursor objects that have been
        copied from the same object.
    */
    bool operator!= (const MouseCursor& other) const throw();

    //==============================================================================
    /** Makes the system show its default 'busy' cursor.

        This will turn the system cursor to an hourglass or spinning beachball
        until the next time the mouse is moved, or hideWaitCursor() is called.

        This is handy if the message loop is about to block for a couple of
        seconds while busy and you want to give the user feedback about this.

        @see MessageManager::setTimeBeforeShowingWaitCursor
    */
    static void showWaitCursor() throw();

    /** If showWaitCursor has been called, this will return the mouse to its
        normal state.

        This will look at what component is under the mouse, and update the
        cursor to be the correct one for that component.

        @see showWaitCursor
    */
    static void hideWaitCursor() throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ReferenceCountedObjectPtr <SharedMouseCursorInternal> cursorHandle;

    friend class Component;
    void showInWindow (ComponentPeer* window) const throw();
    void showInAllWindows() const throw();
    void* getHandle() const throw();
};

#endif   // __JUCE_MOUSECURSOR_JUCEHEADER__
