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

#ifndef JUCE_AUDIOPROCESSOR_H_INCLUDED
#define JUCE_AUDIOPROCESSOR_H_INCLUDED


//==============================================================================
/**
    Base class for audio processing filters or plugins.

    This is intended to act as a base class of audio filter that is general enough to
    be wrapped as a VST, AU, RTAS, etc, or used internally.

    It is also used by the plugin hosting code as the wrapper around an instance
    of a loaded plugin.

    Derive your filter class from this base class, and if you're building a plugin,
    you should implement a global function called createPluginFilter() which creates
    and returns a new instance of your subclass.
*/
class JUCE_API  AudioProcessor
{
protected:
    //==============================================================================
    /** Constructor. */
    AudioProcessor();

public:
    /** Destructor. */
    virtual ~AudioProcessor();

    //==============================================================================
    /** Returns the name of this processor. */
    virtual const String getName() const = 0;

    //==============================================================================
    /** Called before playback starts, to let the filter prepare itself.

        The sample rate is the target sample rate, and will remain constant until
        playback stops.

        The estimatedSamplesPerBlock value is a HINT about the typical number of
        samples that will be processed for each callback, but isn't any kind
        of guarantee. The actual block sizes that the host uses may be different
        each time the callback happens, and may be more or less than this value.
    */
    virtual void prepareToPlay (double sampleRate,
                                int estimatedSamplesPerBlock) = 0;

    /** Called after playback has stopped, to let the filter free up any resources it
        no longer needs.
    */
    virtual void releaseResources() = 0;

    /** Renders the next block.

        When this method is called, the buffer contains a number of channels which is
        at least as great as the maximum number of input and output channels that
        this filter is using. It will be filled with the filter's input data and
        should be replaced with the filter's output.

        So for example if your filter has 2 input channels and 4 output channels, then
        the buffer will contain 4 channels, the first two being filled with the
        input data. Your filter should read these, do its processing, and replace
        the contents of all 4 channels with its output.

        Or if your filter has 5 inputs and 2 outputs, the buffer will have 5 channels,
        all filled with data, and your filter should overwrite the first 2 of these
        with its output. But be VERY careful not to write anything to the last 3
        channels, as these might be mapped to memory that the host assumes is read-only!

        Note that if you have more outputs than inputs, then only those channels that
        correspond to an input channel are guaranteed to contain sensible data - e.g.
        in the case of 2 inputs and 4 outputs, the first two channels contain the input,
        but the last two channels may contain garbage, so you should be careful not to
        let this pass through without being overwritten or cleared.

        Also note that the buffer may have more channels than are strictly necessary,
        but you should only read/write from the ones that your filter is supposed to
        be using.

        The number of samples in these buffers is NOT guaranteed to be the same for every
        callback, and may be more or less than the estimated value given to prepareToPlay().
        Your code must be able to cope with variable-sized blocks, or you're going to get
        clicks and crashes!

        If the filter is receiving a midi input, then the midiMessages array will be filled
        with the midi messages for this block. Each message's timestamp will indicate the
        message's time, as a number of samples from the start of the block.

        Any messages left in the midi buffer when this method has finished are assumed to
        be the filter's midi output. This means that your filter should be careful to
        clear any incoming messages from the array if it doesn't want them to be passed-on.

        Be very careful about what you do in this callback - it's going to be called by
        the audio thread, so any kind of interaction with the UI is absolutely
        out of the question. If you change a parameter in here and need to tell your UI to
        update itself, the best way is probably to inherit from a ChangeBroadcaster, let
        the UI components register as listeners, and then call sendChangeMessage() inside the
        processBlock() method to send out an asynchronous message. You could also use
        the AsyncUpdater class in a similar way.
    */
    virtual void processBlock (AudioSampleBuffer& buffer,
                               MidiBuffer& midiMessages) = 0;

    /** Renders the next block when the processor is being bypassed.
        The default implementation of this method will pass-through any incoming audio, but
        you may override this method e.g. to add latency compensation to the data to match
        the processor's latency characteristics. This will avoid situations where bypassing
        will shift the signal forward in time, possibly creating pre-echo effects and odd timings.
        Another use for this method would be to cross-fade or morph between the wet (not bypassed)
        and dry (bypassed) signals.
    */
    virtual void processBlockBypassed (AudioSampleBuffer& buffer,
                                       MidiBuffer& midiMessages);

    //==============================================================================
    /** Returns the current AudioPlayHead object that should be used to find
        out the state and position of the playhead.

        You can call this from your processBlock() method, and use the AudioPlayHead
        object to get the details about the time of the start of the block currently
        being processed.

        If the host hasn't supplied a playhead object, this will return nullptr.
    */
    AudioPlayHead* getPlayHead() const noexcept                 { return playHead; }


    //==============================================================================
    /** Returns the current sample rate.

        This can be called from your processBlock() method - it's not guaranteed
        to be valid at any other time, and may return 0 if it's unknown.
    */
    double getSampleRate() const noexcept                       { return sampleRate; }

    /** Returns the current typical block size that is being used.

        This can be called from your processBlock() method - it's not guaranteed
        to be valid at any other time.

        Remember it's not the ONLY block size that may be used when calling
        processBlock, it's just the normal one. The actual block sizes used may be
        larger or smaller than this, and will vary between successive calls.
    */
    int getBlockSize() const noexcept                           { return blockSize; }

    //==============================================================================
    /** Returns the number of input channels that the host will be sending the filter.

        If writing a plugin, your configuration macros should specify the number of
        channels that your filter would prefer to have, and this method lets
        you know how many the host is actually using.

        Note that this method is only valid during or after the prepareToPlay()
        method call. Until that point, the number of channels will be unknown.
    */
    int getNumInputChannels() const noexcept                    { return numInputChannels; }

    /** Returns the number of output channels that the host will be sending the filter.

        If writing a plugin, your configuration macros should specify the number of
        channels that your filter would prefer to have, and this method lets
        you know how many the host is actually using.

        Note that this method is only valid during or after the prepareToPlay()
        method call. Until that point, the number of channels will be unknown.
    */
    int getNumOutputChannels() const noexcept                   { return numOutputChannels; }

    /** Returns a string containing a whitespace-separated list of speaker types
        corresponding to each input channel.
        For example in a 5.1 arrangement, the string may be "L R C Lfe Ls Rs"
        If the speaker arrangement is unknown, the returned string will be empty.
    */
    const String& getInputSpeakerArrangement() const noexcept   { return inputSpeakerArrangement; }

    /** Returns a string containing a whitespace-separated list of speaker types
        corresponding to each output channel.
        For example in a 5.1 arrangement, the string may be "L R C Lfe Ls Rs"
        If the speaker arrangement is unknown, the returned string will be empty.
    */
    const String& getOutputSpeakerArrangement() const noexcept  { return outputSpeakerArrangement; }

    //==============================================================================
    /** Returns the name of one of the processor's input channels.

        The processor might not supply very useful names for channels, and this might be
        something like "1", "2", "left", "right", etc.
    */
    virtual const String getInputChannelName (int channelIndex) const = 0;

    /** Returns the name of one of the processor's output channels.

        The processor might not supply very useful names for channels, and this might be
        something like "1", "2", "left", "right", etc.
    */
    virtual const String getOutputChannelName (int channelIndex) const = 0;

    /** Returns true if the specified channel is part of a stereo pair with its neighbour. */
    virtual bool isInputChannelStereoPair (int index) const = 0;

    /** Returns true if the specified channel is part of a stereo pair with its neighbour. */
    virtual bool isOutputChannelStereoPair (int index) const = 0;

    /** This returns the number of samples delay that the filter imposes on the audio
        passing through it.

        The host will call this to find the latency - the filter itself should set this value
        by calling setLatencySamples() as soon as it can during its initialisation.
    */
    int getLatencySamples() const noexcept                      { return latencySamples; }

    /** The filter should call this to set the number of samples delay that it introduces.

        The filter should call this as soon as it can during initialisation, and can call it
        later if the value changes.
    */
    void setLatencySamples (int newLatency);

    /** Returns true if a silent input always produces a silent output. */
    virtual bool silenceInProducesSilenceOut() const = 0;

    /** Returns the length of the filter's tail, in seconds. */
    virtual double getTailLengthSeconds() const = 0;

    /** Returns true if the processor wants midi messages. */
    virtual bool acceptsMidi() const = 0;

    /** Returns true if the processor produces midi messages. */
    virtual bool producesMidi() const = 0;

    //==============================================================================
    /** This returns a critical section that will automatically be locked while the host
        is calling the processBlock() method.

        Use it from your UI or other threads to lock access to variables that are used
        by the process callback, but obviously be careful not to keep it locked for
        too long, because that could cause stuttering playback. If you need to do something
        that'll take a long time and need the processing to stop while it happens, use the
        suspendProcessing() method instead.

        @see suspendProcessing
    */
    const CriticalSection& getCallbackLock() const noexcept     { return callbackLock; }

    /** Enables and disables the processing callback.

        If you need to do something time-consuming on a thread and would like to make sure
        the audio processing callback doesn't happen until you've finished, use this
        to disable the callback and re-enable it again afterwards.

        E.g.
        @code
        void loadNewPatch()
        {
            suspendProcessing (true);

            ..do something that takes ages..

            suspendProcessing (false);
        }
        @endcode

        If the host tries to make an audio callback while processing is suspended, the
        filter will return an empty buffer, but won't block the audio thread like it would
        do if you use the getCallbackLock() critical section to synchronise access.

        Any code that calls processBlock() should call isSuspended() before doing so, and
        if the processor is suspended, it should avoid the call and emit silence or
        whatever is appropriate.

        @see getCallbackLock
    */
    void suspendProcessing (bool shouldBeSuspended);

    /** Returns true if processing is currently suspended.
        @see suspendProcessing
    */
    bool isSuspended() const noexcept                                   { return suspended; }

    /** A plugin can override this to be told when it should reset any playing voices.

        The default implementation does nothing, but a host may call this to tell the
        plugin that it should stop any tails or sounds that have been left running.
    */
    virtual void reset();

    //==============================================================================
    /** Returns true if the processor is being run in an offline mode for rendering.

        If the processor is being run live on realtime signals, this returns false.
        If the mode is unknown, this will assume it's realtime and return false.

        This value may be unreliable until the prepareToPlay() method has been called,
        and could change each time prepareToPlay() is called.

        @see setNonRealtime()
    */
    bool isNonRealtime() const noexcept                                 { return nonRealtime; }

    /** Called by the host to tell this processor whether it's being used in a non-realtime
        capacity for offline rendering or bouncing.
    */
    virtual void setNonRealtime (bool isNonRealtime) noexcept;

    //==============================================================================
    /** Creates the filter's UI.

        This can return nullptr if you want a UI-less filter, in which case the host may create
        a generic UI that lets the user twiddle the parameters directly.

        If you do want to pass back a component, the component should be created and set to
        the correct size before returning it. If you implement this method, you must
        also implement the hasEditor() method and make it return true.

        Remember not to do anything silly like allowing your filter to keep a pointer to
        the component that gets created - it could be deleted later without any warning, which
        would make your pointer into a dangler. Use the getActiveEditor() method instead.

        The correct way to handle the connection between an editor component and its
        filter is to use something like a ChangeBroadcaster so that the editor can
        register itself as a listener, and be told when a change occurs. This lets them
        safely unregister themselves when they are deleted.

        Here are a few things to bear in mind when writing an editor:

        - Initially there won't be an editor, until the user opens one, or they might
          not open one at all. Your filter mustn't rely on it being there.
        - An editor object may be deleted and a replacement one created again at any time.
        - It's safe to assume that an editor will be deleted before its filter.

        @see hasEditor
    */
    virtual AudioProcessorEditor* createEditor() = 0;

    /** Your filter must override this and return true if it can create an editor component.
        @see createEditor
    */
    virtual bool hasEditor() const = 0;

    //==============================================================================
    /** Returns the active editor, if there is one.
        Bear in mind this can return nullptr, even if an editor has previously been opened.
    */
    AudioProcessorEditor* getActiveEditor() const noexcept             { return activeEditor; }

    /** Returns the active editor, or if there isn't one, it will create one.
        This may call createEditor() internally to create the component.
    */
    AudioProcessorEditor* createEditorIfNeeded();

    //==============================================================================
    /** This must return the correct value immediately after the object has been
        created, and mustn't change the number of parameters later.
    */
    virtual int getNumParameters() = 0;

    /** Returns the name of a particular parameter. */
    virtual const String getParameterName (int parameterIndex) = 0;

    /** Called by the host to find out the value of one of the filter's parameters.

        The host will expect the value returned to be between 0 and 1.0.

        This could be called quite frequently, so try to make your code efficient.
        It's also likely to be called by non-UI threads, so the code in here should
        be thread-aware.
    */
    virtual float getParameter (int parameterIndex) = 0;

    /** Returns the value of a parameter as a text string. */
    virtual const String getParameterText (int parameterIndex) = 0;

    /** Returns the name of a parameter as a text string with a preferred maximum length.
        If you want to provide customised short versions of your parameter names that
        will look better in constrained spaces (e.g. the displays on hardware controller
        devices or mixing desks) then you should implement this method.
        If you don't override it, the default implementation will call getParameterText(int),
        and truncate the result.
    */
    virtual String getParameterName (int parameterIndex, int maximumStringLength);

    /** Returns the value of a parameter as a text string with a preferred maximum length.
        If you want to provide customised short versions of your parameter values that
        will look better in constrained spaces (e.g. the displays on hardware controller
        devices or mixing desks) then you should implement this method.
        If you don't override it, the default implementation will call getParameterText(int),
        and truncate the result.
    */
    virtual String getParameterText (int parameterIndex, int maximumStringLength);

    /** Returns the number of discrete steps that this parameter can represent.
        The default return value if you don't implement this method is 0x7fffffff.
        If your parameter is boolean, then you may want to make this return 2.
        The value that is returned may or may not be used, depending on the host.
    */
    virtual int getParameterNumSteps (int parameterIndex);

    /** Returns the default value for the parameter.
        By default, this just returns 0.
        The value that is returned may or may not be used, depending on the host.
    */
    virtual float getParameterDefaultValue (int parameterIndex);

    /** Some plugin types may be able to return a label string for a
        parameter's units.
    */
    virtual String getParameterLabel (int index) const;

    /** This can be overridden to tell the host that particular parameters operate in the
        reverse direction. (Not all plugin formats or hosts will actually use this information).
    */
    virtual bool isParameterOrientationInverted (int index) const;

    /** The host will call this method to change the value of one of the filter's parameters.

        The host may call this at any time, including during the audio processing
        callback, so the filter has to process this very fast and avoid blocking.

        If you want to set the value of a parameter internally, e.g. from your
        editor component, then don't call this directly - instead, use the
        setParameterNotifyingHost() method, which will also send a message to
        the host telling it about the change. If the message isn't sent, the host
        won't be able to automate your parameters properly.

        The value passed will be between 0 and 1.0.
    */
    virtual void setParameter (int parameterIndex, float newValue) = 0;

    /** Your filter can call this when it needs to change one of its parameters.

        This could happen when the editor or some other internal operation changes
        a parameter. This method will call the setParameter() method to change the
        value, and will then send a message to the host telling it about the change.

        Note that to make sure the host correctly handles automation, you should call
        the beginParameterChangeGesture() and endParameterChangeGesture() methods to
        tell the host when the user has started and stopped changing the parameter.
    */
    void setParameterNotifyingHost (int parameterIndex, float newValue);

    /** Returns true if the host can automate this parameter.
        By default, this returns true for all parameters.
    */
    virtual bool isParameterAutomatable (int parameterIndex) const;

    /** Should return true if this parameter is a "meta" parameter.
        A meta-parameter is a parameter that changes other params. It is used
        by some hosts (e.g. AudioUnit hosts).
        By default this returns false.
    */
    virtual bool isMetaParameter (int parameterIndex) const;

    /** Sends a signal to the host to tell it that the user is about to start changing this
        parameter.

        This allows the host to know when a parameter is actively being held by the user, and
        it may use this information to help it record automation.

        If you call this, it must be matched by a later call to endParameterChangeGesture().
    */
    void beginParameterChangeGesture (int parameterIndex);

    /** Tells the host that the user has finished changing this parameter.

        This allows the host to know when a parameter is actively being held by the user, and
        it may use this information to help it record automation.

        A call to this method must follow a call to beginParameterChangeGesture().
    */
    void endParameterChangeGesture (int parameterIndex);

    /** The filter can call this when something (apart from a parameter value) has changed.

        It sends a hint to the host that something like the program, number of parameters,
        etc, has changed, and that it should update itself.
    */
    void updateHostDisplay();

    //==============================================================================
    /** Returns the number of preset programs the filter supports.

        The value returned must be valid as soon as this object is created, and
        must not change over its lifetime.

        This value shouldn't be less than 1.
    */
    virtual int getNumPrograms() = 0;

    /** Returns the number of the currently active program. */
    virtual int getCurrentProgram() = 0;

    /** Called by the host to change the current program. */
    virtual void setCurrentProgram (int index) = 0;

    /** Must return the name of a given program. */
    virtual const String getProgramName (int index) = 0;

    /** Called by the host to rename a program. */
    virtual void changeProgramName (int index, const String& newName) = 0;

    //==============================================================================
    /** The host will call this method when it wants to save the filter's internal state.

        This must copy any info about the filter's state into the block of memory provided,
        so that the host can store this and later restore it using setStateInformation().

        Note that there's also a getCurrentProgramStateInformation() method, which only
        stores the current program, not the state of the entire filter.

        See also the helper function copyXmlToBinary() for storing settings as XML.

        @see getCurrentProgramStateInformation
    */
    virtual void getStateInformation (juce::MemoryBlock& destData) = 0;

    /** The host will call this method if it wants to save the state of just the filter's
        current program.

        Unlike getStateInformation, this should only return the current program's state.

        Not all hosts support this, and if you don't implement it, the base class
        method just calls getStateInformation() instead. If you do implement it, be
        sure to also implement getCurrentProgramStateInformation.

        @see getStateInformation, setCurrentProgramStateInformation
    */
    virtual void getCurrentProgramStateInformation (juce::MemoryBlock& destData);

    /** This must restore the filter's state from a block of data previously created
        using getStateInformation().

        Note that there's also a setCurrentProgramStateInformation() method, which tries
        to restore just the current program, not the state of the entire filter.

        See also the helper function getXmlFromBinary() for loading settings as XML.

        @see setCurrentProgramStateInformation
    */
    virtual void setStateInformation (const void* data, int sizeInBytes) = 0;

    /** The host will call this method if it wants to restore the state of just the filter's
        current program.

        Not all hosts support this, and if you don't implement it, the base class
        method just calls setStateInformation() instead. If you do implement it, be
        sure to also implement getCurrentProgramStateInformation.

        @see setStateInformation, getCurrentProgramStateInformation
    */
    virtual void setCurrentProgramStateInformation (const void* data, int sizeInBytes);

    /** This method is called when the number of input or output channels is changed. */
    virtual void numChannelsChanged();

    //==============================================================================
    /** Adds a listener that will be called when an aspect of this processor changes. */
    virtual void addListener (AudioProcessorListener* newListener);

    /** Removes a previously added listener. */
    virtual void removeListener (AudioProcessorListener* listenerToRemove);

    //==============================================================================
    /** Tells the processor to use this playhead object.
        The processor will not take ownership of the object, so the caller must delete it when
        it is no longer being used.
    */
    virtual void setPlayHead (AudioPlayHead* newPlayHead);

    //==============================================================================
    /** This is called by the processor to specify its details before being played. */
    void setPlayConfigDetails (int numIns, int numOuts, double sampleRate, int blockSize) noexcept;

    //==============================================================================
    /** Not for public use - this is called before deleting an editor component. */
    void editorBeingDeleted (AudioProcessorEditor*) noexcept;

    /** Not for public use - this is called to initialise the processor before playing. */
    void setSpeakerArrangement (const String& inputs, const String& outputs);

    /** Flags to indicate the type of plugin context in which a processor is being used. */
    enum WrapperType
    {
        wrapperType_Undefined = 0,
        wrapperType_VST,
        wrapperType_VST3,
        wrapperType_AudioUnit,
        wrapperType_RTAS,
        wrapperType_AAX,
        wrapperType_Standalone
    };

    /** When loaded by a plugin wrapper, this flag will be set to indicate the type
        of plugin within which the processor is running.
    */
    WrapperType wrapperType;

    //==============================================================================
    /** Helper function that just converts an xml element into a binary blob.

        Use this in your filter's getStateInformation() method if you want to
        store its state as xml.

        Then use getXmlFromBinary() to reverse this operation and retrieve the XML
        from a binary blob.
    */
    static void copyXmlToBinary (const XmlElement& xml,
                                 juce::MemoryBlock& destData);

    /** Retrieves an XML element that was stored as binary with the copyXmlToBinary() method.

        This might return nullptr if the data's unsuitable or corrupted. Otherwise it will return
        an XmlElement object that the caller must delete when no longer needed.
    */
    static XmlElement* getXmlFromBinary (const void* data, int sizeInBytes);

    /** @internal */
    static void JUCE_CALLTYPE setTypeOfNextNewPlugin (WrapperType);

protected:
    /** @internal */
    AudioPlayHead* playHead;

    /** @internal */
    void sendParamChangeMessageToListeners (int parameterIndex, float newValue);

private:
    Array<AudioProcessorListener*> listeners;
    Component::SafePointer<AudioProcessorEditor> activeEditor;
    double sampleRate;
    int blockSize, numInputChannels, numOutputChannels, latencySamples;
    bool suspended, nonRealtime;
    CriticalSection callbackLock, listenerLock;
    String inputSpeakerArrangement, outputSpeakerArrangement;

   #if JUCE_DEBUG
    BigInteger changingParams;
   #endif

    AudioProcessorListener* getListenerLocked (int) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessor)
};


#endif   // JUCE_AUDIOPROCESSOR_H_INCLUDED
