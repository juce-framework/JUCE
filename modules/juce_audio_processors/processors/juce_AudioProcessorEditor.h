/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

class AudioProcessorEditorListener;
//==============================================================================
/**
    Base class for the component that acts as the GUI for an AudioProcessor.

    Derive your editor component from this class, and create an instance of it
    by overriding the AudioProcessor::createEditor() method.

    @see AudioProcessor, GenericAudioProcessorEditor
*/
class JUCE_API  AudioProcessorEditor  : public Component
{
protected:
    //==============================================================================
    /** Creates an editor for the specified processor. */
    AudioProcessorEditor (AudioProcessor&) noexcept;

    /** Creates an editor for the specified processor. */
    AudioProcessorEditor (AudioProcessor*) noexcept;

public:
    /** Destructor. */
    ~AudioProcessorEditor();


    //==============================================================================
    /** The AudioProcessor that this editor represents. */
    AudioProcessor& processor;

    /** Returns a pointer to the processor that this editor represents.
        This method is here to support legacy code, but it's easier to just use the
        AudioProcessorEditor::processor member variable directly to get this object.
    */
    AudioProcessor* getAudioProcessor() const noexcept        { return &processor; }

    //==============================================================================
    /** Used by the setParameterHighlighting() method. */
    struct ParameterControlHighlightInfo
    {
        int parameterIndex;
        bool isHighlighted;
        Colour suggestedColour;
    };

    /** Some types of plugin can call this to suggest that the control for a particular
        parameter should be highlighted.
        Currently only AAX plugins will call this, and implementing it is optional.
    */
    virtual void setControlHighlight (ParameterControlHighlightInfo);

    /** Called by certain plug-in wrappers to find out whether a component is used
        to control a parameter.

        If the given component represents a particular plugin parameter, then this
        method should return the index of that parameter. If not, it should return -1.
        Currently only AAX plugins will call this, and implementing it is optional.
    */
    virtual int getControlParameterIndex (Component&);

    //==============================================================================
    /** Marks the host's editor window as resizable

        @param allowHostToResize   whether the editor's parent window can be resized
                                   by the user or the host. Even if this is false, you
                                   can still resize your window yourself by calling
                                   setBounds (for example, when a user clicks on a button
                                   in your editor to drop out a panel) which will bypass any
                                   resizable/constraints checks. If you are using
                                   your own corner resizer than this will also bypass
                                   any checks.
        @param useBottomRightCornerResizer
        @see setResizeLimits, isResizable
    */
    void setResizable (bool allowHostToResize, bool useBottomRightCornerResizer);

    /** Returns true if the host is allowed to resize editor's parent window

        @see setResizable
    */
    bool isResizable() const noexcept      { return resizable; }

    /** This sets the maximum and minimum sizes for the window.

        If the window's current size is outside these limits, it will be resized to
        make sure it's within them.

        A direct call to setBounds() will bypass any constraint checks, but when the
        window is dragged by the user or resized by other indirect means, the constrainer
        will limit the numbers involved.

        @see setResizable
    */
    void setResizeLimits (int newMinimumWidth,
                          int newMinimumHeight,
                          int newMaximumWidth,
                          int newMaximumHeight) noexcept;


    /** Returns the bounds constrainer object that this window is using.
        You can access this to change its properties.
    */
    ComponentBoundsConstrainer* getConstrainer() noexcept           { return constrainer; }

    /** Sets the bounds-constrainer object to use for resizing and dragging this window.

        A pointer to the object you pass in will be kept, but it won't be deleted
        by this object, so it's the caller's responsibility to manage it.

        If you pass a nullptr, then no contraints will be placed on the positioning of the window.
    */
    void setConstrainer (ComponentBoundsConstrainer* newConstrainer);

    /** Calls the window's setBounds method, after first checking these bounds
        with the current constrainer.

        @see setConstrainer
     */
    void setBoundsConstrained (Rectangle<int> newBounds);

    ScopedPointer<ResizableCornerComponent> resizableCorner;

private:
    //==============================================================================
    struct AudioProcessorEditorListener : ComponentListener
    {
        AudioProcessorEditorListener (AudioProcessorEditor* audioEditor) : e (audioEditor) {}

        void componentMovedOrResized (Component&, bool, bool wasResized) override   { e->editorResized (wasResized); }
        void componentParentHierarchyChanged (Component&) override                  { e->updatePeer(); }
        AudioProcessorEditor* e;
    };

    //==============================================================================
    void initialise();
    void editorResized (bool wasResized);
    void updatePeer();
    void attachConstrainer (ComponentBoundsConstrainer* newConstrainer);

    //==============================================================================
    ScopedPointer<AudioProcessorEditorListener> resizeListener;
    bool resizable;
    ComponentBoundsConstrainer defaultConstrainer;
    ComponentBoundsConstrainer* constrainer;
    Component::SafePointer<Component> splashScreen;

    JUCE_DECLARE_NON_COPYABLE (AudioProcessorEditor)
};
