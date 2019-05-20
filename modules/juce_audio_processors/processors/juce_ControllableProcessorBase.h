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

namespace juce
{

class AudioProcessorParameter;
class AudioProcessorParameterGroup;
class AudioProcessorListener;
class AudioProcessorEditor;

//==============================================================================
/** An abstract base class for processors, so an AudioProcessorValueTreeState can
    be used to controll parameters.

    @tags{Audio}
*/
class JUCE_API  ControllableProcessorBase
{
public:
    ControllableProcessorBase();

    /** Destructor. */
    virtual ~ControllableProcessorBase();

    //==============================================================================
    /** Returns the name of this processor. */
    virtual const String getName() const = 0;

    /** Returns a list of alternative names to use for this processor.

        Some hosts truncate the name of your AudioProcessor when there isn't enough
        space in the GUI to show the full name. Overriding this method, allows the host
        to choose an alternative name (such as an abbreviation) to better fit the
        available space.
     */
    virtual StringArray getAlternateDisplayNames() const;

    //==============================================================================
    /** Adds a parameter to the AudioProcessor.

        The parameter object will be managed and deleted automatically by the
        AudioProcessor when no longer needed.
     */
    virtual void addParameter (AudioProcessorParameter*);

    /** Adds a group of parameters to the AudioProcessor.

        All the parameter objects contained within the group will be managed and
        deleted automatically by the AudioProcessor when no longer needed.

     @see addParameter
     */
    virtual void addParameterGroup (std::unique_ptr<AudioProcessorParameterGroup>);

    /** Returns the group of parameters managed by this AudioProcessor. */
    virtual const AudioProcessorParameterGroup& getParameterTree();

    /** Returns the current list of parameters. */
    virtual const OwnedArray<AudioProcessorParameter>& getParameters() const noexcept;

    //==============================================================================

    /** The processor can call this when something (apart from a parameter value) has changed.

        It sends a hint to the host that something like the program, number of parameters,
        etc, has changed, and that it should update itself.
     */
    void updateHostDisplay();

    /** Your processor can call this when it needs to change one of its parameters.

        This could happen when the editor or some other internal operation changes
        a parameter. This method will call the setParameter() method to change the
        value, and will then send a message to the host telling it about the change.

        Note that to make sure the host correctly handles automation, you should call
        the beginParameterChangeGesture() and endParameterChangeGesture() methods to
        tell the host when the user has started and stopped changing the parameter.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::setValueNotifyingHost() instead.
     */
    void setParameterNotifyingHost (int parameterIndex, float newValue);

    //==============================================================================
    /** Creates the processor's GUI.

        This can return nullptr if you want a GUI-less processor, in which case the host
        may create a generic UI that lets the user twiddle the parameters directly.

        If you do want to pass back a component, the component should be created and set to
        the correct size before returning it. If you implement this method, you must
        also implement the hasEditor() method and make it return true.

        Remember not to do anything silly like allowing your processor to keep a pointer to
        the component that gets created - it could be deleted later without any warning, which
        would make your pointer into a dangler. Use the getActiveEditor() method instead.

        The correct way to handle the connection between an editor component and its
        processor is to use something like a ChangeBroadcaster so that the editor can
        register itself as a listener, and be told when a change occurs. This lets them
        safely unregister themselves when they are deleted.

        Here are a few things to bear in mind when writing an editor:

        - Initially there won't be an editor, until the user opens one, or they might
          not open one at all. Your processor mustn't rely on it being there.
        - An editor object may be deleted and a replacement one created again at any time.
        - It's safe to assume that an editor will be deleted before its processor.

        @see hasEditor
     */
    virtual AudioProcessorEditor* createEditor() = 0;

    /** Your processor subclass must override this and return true if it can create an
        editor component.
        @see createEditor
     */
    virtual bool hasEditor() const = 0;

    /** Not for public use - this is called before deleting an editor component. */
    void editorBeingDeleted (AudioProcessorEditor*) noexcept;

    //==============================================================================
    /** Returns the active editor, if there is one.
        Bear in mind this can return nullptr, even if an editor has previously been opened.
     */
    AudioProcessorEditor* getActiveEditor() const noexcept           { return activeEditor; }

    /** Returns the active editor, or if there isn't one, it will create one.
        This may call createEditor() internally to create the component.
     */
    AudioProcessorEditor* createEditorIfNeeded();

    /** Returns a lock, in case the editor creation needs to be synchronised with the
        processing. The AudioProcessor will override this to use it's own callbackLock.
     */
    virtual const CriticalSection& getCallbackLock() const           { return callbackLock; }

    //==============================================================================

    /** This must return the correct value immediately after the object has been
        created, and mustn't change the number of parameters later.

        NOTE! This method is deprecated! It's recommended that you use the
        AudioProcessorParameter class instead to manage your parameters.
     */
    JUCE_DEPRECATED (virtual int getNumParameters());

    /** Returns the name of a particular parameter.

        NOTE! This method is deprecated! It's recommended that you use the
        AudioProcessorParameter class instead to manage your parameters.
     */
    JUCE_DEPRECATED (virtual const String getParameterName (int parameterIndex));

    /** Returns the ID of a particular parameter.

        The ID is used to communicate the value or mapping of a particular parameter with
        the host. By default this method will simply return a string representation of
        index.

        NOTE! This method is deprecated! It's recommended that you use the
        AudioProcessorParameterWithID class instead to manage your parameters.
     */
    JUCE_DEPRECATED (virtual String getParameterID (int index));

    /** Called by the host to find out the value of one of the processor's parameters.

        The host will expect the value returned to be between 0 and 1.0.

        This could be called quite frequently, so try to make your code efficient.
        It's also likely to be called by non-UI threads, so the code in here should
        be thread-aware.

        NOTE! This method is deprecated! It's recommended that you use the
        AudioProcessorParameter class instead to manage your parameters.
     */
    JUCE_DEPRECATED (virtual float getParameter (int parameterIndex));

    /** Returns the name of a parameter as a text string with a preferred maximum length.
        If you want to provide customised short versions of your parameter names that
        will look better in constrained spaces (e.g. the displays on hardware controller
        devices or mixing desks) then you should implement this method.
        If you don't override it, the default implementation will call getParameterName(int),
        and truncate the result.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getName() instead.
     */
    JUCE_DEPRECATED (virtual String getParameterName (int parameterIndex, int maximumStringLength));

    /** Returns the value of a parameter as a text string.
        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getText() instead.
     */
    JUCE_DEPRECATED (virtual const String getParameterText (int parameterIndex));

    /** Returns the value of a parameter as a text string with a preferred maximum length.
        If you want to provide customised short versions of your parameter values that
        will look better in constrained spaces (e.g. the displays on hardware controller
        devices or mixing desks) then you should implement this method.
        If you don't override it, the default implementation will call getParameterText(int),
        and truncate the result.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getText() instead.
     */
    JUCE_DEPRECATED (virtual String getParameterText (int parameterIndex, int maximumStringLength));

    /** Returns the number of discrete steps that this parameter can represent.

        The default return value if you don't implement this method is
        AudioProcessor::getDefaultNumParameterSteps().

        If your parameter is boolean, then you may want to make this return 2.

        If you want the host to display stepped automation values, rather than a
        continuous interpolation between successive values, you should ensure that
        isParameterDiscrete returns true.

        The value that is returned may or may not be used, depending on the host.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getNumSteps() instead.

        @see isParameterDiscrete
     */
    JUCE_DEPRECATED (virtual int getParameterNumSteps (int parameterIndex));

    /** Returns the default number of steps for a parameter.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getNumSteps() instead.

        @see getParameterNumSteps
     */
    static int getDefaultNumParameterSteps() noexcept;

    /** Returns true if the parameter should take discrete, rather than continuous
        values.

        If the parameter is boolean, this should return true (with getParameterNumSteps
        returning 2).

        The value that is returned may or may not be used, depending on the host.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::isDiscrete() instead.

        @see getParameterNumSteps
     */
    JUCE_DEPRECATED (virtual bool isParameterDiscrete (int parameterIndex) const);

    /** Returns the default value for the parameter.
        By default, this just returns 0.
        The value that is returned may or may not be used, depending on the host.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getDefaultValue() instead.
     */
    JUCE_DEPRECATED (virtual float getParameterDefaultValue (int parameterIndex));

    /** Some plugin types may be able to return a label string for a
        parameter's units.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getLabel() instead.
     */
    JUCE_DEPRECATED (virtual String getParameterLabel (int index) const);

    /** This can be overridden to tell the host that particular parameters operate in the
        reverse direction. (Not all plugin formats or hosts will actually use this information).

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::isOrientationInverted() instead.
     */
    JUCE_DEPRECATED (virtual bool isParameterOrientationInverted (int index) const);

    /** The host will call this method to change the value of one of the processor's parameters.

        The host may call this at any time, including during the audio processing
        callback, so the processor has to process this very fast and avoid blocking.

        If you want to set the value of a parameter internally, e.g. from your
        editor component, then don't call this directly - instead, use the
        setParameterNotifyingHost() method, which will also send a message to
        the host telling it about the change. If the message isn't sent, the host
        won't be able to automate your parameters properly.

        The value passed will be between 0 and 1.0.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::setValue() instead.
     */
    JUCE_DEPRECATED (virtual void setParameter (int parameterIndex, float newValue));

    /** Returns true if the host can automate this parameter.
        By default, this returns true for all parameters.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::isAutomatable() instead.
     */
    JUCE_DEPRECATED (virtual bool isParameterAutomatable (int parameterIndex) const);

    /** Should return true if this parameter is a "meta" parameter.
        A meta-parameter is a parameter that changes other params. It is used
        by some hosts (e.g. AudioUnit hosts).
        By default this returns false.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::isMetaParameter() instead.
     */
    JUCE_DEPRECATED (virtual bool isMetaParameter (int parameterIndex) const);

    /** Should return the parameter's category.
        By default, this returns the "generic" category.

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::getCategory() instead.
     */
    JUCE_DEPRECATED (virtual AudioProcessorParameter::Category getParameterCategory (int parameterIndex) const);

    /** Sends a signal to the host to tell it that the user is about to start changing this
        parameter.

        This allows the host to know when a parameter is actively being held by the user, and
        it may use this information to help it record automation.

        If you call this, it must be matched by a later call to endParameterChangeGesture().

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::beginChangeGesture() instead.
     */
    JUCE_DEPRECATED (void beginParameterChangeGesture (int parameterIndex));

    /** Tells the host that the user has finished changing this parameter.

        This allows the host to know when a parameter is actively being held by the user, and
        it may use this information to help it record automation.

        A call to this method must follow a call to beginParameterChangeGesture().

        NOTE! This method is deprecated! It's recommended that you use
        AudioProcessorParameter::endChangeGesture() instead.
     */
    JUCE_DEPRECATED (void endParameterChangeGesture (int parameterIndex));

    /** @internal */
    virtual void sendParamChangeMessageToListeners (int parameterIndex, float newValue) {}

    /** @internal */
    virtual void sendParamChangeGestureBeginToListeners (int parameterIndex) {}

    /** @internal */
    virtual void sendParamChangeGestureEndToListeners (int parameterIndex) {}

private:
    //==============================================================================
    void addParameterInternal (AudioProcessorParameter*);

    AudioProcessorParameter* getParamChecked (int) const noexcept;


#if JUCE_DEBUG
    bool textRecursionCheck = false;
    bool shouldCheckParamsForDupeIDs = false;
    void checkForDupedParamIDs();
#endif

    AudioProcessorParameterGroup parameterTree { {}, {}, {} };

    OwnedArray<AudioProcessorParameter> managedParameters;

#if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
    BigInteger changingParams;
#endif

    CriticalSection callbackLock;

    Component::SafePointer<AudioProcessorEditor> activeEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControllableProcessorBase)
};

} // namespace juce
