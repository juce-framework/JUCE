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

class AudioProcessorEditorListener;
class AudioProcessor;

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
        If not overridden, this will return -1 for all components.

        This function will be called by the host in AAX and VST3 plug-ins in order to map
        screen locations to parameters. For example, in Steinberg hosts, this enables the
        "AI Knob" functionality, which enables hardware to control the parameter currently
        under the mouse.
    */
    virtual int getControlParameterIndex (Component&);

    /** Override this method to indicate if your editor supports the presence or
        absence of a host-provided MIDI controller.

        Currently only AUv3 plug-ins support this functionality, and even then the
        host may choose to ignore this information.

        The default behaviour is to report support for both cases.
    */
    virtual bool supportsHostMIDIControllerPresence (bool hostMIDIControllerIsAvailable);

    /** Called to indicate if a host is providing a MIDI controller when the host
        reconfigures its layout.

        Use this as an opportunity to hide or display your own onscreen keyboard or
        other input component.

        Currently only AUv3 plug-ins support this functionality.
    */
    virtual void hostMIDIControllerIsAvailable (bool controllerIsAvailable);

    /** Can be called by a host to tell the editor that it should use a non-unity
        GUI scale.
    */
    virtual void setScaleFactor (float newScale);

    //==============================================================================
    /** Sets whether the editor is resizable by the host and/or user.

        @param allowHostToResize            whether the editor's parent window can be resized
                                            by the host. Even if this is false, you can still
                                            resize your window yourself by calling setBounds
                                            (for example, when a user clicks on a button in
                                            your editor to drop out a panel) which will bypass
                                            any resizable/constraints checks.
        @param useBottomRightCornerResizer  if this is true, a ResizableCornerComponent will be
                                            added to the editor's bottom-right to allow the user
                                            to resize the editor regardless of the value of
                                            `allowHostToResize`.

        @see setResizeLimits, isResizable
    */
    void setResizable (bool allowHostToResize, bool useBottomRightCornerResizer);

    /** Returns true if the host is allowed to resize the editor's parent window.

        @see setResizable
    */
    bool isResizable() const noexcept      { return resizableByHost; }

    /** This sets the maximum and minimum sizes for the window.

        If the window's current size is outside these limits, it will be resized to
        make sure it's within them.

        If you pass in a different minimum and maximum size, this will mark the editor
        as resizable by the host.

        A direct call to setBounds() will bypass any constraint checks, but when the
        window is dragged by the user or resized by other indirect means, the constrainer
        will limit the numbers involved.

        Note that if you have set a custom constrainer for this editor then this will have
        no effect, and if you have removed the constrainer with `setConstrainer (nullptr);`
        then this will re-add the default constrainer with the new limits.

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

        If you pass a nullptr, then no constraints will be placed on the positioning of the window.
    */
    void setConstrainer (ComponentBoundsConstrainer* newConstrainer);

    /** Calls the window's setBounds method, after first checking these bounds
        with the current constrainer.

        @see setConstrainer
     */
    void setBoundsConstrained (Rectangle<int> newBounds);

    /** Gets a context object, if one is available.

        Returns nullptr if the host does not provide any information that the editor
        can query.

        The returned pointer is non-owning, so do not attempt to free it.
    */
    AudioProcessorEditorHostContext* getHostContext() const noexcept          { return hostContext; }

    /** Sets a context object that can be queried to find information that the host
        makes available to the plugin.

        You will only need to call this function if you are implementing a plugin host.
    */
    void setHostContext (AudioProcessorEditorHostContext* context) noexcept   { hostContext = context; }

    /** The ResizableCornerComponent which is currently being used by this editor,
        or nullptr if it does not have one.
    */
    std::unique_ptr<ResizableCornerComponent> resizableCorner;

    /** The plugin wrapper will call this function to decide whether to use a layer-backed view to
        host the editor on macOS and iOS.

        Layer-backed views generally provide better performance, and are recommended in most
        situations. However, on older macOS versions (confirmed on 10.12 and 10.13), displaying an
        OpenGL context inside a layer-backed view can lead to deadlocks, so it is recommended to
        avoid layer-backed views when using OpenGL on these OS versions.

        The default behaviour of this function is to return false if and only if the juce_opengl
        module is present and the current platform is macOS 10.13 or earlier.

        You may want to override this behaviour if your plugin has an option to enable and disable
        OpenGL rendering. If you know your plugin editor will never use OpenGL rendering, you can
        set this function to return true in all situations.
    */
    virtual bool wantsLayerBackedView() const;

private:
    //==============================================================================
    struct AudioProcessorEditorListener : public ComponentListener
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
    bool resizableByHost = false;
    ComponentBoundsConstrainer defaultConstrainer;
    ComponentBoundsConstrainer* constrainer = nullptr;
    AudioProcessorEditorHostContext* hostContext = nullptr;
    AffineTransform hostScaleTransform;

    JUCE_DECLARE_NON_COPYABLE (AudioProcessorEditor)
};

} // namespace juce
