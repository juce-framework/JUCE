/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
    simply by overriding FileDragAndDropTarget::filesDropped().

    @see DragAndDropTarget

    @tags{GUI}
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

        @param sourceDescription                 a string or value to use as the description of the thing being dragged -
                                                 this will be passed to the objects that might be dropped-onto so they can
                                                 decide whether they want to handle it
        @param sourceComponent                   the component that is being dragged
        @param dragImage                         the image to drag around underneath the mouse. If this is a null image,
                                                 a snapshot of the sourceComponent will be used instead.
        @param allowDraggingToOtherJuceWindows   if true, the dragged component will appear as a desktop
                                                 window, and can be dragged to DragAndDropTargets that are the
                                                 children of components other than this one.
        @param imageOffsetFromMouse              if an image has been passed-in, this specifies the offset
                                                 at which the image should be drawn from the mouse. If it isn't
                                                 specified, then the image will be centred around the mouse. If
                                                 an image hasn't been passed-in, this will be ignored.
        @param inputSourceCausingDrag            the mouse input source which started the drag. When calling
                                                 from within a mouseDown or mouseDrag event, you can pass
                                                 MouseEvent::source to this method. If this param is nullptr then JUCE
                                                 will use the mouse input source which is currently dragging. If there
                                                 are several dragging mouse input sources (which can often occur on mobile)
                                                 then JUCE will use the mouseInputSource which is closest to the sourceComponent.
    */
    void startDragging (const var& sourceDescription,
                        Component* sourceComponent,
                        const ScaledImage& dragImage = ScaledImage(),
                        bool allowDraggingToOtherJuceWindows = false,
                        const Point<int>* imageOffsetFromMouse = nullptr,
                        const MouseInputSource* inputSourceCausingDrag = nullptr);

    [[deprecated ("This overload does not allow the image's scale to be specified. Use the other overload of startDragging instead.")]]
    void startDragging (const var& sourceDescription,
                        Component* sourceComponent,
                        Image dragImage,
                        bool allowDraggingToOtherJuceWindows = false,
                        const Point<int>* imageOffsetFromMouse = nullptr,
                        const MouseInputSource* inputSourceCausingDrag = nullptr)
    {
        startDragging (sourceDescription,
                       sourceComponent,
                       ScaledImage (dragImage),
                       allowDraggingToOtherJuceWindows,
                       imageOffsetFromMouse,
                       inputSourceCausingDrag);
    }

    /** Returns true if something is currently being dragged. */
    bool isDragAndDropActive() const;

    /** Returns the number of things currently being dragged. */
    int getNumCurrentDrags() const;

    /** Returns the description of the thing that's currently being dragged.

        If nothing's being dragged, this will return a null var, otherwise it'll return
        the var that was passed into startDragging().

        If you are using drag and drop in a multi-touch environment then you should use the
        getDragDescriptionForIndex() method instead which takes a touch index parameter.

        @see startDragging, getDragDescriptionForIndex
    */
    var getCurrentDragDescription() const;

    /** Same as the getCurrentDragDescription() method but takes a touch index parameter.

        @see getCurrentDragDescription
    */
    var getDragDescriptionForIndex (int index) const;

    /** If a drag is in progress, this allows the image being shown to be dynamically updated.

        If you are using drag and drop in a multi-touch environment then you should use the
        setDragImageForIndex() method instead which takes a touch index parameter.

        @see setDragImageForIndex
    */
    void setCurrentDragImage (const ScaledImage& newImage);

    [[deprecated ("This overload does not allow the image's scale to be specified. Use the other overload of setCurrentDragImage instead.")]]
    void setCurrentDragImage (const Image& newImage) { setCurrentDragImage (ScaledImage (newImage)); }

    /** Same as the setCurrentDragImage() method but takes a touch index parameter.

        @see setCurrentDragImage
    */
    void setDragImageForIndex (int index, const ScaledImage& newImage);

    [[deprecated ("This overload does not allow the image's scale to be specified. Use the other overload of setDragImageForIndex instead.")]]
    void setDragImageForIndex (int index, const Image& newImage) { setDragImageForIndex (index, ScaledImage (newImage)); }

    /** Utility to find the DragAndDropContainer for a given Component.

        This will search up this component's parent hierarchy looking for the first
        parent component which is a DragAndDropContainer.

        It's useful when a component wants to call startDragging but doesn't know
        the DragAndDropContainer it should to use.

        Obviously this may return nullptr if it doesn't find a suitable component.
    */
    static DragAndDropContainer* findParentDragContainerFor (Component* childComponent);


    //==============================================================================
    /** This performs an asynchronous drag-and-drop of a set of files to some external
        application.

        You can call this function in response to a mouseDrag callback, and it will
        use a native operating system drag-and-drop operation to move or copy some
        files to another application.

        @param files            a list of filenames to drag
        @param canMoveFiles     if true, the app that receives the files is allowed to move the files to a new location
                                (if this is appropriate). If false, the receiver is expected to make a copy of them.
        @param sourceComponent  normally, JUCE will assume that the component under the mouse is the source component
                                of the drag, but you can use this parameter to override this.
        @param callback         an optional completion callback that will be called when the operation has ended.

        @returns                true if the drag operation was successfully started, or false if it failed for some reason

        @see performExternalDragDropOfText
    */
    static bool performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles,
                                                Component* sourceComponent = nullptr,
                                                std::function<void()> callback = nullptr);

    /** This performs an asynchronous drag-and-drop of a block of text to some external
        application.

        You can call this function in response to a mouseDrag callback, and it will
        use a native operating system drag-and-drop operation to move or copy some
        text to another application.

        @param text             the text to copy
        @param sourceComponent  Normally, JUCE will assume that the component under the mouse is the source component
                                of the drag, but you can use this parameter to override this.
        @param callback         an optional completion callback that will be called when the operation has ended.

        @returns                true if the drag operation was successfully started, or false if it failed for some reason

        @see performExternalDragDropOfFiles
    */
    static bool performExternalDragDropOfText (const String& text, Component* sourceComponent = nullptr,
                                               std::function<void()> callback = nullptr);

protected:
    /** Override this if you want to be able to perform an external drag of a set of files
        when the user drags outside of this container component.

        This method will be called when a drag operation moves outside the JUCE window,
        and if you want it to then perform a file drag-and-drop, add the filenames you want
        to the array passed in, and return true.

        @param sourceDetails    information about the source of the drag operation
        @param files            on return, the filenames you want to drag
        @param canMoveFiles     on return, true if it's ok for the receiver to move the files; false if
                                it must make a copy of them (see the performExternalDragDropOfFiles() method)
        @see performExternalDragDropOfFiles, shouldDropTextWhenDraggedExternally
    */
    virtual bool shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                                       StringArray& files, bool& canMoveFiles);

    /** Override this if you want to be able to perform an external drag of text
        when the user drags outside of this container component.

        This method will be called when a drag operation moves outside the JUCE window,
        and if you want it to then perform a text drag-and-drop, copy the text you want to
        be dragged into the argument provided and return true.

        @param sourceDetails    information about the source of the drag operation
        @param text             on return, the text you want to drag
        @see shouldDropFilesWhenDraggedExternally
    */
    virtual bool shouldDropTextWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                                      String& text);

    /** Subclasses can override this to be told when a drag starts. */
    virtual void dragOperationStarted (const DragAndDropTarget::SourceDetails&);

    /** Subclasses can override this to be told when a drag finishes. */
    virtual void dragOperationEnded (const DragAndDropTarget::SourceDetails&);

private:
    //==============================================================================
    class DragImageComponent;
    OwnedArray<DragImageComponent> dragImageComponents;

    const MouseInputSource* getMouseInputSourceForDrag (Component* sourceComponent, const MouseInputSource* inputSourceCausingDrag);
    bool isAlreadyDragging (Component* sourceComponent) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragAndDropContainer)
};

} // namespace juce
