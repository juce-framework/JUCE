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

static ThreadLocalValue<AudioProcessor::WrapperType> wrapperTypeBeingCreated;

void JUCE_CALLTYPE AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::WrapperType type)
{
    wrapperTypeBeingCreated = type;
}

AudioProcessor::AudioProcessor()
{
    initialise (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), false)
                                 .withOutput ("Output", AudioChannelSet::stereo(), false));
}

AudioProcessor::AudioProcessor(const BusesProperties& ioConfig)
{
    initialise (ioConfig);
}

void AudioProcessor::initialise (const BusesProperties& ioConfig)
{
    cachedTotalIns  = 0;
    cachedTotalOuts = 0;

    wrapperType = wrapperTypeBeingCreated.get();
    playHead = nullptr;
    currentSampleRate = 0;
    blockSize = 0;
    latencySamples = 0;

   #if JUCE_DEBUG
    textRecursionCheck = false;
   #endif

    suspended = false;
    nonRealtime = false;

    processingPrecision = singlePrecision;

    const int numInputBuses  = ioConfig.inputLayouts.size();
    const int numOutputBuses = ioConfig.outputLayouts.size();

    for (int i = 0; i < numInputBuses;  ++i)
        createBus (true,  ioConfig.inputLayouts. getReference (i));

    for (int i = 0; i < numOutputBuses; ++i)
        createBus (false, ioConfig.outputLayouts.getReference (i));

    updateSpeakerFormatStrings();
}

AudioProcessor::~AudioProcessor()
{
    // ooh, nasty - the editor should have been deleted before the filter
    // that it refers to is deleted..
    jassert (activeEditor == nullptr);

   #if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
    // This will fail if you've called beginParameterChangeGesture() for one
    // or more parameters without having made a corresponding call to endParameterChangeGesture...
    jassert (changingParams.countNumberOfSetBits() == 0);
   #endif
}

//==============================================================================
StringArray AudioProcessor::getAlternateDisplayNames() const     { return StringArray (getName()); }

//==============================================================================
bool AudioProcessor::addBus (bool isInput)
{
    if (! canAddBus (isInput))
        return false;

    BusProperties busesProps;
    if (! canApplyBusCountChange (isInput, true, busesProps))
        return false;

    createBus (isInput, busesProps);
    return true;
}

bool AudioProcessor::removeBus (bool inputBus)
{
    const int numBuses = getBusCount (inputBus);
    if (numBuses == 0)
        return false;

    if (! canRemoveBus (inputBus))
        return false;

    BusProperties busesProps;
    if (! canApplyBusCountChange (inputBus, false, busesProps))
        return false;

    const int busIdx = numBuses - 1;
    const int numChannels = getChannelCountOfBus (inputBus, busIdx);
    (inputBus ? inputBuses : outputBuses).remove (busIdx);

    audioIOChanged (true, numChannels > 0);

    return true;
}


//==============================================================================
bool AudioProcessor::setBusesLayout (const BusesLayout& arr)
{
    jassert (arr.inputBuses. size() == getBusCount (true)
          && arr.outputBuses.size() == getBusCount (false));

    if (arr == getBusesLayout())
        return true;

    BusesLayout copy = arr;
    if (! canApplyBusesLayout (copy))
        return false;

    return applyBusLayouts (copy);
}

bool AudioProcessor::setBusesLayoutWithoutEnabling (const BusesLayout& arr)
{
    const int numIns = getBusCount (true);
    const int numOuts = getBusCount (false);

    jassert (arr.inputBuses. size() == numIns
          && arr.outputBuses.size() == numOuts);

    BusesLayout request = arr;
    const BusesLayout current = getBusesLayout();

    for (int i = 0; i < numIns; ++i)
        if (request.getNumChannels (true, i) == 0)
            request.getChannelSet (true, i) = current.getChannelSet (true, i);

    for (int i = 0; i < numOuts; ++i)
        if (request.getNumChannels (false, i) == 0)
            request.getChannelSet (false, i) = current.getChannelSet (false, i);

    if (! checkBusesLayoutSupported(request))
        return false;

    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir != 0);

        for (int i = 0; i < (isInput ? numIns : numOuts); ++i)
        {
            Bus& bus = *getBus (isInput, i);
            AudioChannelSet& set = request.getChannelSet (isInput, i);

            if (! bus.isEnabled())
            {
                if (! set.isDisabled())
                    bus.lastLayout = set;

                set = AudioChannelSet::disabled();
            }
        }
    }

    return setBusesLayout (request);
}

AudioProcessor::BusesLayout AudioProcessor::getBusesLayout() const
{
    BusesLayout layouts;
    const int numInputs  = getBusCount (true);
    const int numOutputs = getBusCount (false);

    for (int i = 0; i < numInputs;  ++i)
        layouts.inputBuses. add (getBus (true,  i)->getCurrentLayout());

    for (int i = 0; i < numOutputs; ++i)
        layouts.outputBuses.add (getBus (false, i)->getCurrentLayout());

    return layouts;
}

AudioChannelSet AudioProcessor::getChannelLayoutOfBus (bool isInput, int busIdx) const noexcept
{
    const OwnedArray<Bus>& buses = (isInput ? inputBuses : outputBuses);
    if (Bus* bus = buses[busIdx])
        return bus->getCurrentLayout();

    return AudioChannelSet();
}

bool AudioProcessor::setChannelLayoutOfBus (bool isInputBus, int busIdx, const AudioChannelSet& layout)
{
    if (Bus* bus = getBus (isInputBus, busIdx))
    {
        BusesLayout layouts = bus->getBusesLayoutForLayoutChangeOfBus (layout);

        if (layouts.getChannelSet (isInputBus, busIdx) == layout)
            return applyBusLayouts (layouts);

        return false;
    }

    // busIdx parameter is invalid
    jassertfalse;

    return false;
}

bool AudioProcessor::enableAllBuses()
{
    BusesLayout layouts;
    const int numInputs  = getBusCount (true);
    const int numOutputs = getBusCount (false);

    for (int i = 0; i < numInputs;  ++i)
        layouts.inputBuses. add (getBus (true,  i)->lastLayout);

    for (int i = 0; i < numOutputs; ++i)
        layouts.outputBuses.add (getBus (false, i)->lastLayout);

    return setBusesLayout (layouts);
}

bool AudioProcessor::checkBusesLayoutSupported (const BusesLayout& layouts) const
{
    const int numInputBuses  = getBusCount (true);
    const int numOutputBuses = getBusCount (false);

    if (layouts.inputBuses. size() == numInputBuses
     && layouts.outputBuses.size() == numOutputBuses)
        return isBusesLayoutSupported (layouts);

    return false;
}

void AudioProcessor::getNextBestLayout (const BusesLayout& desiredLayout, BusesLayout& actualLayouts) const
{
    // if you are hitting this assertion then you are requesting a next
    // best layout which does not have the same number of buses as the
    // audio processor.
    jassert (desiredLayout.inputBuses. size() == getBusCount (true)
          && desiredLayout.outputBuses.size() == getBusCount (false));

    if (checkBusesLayoutSupported (desiredLayout))
    {
        actualLayouts = desiredLayout;
        return;
    }

    BusesLayout originalState = actualLayouts;
    BusesLayout currentState = originalState;
    BusesLayout bestSupported = currentState;

    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir > 0);

        Array<AudioChannelSet>& currentLayouts         = (isInput ? currentState.inputBuses  : currentState.outputBuses);
        const Array<AudioChannelSet>& bestLayouts      = (isInput ? bestSupported.inputBuses : bestSupported.outputBuses);
        const Array<AudioChannelSet>& requestedLayouts = (isInput ? desiredLayout.inputBuses : desiredLayout.outputBuses);
        const Array<AudioChannelSet>& originalLayouts  = (isInput ? originalState.inputBuses : originalState.outputBuses);

        for (int busIdx = 0; busIdx < requestedLayouts.size(); ++busIdx)
        {
            AudioChannelSet& best            = bestLayouts     .getReference (busIdx);
            const AudioChannelSet& requested = requestedLayouts.getReference (busIdx);
            const AudioChannelSet& original  = originalLayouts .getReference (busIdx);

            // do we need to do anything
            if (original == requested)
                continue;

            currentState = bestSupported;
            AudioChannelSet& current = currentLayouts  .getReference (busIdx);

            // already supported?
            current = requested;
            if (checkBusesLayoutSupported (currentState))
            {
                bestSupported = currentState;
                continue;
            }

            // try setting the opposite bus to the identical layout
            const bool oppositeDirection = ! isInput;
            if (getBusCount (oppositeDirection) > busIdx)
            {
                AudioChannelSet& oppositeLayout = (oppositeDirection ? currentState.inputBuses : currentState.outputBuses).getReference (busIdx);
                oppositeLayout = requested;

                if (checkBusesLayoutSupported (currentState))
                {
                    bestSupported = currentState;
                    continue;
                }

                // try setting the default layout
                oppositeLayout = getBus (oppositeDirection, busIdx)->getDefaultLayout();
                if (checkBusesLayoutSupported (currentState))
                {
                    bestSupported = currentState;
                    continue;
                }
            }

            // try setting all other buses to the identical layout
            BusesLayout allTheSame;
            for (int oDir = 0; oDir < 2; ++oDir)
            {
                const bool oIsInput = (oDir == 0);
                const int oBusNum = getBusCount (oIsInput);

                for (int oBusIdx = 0; oBusIdx < oBusNum; ++oBusIdx)
                    (oIsInput ? allTheSame.inputBuses : allTheSame.outputBuses).add (requested);
            }

            if (checkBusesLayoutSupported (allTheSame))
            {
                bestSupported = allTheSame;
                continue;
            }

            // what is closer the default or the current layout?
            int distance = abs (best.size() - requested.size());
            const AudioChannelSet& defaultLayout = getBus (isInput, busIdx)->getDefaultLayout();

            if (abs (defaultLayout.size() - requested.size()) < distance)
            {
                current = defaultLayout;
                if (checkBusesLayoutSupported (currentState))
                    bestSupported = currentState;
            }
        }
    }

    actualLayouts = bestSupported;
}

//==============================================================================
void AudioProcessor::setPlayHead (AudioPlayHead* const newPlayHead)
{
    playHead = newPlayHead;
}

void AudioProcessor::addListener (AudioProcessorListener* const newListener)
{
    const ScopedLock sl (listenerLock);
    listeners.addIfNotAlreadyThere (newListener);
}

void AudioProcessor::removeListener (AudioProcessorListener* const listenerToRemove)
{
    const ScopedLock sl (listenerLock);
    listeners.removeFirstMatchingValue (listenerToRemove);
}

void AudioProcessor::setPlayConfigDetails (const int newNumIns,
                                           const int newNumOuts,
                                           const double newSampleRate,
                                           const int newBlockSize)
{
    bool success = true;

    if (getTotalNumInputChannels()  != newNumIns)
        success &= setChannelLayoutOfBus (true,  0, AudioChannelSet::canonicalChannelSet (newNumIns));

    if (getTotalNumOutputChannels() != newNumOuts)
        success &= setChannelLayoutOfBus (false, 0, AudioChannelSet::canonicalChannelSet (newNumOuts));

    // if the user is using this method then they do not want any side-buses or aux outputs
    success &= disableNonMainBuses();
    jassert (success);

    // the processor may not support this arrangement at all
    jassert (success && newNumIns == getTotalNumInputChannels() && newNumOuts == getTotalNumOutputChannels());

    setRateAndBufferSizeDetails (newSampleRate, newBlockSize);
}

void AudioProcessor::setRateAndBufferSizeDetails (double newSampleRate, int newBlockSize) noexcept
{
    currentSampleRate = newSampleRate;
    blockSize = newBlockSize;
}

//==============================================================================
static int countTotalChannels (const OwnedArray<AudioProcessor::Bus>& buses) noexcept
{
    int n = 0;

    for (int i = 0; i < buses.size(); ++i)
        n += buses[i]->getNumberOfChannels();

    return n;
}

void AudioProcessor::numChannelsChanged()      {}
void AudioProcessor::numBusesChanged()         {}
void AudioProcessor::processorLayoutsChanged() {}

int AudioProcessor::getChannelIndexInProcessBlockBuffer (bool isInput, int busIndex, int channelIndex) const noexcept
{
    const OwnedArray<Bus>& ioBus = isInput ? inputBuses : outputBuses;
    jassert (isPositiveAndBelow(busIndex, ioBus.size()));

    for (int i = 0; i < ioBus.size() && i < busIndex; ++i)
        channelIndex += getChannelCountOfBus (isInput, i);

    return channelIndex;
}

int AudioProcessor::getOffsetInBusBufferForAbsoluteChannelIndex (bool isInput, int absoluteChannelIndex, /*out*/ int& busIdx) const noexcept
{
    const int n = getBusCount (isInput);
    int numChannels = 0;

    for (busIdx = 0; busIdx < n && absoluteChannelIndex >= (numChannels = getChannelLayoutOfBus (isInput, busIdx).size()); ++busIdx)
        absoluteChannelIndex -= numChannels;

    return busIdx >= n ? -1 : absoluteChannelIndex;
}

//==============================================================================
void AudioProcessor::setNonRealtime (const bool newNonRealtime) noexcept
{
    nonRealtime = newNonRealtime;
}

void AudioProcessor::setLatencySamples (const int newLatency)
{
    if (latencySamples != newLatency)
    {
        latencySamples = newLatency;
        updateHostDisplay();
    }
}

void AudioProcessor::setParameterNotifyingHost (const int parameterIndex,
                                                const float newValue)
{
    setParameter (parameterIndex, newValue);
    sendParamChangeMessageToListeners (parameterIndex, newValue);
}

AudioProcessorListener* AudioProcessor::getListenerLocked (const int index) const noexcept
{
    const ScopedLock sl (listenerLock);
    return listeners [index];
}

void AudioProcessor::sendParamChangeMessageToListeners (const int parameterIndex, const float newValue)
{
    if (isPositiveAndBelow (parameterIndex, getNumParameters()))
    {
        for (int i = listeners.size(); --i >= 0;)
            if (auto* l = getListenerLocked (i))
                l->audioProcessorParameterChanged (this, parameterIndex, newValue);
    }
    else
    {
        jassertfalse; // called with an out-of-range parameter index!
    }
}

void AudioProcessor::beginParameterChangeGesture (int parameterIndex)
{
    if (isPositiveAndBelow (parameterIndex, getNumParameters()))
    {
       #if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
        // This means you've called beginParameterChangeGesture twice in succession without a matching
        // call to endParameterChangeGesture. That might be fine in most hosts, but better to avoid doing it.
        jassert (! changingParams [parameterIndex]);
        changingParams.setBit (parameterIndex);
       #endif

        for (int i = listeners.size(); --i >= 0;)
            if (auto* l = getListenerLocked (i))
                l->audioProcessorParameterChangeGestureBegin (this, parameterIndex);
    }
    else
    {
        jassertfalse; // called with an out-of-range parameter index!
    }
}

void AudioProcessor::endParameterChangeGesture (int parameterIndex)
{
    if (isPositiveAndBelow (parameterIndex, getNumParameters()))
    {
       #if JUCE_DEBUG && ! JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING
        // This means you've called endParameterChangeGesture without having previously called
        // beginParameterChangeGesture. That might be fine in most hosts, but better to keep the
        // calls matched correctly.
        jassert (changingParams [parameterIndex]);
        changingParams.clearBit (parameterIndex);
       #endif

        for (int i = listeners.size(); --i >= 0;)
            if (AudioProcessorListener* l = getListenerLocked (i))
                l->audioProcessorParameterChangeGestureEnd (this, parameterIndex);
    }
    else
    {
        jassertfalse; // called with an out-of-range parameter index!
    }
}

void AudioProcessor::updateHostDisplay()
{
    for (int i = listeners.size(); --i >= 0;)
        if (AudioProcessorListener* l = getListenerLocked (i))
            l->audioProcessorChanged (this);
}

const OwnedArray<AudioProcessorParameter>& AudioProcessor::getParameters() const noexcept
{
    return managedParameters;
}

int AudioProcessor::getNumParameters()
{
    return managedParameters.size();
}

float AudioProcessor::getParameter (int index)
{
    if (auto* p = getParamChecked (index))
        return p->getValue();

    return 0;
}

void AudioProcessor::setParameter (int index, float newValue)
{
    if (auto* p = getParamChecked (index))
        p->setValue (newValue);
}

float AudioProcessor::getParameterDefaultValue (int index)
{
    if (auto* p = managedParameters[index])
        return p->getDefaultValue();

    return 0;
}

const String AudioProcessor::getParameterName (int index)
{
    if (auto* p = getParamChecked (index))
        return p->getName (512);

    return {};
}

String AudioProcessor::getParameterID (int index)
{
    // Don't use getParamChecked here, as this must also work for legacy plug-ins
    if (auto* p = dynamic_cast<AudioProcessorParameterWithID*> (managedParameters[index]))
        return p->paramID;

    return String (index);
}

String AudioProcessor::getParameterName (int index, int maximumStringLength)
{
    if (auto* p = managedParameters[index])
        return p->getName (maximumStringLength);

    return getParameterName (index).substring (0, maximumStringLength);
}

const String AudioProcessor::getParameterText (int index)
{
   #if JUCE_DEBUG
    // if you hit this, then you're probably using the old parameter control methods,
    // but have forgotten to implement either of the getParameterText() methods.
    jassert (! textRecursionCheck);
    ScopedValueSetter<bool> sv (textRecursionCheck, true, false);
   #endif

    return getParameterText (index, 1024);
}

String AudioProcessor::getParameterText (int index, int maximumStringLength)
{
    if (auto* p = managedParameters[index])
        return p->getText (p->getValue(), maximumStringLength);

    return getParameterText (index).substring (0, maximumStringLength);
}

int AudioProcessor::getParameterNumSteps (int index)
{
    if (auto* p = managedParameters[index])
        return p->getNumSteps();

    return AudioProcessor::getDefaultNumParameterSteps();
}

int AudioProcessor::getDefaultNumParameterSteps() noexcept
{
    return 0x7fffffff;
}

bool AudioProcessor::isParameterDiscrete (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isDiscrete();

    return false;
}

String AudioProcessor::getParameterLabel (int index) const
{
    if (auto* p = managedParameters[index])
        return p->getLabel();

    return {};
}

bool AudioProcessor::isParameterAutomatable (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isAutomatable();

    return true;
}

bool AudioProcessor::isParameterOrientationInverted (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isOrientationInverted();

    return false;
}

bool AudioProcessor::isMetaParameter (int index) const
{
    if (auto* p = managedParameters[index])
        return p->isMetaParameter();

    return false;
}

AudioProcessorParameter::Category AudioProcessor::getParameterCategory (int index) const
{
    if (auto* p = managedParameters[index])
        return p->getCategory();

    return AudioProcessorParameter::genericParameter;
}

AudioProcessorParameter* AudioProcessor::getParamChecked (int index) const noexcept
{
    AudioProcessorParameter* p = managedParameters[index];

    // If you hit this, then you're either trying to access parameters that are out-of-range,
    // or you're not using addParameter and the managed parameter list, but have failed
    // to override some essential virtual methods and implement them appropriately.
    jassert (p != nullptr);
    return p;
}

void AudioProcessor::addParameter (AudioProcessorParameter* p)
{
    p->processor = this;
    p->parameterIndex = managedParameters.size();
    managedParameters.add (p);

    // if you're using parameter objects, then you must not override the
    // deprecated getNumParameters() method!
    jassert (getNumParameters() == AudioProcessor::getNumParameters());

    // check that no two parameters have the same id
   #ifdef JUCE_DEBUG
    auto paramId = getParameterID (p->parameterIndex);

    for (auto q : managedParameters)
    {
        jassert (q == nullptr || q == p || paramId != getParameterID (q->parameterIndex));
    }
   #endif
}

void AudioProcessor::suspendProcessing (const bool shouldBeSuspended)
{
    const ScopedLock sl (callbackLock);
    suspended = shouldBeSuspended;
}

void AudioProcessor::reset() {}

template <typename floatType>
void AudioProcessor::processBypassed (AudioBuffer<floatType>& buffer, MidiBuffer&)
{
    for (int ch = getMainBusNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

void AudioProcessor::processBlockBypassed (AudioBuffer<float>&  buffer, MidiBuffer& midi)    { processBypassed (buffer, midi); }
void AudioProcessor::processBlockBypassed (AudioBuffer<double>& buffer, MidiBuffer& midi)    { processBypassed (buffer, midi); }

void AudioProcessor::processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages)
{
    ignoreUnused (buffer, midiMessages);

    // If you hit this assertion then either the caller called the double
    // precision version of processBlock on a processor which does not support it
    // (i.e. supportsDoublePrecisionProcessing() returns false), or the implementation
    // of the AudioProcessor forgot to override the double precision version of this method
    jassertfalse;
}

bool AudioProcessor::supportsDoublePrecisionProcessing() const
{
    return false;
}

void AudioProcessor::setProcessingPrecision (ProcessingPrecision precision) noexcept
{
    // If you hit this assertion then you're trying to use double precision
    // processing on a processor which does not support it!
    jassert (precision != doublePrecision || supportsDoublePrecisionProcessing());

    processingPrecision = precision;
}

//==============================================================================
static String getChannelName (const OwnedArray<AudioProcessor::Bus>& buses, int index)
{
    return buses.size() > 0 ? AudioChannelSet::getChannelTypeName (buses[0]->getCurrentLayout().getTypeOfChannel (index)) : String();
}

const String AudioProcessor::getInputChannelName (int index) const   { return getChannelName (inputBuses,  index); }
const String AudioProcessor::getOutputChannelName (int index) const  { return getChannelName (outputBuses, index); }

static bool isStereoPair (const OwnedArray<AudioProcessor::Bus>& buses, int index)
{
    return index < 2
            && buses.size() > 0
            && buses[0]->getCurrentLayout() == AudioChannelSet::stereo();
}

bool AudioProcessor::isInputChannelStereoPair  (int index) const    { return isStereoPair (inputBuses, index); }
bool AudioProcessor::isOutputChannelStereoPair (int index) const    { return isStereoPair (outputBuses, index); }

//==============================================================================
void AudioProcessor::createBus (bool inputBus, const BusProperties& ioConfig)
{
    (inputBus ? inputBuses : outputBuses).add (new Bus (*this, ioConfig.busName, ioConfig.defaultLayout, ioConfig.isActivatedByDefault));

    audioIOChanged (true, ioConfig.isActivatedByDefault);
}

//==============================================================================
AudioProcessor::BusesProperties AudioProcessor::busesPropertiesFromLayoutArray (const Array<InOutChannelPair>& config)
{
    BusesProperties ioProps;

    if (config[0].inChannels > 0)
        ioProps.addBus (true, String ("Input"), AudioChannelSet::canonicalChannelSet (config[0].inChannels));

    if (config[0].outChannels > 0)
        ioProps.addBus (false, String ("Output"), AudioChannelSet::canonicalChannelSet (config[0].outChannels));

    return ioProps;
}

AudioProcessor::BusesLayout AudioProcessor::getNextBestLayoutInList (const BusesLayout& layouts,
                                                                     const Array<InOutChannelPair>& legacyLayouts) const
{
    const int numChannelConfigs = legacyLayouts.size();
    jassert (numChannelConfigs > 0);

    bool hasInputs = false, hasOutputs = false;

    for (int i = 0; i < numChannelConfigs; ++i)
    {
        if (legacyLayouts[i].inChannels > 0)
        {
            hasInputs = true;
            break;
        }
    }

    for (int i = 0; i < numChannelConfigs; ++i)
    {
        if (legacyLayouts[i].outChannels > 0)
        {
            hasOutputs = true;
            break;
        }
    }

    BusesLayout nearest = layouts;
    nearest.inputBuses .resize (hasInputs  ? 1 : 0);
    nearest.outputBuses.resize (hasOutputs ? 1 : 0);

    AudioChannelSet* inBus  = (hasInputs  ? &nearest.inputBuses. getReference (0) : nullptr);
    AudioChannelSet* outBus = (hasOutputs ? &nearest.outputBuses.getReference (0) : nullptr);

    const int16 inNumChannelsRequested  = static_cast<int16> (inBus  != nullptr ? inBus->size()  : 0);
    const int16 outNumChannelsRequested = static_cast<int16> (outBus != nullptr ? outBus->size() : 0);

    int32 distance = std::numeric_limits<int32>::max();
    int bestConfiguration = 0;

    for (int i = 0; i < numChannelConfigs; ++i)
    {
        const int16 inChannels  = legacyLayouts.getReference (i).inChannels;
        const int16 outChannels = legacyLayouts.getReference (i).outChannels;

        const int32 channelDifference = ((std::abs (inChannels  - inNumChannelsRequested)  & 0xffff) << 16) |
        ((std::abs (outChannels - outNumChannelsRequested) & 0xffff) << 0);

        if (channelDifference < distance)
        {
            distance = channelDifference;
            bestConfiguration = i;

            // we can exit if we found a perfect match
            if (distance == 0) return nearest;
        }
    }

    const int16 inChannels  = legacyLayouts.getReference (bestConfiguration).inChannels;
    const int16 outChannels = legacyLayouts.getReference (bestConfiguration).outChannels;

    BusesLayout currentState = getBusesLayout();
    AudioChannelSet currentInLayout  = (getBusCount (true)  > 0 ? currentState.inputBuses .getReference(0) : AudioChannelSet());
    AudioChannelSet currentOutLayout = (getBusCount (false) > 0 ? currentState.outputBuses.getReference(0) : AudioChannelSet());


    if (inBus != nullptr)
    {
        if      (inChannels == 0)                       *inBus = AudioChannelSet::disabled();
        else if (inChannels == currentInLayout. size()) *inBus = currentInLayout;
        else if (inChannels == currentOutLayout.size()) *inBus = currentOutLayout;
        else                                            *inBus = AudioChannelSet::canonicalChannelSet (inChannels);
    }

    if (outBus != nullptr)
    {
        if      (outChannels == 0)                       *outBus = AudioChannelSet::disabled();
        else if (outChannels == currentOutLayout.size()) *outBus = currentOutLayout;
        else if (outChannels == currentInLayout .size()) *outBus = currentInLayout;
        else                                             *outBus = AudioChannelSet::canonicalChannelSet (outChannels);
    }

    return nearest;
}

bool AudioProcessor::containsLayout (const BusesLayout& layouts, const Array<InOutChannelPair>& channelLayouts)
{
    if (layouts.inputBuses.size() > 1 || layouts.outputBuses.size() > 1)
        return false;

    const InOutChannelPair mainLayout (static_cast<int16> (layouts.getNumChannels (true, 0)),
                                       static_cast<int16> (layouts.getNumChannels (false, 0)));

    return channelLayouts.contains (mainLayout);
}

//==============================================================================
bool AudioProcessor::disableNonMainBuses()
{
    BusesLayout layouts = getBusesLayout();

    for (int busIdx = 1; busIdx < layouts.inputBuses.size(); ++busIdx)
        layouts.inputBuses.getReference (busIdx) = AudioChannelSet::disabled();

    for (int busIdx = 1; busIdx < layouts.outputBuses.size(); ++busIdx)
        layouts.outputBuses.getReference (busIdx) = AudioChannelSet::disabled();

    return setBusesLayout (layouts);
}

// Unfortunately the deprecated getInputSpeakerArrangement/getOutputSpeakerArrangement return
// references to strings. Therefore we need to keep a copy. Once getInputSpeakerArrangement is
// removed, we can also remove this function
void AudioProcessor::updateSpeakerFormatStrings()
{
    cachedInputSpeakerArrString.clear();
    cachedOutputSpeakerArrString.clear();

    if (getBusCount (true) > 0)
        cachedInputSpeakerArrString  = getBus (true,  0)->getCurrentLayout().getSpeakerArrangementAsString();

    if (getBusCount (false) > 0)
        cachedOutputSpeakerArrString = getBus (false, 0)->getCurrentLayout().getSpeakerArrangementAsString();
}

bool AudioProcessor::applyBusLayouts (const BusesLayout& layouts)
{
    if (layouts == getBusesLayout())
        return true;

    const int numInputBuses  = getBusCount (true);
    const int numOutputBuses = getBusCount (false);

    const int oldNumberOfIns  = getTotalNumInputChannels();
    const int oldNumberOfOuts = getTotalNumOutputChannels();

    if (layouts.inputBuses. size() != numInputBuses
     || layouts.outputBuses.size() != numOutputBuses)
        return false;

    int newNumberOfIns = 0, newNumberOfOuts = 0;

    for (int busIdx = 0; busIdx < numInputBuses;  ++busIdx)
    {
        Bus& bus = *getBus (true, busIdx);
        const AudioChannelSet& set = layouts.getChannelSet (true, busIdx);

        bus.layout = set;
        if (! set.isDisabled())
            bus.lastLayout = set;

        newNumberOfIns += set.size();
    }

    for (int busIdx = 0; busIdx < numOutputBuses;  ++busIdx)
    {
        Bus& bus = *getBus (false, busIdx);
        const AudioChannelSet& set = layouts.getChannelSet (false, busIdx);

        bus.layout = set;
        if (! set.isDisabled())
            bus.lastLayout = set;

        newNumberOfOuts += set.size();
    }

    const bool channelNumChanged = (oldNumberOfIns != newNumberOfIns || oldNumberOfOuts != newNumberOfOuts);
    audioIOChanged (false, channelNumChanged);

    return true;
}

void AudioProcessor::audioIOChanged (bool busNumberChanged, bool channelNumChanged)
{
    const int numInputBuses  = getBusCount (true);
    const int numOutputBuses = getBusCount (false);

    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir == 0);
        const int n = (isInput ? numInputBuses : numOutputBuses);

        for (int i = 0; i < n; ++i)
        {
            if (Bus* bus = getBus (isInput, i))
                bus->updateChannelCount();
        }
    }

    cachedTotalIns  = countTotalChannels (inputBuses);
    cachedTotalOuts = countTotalChannels (outputBuses);

    updateSpeakerFormatStrings();

    if (busNumberChanged)
        numBusesChanged();

    if (channelNumChanged)
        numChannelsChanged();

    processorLayoutsChanged();
}

//==============================================================================
void AudioProcessor::editorBeingDeleted (AudioProcessorEditor* const editor) noexcept
{
    const ScopedLock sl (callbackLock);

    if (activeEditor == editor)
        activeEditor = nullptr;
}

AudioProcessorEditor* AudioProcessor::createEditorIfNeeded()
{
    if (activeEditor != nullptr)
        return activeEditor;

    AudioProcessorEditor* const ed = createEditor();

    if (ed != nullptr)
    {
        // you must give your editor comp a size before returning it..
        jassert (ed->getWidth() > 0 && ed->getHeight() > 0);

        const ScopedLock sl (callbackLock);
        activeEditor = ed;
    }

    // You must make your hasEditor() method return a consistent result!
    jassert (hasEditor() == (ed != nullptr));

    return ed;
}

//==============================================================================
void AudioProcessor::getCurrentProgramStateInformation (juce::MemoryBlock& destData)
{
    getStateInformation (destData);
}

void AudioProcessor::setCurrentProgramStateInformation (const void* data, int sizeInBytes)
{
    setStateInformation (data, sizeInBytes);
}

//==============================================================================
void AudioProcessor::updateTrackProperties (const AudioProcessor::TrackProperties&)    {}

//==============================================================================
// magic number to identify memory blocks that we've stored as XML
const uint32 magicXmlNumber = 0x21324356;

void AudioProcessor::copyXmlToBinary (const XmlElement& xml, juce::MemoryBlock& destData)
{
    {
        MemoryOutputStream out (destData, false);
        out.writeInt (magicXmlNumber);
        out.writeInt (0);
        xml.writeToStream (out, String(), true, false);
        out.writeByte (0);
    }

    // go back and write the string length..
    static_cast<uint32*> (destData.getData())[1]
        = ByteOrder::swapIfBigEndian ((uint32) destData.getSize() - 9);
}

XmlElement* AudioProcessor::getXmlFromBinary (const void* data, const int sizeInBytes)
{
    if (sizeInBytes > 8
         && ByteOrder::littleEndianInt (data) == magicXmlNumber)
    {
        const int stringLength = (int) ByteOrder::littleEndianInt (addBytesToPointer (data, 4));

        if (stringLength > 0)
            return XmlDocument::parse (String::fromUTF8 (static_cast<const char*> (data) + 8,
                                                         jmin ((sizeInBytes - 8), stringLength)));
    }

    return nullptr;
}

bool AudioProcessor::canApplyBusCountChange (bool isInput, bool isAdding,
                                             AudioProcessor::BusProperties& outProperties)
{
    if (  isAdding && ! canAddBus    (isInput)) return false;
    if (! isAdding && ! canRemoveBus (isInput)) return false;

    const int num = getBusCount (isInput);

    // No way for me to find out the default layout if there are no other busses!!
    if (num == 0) return false;

    if (isAdding)
    {
        outProperties.busName = String (isInput ? "Input #" : "Output #") + String (getBusCount (isInput));
        outProperties.defaultLayout = (num > 0 ? getBus (isInput, num - 1)->getDefaultLayout() : AudioChannelSet());
        outProperties.isActivatedByDefault = true;
    }

    return true;
}

//==============================================================================
AudioProcessor::Bus::Bus (AudioProcessor& processor, const String& busName,
                                                      const AudioChannelSet& defaultLayout, bool isDfltEnabled)
    : owner (processor), name (busName),
      layout (isDfltEnabled ? defaultLayout : AudioChannelSet()),
      dfltLayout (defaultLayout), lastLayout (defaultLayout),
      enabledByDefault (isDfltEnabled)
{
    // Your default layout cannot be disabled
    jassert (! dfltLayout.isDisabled());
}

bool AudioProcessor::Bus::isInput() const
{
    return owner.inputBuses.contains (this);
}

int AudioProcessor::Bus::getBusIndex() const
{
    bool ignore;
    int idx;
    busDirAndIndex (ignore, idx);

    return idx;
}

void AudioProcessor::Bus::busDirAndIndex (bool& input, int& idx) const noexcept
{
    idx = owner.inputBuses.indexOf (this);
    input = (idx >= 0);

    if (! input)
        idx = owner.outputBuses.indexOf (this);
}

bool AudioProcessor::Bus::setCurrentLayout (const AudioChannelSet& busLayout)
{
    bool isInput;
    int idx;
    busDirAndIndex (isInput, idx);

    return owner.setChannelLayoutOfBus (isInput, idx, busLayout);
}

bool AudioProcessor::Bus::setCurrentLayoutWithoutEnabling (const AudioChannelSet& set)
{
    if (! set.isDisabled())
    {
        if (isEnabled())
            return setCurrentLayout (set);

        if (isLayoutSupported (set))
        {
            lastLayout = set;
            return true;
        }

        return false;
    }

    return isLayoutSupported (set);
}

bool AudioProcessor::Bus::setNumberOfChannels (int channels)
{
    bool isInputBus;
    int busIdx;
    busDirAndIndex (isInputBus, busIdx);

    if (owner.setChannelLayoutOfBus (isInputBus, busIdx, AudioChannelSet::canonicalChannelSet (channels)))
        return true;

    if (channels == 0)
        return false;

    AudioChannelSet namedSet = AudioChannelSet::namedChannelSet (channels);
    if (! namedSet.isDisabled() && owner.setChannelLayoutOfBus (isInputBus, busIdx, namedSet))
        return true;

    return owner.setChannelLayoutOfBus (isInputBus, busIdx, AudioChannelSet::discreteChannels (channels));
}

bool AudioProcessor::Bus::enable (bool shouldEnable)
{
    if (isEnabled() == shouldEnable)
        return true;

    return setCurrentLayout (shouldEnable ? lastLayout : AudioChannelSet::disabled());
}

int AudioProcessor::Bus::getMaxSupportedChannels (int limit) const
{
    for (int ch = limit; ch > 0; --ch)
        if (isNumberOfChannelsSupported (ch))
            return ch;

    return (isMain() && isLayoutSupported (AudioChannelSet::disabled())) ? 0 : -1;
}

bool AudioProcessor::Bus::isLayoutSupported (const AudioChannelSet& set, BusesLayout* ioLayout) const
{
    bool isInputBus;
    int busIdx;
    busDirAndIndex (isInputBus, busIdx);

    // check that supplied ioLayout is actually valid
    if (ioLayout != nullptr)
    {
        bool suppliedCurrentSupported = owner.checkBusesLayoutSupported (*ioLayout);

        if (! suppliedCurrentSupported)
        {
            *ioLayout = owner.getBusesLayout();

            // the current layout you supplied is not a valid layout
            jassertfalse;
        }
    }

    BusesLayout currentLayout = (ioLayout != nullptr ? *ioLayout : owner.getBusesLayout());
    const Array<AudioChannelSet>& actualBuses =
        (isInputBus ? currentLayout.inputBuses : currentLayout.outputBuses);

    if (actualBuses.getReference (busIdx) == set)
        return true;

    BusesLayout desiredLayout = currentLayout;
    {
        Array<AudioChannelSet>& desiredBuses =
            (isInputBus ? desiredLayout.inputBuses : desiredLayout.outputBuses);

        desiredBuses.getReference (busIdx) = set;
    }

    owner.getNextBestLayout (desiredLayout, currentLayout);

    if (ioLayout != nullptr)
        *ioLayout = currentLayout;

    // Nearest layout has a different number of buses. JUCE plug-ins MUST
    // have fixed number of buses.
    jassert (currentLayout.inputBuses. size() == owner.getBusCount (true)
          && currentLayout.outputBuses.size() == owner.getBusCount (false));

    return (actualBuses.getReference (busIdx) == set);
}

bool AudioProcessor::Bus::isNumberOfChannelsSupported (int channels) const
{
    if (channels == 0) return isLayoutSupported(AudioChannelSet::disabled());

    const AudioChannelSet set = supportedLayoutWithChannels (channels);
    return (! set.isDisabled()) && isLayoutSupported (set);
}

AudioChannelSet AudioProcessor::Bus::supportedLayoutWithChannels (int channels) const
{
    if (channels == 0) return AudioChannelSet::disabled();

    {
        AudioChannelSet set;
        if (! (set = AudioChannelSet::namedChannelSet  (channels)).isDisabled() && isLayoutSupported (set))
            return set;

        if (! (set = AudioChannelSet::discreteChannels (channels)).isDisabled() && isLayoutSupported (set))
            return set;
    }

    Array<AudioChannelSet> sets = AudioChannelSet::channelSetsWithNumberOfChannels (channels);
    const int n = sets.size();

    for (int i = 0; i < n; ++i)
    {
        const AudioChannelSet set = sets.getReference (i);

        if (isLayoutSupported (set))
            return set;
    }

    return AudioChannelSet::disabled();
}

AudioProcessor::BusesLayout AudioProcessor::Bus::getBusesLayoutForLayoutChangeOfBus (const AudioChannelSet& set) const
{
    bool isInputBus;
    int busIdx;
    busDirAndIndex (isInputBus, busIdx);

    BusesLayout layouts = owner.getBusesLayout();
    isLayoutSupported (set, &layouts);

    return layouts;
}

int AudioProcessor::Bus::getChannelIndexInProcessBlockBuffer (int channelIndex) const noexcept
{
    bool isInputBus;
    int busIdx;
    busDirAndIndex (isInputBus, busIdx);

    return owner.getChannelIndexInProcessBlockBuffer (isInputBus, busIdx, channelIndex);
}

void AudioProcessor::Bus::updateChannelCount() noexcept
{
    cachedChannelCount = layout.size();
}

//==============================================================================
void AudioProcessor::BusesProperties::addBus (bool isInput, const String& name,
                                                const AudioChannelSet& dfltLayout, bool isActivatedByDefault)
{
    jassert (dfltLayout.size() != 0);

    BusProperties props;

    props.busName = name;
    props.defaultLayout = dfltLayout;
    props.isActivatedByDefault = isActivatedByDefault;

    (isInput ? inputLayouts : outputLayouts).add (props);
}

AudioProcessor::BusesProperties AudioProcessor::BusesProperties::withInput  (const String& name,
                                                                                 const AudioChannelSet& dfltLayout,
                                                                                 bool isActivatedByDefault) const
{
    BusesProperties retval (*this);
    retval.addBus (true, name, dfltLayout, isActivatedByDefault);

    return retval;
}

AudioProcessor::BusesProperties AudioProcessor::BusesProperties::withOutput (const String& name,
                                                                                 const AudioChannelSet& dfltLayout,
                                                                                 bool isActivatedByDefault) const
{
    BusesProperties retval (*this);
    retval.addBus (false, name, dfltLayout, isActivatedByDefault);

    return retval;
}

//==============================================================================
int32 AudioProcessor::getAAXPluginIDForMainBusConfig (const AudioChannelSet& mainInputLayout,
                                                      const AudioChannelSet& mainOutputLayout,
                                                      const bool idForAudioSuite) const
{
    int uniqueFormatId = 0;
    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir == 0);
        const AudioChannelSet& set = (isInput ? mainInputLayout : mainOutputLayout);
        int aaxFormatIndex = 0;

        if      (set == AudioChannelSet::disabled())           aaxFormatIndex = 0;
        else if (set == AudioChannelSet::mono())               aaxFormatIndex = 1;
        else if (set == AudioChannelSet::stereo())             aaxFormatIndex = 2;
        else if (set == AudioChannelSet::createLCR())          aaxFormatIndex = 3;
        else if (set == AudioChannelSet::createLCRS())         aaxFormatIndex = 4;
        else if (set == AudioChannelSet::quadraphonic())       aaxFormatIndex = 5;
        else if (set == AudioChannelSet::create5point0())      aaxFormatIndex = 6;
        else if (set == AudioChannelSet::create5point1())      aaxFormatIndex = 7;
        else if (set == AudioChannelSet::create6point0())      aaxFormatIndex = 8;
        else if (set == AudioChannelSet::create6point1())      aaxFormatIndex = 9;
        else if (set == AudioChannelSet::create7point0())      aaxFormatIndex = 10;
        else if (set == AudioChannelSet::create7point1())      aaxFormatIndex = 11;
        else if (set == AudioChannelSet::create7point0SDDS())  aaxFormatIndex = 12;
        else if (set == AudioChannelSet::create7point1SDDS())  aaxFormatIndex = 13;
        else
        {
            // AAX does not support this format and the wrapper should not have
            // called this method with this layout
            jassertfalse;
        }

        uniqueFormatId = (uniqueFormatId << 8) | aaxFormatIndex;
    }

    return (idForAudioSuite ? 0x6a796161 /* 'jyaa' */ : 0x6a636161 /* 'jcaa' */) + uniqueFormatId;
}


//==============================================================================
void AudioProcessorListener::audioProcessorParameterChangeGestureBegin (AudioProcessor*, int) {}
void AudioProcessorListener::audioProcessorParameterChangeGestureEnd   (AudioProcessor*, int) {}

//==============================================================================
AudioProcessorParameter::AudioProcessorParameter() noexcept
    : processor (nullptr), parameterIndex (-1)
{}

AudioProcessorParameter::~AudioProcessorParameter() {}

void AudioProcessorParameter::setValueNotifyingHost (float newValue)
{
    // This method can't be used until the parameter has been attached to a processor!
    jassert (processor != nullptr && parameterIndex >= 0);

    return processor->setParameterNotifyingHost (parameterIndex, newValue);
}

void AudioProcessorParameter::beginChangeGesture()
{
    // This method can't be used until the parameter has been attached to a processor!
    jassert (processor != nullptr && parameterIndex >= 0);

    processor->beginParameterChangeGesture (parameterIndex);
}

void AudioProcessorParameter::endChangeGesture()
{
    // This method can't be used until the parameter has been attached to a processor!
    jassert (processor != nullptr && parameterIndex >= 0);

    processor->endParameterChangeGesture (parameterIndex);
}

bool AudioProcessorParameter::isOrientationInverted() const                      { return false; }
bool AudioProcessorParameter::isAutomatable() const                              { return true; }
bool AudioProcessorParameter::isMetaParameter() const                            { return false; }
AudioProcessorParameter::Category AudioProcessorParameter::getCategory() const   { return genericParameter; }
int AudioProcessorParameter::getNumSteps() const                                 { return AudioProcessor::getDefaultNumParameterSteps(); }
bool AudioProcessorParameter::isDiscrete() const                                 { return false; }

String AudioProcessorParameter::getText (float value, int /*maximumStringLength*/) const
{
    return String (value, 2);
}

//==============================================================================
bool AudioPlayHead::CurrentPositionInfo::operator== (const CurrentPositionInfo& other) const noexcept
{
    return timeInSamples == other.timeInSamples
        && ppqPosition == other.ppqPosition
        && editOriginTime == other.editOriginTime
        && ppqPositionOfLastBarStart == other.ppqPositionOfLastBarStart
        && frameRate == other.frameRate
        && isPlaying == other.isPlaying
        && isRecording == other.isRecording
        && bpm == other.bpm
        && timeSigNumerator == other.timeSigNumerator
        && timeSigDenominator == other.timeSigDenominator
        && ppqLoopStart == other.ppqLoopStart
        && ppqLoopEnd == other.ppqLoopEnd
        && isLooping == other.isLooping;
}

bool AudioPlayHead::CurrentPositionInfo::operator!= (const CurrentPositionInfo& other) const noexcept
{
    return ! operator== (other);
}

void AudioPlayHead::CurrentPositionInfo::resetToDefault()
{
    zerostruct (*this);
    timeSigNumerator = 4;
    timeSigDenominator = 4;
    bpm = 120;
}

} // namespace juce
