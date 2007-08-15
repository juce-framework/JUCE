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

#ifndef __JUCE_AUDIOFILTERBASE_JUCEHEADER__
#define __JUCE_AUDIOFILTERBASE_JUCEHEADER__

#ifdef _MSC_VER
  #pragma pack (push, 8)
#endif

#include "../../../juce.h"
#include "juce_AudioFilterEditor.h"
#undef MemoryBlock


//==============================================================================
/**
    Base class for audio filters or plugins written using JUCE.

    This is intended to act as a base class of audio filter that is general enough to
    be wrapped as a VST, AU, RTAS, etc, or used internally.

    It is also used by the plugin hosting code as the wrapper around an instance 
    of a loaded plugin.

    Derive your filter class from this base class, and if you're building a plugin,
    you should implement a global function called initialiseFilterInfo() which creates 
    and returns a new instance of your subclass.
*/
class AudioFilterBase
{
protected:
    //==============================================================================
    /** Constructor.

        You can also do your initialisation tasks in the initialiseFilterInfo()
        call, which will be made after this object has been created.
    */
    AudioFilterBase();

public:
    /** Destructor. */
    virtual ~AudioFilterBase();

    //==============================================================================
    /** Called before playback starts, to let the filter prepare itself.

        The sample rate is the target sample rate, and will remain constant until
        playback stops.

        The estimatedSamplesPerBlock value is a HINT about the typical number of
        samples that will be processed for each callback, but isn't any kind
        of guarantee. The actual block sizes that the host uses may be different
        each time the callback happens, and may be more or less than this value.
    */
    virtual void JUCE_CALLTYPE prepareToPlay (double sampleRate,
                                              int estimatedSamplesPerBlock) = 0;

    /** Called after playback has stopped, to let the filter free up any resources it
        no longer needs.
    */
    virtual void JUCE_CALLTYPE releaseResources() = 0;

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
        but your should only read/write from the ones that your filter is supposed to
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
    virtual void JUCE_CALLTYPE processBlock (AudioSampleBuffer& buffer,
                                             MidiBuffer& midiMessages) = 0;

    //==============================================================================
    /** Structure containing details of the playback position.

        @see AudioFilterBase::getCurrentPositionInfo
    */
    struct CurrentPositionInfo
    {
        /** The tempo in BPM */
        double bpm;

        /** Time signature numerator, e.g. the 3 of a 3/4 time sig */
        int timeSigNumerator;
        /** Time signature denominator, e.g. the 4 of a 3/4 time sig */
        int timeSigDenominator;

        /** The current play position, in seconds from the start of the edit. */
        double timeInSeconds;

        /** For timecode, the position of the start of the edit, in seconds from 00:00:00:00. */
        double editOriginTime;

        /** The current play position in pulses-per-quarter-note.

            This is the number of quarter notes since the edit start.
        */
        double ppqPosition;

        /** The position of the start of the last bar, in pulses-per-quarter-note.

            This is the number of quarter notes from the start of the edit to the
            start of the current bar.

            Note - this value may be unavailable on some hosts, e.g. Pro-Tools. If
            it's not available, the value will be 0.
        */
        double ppqPositionOfLastBarStart;

        /** Frame rate types */
        enum FrameRateType
        {
            fps24           = 0,
            fps25           = 1,
            fps2997         = 2,
            fps30           = 3,
            fps2997drop     = 4,
            fps30drop       = 5,
            fpsUnknown      = 99
        };

        /** The video frame rate, if applicable. */
        FrameRateType frameRate;

        /** True if the transport is currently playing. */
        bool isPlaying;

        /** True if the transport is currently recording.

            (When isRecording is true, then isPlaying will also be true).
        */
        bool isRecording;
    };

    /** Asks the host to return the current playback position.

        You can call this from your processBlock() method to get info about
        the time of the start of the block currently being processed.

        If the host can't supply this for some reason, this will return false, otherwise
        it'll fill in the structure passed in.
    */
    bool JUCE_CALLTYPE getCurrentPositionInfo (CurrentPositionInfo& info);

    //==============================================================================
    /** Returns the current sample rate.

        This can be called from your processBlock() method - it's not guaranteed
        to be valid at any other time, and may return 0 if it's unknown.
    */
    double JUCE_CALLTYPE getSampleRate() const throw()                      { return sampleRate; }

    /** Returns the current typical block size that is being used.

        This can be called from your processBlock() method - it's not guaranteed
        to be valid at any other time.

        Remember it's not the ONLY block size that may be used when calling
        processBlock, it's just the normal one. The actual block sizes used may be
        larger or smaller than this, and will vary between successive calls.
    */
    int JUCE_CALLTYPE getBlockSize() const throw()                          { return blockSize; }

    //==============================================================================
    /** Returns the number of input channels that the host will be sending the filter.

        In your JucePluginCharacteristics.h file, you specify the number of channels
        that your filter would prefer to get, and this method lets you know how
        many the host is actually going to send.

        Note that this method is only valid during or after the prepareToPlay()
        method call. Until that point, the number of channels will be unknown.
    */
    int JUCE_CALLTYPE getNumInputChannels() const throw()                   { return numInputChannels; }

    /** Returns the number of output channels that the host will be sending the filter.

        In your JucePluginCharacteristics.h file, you specify the number of channels
        that your filter would prefer to get, and this method lets you know how
        many the host is actually going to send.

        Note that this method is only valid during or after the prepareToPlay()
        method call. Until that point, the number of channels will be unknown.
    */
    int JUCE_CALLTYPE getNumOutputChannels() const throw()                  { return numOutputChannels; }

    /** Returns the name of one of the input channels, as returned by the host.

        The host might not supply very useful names for channels, and this might be
        something like "1", "2", "left", "right", etc.
    */
    virtual const String JUCE_CALLTYPE getInputChannelName (const int channelIndex) const = 0;

    /** Returns the name of one of the output channels, as returned by the host.

        The host might not supply very useful names for channels, and this might be
        something like "1", "2", "left", "right", etc.
    */
    virtual const String JUCE_CALLTYPE getOutputChannelName (const int channelIndex) const = 0;

    /** Returns true if the specified channel is part of a stereo pair with its neighbour. */
    virtual bool JUCE_CALLTYPE isInputChannelStereoPair (int index) const = 0;

    /** Returns true if the specified channel is part of a stereo pair with its neighbour. */
    virtual bool JUCE_CALLTYPE isOutputChannelStereoPair (int index) const = 0;

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
    const CriticalSection& JUCE_CALLTYPE getCallbackLock() const throw()              { return callbackLock; }

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

        @see getCallbackLock
    */
    void JUCE_CALLTYPE suspendProcessing (const bool shouldBeSuspended);

    /** Returns true if processing is currently suspended.
        @see suspendProcessing
    */
    bool JUCE_CALLTYPE isSuspended() const throw()                                  { return suspended; }

    //==============================================================================
    /** Creates the filter's UI.

        This can return 0 if you want a UI-less filter, in which case the host may create
        a generic UI that lets the user twiddle the parameters directly. 
        
        If you do want to pass back a component, the component should be created and set to
        the correct size before returning it.

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
    */
    virtual AudioFilterEditor* JUCE_CALLTYPE createEditor() = 0;

    //==============================================================================
    /** Returns the active editor, if there is one.

        Bear in mind this can return 0, even if an editor has previously been
        opened.
    */
    AudioFilterEditor* JUCE_CALLTYPE getActiveEditor() const throw()              { return activeEditor; }

    /** Returns the active editor, or if there isn't one, it will create one.

        This may call createEditor() internally to create the component.
    */
    AudioFilterEditor* JUCE_CALLTYPE createEditorIfNeeded();

    //==============================================================================
    /** This must return the correct value immediately after the object has been
        created, and mustn't change the number of parameters later.
    */
    virtual int JUCE_CALLTYPE getNumParameters() = 0;

    /** Returns the name of a particular parameter. */
    virtual const String JUCE_CALLTYPE getParameterName (int parameterIndex) = 0;

    /** Called by the host to find out the value of one of the filter's parameters.

        The host will expect the value returned to be between 0 and 1.0.

        This could be called quite frequently, so try to make your code efficient.
        It's also likely to be called by non-UI threads, so the code in here should
        be thread-aware.
    */
    virtual float JUCE_CALLTYPE getParameter (int parameterIndex) = 0;

    /** Returns the value of a parameter as a text string. */
    virtual const String JUCE_CALLTYPE getParameterText (int parameterIndex) = 0;

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
    virtual void JUCE_CALLTYPE setParameter (int parameterIndex,
                                             float newValue) = 0;

    /** Your filter can call this when it needs to change one of its parameters.

        This could happen when the editor or some other internal operation changes
        a parameter. This method will call the setParameter() method to change the
        value, and will then send a message to the host telling it about the change.
    */
    void JUCE_CALLTYPE setParameterNotifyingHost (int parameterIndex,
                                                  float newValue);

    /** Returns true if the host can automate this parameter.

        By default, this returns true for all parameters.
    */
    virtual bool isParameterAutomatable (int index) const;

    /** The filter can call this when something (apart from a parameter value) has changed.

        It sends a hint to the host that something like the program, number of parameters,
        etc, has changed, and that it should update itself.
    */
    void JUCE_CALLTYPE updateHostDisplay();

    //==============================================================================
    /** Returns the number of preset programs the filter supports.

        The value returned must be valid as soon as this object is created, and
        must not change over its lifetime.

        This value shouldn't be less than 1.
    */
    virtual int JUCE_CALLTYPE getNumPrograms() = 0;

    /** Returns the number of the currently active program.
    */
    virtual int JUCE_CALLTYPE getCurrentProgram() = 0;

    /** Called by the host to change the current program.
    */
    virtual void JUCE_CALLTYPE setCurrentProgram (int index) = 0;

    /** Must return the name of a given program. */
    virtual const String JUCE_CALLTYPE getProgramName (int index) = 0;

    /** Called by the host to rename a program.
    */
    virtual void JUCE_CALLTYPE changeProgramName (int index, const String& newName) = 0;

    //==============================================================================
    /** The host will call this method when it wants to save the filter's internal state.

        This must copy any info about the filter's state into the block of memory provided,
        so that the host can store this and later restore it using setStateInformation().

        Note that there's also a getCurrentProgramStateInformation() method, which only
        stores the current program, not the state of the entire filter.

        See also the helper function copyXmlToBinary() for storing settings as XML.

        @see getCurrentProgramStateInformation
    */
    virtual void JUCE_CALLTYPE getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData) = 0;

    /** The host will call this method if it wants to save the state of just the filter's
        current program.

        Unlike getStateInformation, this should only return the current program's state.

        Not all hosts support this, and if you don't implement it, the base class
        method just calls getStateInformation() instead. If you do implement it, be
        sure to also implement getCurrentProgramStateInformation.

        @see getStateInformation, setCurrentProgramStateInformation
    */
    virtual void JUCE_CALLTYPE getCurrentProgramStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);

    /** This must restore the filter's state from a block of data previously created
        using getStateInformation().

        Note that there's also a setCurrentProgramStateInformation() method, which tries
        to restore just the current program, not the state of the entire filter.

        See also the helper function getXmlFromBinary() for loading settings as XML.

        @see setCurrentProgramStateInformation
    */
    virtual void JUCE_CALLTYPE setStateInformation (const void* data, int sizeInBytes) = 0;

    /** The host will call this method if it wants to restore the state of just the filter's
        current program.

        Not all hosts support this, and if you don't implement it, the base class
        method just calls setStateInformation() instead. If you do implement it, be
        sure to also implement getCurrentProgramStateInformation.

        @see setStateInformation, getCurrentProgramStateInformation
    */
    virtual void JUCE_CALLTYPE setCurrentProgramStateInformation (const void* data, int sizeInBytes);


    //==============================================================================
    /** @internal */
    class HostCallbacks
    {
    public:
        virtual ~HostCallbacks() {}

        virtual bool JUCE_CALLTYPE getCurrentPositionInfo (CurrentPositionInfo& info) = 0;
        virtual void JUCE_CALLTYPE informHostOfParameterChange (int index, float newValue) = 0;

        /** Callback to indicate that something (other than a parameter) has changed in the 
            filter, such as its current program, parameter list, etc. */
        virtual void JUCE_CALLTYPE informHostOfStateChange() = 0;
    };


    //==============================================================================
    /** Not for public use - this is called by the wrapper code before deleting an
        editor component.
    */
    void JUCE_CALLTYPE editorBeingDeleted (AudioFilterEditor* const editor);

    /** Not for public use - this is called by the wrapper code to initialise the
        filter.
    */
    void JUCE_CALLTYPE setHostCallbacks (HostCallbacks* const);

    /** Not for public use - this is called by the wrapper code to initialise the
        filter.
    */
    void setPlayConfigDetails (const int numIns, const int numOuts, 
                               const double sampleRate,
                               const int blockSize) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    /** Helper function that just converts an xml element into a binary blob.

        Use this in your filter's getStateInformation() method if you want to
        store its state as xml.

        Then use getXmlFromBinary() to reverse this operation and retrieve the XML
        from a binary blob.
    */
    static void JUCE_CALLTYPE copyXmlToBinary (const XmlElement& xml,
                                               JUCE_NAMESPACE::MemoryBlock& destData);

    /** Retrieves an XML element that was stored as binary with the copyXmlToBinary() method.

        This might return 0 if the data's unsuitable or corrupted. Otherwise it will return
        an XmlElement object that the caller must delete when no longer needed.
    */
    static XmlElement* JUCE_CALLTYPE getXmlFromBinary (const void* data,
                                                       const int sizeInBytes);

    /** @internal */
    HostCallbacks* callbacks;

private:
    AudioFilterEditor* activeEditor;
    double sampleRate;
    int blockSize, numInputChannels, numOutputChannels;
    bool suspended;
    CriticalSection callbackLock;
};

//==============================================================================
/** Somewhere in the code for an actual plugin, you need to implement this function
    and make it create an instance of the filter subclass that you're building.
*/
extern AudioFilterBase* JUCE_CALLTYPE createPluginFilter();

#ifdef _MSC_VER
  #pragma pack (pop)
#endif


#endif   // __JUCE_AUDIOFILTERBASE_JUCEHEADER__
