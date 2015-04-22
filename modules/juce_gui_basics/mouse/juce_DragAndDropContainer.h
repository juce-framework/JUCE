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

#ifndef JUCE_DRAGANDDROPCONTAINER_H_INCLUDED
#define JUCE_DRAGANDDROPCONTAINER_H_INCLUDED


//==============================================================================
/**
    Enables drag-and-drop behaviour for a component and all its sub-components.

    For a component to be able to make or receive drag-and-drop events, one of its parent
    components must derive from this class. It's probably best for the top-level
    component to implement it.

    Then to start a drag operation, any sub-component can just call the startDragging()
    method, and this object will take over, tracking the mouse and sending appropriate
    callbacks to any child components derived from DragAndDropTarget which the mouse
    moves over.

    Note: If all that you need to do is to respond to files being drag-and-dropped from
    the operating system onto your component, you don't need any of these classes: you can do this
    simply by overriding Component::filesDropped().

    @see DragAndDropTarget
*/
class JUCE_API  DragAndDropContainer
{
public:
    //==============================================================================
    /** Creates a DragAndDropContainer.

        The object that derives from this class must also be a Component.
    */
    DragAndDropContainer();

    /** Destructor. */
    virtual ~DragAndDropContainer();

    //==============================================================================
    /** Begins a drag-and-drop operation.

        This starts a drag-and-drop operation - call it when the user drags the
        mouse in your drag-source component, and this object will track mouse
        movements until the user lets go of the mouse button, and will send
        appropriate messages to DragAndDropTarget objects that the mouse moves
        over.

        findParentDragContainerFor() is a handy method to call to find the
        drag container to use for a component.

        @param sourceDescription    a string or value to use as the description of the thing being dragged -
                                    this will be passed to the objects that might be dropped-onto so they can
                                    decide whether they want to handle it
        @param sourceComponent      the component that is being dragged
        @param dragImage            the image to drag around underneath the mouse. If this is a null image,
                                    a snapshot of the sourceComponent will be used instead.
        @param allowDraggingToOtherJuceWindows   if true, the dragged component will appear as a desktop
                                    window, and can be dragged to DragAndDropTargets that are the
                                    children of components other than this one.
        @param imageOffsetFromMouse if an image has been passed-in, this specifies the offset
                                    at which the image should be drawn from the mouse. If it isn't
                                    specified, then the image will be centred around the mouse. If
                                    an image hasn't been passed-in, this will be ignored.
    */
    void startDragging (const var& sourceDescription,
                        Component* sourceComponent,
                        Image dragImage = Image::null,
                        bool allowDraggingToOtherJuceWindows = false,
                        const Point<int>* imageOffsetFromMouse = nullptr);

    /** Returns true if something is currently being dragged. */
    bool isDragAndDropActive() const;

    /** Returns the description of the thing that's currently being dragged.

        If nothing's being dragged, this will return a null var, otherwise it'll return
        the var that was passed into startDragging().

        @see startDragging
    */
    var getCurrentDragDescription() const;

    /** Utility to find the DragAndDropContainer for a given Component.

        This will search up this component's parent hierarchy looking for the first
        parent component which is a DragAndDropContainer.

        It's useful when a component wants to call startDragging but doesn't know
        the DragAndDropContainer it should to use.

        Obviously this may return nullptr if it doesn't find a suitable component.
    */
    static DragAndDropContainer* findParentDragContainerFor (Component* childComponent);


    //==============================================================================
    /** This performs a synchronous drag-and-drop of a set of files to some external
        application.

        You can call this function in response to a mouseDrag callback, and it will
        block, running its own internal message loop and tracking the mouse, while it
        uses a native operating system drag-and-drop operation to move or copy some
        files to another application.

        @param files            a list of filenames to drag
        @param canMoveFiles     if true, the app that receives the files is allowed to move the files to a new location
                                (if this is appropriate). If false, the receiver is expected to make a copy of them.
        @returns        true if the files were successfully dropped somewhere, or false if it
                        was interrupted
        @see performExternalDragDropOfText
    */
    static bool performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles);

    /** This performs a synchronous drag-and-drop of a block of text to some external
        application.

        You can call this function in response to a mouseDrag callback, and it will
        block, running its own internal message loop and tracking the mouse, while it
        uses a native operating system drag-and-drop operation to move or copy some
        text to another application.

        @param text     the text to copy
        @returns        true if the text was successfully dropped somewhere, or false if it
                        was interrupted
        @see performExternalDragDropOfFiles
    */
    static bool performExternalDragDropOfText (const String& text);

protected:
    /** Override this if you want to be able to perform an external drag a set of files
        when the user drags outside of this container component.

        This method will be called when a drag operation moves outside the Juce-based window,
        and if you want it to then perform a file drag-and-drop, add the filenames you want
        to the array passed in, and return true.

        @param sourceDetails    information about the source of the drag operation
        @param files            on return, the filenames you want to drag
        @param canMoveFiles     on return, true if it's ok for the receiver to move the files; false if
                                it must make a copy of them (see the performExternalDragDropOfFiles() method)
        @see performExternalDragDropOfFiles
    */
    virtual bool shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                                       StringArray& files, bool& canMoveFiles);

    /** Subclasses can override this to be told when a drag starts. */
    virtual void dragOperationStarted();

    /** Subclasses can override this to be told when a drag finishes. */
    virtual void dragOperationEnded();

private:
    //==============================================================================
    class DragImageComponent;
    friend class DragImageComponent;
    friend struct ContainerDeletePolicy<DragImageComponent>;
    ScopedPointer<DragImageComponent> dragImageComponent;

    JUCE_DEPRECATED (virtual bool shouldDropFilesWhenDraggedExternally (const String&, Component*, StringArray&, bool&)) { return false; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragAndDropContainer)
};


#endif   // JUCE_DRAGANDDROPCONTAINER_H_INCLUDED
