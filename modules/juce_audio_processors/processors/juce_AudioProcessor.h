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

//==============================================================================
/**
    Base class for audio processing classes or plugins.

    This is intended to act as a base class of audio processor that is general enough
    to be wrapped as a VST, AU, RTAS, etc, or used internally.

    It is also used by the plugin hosting code as the wrapper around an instance
    of a loaded plugin.

    You should derive your own class from this base class, and if you're building a
    plugin, you should implement a global function called createPluginFilter() which
    creates and returns a new instance of your subclass.
*/
class JUCE_API  AudioProcessor
{
protected:
    struct BusesProperties;

    //==============================================================================
    /** Constructor.

        This constructor will create a main input and output bus which are diabled
        by default. If you need more fine-grained control then use the other constructors.
    */
    AudioProcessor();

    /** Constructor for multi-bus AudioProcessors

        If your AudioProcessor supports multiple buses than use this constructor
        to initialise the bus layouts and bus names of your plug-in.
    */
    AudioProcessor (const BusesProperties& ioLayouts);

    /** Constructor for AudioProcessors which use layout maps
        If your AudioProcessor uses layout maps then use this constructor.
    */
   #if JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS
    AudioProcessor (const std::initializer_list<const short[2]>& channelLayoutList)
        : AudioProcessor (busesPropertiesFromLayoutArray (layoutListToArray (channelLayoutList)))
    {
    }
   #else
    template <int numLayouts>
    AudioProcessor (const short (&channelLayoutList) [numLayouts][2])
        : AudioProcessor (busesPropertiesFromLayoutArray (layoutListToArray (channelLayoutList)))
    {
    }
   #endif

public:
    //==============================================================================
    enum ProcessingPrecision
    {
        singlePrecision,
        doublePrecision
    };

    //==============================================================================
    /** Destructor. */
    virtual ~AudioProcessor();

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
    /** Called before playback starts, to let the processor prepare itself.

        The sample rate is the target sample rate, and will remain constant until
        playback stops.

        You can call getTotalNumInputChannels and getTotalNumOutputChannels
        or query the busLayout member variable to find out the number of
        channels your processBlock callback must process.

        The maximumExpectedSamplesPerBlock value is a strong hint about the maximum
        number of samples that will be provided in each block. You may want to use
        this value to resize internal buffers. You should program defensively in
        case a buggy host exceeds this value. The actual block sizes that the host
        uses may be different each time the callback happens: completely variable
        block sizes can be expected from some hosts.

       @see busLayout, getTotalNumInputChannels, getTotalNumOutputChannels
    */
    virtual void prepareToPlay (double sampleRate,
                                int maximumExpectedSamplesPerBlock) = 0;

    /** Called after playback has stopped, to let the object free up any resources it
        no longer needs.
    */
    virtual void releaseResources() = 0;

    /** Called by the host to indicate that you should reduce your memory footprint.

        You should override this method to free up some memory gracefully, if possible,
        otherwise the host may forcibly unload your AudioProcessor.

        At the moment this method is only called when your AudioProcessor is an AUv3
        plug-in running on iOS.
    */
    virtual void memoryWarningReceived()     { jassertfalse; }

    /** Renders the next block.

        When this method is called, the buffer contains a number of channels which is
        at least as great as the maximum number of input and output channels that
        this processor is using. It will be filled with the processor's input data and
        should be replaced with the processor's output.

        So for example if your processor has a total of 2 input channels and 4 output
        channels, then the buffer will contain 4 channels, the first two being filled
        with the input data. Your processor should read these, do its processing, and
        replace the contents of all 4 channels with its output.

        Or if your processor has a total of 5 inputs and 2 outputs, the buffer will have 5
        channels, all filled with data, and your processor should overwrite the first 2 of
        these with its output. But be VERY careful not to write anything to the last 3
        channels, as these might be mapped to memory that the host assumes is read-only!

        If your plug-in has more than one input or output buses then the buffer passed
        to the processBlock methods will contain a bundle of all channels of each bus.
        Use AudiobusLayout::getBusBuffer to obtain an audio buffer for a
        particular bus.

        Note that if you have more outputs than inputs, then only those channels that
        correspond to an input channel are guaranteed to contain sensible data - e.g.
        in the case of 2 inputs and 4 outputs, the first two channels contain the input,
        but the last two channels may contain garbage, so you should be careful not to
        let this pass through without being overwritten or cleared.

        Also note that the buffer may have more channels than are strictly necessary,
        but you should only read/write from the ones that your processor is supposed to
        be using.

        The number of samples in these buffers is NOT guaranteed to be the same for every
        callback, and may be more or less than the estimated value given to prepareToPlay().
        Your code must be able to cope with variable-sized blocks, or you're going to get
        clicks and crashes!

        Also note that some hosts will occasionally decide to pass a buffer containing
        zero samples, so make sure that your algorithm can deal with that!

        If the processor is receiving a MIDI input, then the midiMessages array will be filled
        with the MIDI messages for this block. Each message's timestamp will indicate the
        message's time, as a number of samples from the start of the block.

        Any messages left in the MIDI buffer when this method has finished are assumed to
        be the processor's MIDI output. This means that your processor should be careful to
        clear any incoming messages from the array if it doesn't want them to be passed-on.

        Be very careful about what you do in this callback - it's going to be called by
        the audio thread, so any kind of interaction with the UI is absolutely
        out of the question. If you change a parameter in here and need to tell your UI to
        update itself, the best way is probably to inherit from a ChangeBroadcaster, let
        the UI components register as listeners, and then call sendChangeMessage() inside the
        processBlock() method to send out an asynchronous message. You could also use
        the AsyncUpdater class in a similar way.

        @see AudiobusLayout::getBusBuffer
    */
    virtual void processBlock (AudioBuffer<float>& buffer,
                               MidiBuffer& midiMessages) = 0;

    /** Renders the next block.

        When this method is called, the buffer contains a number of channels which is
        at least as great as the maximum number of input and output channels that
        this processor is using. It will be filled with the processor's input data and
        should be replaced with the processor's output.

        So for example if your processor has a combined total of 2 input channels and
        4 output channels, then the buffer will contain 4 channels, the first two
        being filled with the input data. Your processor should read these, do its
        processing, and replace the contents of all 4 channels with its output.

        Or if your processor has 5 inputs and 2 outputs, the buffer will have 5 channels,
        all filled with data, and your processor should overwrite the first 2 of these
        with its output. But be VERY careful not to write anything to the last 3
        channels, as these might be mapped to memory that the host assumes is read-only!

        If your plug-in has more than one input or output buses then the buffer passed
        to the processBlock methods will contain a bundle of all channels of
        each bus. Use AudiobusLayout::getBusBuffer to obtain a audio buffer
        for a particular bus.

        Note that if you have more outputs than inputs, then only those channels that
        correspond to an input channel are guaranteed to contain sensible data - e.g.
        in the case of 2 inputs and 4 outputs, the first two channels contain the input,
        but the last two channels may contain garbage, so you should be careful not to
        let this pass through without being overwritten or cleared.

        Also note that the buffer may have more channels than are strictly necessary,
        but you should only read/write from the ones that your processor is supposed to
        be using.

        If your plugin uses buses, then you should use AudiobusLayout::getBusBuffer()
        or AudiobusLayout::getChannelIndexInProcessBlockBuffer() to find out which
        of the input and output channels correspond to which of the buses.

        The number of samples in these buffers is NOT guaranteed to be the same for every
        callback, and may be more or less than the estimated value given to prepareToPlay().
        Your code must be able to cope with variable-sized blocks, or you're going to get
        clicks and crashes!

        Also note that some hosts will occasionally decide to pass a buffer containing
        zero samples, so make sure that your algorithm can deal with that!

        If the processor is receiving a MIDI input, then the midiMessages array will be filled
        with the MIDI messages for this block. Each message's timestamp will indicate the
        message's time, as a number of samples from the start of the block.

        Any messages left in the MIDI buffer when this method has finished are assumed to
        be the processor's MIDI output. This means that your processor should be careful to
        clear any incoming messages from the array if it doesn't want them to be passed-on.

        Be very careful about what you do in this callback - it's going to be called by
        the audio thread, so any kind of interaction with the UI is absolutely
        out of the question. If you change a parameter in here and need to tell your UI to
        update itself, the best way is probably to inherit from a ChangeBroadcaster, let
        the UI components register as listeners, and then call sendChangeMessage() inside the
        processBlock() method to send out an asynchronous message. You could also use
        the AsyncUpdater class in a similar way.

        @see AudiobusLayout::getBusBuffer
    */
    virtual void processBlock (AudioBuffer<double>& buffer,
                               MidiBuffer& midiMessages);

    /** Renders the next block when the processor is being bypassed.

        The default implementation of this method will pass-through any incoming audio, but
        you may override this method e.g. to add latency compensation to the data to match
        the processor's latency characteristics. This will avoid situations where bypassing
        will shift the signal forward in time, possibly creating pre-echo effects and odd timings.
        Another use for this method would be to cross-fade or morph between the wet (not bypassed)
        and dry (bypassed) signals.
    */
    virtual void processBlockBypassed (AudioBuffer<float>& buffer,
                                       MidiBuffer& midiMessages);

    /** Renders the next block when the processor is being bypassed.

        The default implementation of this method will pass-through any incoming audio, but
        you may override this method e.g. to add latency compensation to the data to match
        the processor's latency characteristics. This will avoid situations where bypassing
        will shift the signal forward in time, possibly creating pre-echo effects and odd timings.
        Another use for this method would be to cross-fade or morph between the wet (not bypassed)
        and dry (bypassed) signals.
    */
    virtual void processBlockBypassed (AudioBuffer<double>& buffer,
                                       MidiBuffer& midiMessages);


    //==============================================================================
    /**
        Represents the bus layout state of a plug-in
    */
    struct BusesLayout
    {
        /** An array containing the list of input buses that this processor supports. */
        Array<AudioChannelSet> inputBuses;

        /** An array containing the list of output buses that this processor supports. */
        Array<AudioChannelSet> outputBuses;

        /** Get the number of channels of a particular bus */
        int getNumChannels (bool isInput, int busIndex) const noexcept
        {
            auto& bus = (isInput ? inputBuses : outputBuses);
            return isPositiveAndBelow (busIndex, bus.size()) ? bus.getReference (busIndex).size() : 0;
        }

        /** Get the channel set of a particular bus */
        AudioChannelSet& getChannelSet (bool isInput, int busIndex) noexcept
        {
            return (isInput ? inputBuses : outputBuses).getReference (busIndex);
        }

        /** Get the channel set of a particular bus */
        AudioChannelSet getChannelSet (bool isInput, int busIndex) const noexcept
        {
            return (isInput ? inputBuses : outputBuses)[busIndex];
        }

        /** Get the input channel layout on the main bus. */
        AudioChannelSet getMainInputChannelSet()  const noexcept    { return getChannelSet (true,  0); }

        /** Get the output channel layout on the main bus. */
        AudioChannelSet getMainOutputChannelSet() const noexcept    { return getChannelSet (false, 0); }

        /** Get the number of input channels on the main bus. */
        int getMainInputChannels()  const noexcept                  { return getNumChannels (true, 0); }

        /** Get the number of output channels on the main bus. */
        int getMainOutputChannels() const noexcept                  { return getNumChannels (false, 0); }

        bool operator== (const BusesLayout& other) const noexcept   { return inputBuses == other.inputBuses && outputBuses == other.outputBuses; }
        bool operator!= (const BusesLayout& other) const noexcept   { return inputBuses != other.inputBuses || outputBuses != other.outputBuses; }
    };

    //==============================================================================
    /**
        Describes the layout and properties of an audio bus.
        Effectively a bus description is a named set of channel types.

        @see AudioChannelSet, AudioProcessor::addBus
     */
    class Bus
    {
    public:
        /** Returns true if this bus is an input bus. */
        bool isInput() const noexcept;

        /** Returns the index of this bus. */
        int getBusIndex() const noexcept;

        /** Returns true if the current bus is the main input or output bus. */
        bool isMain() const noexcept                                    { return getBusIndex() == 0; }

        //==============================================================================
        /** The bus's name. */
        const String& getName() const noexcept                          { return name; }

        /** Get the default layout of this bus.
            @see AudioChannelSet
        */
        const AudioChannelSet& getDefaultLayout() const noexcept        { return dfltLayout; }

        //==============================================================================
        /** The bus's current layout. This will be AudioChannelSet::disabled() if the current
            layout is disabled.
            @see AudioChannelSet
        */
        const AudioChannelSet& getCurrentLayout() const noexcept        { return layout; }

        /** Return the bus's last active channel layout.
            If the bus is currently enabled then the result will be identical to getCurrentLayout
            otherwise it will return the last enabled layout.
            @see AudioChannelSet
        */
        const AudioChannelSet& getLastEnabledLayout() const noexcept    { return lastLayout; }

        /** Sets the bus's current layout.
            If the AudioProcessor does not support this layout then this will return false.
            @see AudioChannelSet
        */
        bool setCurrentLayout (const AudioChannelSet& layout);

        /** Sets the bus's current layout without changing the enabled state.
            If the AudioProcessor does not support this layout then this will return false.
            @see AudioChannelSet
         */
        bool setCurrentLayoutWithoutEnabling (const AudioChannelSet& layout);

        /** Return the number of channels of the current bus. */
        inline int getNumberOfChannels() const noexcept                 { return cachedChannelCount; }

        /** Set the number of channles of this bus. This will return false if the AudioProcessor
            does not support this layout.
        */
        bool setNumberOfChannels (int channels);

        //==============================================================================
        /** Checks if a particular layout is supported.

            @param set           The AudioChannelSet which is to be probed.
            @param currentLayout If non-null, pretend that the current layout of the AudioProcessor is
                                 currentLayout. On exit, currentLayout will be modified to
                                 to represent the buses layouts of the AudioProcessor as if the layout
                                 of the reciever had been succesfully changed. This is useful as changing
                                 the layout of the reciever may change the bus layout of other buses.

            @see AudioChannelSet
        */
        bool isLayoutSupported (const AudioChannelSet& set, BusesLayout* currentLayout = nullptr) const;

        /** Checks if this bus can support a given number of channels. */
        bool isNumberOfChannelsSupported (int channels) const;

        /** Returns a ChannelSet that the bus supports with a given number of channels. */
        AudioChannelSet supportedLayoutWithChannels (int channels) const;

        /** Returns the maximum number of channels that this bus can support.
            @param limit The maximum value to return.
        */
        int getMaxSupportedChannels (int limit = AudioChannelSet::maxChannelsOfNamedLayout) const;

        /** Returns the resulting layouts of all buses after changing the layout of this bus.

            Changing an individual layout of a bus may also change the layout of all the other
            buses. This method returns what the layouts of all the buses of the audio processor
            would be, if you were to change the layout of this bus to the given layout. If there
            is no way to support the given layout then this method will return the next best
            layout.
        */
        BusesLayout getBusesLayoutForLayoutChangeOfBus (const AudioChannelSet& set) const;

        //==============================================================================
        /** Returns true if the current bus is enabled. */
        bool isEnabled() const noexcept                             { return ! layout.isDisabled(); }

        /** Enable or disable this bus. This will return false if the AudioProcessor
            does not support disabling this bus. */
        bool enable (bool shouldEnable = true);

        /** Returns if this bus is enabled by default. */
        bool isEnabledByDefault() const noexcept                    { return enabledByDefault; }

        //==============================================================================
        /** Returns the position of a bus's channels within the processBlock buffer.
            This can be called in processBlock to figure out which channel of the master AudioBuffer
            maps onto a specific bus's channel.
        */
        int getChannelIndexInProcessBlockBuffer (int channelIndex) const noexcept;


        /** Returns an AudioBuffer containing a set of channel pointers for a specific bus.
            This can be called in processBlock to get a buffer containing a sub-group of the master
            AudioBuffer which contains all the plugin channels.
        */
        template <typename FloatType>
        AudioBuffer<FloatType> getBusBuffer (AudioBuffer<FloatType>& processBlockBuffer) const
        {
            auto di = getDirectionAndIndex();
            return owner.getBusBuffer (processBlockBuffer, di.isInput, di.index);
        }

    private:
        friend class AudioProcessor;
        Bus (AudioProcessor&, const String&, const AudioChannelSet&, bool);

        struct BusDirectionAndIndex
        {
            bool isInput;
            int index;
        };

        BusDirectionAndIndex getDirectionAndIndex() const noexcept;
        void updateChannelCount() noexcept;

        AudioProcessor& owner;
        String name;
        AudioChannelSet layout, dfltLayout, lastLayout;
        bool enabledByDefault;
        int cachedChannelCount;

        JUCE_DECLARE_NON_COPYABLE (Bus)
    };

    //==============================================================================
    /** Returns the number of buses on the input or output side */
    int getBusCount (bool isInput) const noexcept                   { return (isInput ? inputBuses : outputBuses).size(); }

    /** Returns the audio bus with a given index and direction.
        If busIndex is invalid then this method will return a nullptr.
    */
    Bus* getBus (bool isInput, int busIndex) noexcept               { return (isInput ? inputBuses : outputBuses)[busIndex]; }

    /** Returns the audio bus with a given index and direction.
        If busIndex is invalid then this method will return a nullptr.
    */
    const Bus* getBus (bool isInput, int busIndex) const noexcept   { return const_cast<AudioProcessor*> (this)->getBus (isInput, busIndex); }

    //==============================================================================
    /**  Callback to query if a bus can currently be added.

         This callback probes if a bus can currently be added. You should override
         this callback if you want to support dynamically adding/removing buses by
         the host. This is useful for mixer audio processors.

         The default implementation will always return false.

         @see addBus
    */
    virtual bool canAddBus (bool isInput) const                     { ignoreUnused (isInput); return false; }

    /**  Callback to query if the last bus can currently be removed.

         This callback probes if the last bus can currently be removed. You should
         override this callback if you want to support dynamically adding/removing
         buses by the host. This is useful for mixer audio processors.

         If you return true in this callback then the AudioProcessor will go ahead
         and delete the bus.

         The default implementation will always return false.
    */
    virtual bool canRemoveBus (bool isInput) const                  { ignoreUnused (isInput); return false; }

    /** Dynamically request an additional bus.

        Request an additional bus from the audio processor. If the audio processor
        does not support adding additional buses then this method will return false.

        Most audio processors will not allow you to dynamically add/remove
        audio buses and will return false.

        This method will invoke the canApplyBusCountChange callback to probe
        if a bus can be added and, if yes, will use the supplied bus properties
        of the canApplyBusCountChange callback to create a new bus.

        @see canApplyBusCountChange, removeBus
    */
    bool addBus (bool isInput);

    /** Dynamically remove the latest added bus.

        Request the removal of the last bus from the audio processor. If the
        audio processor does not support removing buses then this method will
        return false.

        Most audio processors will not allow you to dynamically add/remove
        audio buses and will return false.

        The default implementation will return false.

        This method will invoke the canApplyBusCountChange callback to probe if
        a bus can currently be removed and, if yes, will go ahead and remove it.

        @see addBus, canRemoveBus
    */
    bool removeBus (bool isInput);

    //==============================================================================
    /** Set the channel layouts of this audio processor.

        If the layout is not supported by this audio processor then
        this method will return false. You can use the checkBusesLayoutSupported
        and getNextBestLayout methods to probe which layouts this audio
        processor supports.
    */
    bool setBusesLayout (const BusesLayout&);

    /** Set the channel layouts of this audio processor without changing the
        enablement state of the buses.

        If the layout is not supported by this audio processor then
        this method will return false. You can use the checkBusesLayoutSupported
        methods to probe which layouts this audio processor supports.
    */
    bool setBusesLayoutWithoutEnabling (const BusesLayout&);

    /** Provides the current channel layouts of this audio processor. */
    BusesLayout getBusesLayout() const;

    /** Provides the channel layout of the bus with a given index and direction.

        If the index, direction combination is invalid then this will return an
        AudioChannelSet with no channels.
    */
    AudioChannelSet getChannelLayoutOfBus (bool isInput, int busIndex) const noexcept;

    /** Set the channel layout of the bus with a given index and direction.

        If the index, direction combination is invalid or the layout is not
        supported by the audio processor then this method will return false.
    */
    bool setChannelLayoutOfBus (bool isInput, int busIndex, const AudioChannelSet& layout);

    /** Provides the number of channels of the bus with a given index and direction.

        If the index, direction combination is invalid then this will return zero.
    */
    inline int getChannelCountOfBus (bool isInput, int busIndex) const noexcept
    {
        if (auto* bus = getBus (isInput, busIndex))
            return bus->getNumberOfChannels();

        return 0;
    }

    /** Enables all buses */
    bool enableAllBuses();

    /** Disables all non-main buses (aux and sidechains). */
    bool disableNonMainBuses();

    //==============================================================================
    /** Returns the position of a bus's channels within the processBlock buffer.
        This can be called in processBlock to figure out which channel of the master AudioBuffer
        maps onto a specific bus's channel.
     */
    int getChannelIndexInProcessBlockBuffer (bool isInput, int busIndex, int channelIndex) const noexcept;

    /** Returns the offset in a bus's buffer from an absolute channel indes.

        This method returns the offset in a bus's buffer given an absolute channel index.
        It also provides the bus index. For example, this method would return one
        for a processor with two stereo buses when given the absolute channel index.
    */
    int getOffsetInBusBufferForAbsoluteChannelIndex (bool isInput, int absoluteChannelIndex, int& busIndex) const noexcept;

    /** Returns an AudioBuffer containing a set of channel pointers for a specific bus.
        This can be called in processBlock to get a buffer containing a sub-group of the master
        AudioBuffer which contains all the plugin channels.
     */
    template <typename FloatType>
    AudioBuffer<FloatType> getBusBuffer (AudioBuffer<FloatType>& processBlockBuffer, bool isInput, int busIndex) const
    {
        auto busNumChannels = getChannelCountOfBus (isInput, busIndex);
        auto channelOffset = getChannelIndexInProcessBlockBuffer (isInput, busIndex, 0);

        return AudioBuffer<FloatType> (processBlockBuffer.getArrayOfWritePointers() + channelOffset,
                                       busNumChannels, processBlockBuffer.getNumSamples());
    }

    //==============================================================================
    /** Returns true if the Audio processor is likely to support a given layout.
        This can be called regardless if the processor is currently running.
    */
    bool checkBusesLayoutSupported (const BusesLayout&) const;

    //==============================================================================
    /** Returns true if the Audio processor supports double precision floating point processing.
        The default implementation will always return false.
        If you return true here then you must override the double precision versions
        of processBlock. Additionally, you must call getProcessingPrecision() in
        your prepareToPlay method to determine the precision with which you need to
        allocate your internal buffers.
        @see getProcessingPrecision, setProcessingPrecision
    */
    virtual bool supportsDoublePrecisionProcessing() const;

    /** Returns the precision-mode of the processor.
        Depending on the result of this method you MUST call the corresponding version
        of processBlock. The default processing precision is single precision.
        @see setProcessingPrecision, supportsDoublePrecisionProcessing
    */
    ProcessingPrecision getProcessingPrecision() const noexcept         { return processingPrecision; }

    /** Returns true if the current precision is set to doublePrecision. */
    bool isUsingDoublePrecision() const noexcept                        { return processingPrecision == doublePrecision; }

    /** Changes the processing precision of the receiver. A client of the AudioProcessor
        calls this function to indicate which version of processBlock (single or double
        precision) it intends to call. The client MUST call this function before calling
        the prepareToPlay method so that the receiver can do any necessary allocations
        in the prepareToPlay() method. An implementation of prepareToPlay() should call
        getProcessingPrecision() to determine with which precision it should allocate
        it's internal buffers.

        Note that setting the processing precision to double floating point precision
        on a receiver which does not support double precision processing (i.e.
        supportsDoublePrecisionProcessing() returns false) will result in an assertion.

        @see getProcessingPrecision, supportsDoublePrecisionProcessing
    */
    void setProcessingPrecision (ProcessingPrecision newPrecision) noexcept;

    //==============================================================================
    /** Returns the current AudioPlayHead object that should be used to find
        out the state and position of the playhead.

        You can ONLY call this from your processBlock() method! Calling it at other
        times will produce undefined behaviour, as the host may not have any context
        in which a time would make sense, and some hosts will almost certainly have
        multithreading issues if it's not called on the audio thread.

        The AudioPlayHead object that is returned can be used to get the details about
        the time of the start of the block currently being processed. But do not
        store this pointer or use it outside of the current audio callback, because
        the host may delete or re-use it.

        If the host can't or won't provide any time info, this will return nullptr.
    */
    AudioPlayHead* getPlayHead() const noexcept                 { return playHead; }

    //==============================================================================
    /** Returns the total number of input channels.

        This method will return the total number of input channels by accumulating
        the number of channels on each input bus. The number of channels of the
        buffer passed to your processBlock callback will be equivalent to either
        getTotalNumInputChannels or getTotalNumOutputChannels - which ever
        is greater.

        Note that getTotalNumInputChannels is equivalent to
        getMainBusNumInputChannels if your processor does not have any sidechains
        or aux buses.
     */
    int getTotalNumInputChannels()  const noexcept              { return cachedTotalIns; }

    /** Returns the total number of output channels.

        This method will return the total number of output channels by accumulating
        the number of channels on each output bus. The number of channels of the
        buffer passed to your processBlock callback will be equivalent to either
        getTotalNumInputChannels or getTotalNumOutputChannels - which ever
        is greater.

        Note that getTotalNumOutputChannels is equivalent to
        getMainBusNumOutputChannels if your processor does not have any sidechains
        or aux buses.
     */
    int getTotalNumOutputChannels() const noexcept              { return cachedTotalOuts; }

    /** Returns the number of input channels on the main bus. */
    inline int getMainBusNumInputChannels()   const noexcept    { return getChannelCountOfBus (true,  0); }

    /** Returns the number of output channels on the main bus. */
    inline int getMainBusNumOutputChannels()  const noexcept    { return getChannelCountOfBus (false, 0); }

    //==============================================================================
    /** Returns true if the channel layout map contains a certain layout.

        You can use this method to help you implement the checkBusesLayoutSupported
        method. For example

        @code
        bool checkBusesLayoutSupported (const BusesLayout& layouts) override
        {
            return containsLayout (layouts, {{1,1},{2,2}});
        }
        @endcode
    */
   #if JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS
    static bool containsLayout (const BusesLayout& layouts, const std::initializer_list<const short[2]>& channelLayoutList)
    {
        return containsLayout (layouts, layoutListToArray (channelLayoutList));
    }
   #endif

    template <int numLayouts>
    static bool containsLayout (const BusesLayout& layouts, const short (&channelLayoutList) [numLayouts][2])
    {
        return containsLayout (layouts, layoutListToArray (channelLayoutList));
    }

    /** Returns the next best layout which is contained in a channel layout map.

        You can use this mehtod to help you implement getNextBestLayout. For example:

        @code
        BusesLayout getNextBestLayout (const BusesLayout& layouts) override
        {
            return getNextBestLayoutInLayoutList (layouts, {{1,1},{2,2}});
        }
        @endcode
    */
    template <int numLayouts>
    BusesLayout getNextBestLayoutInLayoutList (const BusesLayout& layouts,
                                               const short (&channelLayoutList) [numLayouts][2])
    {
        return getNextBestLayoutInList (layouts, layoutListToArray (channelLayoutList));
    }

    //==============================================================================
    /** Returns the current sample rate.

        This can be called from your processBlock() method - it's not guaranteed
        to be valid at any other time, and may return 0 if it's unknown.
    */
    double getSampleRate() const noexcept                       { return currentSampleRate; }

    /** Returns the current typical block size that is being used.

        This can be called from your processBlock() method - it's not guaranteed
        to be valid at any other time.

        Remember it's not the ONLY block size that may be used when calling
        processBlock, it's just the normal one. The actual block sizes used may be
        larger or smaller than this, and will vary between successive calls.
    */
    int getBlockSize() const noexcept                           { return blockSize; }

    //==============================================================================

    /** This returns the number of samples delay that the processor imposes on the audio
        passing through it.

        The host will call this to find the latency - the processor itself should set this value
        by calling setLatencySamples() as soon as it can during its initialisation.
    */
    int getLatencySamples() const noexcept                      { return latencySamples; }

    /** Your processor subclass should call this to set the number of samples delay that it introduces.

        The processor should call this as soon as it can during initialisation, and can call it
        later if the value changes.
    */
    void setLatencySamples (int newLatency);

    /** Returns the length of the processor's tail, in seconds. */
    virtual double getTailLengthSeconds() const = 0;

    /** Returns true if the processor wants MIDI messages. */
    virtual bool acceptsMidi() const = 0;

    /** Returns true if the processor produces MIDI messages. */
    virtual bool producesMidi() const = 0;

    /** Returns true if the processor supports MPE. */
    virtual bool supportsMPE() const                            { return false; }

    /** Returns true if this is a MIDI effect plug-in and does no audio processing. */
    virtual bool isMidiEffect() const                           { return false; }

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
        processor will return an empty buffer, but won't block the audio thread like it would
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

        NOTE! This method will eventually be deprecated! It's recommended that you use the
        AudioProcessorParameter class instead to manage your parameters.
    */
    virtual int getNumParameters();

    /** Returns the name of a particular parameter.

        NOTE! This method will eventually be deprecated! It's recommended that you use the
        AudioProcessorParameter class instead to manage your parameters.
    */
    virtual const String getParameterName (int parameterIndex);

    /** Returns the ID of a particular parameter.

        The ID is used to communicate the value or mapping of a particular parameter with
        the host. By default this method will simply return a string representation of
        index.

        NOTE! This method will eventually be deprecated! It's recommended that you use the
        AudioProcessorParameterWithID class instead to manage your parameters.
     */
    virtual String getParameterID (int index);

    /** Called by the host to find out the value of one of the processor's parameters.

        The host will expect the value returned to be between 0 and 1.0.

        This could be called quite frequently, so try to make your code efficient.
        It's also likely to be called by non-UI threads, so the code in here should
        be thread-aware.

        NOTE! This method will eventually be deprecated! It's recommended that you use the
        AudioProcessorParameter class instead to manage your parameters.
    */
    virtual float getParameter (int parameterIndex);

    /** Returns the name of a parameter as a text string with a preferred maximum length.
        If you want to provide customised short versions of your parameter names that
        will look better in constrained spaces (e.g. the displays on hardware controller
        devices or mixing desks) then you should implement this method.
        If you don't override it, the default implementation will call getParameterName(int),
        and truncate the result.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getName() instead.
    */
    virtual String getParameterName (int parameterIndex, int maximumStringLength);

    /** Returns the value of a parameter as a text string.
        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getText() instead.
    */
    virtual const String getParameterText (int parameterIndex);

    /** Returns the value of a parameter as a text string with a preferred maximum length.
        If you want to provide customised short versions of your parameter values that
        will look better in constrained spaces (e.g. the displays on hardware controller
        devices or mixing desks) then you should implement this method.
        If you don't override it, the default implementation will call getParameterText(int),
        and truncate the result.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getText() instead.
    */
    virtual String getParameterText (int parameterIndex, int maximumStringLength);

    /** Returns the number of discrete steps that this parameter can represent.

        The default return value if you don't implement this method is
        AudioProcessor::getDefaultNumParameterSteps().

        If your parameter is boolean, then you may want to make this return 2.

        If you want the host to display stepped automation values, rather than a
        continuous interpolation between successive values, you should ensure that
        isParameterDiscrete returns true.

        The value that is returned may or may not be used, depending on the host.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getNumSteps() instead.

        @see isParameterDiscrete
    */
    virtual int getParameterNumSteps (int parameterIndex);

    /** Returns the default number of steps for a parameter.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getNumSteps() instead.

        @see getParameterNumSteps
    */
    static int getDefaultNumParameterSteps() noexcept;

    /** Returns true if the parameter should take discrete, rather than continuous
        values.

        If the parameter is boolean, this should return true (with getParameterNumSteps
        returning 2).

        The value that is returned may or may not be used, depending on the host.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::isDiscrete() instead.

        @see getParameterNumSteps
    */
    virtual bool isParameterDiscrete (int parameterIndex) const;

    /** Returns the default value for the parameter.
        By default, this just returns 0.
        The value that is returned may or may not be used, depending on the host.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getDefaultValue() instead.
    */
    virtual float getParameterDefaultValue (int parameterIndex);

    /** Some plugin types may be able to return a label string for a
        parameter's units.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getLabel() instead.
    */
    virtual String getParameterLabel (int index) const;

    /** This can be overridden to tell the host that particular parameters operate in the
        reverse direction. (Not all plugin formats or hosts will actually use this information).

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::isOrientationInverted() instead.
    */
    virtual bool isParameterOrientationInverted (int index) const;

    /** The host will call this method to change the value of one of the processor's parameters.

        The host may call this at any time, including during the audio processing
        callback, so the processor has to process this very fast and avoid blocking.

        If you want to set the value of a parameter internally, e.g. from your
        editor component, then don't call this directly - instead, use the
        setParameterNotifyingHost() method, which will also send a message to
        the host telling it about the change. If the message isn't sent, the host
        won't be able to automate your parameters properly.

        The value passed will be between 0 and 1.0.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::setValue() instead.
    */
    virtual void setParameter (int parameterIndex, float newValue);

    /** Your processor can call this when it needs to change one of its parameters.

        This could happen when the editor or some other internal operation changes
        a parameter. This method will call the setParameter() method to change the
        value, and will then send a message to the host telling it about the change.

        Note that to make sure the host correctly handles automation, you should call
        the beginParameterChangeGesture() and endParameterChangeGesture() methods to
        tell the host when the user has started and stopped changing the parameter.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::setValueNotifyingHost() instead.
    */
    void setParameterNotifyingHost (int parameterIndex, float newValue);

    /** Returns true if the host can automate this parameter.
        By default, this returns true for all parameters.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::isAutomatable() instead.
    */
    virtual bool isParameterAutomatable (int parameterIndex) const;

    /** Should return true if this parameter is a "meta" parameter.
        A meta-parameter is a parameter that changes other params. It is used
        by some hosts (e.g. AudioUnit hosts).
        By default this returns false.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::isMetaParameter() instead.
    */
    virtual bool isMetaParameter (int parameterIndex) const;

    /** Should return the parameter's category.
        By default, this returns the "generic" category.

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::getCategory() instead.
    */
    virtual AudioProcessorParameter::Category getParameterCategory (int parameterIndex) const;

    /** Sends a signal to the host to tell it that the user is about to start changing this
        parameter.

        This allows the host to know when a parameter is actively being held by the user, and
        it may use this information to help it record automation.

        If you call this, it must be matched by a later call to endParameterChangeGesture().

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::beginChangeGesture() instead.
    */
    void beginParameterChangeGesture (int parameterIndex);

    /** Tells the host that the user has finished changing this parameter.

        This allows the host to know when a parameter is actively being held by the user, and
        it may use this information to help it record automation.

        A call to this method must follow a call to beginParameterChangeGesture().

        NOTE! This method will eventually be deprecated! It's recommended that you use
        AudioProcessorParameter::endChangeGesture() instead.
    */
    void endParameterChangeGesture (int parameterIndex);

    /** The processor can call this when something (apart from a parameter value) has changed.

        It sends a hint to the host that something like the program, number of parameters,
        etc, has changed, and that it should update itself.
    */
    void updateHostDisplay();

    //==============================================================================
    /** Adds a parameter to the list.
        The parameter object will be managed and deleted automatically by the list
        when no longer needed.
    */
    void addParameter (AudioProcessorParameter*);

    /** Returns the current list of parameters. */
    const OwnedArray<AudioProcessorParameter>& getParameters() const noexcept;

    //==============================================================================
    /** Returns the number of preset programs the processor supports.

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
    /** The host will call this method when it wants to save the processor's internal state.

        This must copy any info about the processor's state into the block of memory provided,
        so that the host can store this and later restore it using setStateInformation().

        Note that there's also a getCurrentProgramStateInformation() method, which only
        stores the current program, not the state of the entire processor.

        See also the helper function copyXmlToBinary() for storing settings as XML.

        @see getCurrentProgramStateInformation
    */
    virtual void getStateInformation (juce::MemoryBlock& destData) = 0;

    /** The host will call this method if it wants to save the state of just the processor's
        current program.

        Unlike getStateInformation, this should only return the current program's state.

        Not all hosts support this, and if you don't implement it, the base class
        method just calls getStateInformation() instead. If you do implement it, be
        sure to also implement getCurrentProgramStateInformation.

        @see getStateInformation, setCurrentProgramStateInformation
    */
    virtual void getCurrentProgramStateInformation (juce::MemoryBlock& destData);

    /** This must restore the processor's state from a block of data previously created
        using getStateInformation().

        Note that there's also a setCurrentProgramStateInformation() method, which tries
        to restore just the current program, not the state of the entire processor.

        See also the helper function getXmlFromBinary() for loading settings as XML.

        @see setCurrentProgramStateInformation
    */
    virtual void setStateInformation (const void* data, int sizeInBytes) = 0;

    /** The host will call this method if it wants to restore the state of just the processor's
        current program.

        Not all hosts support this, and if you don't implement it, the base class
        method just calls setStateInformation() instead. If you do implement it, be
        sure to also implement getCurrentProgramStateInformation.

        @see setStateInformation, getCurrentProgramStateInformation
    */
    virtual void setCurrentProgramStateInformation (const void* data, int sizeInBytes);

    /** This method is called when the total number of input or output channels is changed. */
    virtual void numChannelsChanged();

    /** This method is called when the number of buses is changed. */
    virtual void numBusesChanged();

    /** This method is called when the layout of the audio processor changes. */
    virtual void processorLayoutsChanged();

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
    /** This is called by the processor to specify its details before being played. Use this
        version of the function if you are not interested in any sidechain and/or aux buses
        and do not care about the layout of channels. Otherwise use setRateAndBufferSizeDetails.*/
    void setPlayConfigDetails (int numIns, int numOuts, double sampleRate, int blockSize);

    /** This is called by the processor to specify its details before being played. You
        should call this function after having informed the processor about the channel
        and bus layouts via setBusesLayout.

        @see setBusesLayout
    */
    void setRateAndBufferSizeDetails (double sampleRate, int blockSize) noexcept;

    //==============================================================================
    /** AAX plug-ins need to report a unique "plug-in id" for every audio layout
        configuration that your AudioProcessor supports on the main bus. Override this
        function if you want your AudioProcessor to use a custom "plug-in id" (for example
        to stay backward compatible with older versions of JUCE).

        The default implementation will compute a unique integer from the input and output
        layout and add this value to the 4 character code 'jcaa' (for native AAX) or 'jyaa'
        (for AudioSuite plug-ins).
    */
    virtual int32 getAAXPluginIDForMainBusConfig (const AudioChannelSet& mainInputLayout,
                                                  const AudioChannelSet& mainOutputLayout,
                                                  bool idForAudioSuite) const;

    //==============================================================================
    /** Not for public use - this is called before deleting an editor component. */
    void editorBeingDeleted (AudioProcessorEditor*) noexcept;

    /** Flags to indicate the type of plugin context in which a processor is being used. */
    enum WrapperType
    {
        wrapperType_Undefined = 0,
        wrapperType_VST,
        wrapperType_VST3,
        wrapperType_AudioUnit,
        wrapperType_AudioUnitv3,
        wrapperType_RTAS,
        wrapperType_AAX,
        wrapperType_Standalone
    };

    /** When loaded by a plugin wrapper, this flag will be set to indicate the type
        of plugin within which the processor is running.
    */
    WrapperType wrapperType;

    /** A struct containing information about the DAW track inside which your
        AudioProcessor is loaded. */
    struct TrackProperties
    {
        String name;    // The name of the track - this will be empty if the track name is not known
        Colour colour;  // The colour of the track - this will be transparentBlack if the colour is not known

        // other properties may be added in the future
    };

    /** Informs the AudioProcessor that track properties such as the track's name or
        colour has been changed.

        If you are hosting this AudioProcessor then use this method to inform the
        AudioProcessor about which track the AudioProcessor is loaded on. This method
        may only be called on the message thread.

        If you are implemeting an AudioProcessor then you can override this callback
        to do something useful with the track properties such as changing the colour
        of your AudioProcessor's editor. It's entirely up to the host when and how
        often this callback will be called.

        The default implementation of this callback will do nothing.
    */
    virtual void updateTrackProperties (const TrackProperties& properties);

    //==============================================================================
   #ifndef DOXYGEN
    /** Deprecated: use getTotalNumInputChannels instead. */
    JUCE_DEPRECATED_WITH_BODY (int getNumInputChannels()  const noexcept, { return getTotalNumInputChannels(); })
    JUCE_DEPRECATED_WITH_BODY (int getNumOutputChannels() const noexcept, { return getTotalNumOutputChannels(); })

    /** Returns a string containing a whitespace-separated list of speaker types
        These functions are deprecated: use the methods provided in the AudioChannelSet
        class.
     */
    JUCE_DEPRECATED_WITH_BODY (const String getInputSpeakerArrangement()  const noexcept, { return cachedInputSpeakerArrString; })
    JUCE_DEPRECATED_WITH_BODY (const String getOutputSpeakerArrangement() const noexcept, { return cachedOutputSpeakerArrString; })

    /** Returns the name of one of the processor's input channels.

        These functions are deprecated: your audio processor can inform the host
        on channel layouts and names via the methods in the AudiobusLayout class.
     */
    JUCE_DEPRECATED (virtual const String getInputChannelName  (int channelIndex) const);
    JUCE_DEPRECATED (virtual const String getOutputChannelName (int channelIndex) const);

    /** Returns true if the specified channel is part of a stereo pair with its neighbour.

        These functions are deprecated: your audio processor should specify the audio
        channel pairing information by modifying the busLayout member variable in
        the constructor. */
    JUCE_DEPRECATED (virtual bool isInputChannelStereoPair  (int index) const);
    JUCE_DEPRECATED (virtual bool isOutputChannelStereoPair (int index) const);
   #endif

    //==============================================================================
    /** Helper function that just converts an xml element into a binary blob.

        Use this in your processor's getStateInformation() method if you want to
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
    /** Callback to query if the AudioProcessor supports a specific layout.

        This callback is called when the host probes the supported bus layouts via
        the checkBusesLayoutSupported method. You should override this callback if you
        would like to limit the layouts that your AudioProcessor supports. The default
        implementation will accept any layout. JUCE does basic sanity checks so that
        the provided layouts parameter will have the same number of buses as your
        AudioProcessor.

        @see checkBusesLayoutSupported
    */
    virtual bool isBusesLayoutSupported (const BusesLayout&) const          { return true; }

    /** Callback to check if a certain bus layout can now be applied

        Most subclasses will not need to override this method and should instead
        override the isBusesLayoutSupported callback to reject certain layout changes.

        This callback is called when the user requests a layout change. It will only be
        called if processing of the AudioProcessor has been stopped by a previous call to
        releaseResources or after the construction of the AudioProcessor. It will be called
        just before the actual layout change. By returning false you will abort the layout
        change and setBusesLayout will return false indicating that the layout change
        was not successful.

        The default implementation will simply call isBusesLayoutSupported.

        You only need to override this method if there is a chance that your AudioProcessor
        may not accept a layout although you have previously claimed to support it via the
        isBusesLayoutSupported callback. This can occur if your AudioProcessor's supported
        layouts depend on other plug-in parameters which may have changed since the last
        call to isBusesLayoutSupported, such as the format of an audio file which can be
        selected by the user in the AudioProcessor's editor. This callback gives the
        AudioProcessor a last chance to reject a layout if conditions have changed as it
        is always called just before the actual layout change.

        As it is never called while the AudioProcessor is processing audio, it can also
        be used for AudioProcessors which wrap other plug-in formats to apply the current
        layout to the underlying plug-in. This callback gives such AudioProcessors a
        chance to reject the layout change should an error occur with the underlying plug-in
        during the layout change.

        @see isBusesLayoutSupported, setBusesLayout
    */
    virtual bool canApplyBusesLayout (const BusesLayout& layouts) const     { return isBusesLayoutSupported (layouts); }

    //==============================================================================
    /** Structure used for AudioProcessor Callbacks */
    struct BusProperties
    {
        /** The name of the bus */
        String busName;

        /** The default layout of the bus */
        AudioChannelSet defaultLayout;

        /** Is this bus activated by default? */
        bool isActivatedByDefault;
    };

    struct BusesProperties
    {
        /** The layouts of the input buses */
        Array<BusProperties> inputLayouts;

        /** The layouts of the output buses */
        Array<BusProperties> outputLayouts;

        void addBus (bool isInput, const String& name, const AudioChannelSet& defaultLayout, bool isActivatedByDefault = true);

        BusesProperties withInput  (const String& name, const AudioChannelSet& defaultLayout, bool isActivatedByDefault = true) const;
        BusesProperties withOutput (const String& name, const AudioChannelSet& defaultLayout, bool isActivatedByDefault = true) const;
    };

    /** Callback to query if adding/removing buses currently possible.

        This callback is called when the host calls addBus or removeBus.
        Similar to canApplyBusesLayout, this callback is only called while
        the AudioProcessor is stopped and gives the processor a last
        chance to reject a requested bus change. It can also be used to apply
        the bus count change to an underlying wrapped plug-in.

        When adding a bus, isAddingBuses will be true and the plug-in is
        expected to fill out outNewBusProperties with the properties of the
        bus which will be created just after the succesful return of this callback.

        Implementations of AudioProcessor will rarely need to override this
        method. Only override this method if your processor supports adding
        and removing buses and if it needs more fine grain control over the
        naming of new buses or may reject bus number changes although canAddBus
        or canRemoveBus returned true.

        The default implementation will return false if canAddBus/canRemoveBus
        returns false (the default behavior). Otherwise, this method returns
        "Input #busIndex" for input buses and "Output #busIndex" for output buses
        where busIndex is the index for newly created buses. The default layout
        in this case will be the layout of the previous bus of the same direction.
    */
    virtual bool canApplyBusCountChange (bool isInput, bool isAddingBuses,
                                         BusProperties& outNewBusProperties);

    //==============================================================================
    /** @internal */
    AudioPlayHead* playHead = nullptr;

    /** @internal */
    void sendParamChangeMessageToListeners (int parameterIndex, float newValue);

private:
    //==============================================================================
    struct InOutChannelPair
    {
        int16 inChannels = 0, outChannels = 0;

        InOutChannelPair() noexcept {}
        InOutChannelPair (const InOutChannelPair& o) noexcept  : inChannels (o.inChannels), outChannels (o.outChannels) {}
        InOutChannelPair (int16 inCh, int16 outCh) noexcept    : inChannels (inCh), outChannels (outCh) {}
        InOutChannelPair (const int16 (&config)[2]) noexcept   : inChannels (config[0]), outChannels (config[1]) {}

        InOutChannelPair& operator= (const InOutChannelPair& o) noexcept    { inChannels = o.inChannels; outChannels = o.outChannels; return *this; }

        bool operator== (const InOutChannelPair& other) const noexcept
        {
            return other.inChannels == inChannels && other.outChannels == outChannels;
        }
    };

    template <int numLayouts>
    static Array<InOutChannelPair> layoutListToArray (const short (&configuration) [numLayouts][2])
    {
        Array<InOutChannelPair> layouts;

        for (int i = 0; i < numLayouts; ++i)
            layouts.add (InOutChannelPair (configuration[i]));

        return layouts;
    }

   #if JUCE_COMPILER_SUPPORTS_INITIALIZER_LISTS
    static Array<InOutChannelPair> layoutListToArray (const std::initializer_list<const short[2]>& configuration)
    {
        Array<InOutChannelPair> layouts;

        for (auto&& i : configuration)
            layouts.add (InOutChannelPair (i));

        return layouts;
    }
   #endif

    //==============================================================================
    static BusesProperties busesPropertiesFromLayoutArray (const Array<InOutChannelPair>&);

    BusesLayout getNextBestLayoutInList (const BusesLayout&, const Array<InOutChannelPair>&) const;
    static bool containsLayout (const BusesLayout&, const Array<InOutChannelPair>&);

    //==============================================================================
    void createBus (bool isInput, const BusProperties&);

    //==============================================================================
    Array<AudioProcessorListener*> listeners;
    Component::SafePointer<AudioProcessorEditor> activeEditor;
    double currentSampleRate = 0;
    int blockSize = 0, latencySamples = 0;
    bool suspended = false, nonRealtime = false;
    ProcessingPrecision processingPrecision = singlePrecision;
    CriticalSection callbackLock, listenerLock;

    friend class Bus;
    mutable OwnedArray<Bus> inputBuses, outputBuses;

    String cachedInputSpeakerArrString, cachedOutputSpeakerArrString;
    int cachedTotalIns = 0, cachedTotalOuts = 0;

    OwnedArray<AudioProcessorParameter> managedParameters;
    AudioProcessorParameter* getParamChecked (int) const noexcept;

   #if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
    BigInteger changingParams;
   #endif

   #if JUCE_DEBUG
    bool textRecursionCheck = false;
    bool shouldCheckParamsForDupeIDs = false;
    void checkForDupedParamIDs();
   #endif

    AudioProcessorListener* getListenerLocked (int) const noexcept;
    void updateSpeakerFormatStrings();
    bool applyBusLayouts (const BusesLayout&);
    void audioIOChanged (bool busNumberChanged, bool channelNumChanged);
    void getNextBestLayout (const BusesLayout&, BusesLayout&) const;

    template <typename floatType>
    void processBypassed (AudioBuffer<floatType>&, MidiBuffer&);

   #if JucePlugin_Build_VST3
    friend class JuceVST3EditController;
    friend class JuceVST3Component;
   #endif

    Atomic<int> vst3IsPlaying { 0 };

    // This method is no longer used - you can delete it from your AudioProcessor classes.
    JUCE_DEPRECATED_WITH_BODY (virtual bool silenceInProducesSilenceOut() const, { return false; })

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessor)
};

} // namespace juce
