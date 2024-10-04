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

/**
    Components derived from this class can have files dropped onto them by an external application.

    @see DragAndDropContainer

    @tags{GUI}
*/
class JUCE_API  FileDragAndDropTarget
{
public:
    /** Destructor. */
    virtual ~FileDragAndDropTarget() = default;

    /** Callback to check whether this target is interested in the set of files being offered.

        Note that this will be called repeatedly when the user is dragging the mouse around over your
        component, so don't do anything time-consuming in here, like opening the files to have a look
        inside them!

        @param files        the set of (absolute) pathnames of the files that the user is dragging
        @returns            true if this component wants to receive the other callbacks regarding this
                            type of object; if it returns false, no other callbacks will be made.
    */
    virtual bool isInterestedInFileDrag (const StringArray& files) = 0;

    /** Callback to indicate that some files are being dragged over this component.

        This gets called when the user moves the mouse into this component while dragging.

        Use this callback as a trigger to make your component repaint itself to give the
        user feedback about whether the files can be dropped here or not.

        @param files        the set of (absolute) pathnames of the files that the user is dragging
        @param x            the mouse x position, relative to this component
        @param y            the mouse y position, relative to this component
    */
    virtual void fileDragEnter (const StringArray& files, int x, int y);

    /** Callback to indicate that the user is dragging some files over this component.

        This gets called when the user moves the mouse over this component while dragging.
        Normally overriding itemDragEnter() and itemDragExit() are enough, but
        this lets you know what happens in-between.

        @param files        the set of (absolute) pathnames of the files that the user is dragging
        @param x            the mouse x position, relative to this component
        @param y            the mouse y position, relative to this component
    */
    virtual void fileDragMove (const StringArray& files, int x, int y);

    /** Callback to indicate that the mouse has moved away from this component.

        This gets called when the user moves the mouse out of this component while dragging
        the files.

        If you've used fileDragEnter() to repaint your component and give feedback, use this
        as a signal to repaint it in its normal state.

        @param files        the set of (absolute) pathnames of the files that the user is dragging
    */
    virtual void fileDragExit (const StringArray& files);

    /** Callback to indicate that the user has dropped the files onto this component.

        When the user drops the files, this get called, and you can use the files in whatever
        way is appropriate.

        Note that after this is called, the fileDragExit method may not be called, so you should
        clean up in here if there's anything you need to do when the drag finishes.

        @param files        the set of (absolute) pathnames of the files that the user is dragging
        @param x            the mouse x position, relative to this component
        @param y            the mouse y position, relative to this component
    */
    virtual void filesDropped (const StringArray& files, int x, int y) = 0;
};

} // namespace juce
