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

class AudioProcessor;
class AudioProcessorEditorListener;

//==============================================================================
/**
    Base class for the component that acts as the GUI for an AudioProcessor.

    Derive your editor component from this class, and create an instance of it
    by overriding the AudioProcessor::createEditor() method.

    @see AudioProcessor, GenericAudioProcessorEditor

    @tags{Audio}
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
    ~AudioProcessorEditor() override;


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

    /** Override this method to indicate if your editor supports the presence or
        absence of a host-provided MIDI controller.

        Currently only AUv3 plug-ins compiled for MacOS 10.13 or iOS 11.0 (or later)
        support this functionality, and even then the host may choose to ignore this
        information.

        The default behaviour is to report support for both cases.
    */
    virtual bool supportsHostMIDIControllerPresence (bool hostMIDIControllerIsAvailable);

    /** Called to indicate if a host is providing a MIDI controller when the host
        reconfigures its layout.

        Use this as an opportunity to hide or display your own onscreen keyboard or
        other input component.

        Currently only AUv3 plug-ins compiled for MacOS 10.13 or iOS 11.0 (or later)
        support this functionality.
    */
    virtual void hostMIDIControllerIsAvailable (bool controllerIsAvailable);

    /** Can be called by a host to tell the editor that it should use a non-unity
        GUI scale.
    */
    virtual void setScaleFactor (float newScale);

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

    std::unique_ptr<ResizableCornerComponent> resizableCorner;

private:
    //==============================================================================
    struct AudioProcessorEditorListener : ComponentListener
    {
        AudioProcessorEditorListener (AudioProcessorEditor& e) : ed (e) {}

        void componentMovedOrResized (Component&, bool, bool wasResized) override   { ed.editorResized (wasResized); }
        void componentParentHierarchyChanged (Component&) override                  { ed.updatePeer(); }

        AudioProcessorEditor& ed;

        JUCE_DECLARE_NON_COPYABLE (AudioProcessorEditorListener)
    };

    ComponentPeer* createNewPeer (int styleFlags, void*) override;

    //==============================================================================
    void initialise();
    void editorResized (bool wasResized);
    void updatePeer();
    void attachConstrainer (ComponentBoundsConstrainer*);
    void attachResizableCornerComponent();

    //==============================================================================
    std::unique_ptr<AudioProcessorEditorListener> resizeListener;
    bool resizable;
    ComponentBoundsConstrainer defaultConstrainer;
    ComponentBoundsConstrainer* constrainer = {};
    Component::SafePointer<Component> splashScreen;
    AffineTransform hostScaleTransform;

    JUCE_DECLARE_NON_COPYABLE (AudioProcessorEditor)
};

} // namespace juce
