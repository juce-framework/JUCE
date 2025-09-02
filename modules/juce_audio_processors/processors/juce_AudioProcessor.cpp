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

static ThreadLocalValue<AudioProcessor::WrapperType> wrapperTypeBeingCreated;

void JUCE_CALLTYPE AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::WrapperType type)
{
    wrapperTypeBeingCreated = type;
}

AudioProcessor::AudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), false)
                                       .withOutput ("Output", AudioChannelSet::stereo(), false))
{
}

AudioProcessor::AudioProcessor (const BusesProperties& ioConfig)
    : wrapperType (wrapperTypeBeingCreated.get())
{
    for (auto& layout : ioConfig.inputLayouts)   createBus (true,  layout);
    for (auto& layout : ioConfig.outputLayouts)  createBus (false, layout);

    updateSpeakerFormatStrings();
}

AudioProcessor::~AudioProcessor()
{
    {
        const ScopedLock sl (activeEditorLock);

        // ooh, nasty - the editor should have been deleted before its AudioProcessor.
        jassert (activeEditor == nullptr);
    }

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
    auto numBuses = getBusCount (inputBus);

    if (numBuses == 0)
        return false;

    if (! canRemoveBus (inputBus))
        return false;

    BusProperties busesProps;

    if (! canApplyBusCountChange (inputBus, false, busesProps))
        return false;

    auto busIndex = numBuses - 1;
    auto numChannels = getChannelCountOfBus (inputBus, busIndex);
    (inputBus ? inputBuses : outputBuses).remove (busIndex);

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

    auto copy = arr;

    if (! canApplyBusesLayout (copy))
        return false;

    return applyBusLayouts (copy);
}

bool AudioProcessor::setBusesLayoutWithoutEnabling (const BusesLayout& arr)
{
    auto numIns  = getBusCount (true);
    auto numOuts = getBusCount (false);

    jassert (arr.inputBuses. size() == numIns
          && arr.outputBuses.size() == numOuts);

    auto request = arr;
    auto current = getBusesLayout();

    for (int i = 0; i < numIns; ++i)
        if (request.getNumChannels (true, i) == 0)
            request.getChannelSet (true, i) = current.getChannelSet (true, i);

    for (int i = 0; i < numOuts; ++i)
        if (request.getNumChannels (false, i) == 0)
            request.getChannelSet (false, i) = current.getChannelSet (false, i);

    if (! checkBusesLayoutSupported (request))
        return false;

    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir != 0);

        for (int i = 0; i < (isInput ? numIns : numOuts); ++i)
        {
            auto& bus = *getBus (isInput, i);
            auto& set = request.getChannelSet (isInput, i);

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

    for (auto& i : inputBuses)   layouts.inputBuses.add (i->getCurrentLayout());
    for (auto& i : outputBuses)  layouts.outputBuses.add (i->getCurrentLayout());

    return layouts;
}

AudioChannelSet AudioProcessor::getChannelLayoutOfBus (bool isInput, int busIndex) const noexcept
{
    if (auto* bus = (isInput ? inputBuses : outputBuses)[busIndex])
        return bus->getCurrentLayout();

    return {};
}

bool AudioProcessor::setChannelLayoutOfBus (bool isInputBus, int busIndex, const AudioChannelSet& layout)
{
    if (auto* bus = getBus (isInputBus, busIndex))
    {
        auto layouts = bus->getBusesLayoutForLayoutChangeOfBus (layout);

        if (layouts.getChannelSet (isInputBus, busIndex) == layout)
            return applyBusLayouts (layouts);

        return false;
    }

    jassertfalse;  // busIndex parameter is invalid
    return false;
}

bool AudioProcessor::enableAllBuses()
{
    BusesLayout layouts;

    for (auto& i : inputBuses)   layouts.inputBuses.add (i->lastLayout);
    for (auto& i : outputBuses)  layouts.outputBuses.add (i->lastLayout);

    return setBusesLayout (layouts);
}

bool AudioProcessor::checkBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.inputBuses.size() == inputBuses.size()
          && layouts.outputBuses.size() == outputBuses.size())
        return isBusesLayoutSupported (layouts);

    return false;
}

void AudioProcessor::getNextBestLayout (const BusesLayout& desiredLayout, BusesLayout& actualLayouts) const
{
    // if you are hitting this assertion then you are requesting a next
    // best layout which does not have the same number of buses as the
    // audio processor.
    jassert (desiredLayout.inputBuses.size() == inputBuses.size()
          && desiredLayout.outputBuses.size() == outputBuses.size());

    if (checkBusesLayoutSupported (desiredLayout))
    {
        actualLayouts = desiredLayout;
        return;
    }

    auto originalState = actualLayouts;
    auto currentState = originalState;
    auto bestSupported = currentState;

    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir > 0);

        auto& currentLayouts   = (isInput ? currentState.inputBuses  : currentState.outputBuses);
        auto& bestLayouts      = (isInput ? bestSupported.inputBuses : bestSupported.outputBuses);
        auto& requestedLayouts = (isInput ? desiredLayout.inputBuses : desiredLayout.outputBuses);
        auto& originalLayouts  = (isInput ? originalState.inputBuses : originalState.outputBuses);

        for (int busIndex = 0; busIndex < requestedLayouts.size(); ++busIndex)
        {
            auto& best       = bestLayouts     .getReference (busIndex);
            auto& requested  = requestedLayouts.getReference (busIndex);
            auto& original   = originalLayouts .getReference (busIndex);

            // do we need to do anything
            if (original == requested)
                continue;

            currentState = bestSupported;
            auto& current = currentLayouts  .getReference (busIndex);

            // already supported?
            current = requested;

            if (checkBusesLayoutSupported (currentState))
            {
                bestSupported = currentState;
                continue;
            }

            // try setting the opposite bus to the identical layout
            const bool oppositeDirection = ! isInput;

            if (getBusCount (oppositeDirection) > busIndex)
            {
                auto& oppositeLayout = (oppositeDirection ? currentState.inputBuses : currentState.outputBuses).getReference (busIndex);
                oppositeLayout = requested;

                if (checkBusesLayoutSupported (currentState))
                {
                    bestSupported = currentState;
                    continue;
                }

                // try setting the default layout
                oppositeLayout = getBus (oppositeDirection, busIndex)->getDefaultLayout();

                if (checkBusesLayoutSupported (currentState))
                {
                    bestSupported = currentState;
                    continue;
                }
            }

            // try setting all other buses to the identical layout
            BusesLayout allTheSame;
            allTheSame.inputBuses.insertMultiple (-1, requested, getBusCount (true));
            allTheSame.outputBuses.insertMultiple (-1, requested, getBusCount (false));

            if (checkBusesLayoutSupported (allTheSame))
            {
                bestSupported = allTheSame;
                continue;
            }

            // what is closer the default or the current layout?
            auto distance = std::abs (best.size() - requested.size());
            auto& defaultLayout = getBus (isInput, busIndex)->getDefaultLayout();

            if (std::abs (defaultLayout.size() - requested.size()) < distance)
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
void AudioProcessor::setPlayHead (AudioPlayHead* newPlayHead)
{
    playHead = newPlayHead;
}

void AudioProcessor::addListener (AudioProcessorListener* newListener)
{
    const ScopedLock sl (listenerLock);
    listeners.addIfNotAlreadyThere (newListener);
}

void AudioProcessor::removeListener (AudioProcessorListener* listenerToRemove)
{
    const ScopedLock sl (listenerLock);
    listeners.removeFirstMatchingValue (listenerToRemove);
}

void AudioProcessor::setPlayConfigDetails (int newNumIns, int newNumOuts, double newSampleRate, int newBlockSize)
{
    [[maybe_unused]] bool success = true;

    if (getTotalNumInputChannels() != newNumIns)
        success &= setChannelLayoutOfBus (true,  0, AudioChannelSet::canonicalChannelSet (newNumIns));

    // failed to find a compatible input configuration
    jassert (success);

    if (getTotalNumOutputChannels() != newNumOuts)
        success &= setChannelLayoutOfBus (false, 0, AudioChannelSet::canonicalChannelSet (newNumOuts));

    // failed to find a compatible output configuration
    jassert (success);

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
void AudioProcessor::numChannelsChanged()      {}
void AudioProcessor::numBusesChanged()         {}
void AudioProcessor::processorLayoutsChanged() {}

int AudioProcessor::getChannelIndexInProcessBlockBuffer (bool isInput, int busIndex, int channelIndex) const noexcept
{
    auto& ioBus = isInput ? inputBuses : outputBuses;
    jassert (isPositiveAndBelow (busIndex, ioBus.size()));

    for (int i = 0; i < ioBus.size() && i < busIndex; ++i)
        channelIndex += getChannelCountOfBus (isInput, i);

    return channelIndex;
}

int AudioProcessor::getOffsetInBusBufferForAbsoluteChannelIndex (bool isInput, int absoluteChannelIndex, int& busIndex) const noexcept
{
    auto numBuses = getBusCount (isInput);
    int numChannels = 0;

    for (busIndex = 0; busIndex < numBuses && absoluteChannelIndex >= (numChannels = getChannelLayoutOfBus (isInput, busIndex).size()); ++busIndex)
        absoluteChannelIndex -= numChannels;

    return busIndex >= numBuses ? -1 : absoluteChannelIndex;
}

//==============================================================================
void AudioProcessor::setNonRealtime (bool newNonRealtime) noexcept
{
    nonRealtime = newNonRealtime;
}

void AudioProcessor::setLatencySamples (int newLatency)
{
    if (latencySamples != newLatency)
    {
        latencySamples = newLatency;
        updateHostDisplay (AudioProcessorListener::ChangeDetails().withLatencyChanged (true));
    }
}

//==============================================================================
AudioProcessorListener* AudioProcessor::getListenerLocked (int index) const noexcept
{
    const ScopedLock sl (listenerLock);
    return listeners[index];
}

void AudioProcessor::updateHostDisplay (const AudioProcessorListener::ChangeDetails& details)
{
    for (int i = listeners.size(); --i >= 0;)
        if (auto l = getListenerLocked (i))
            l->audioProcessorChanged (this, details);
}

void AudioProcessor::validateParameter (AudioProcessorParameter* param)
{
    checkForDuplicateParamID (param);
    checkForDuplicateTrimmedParamID (param);

    /*  If you're building this plugin as an AudioUnit, and you intend to use the plugin in
        Logic Pro or GarageBand, it's a good idea to set version hints on all of your parameters
        so that you can add parameters safely in future versions of the plugin.
        See the documentation for AudioProcessorParameter (int) for more information.
    */
   #if JucePlugin_Build_AU
    static std::once_flag flag;
    if (wrapperType != wrapperType_Undefined && param->getVersionHint() == 0)
        std::call_once (flag, [] { jassertfalse; });
   #endif
}

void AudioProcessor::checkForDuplicateTrimmedParamID ([[maybe_unused]] AudioProcessorParameter* param)
{
   #if JUCE_DEBUG && ! JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING
    if (auto* withID = dynamic_cast<HostedAudioProcessorParameter*> (param))
    {
        [[maybe_unused]] constexpr auto maximumSafeAAXParameterIdLength = 31;

        const auto paramID = withID->getParameterID();

        // If you hit this assertion, a parameter name is too long to be supported
        // by the AAX plugin format.
        // If there's a chance that you'll release this plugin in AAX format, you
        // should consider reducing the length of this paramID.
        // If you need to retain backwards-compatibility and are unable to change
        // the paramID for this reason, you can add JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING
        // to your preprocessor definitions to silence this assertion.
        jassert (paramID.length() <= maximumSafeAAXParameterIdLength);

        // If you hit this assertion, two or more parameters have duplicate paramIDs
        // after they have been truncated to support the AAX format.
        // This is a serious issue, and will prevent the duplicated parameters from
        // being automated when running as an AAX plugin.
        // If there's a chance that you'll release this plugin in AAX format, you
        // should reduce the length of this paramID.
        // If you need to retain backwards-compatibility and are unable to change
        // the paramID for this reason, you can add JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING
        // to your preprocessor definitions to silence this assertion.
        jassert (trimmedParamIDs.insert (paramID.substring (0, maximumSafeAAXParameterIdLength)).second);
    }
   #endif
}

void AudioProcessor::checkForDuplicateParamID ([[maybe_unused]] AudioProcessorParameter* param)
{
   #if JUCE_DEBUG
    if (auto* withID = dynamic_cast<HostedAudioProcessorParameter*> (param))
    {
        auto insertResult = paramIDs.insert (withID->getParameterID());

        // If you hit this assertion then the parameter ID is not unique
        jassert (insertResult.second);
    }
   #endif
}

void AudioProcessor::checkForDuplicateGroupIDs ([[maybe_unused]] const AudioProcessorParameterGroup& newGroup)
{
   #if JUCE_DEBUG
    auto groups = newGroup.getSubgroups (true);
    groups.add (&newGroup);

    for (auto* group : groups)
    {
        auto insertResult = groupIDs.insert (group->getID());

        // If you hit this assertion then a group ID is not unique
        jassert (insertResult.second);
    }
   #endif
}

const Array<AudioProcessorParameter*>& AudioProcessor::getParameters() const   { return flatParameterList; }
const AudioProcessorParameterGroup& AudioProcessor::getParameterTree() const   { return parameterTree; }

void AudioProcessor::addParameter (AudioProcessorParameter* param)
{
    jassert (param != nullptr);
    parameterTree.addChild (std::unique_ptr<AudioProcessorParameter> (param));

    param->setOwner (&parameterListener);
    param->setParameterIndex (flatParameterList.size());
    flatParameterList.add (param);

    validateParameter (param);
}

void AudioProcessor::addParameterGroup (std::unique_ptr<AudioProcessorParameterGroup> group)
{
    jassert (group != nullptr);
    checkForDuplicateGroupIDs (*group);

    auto oldSize = flatParameterList.size();
    flatParameterList.addArray (group->getParameters (true));

    for (int i = oldSize; i < flatParameterList.size(); ++i)
    {
        auto p = flatParameterList.getUnchecked (i);
        p->setOwner (&parameterListener);
        p->setParameterIndex (i);

        validateParameter (p);
    }

    parameterTree.addChild (std::move (group));
}

void AudioProcessor::setParameterTree (AudioProcessorParameterGroup&& newTree)
{
  #if JUCE_DEBUG
    paramIDs.clear();
    groupIDs.clear();
   #if ! JUCE_DISABLE_CAUTIOUS_PARAMETER_ID_CHECKING
    trimmedParamIDs.clear();
   #endif
  #endif

    parameterTree = std::move (newTree);
    checkForDuplicateGroupIDs (parameterTree);

    flatParameterList = parameterTree.getParameters (true);

    for (int i = 0; i < flatParameterList.size(); ++i)
    {
        auto p = flatParameterList.getUnchecked (i);
        p->setOwner (&parameterListener);
        p->setParameterIndex (i);

        validateParameter (p);
    }
}

void AudioProcessor::refreshParameterList() {}

int AudioProcessor::getDefaultNumParameterSteps() noexcept
{
    return AudioProcessorParameter::getDefaultNumParameterSteps();
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
    // If you hit this assertion then your plug-in is reporting that it introduces
    // some latency, but you haven't overridden processBlockBypassed to produce
    // an identical amount of latency. Without identical latency in
    // processBlockBypassed a host's latency compensation could shift the audio
    // passing through your bypassed plug-in forward in time.
    jassert (getLatencySamples() == 0);

    for (int ch = getMainBusNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

void AudioProcessor::processBlockBypassed (AudioBuffer<float>&  buffer, MidiBuffer& midi)    { processBypassed (buffer, midi); }
void AudioProcessor::processBlockBypassed (AudioBuffer<double>& buffer, MidiBuffer& midi)    { processBypassed (buffer, midi); }

void AudioProcessor::processBlock ([[maybe_unused]] AudioBuffer<double>& buffer,
                                   [[maybe_unused]] MidiBuffer& midiMessages)
{
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
        ioProps.addBus (true, "Input", AudioChannelSet::canonicalChannelSet (config[0].inChannels));

    if (config[0].outChannels > 0)
        ioProps.addBus (false, "Output", AudioChannelSet::canonicalChannelSet (config[0].outChannels));

    return ioProps;
}

AudioProcessor::BusesLayout AudioProcessor::getNextBestLayoutInList (const BusesLayout& layouts,
                                                                     const Array<InOutChannelPair>& legacyLayouts) const
{
    auto numChannelConfigs = legacyLayouts.size();
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

    auto nearest = layouts;
    nearest.inputBuses .resize (hasInputs  ? 1 : 0);
    nearest.outputBuses.resize (hasOutputs ? 1 : 0);

    auto* inBus  = (hasInputs  ? &nearest.inputBuses. getReference (0) : nullptr);
    auto* outBus = (hasOutputs ? &nearest.outputBuses.getReference (0) : nullptr);

    auto inNumChannelsRequested  = static_cast<int16> (inBus  != nullptr ? inBus->size()  : 0);
    auto outNumChannelsRequested = static_cast<int16> (outBus != nullptr ? outBus->size() : 0);

    auto distance = std::numeric_limits<int32>::max();
    int bestConfiguration = 0;

    for (int i = 0; i < numChannelConfigs; ++i)
    {
        auto inChannels  = legacyLayouts.getReference (i).inChannels;
        auto outChannels = legacyLayouts.getReference (i).outChannels;

        auto channelDifference = ((std::abs (inChannels  - inNumChannelsRequested)  & 0xffff) << 16)
                                    | ((std::abs (outChannels - outNumChannelsRequested) & 0xffff) << 0);

        if (channelDifference < distance)
        {
            distance = channelDifference;
            bestConfiguration = i;

            // we can exit if we found a perfect match
            if (distance == 0)
                return nearest;
        }
    }

    auto inChannels  = legacyLayouts.getReference (bestConfiguration).inChannels;
    auto outChannels = legacyLayouts.getReference (bestConfiguration).outChannels;

    auto currentState = getBusesLayout();
    auto currentInLayout  = (getBusCount (true)  > 0 ? currentState.inputBuses .getReference (0) : AudioChannelSet());
    auto currentOutLayout = (getBusCount (false) > 0 ? currentState.outputBuses.getReference (0) : AudioChannelSet());

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
    auto layouts = getBusesLayout();

    for (int busIndex = 1; busIndex < layouts.inputBuses.size(); ++busIndex)
        layouts.inputBuses.getReference (busIndex) = AudioChannelSet::disabled();

    for (int busIndex = 1; busIndex < layouts.outputBuses.size(); ++busIndex)
        layouts.outputBuses.getReference (busIndex) = AudioChannelSet::disabled();

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

    auto numInputBuses  = getBusCount (true);
    auto numOutputBuses = getBusCount (false);

    auto oldNumberOfIns  = getTotalNumInputChannels();
    auto oldNumberOfOuts = getTotalNumOutputChannels();

    if (layouts.inputBuses. size() != numInputBuses
     || layouts.outputBuses.size() != numOutputBuses)
        return false;

    int newNumberOfIns = 0, newNumberOfOuts = 0;

    for (int busIndex = 0; busIndex < numInputBuses;  ++busIndex)
    {
        auto& bus = *getBus (true, busIndex);
        const auto& set = layouts.getChannelSet (true, busIndex);
        bus.layout = set;

        if (! set.isDisabled())
            bus.lastLayout = set;

        newNumberOfIns += set.size();
    }

    for (int busIndex = 0; busIndex < numOutputBuses;  ++busIndex)
    {
        auto& bus = *getBus (false, busIndex);
        const auto& set = layouts.getChannelSet (false, busIndex);
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
    auto numInputBuses  = getBusCount (true);
    auto numOutputBuses = getBusCount (false);

    for (int dir = 0; dir < 2; ++dir)
    {
        const bool isInput = (dir == 0);
        auto num = (isInput ? numInputBuses : numOutputBuses);

        for (int i = 0; i < num; ++i)
            if (auto* bus = getBus (isInput, i))
                bus->updateChannelCount();
    }

    auto countTotalChannels = [] (const OwnedArray<AudioProcessor::Bus>& buses) noexcept
    {
        int n = 0;

        for (auto* bus : buses)
            n += bus->getNumberOfChannels();

        return n;
    };

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
    const ScopedLock sl (activeEditorLock);

    if (activeEditor == editor)
        activeEditor = nullptr;
}

AudioProcessorEditor* AudioProcessor::getActiveEditor() const noexcept
{
    const ScopedLock sl (activeEditorLock);
    return activeEditor;
}

AudioProcessorEditor* AudioProcessor::createEditorIfNeeded()
{
    const ScopedLock sl (activeEditorLock);

    if (activeEditor != nullptr)
        return activeEditor;

    auto* ed = createEditor();

    if (ed != nullptr)
    {
        // you must give your editor comp a size before returning it..
        jassert (ed->getWidth() > 0 && ed->getHeight() > 0);
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
        xml.writeTo (out, XmlElement::TextFormat().singleLine());
        out.writeByte (0);
    }

    // go back and write the string length..
    static_cast<uint32*> (destData.getData())[1]
        = ByteOrder::swapIfBigEndian ((uint32) destData.getSize() - 9);
}

std::optional<String> AudioProcessor::getNameForMidiNoteNumber (int /*note*/, int /*midiChannel*/)
{
    return std::nullopt;
}

std::unique_ptr<XmlElement> AudioProcessor::getXmlFromBinary (const void* data, const int sizeInBytes)
{
    if (sizeInBytes > 8 && ByteOrder::littleEndianInt (data) == magicXmlNumber)
    {
        auto stringLength = (int) ByteOrder::littleEndianInt (addBytesToPointer (data, 4));

        if (stringLength > 0)
            return parseXML (String::fromUTF8 (static_cast<const char*> (data) + 8,
                                                jmin ((sizeInBytes - 8), stringLength)));
    }

    return {};
}

bool AudioProcessor::canApplyBusCountChange (bool isInput, bool isAdding,
                                             AudioProcessor::BusProperties& outProperties)
{
    if (  isAdding && ! canAddBus    (isInput)) return false;
    if (! isAdding && ! canRemoveBus (isInput)) return false;

    auto num = getBusCount (isInput);

    // No way for me to find out the default layout if there are no other busses!!
    if (num == 0)
        return false;

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

bool AudioProcessor::Bus::isInput() const noexcept      { return owner.inputBuses.contains (this); }
int AudioProcessor::Bus::getBusIndex() const noexcept   { return getDirectionAndIndex().index; }

AudioProcessor::Bus::BusDirectionAndIndex AudioProcessor::Bus::getDirectionAndIndex() const noexcept
{
    BusDirectionAndIndex di;
    di.index = owner.inputBuses.indexOf (this);
    di.isInput = (di.index >= 0);

    if (! di.isInput)
        di.index = owner.outputBuses.indexOf (this);

    return di;
}

bool AudioProcessor::Bus::setCurrentLayout (const AudioChannelSet& busLayout)
{
    auto di = getDirectionAndIndex();
    return owner.setChannelLayoutOfBus (di.isInput, di.index, busLayout);
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
    auto di = getDirectionAndIndex();

    if (owner.setChannelLayoutOfBus (di.isInput, di.index, AudioChannelSet::canonicalChannelSet (channels)))
        return true;

    if (channels == 0)
        return false;

    auto namedSet = AudioChannelSet::namedChannelSet (channels);

    if (! namedSet.isDisabled() && owner.setChannelLayoutOfBus (di.isInput, di.index, namedSet))
        return true;

    return owner.setChannelLayoutOfBus (di.isInput, di.index, AudioChannelSet::discreteChannels (channels));
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
    auto di = getDirectionAndIndex();

    // check that supplied ioLayout is actually valid
    if (ioLayout != nullptr)
    {
        if (! owner.checkBusesLayoutSupported (*ioLayout))
        {
            *ioLayout = owner.getBusesLayout();

            // the current layout you supplied is not a valid layout
            jassertfalse;
        }
    }

    auto currentLayout = (ioLayout != nullptr ? *ioLayout : owner.getBusesLayout());
    auto& actualBuses = (di.isInput ? currentLayout.inputBuses : currentLayout.outputBuses);

    if (actualBuses.getReference (di.index) == set)
        return true;

    auto desiredLayout = currentLayout;

    (di.isInput ? desiredLayout.inputBuses
                : desiredLayout.outputBuses).getReference (di.index) = set;

    owner.getNextBestLayout (desiredLayout, currentLayout);

    if (ioLayout != nullptr)
        *ioLayout = currentLayout;

    // Nearest layout has a different number of buses. JUCE plug-ins MUST
    // have fixed number of buses.
    jassert (currentLayout.inputBuses. size() == owner.getBusCount (true)
          && currentLayout.outputBuses.size() == owner.getBusCount (false));

    return actualBuses.getReference (di.index) == set;
}

bool AudioProcessor::Bus::isNumberOfChannelsSupported (int channels) const
{
    if (channels == 0)
        return isLayoutSupported (AudioChannelSet::disabled());

    auto set = supportedLayoutWithChannels (channels);
    return (! set.isDisabled()) && isLayoutSupported (set);
}

AudioChannelSet AudioProcessor::Bus::supportedLayoutWithChannels (int channels) const
{
    if (channels == 0)
        return AudioChannelSet::disabled();

    {
        AudioChannelSet set;

        if (! (set = AudioChannelSet::namedChannelSet  (channels)).isDisabled() && isLayoutSupported (set))
            return set;

        if (! (set = AudioChannelSet::discreteChannels (channels)).isDisabled() && isLayoutSupported (set))
            return set;
    }

    for (auto& set : AudioChannelSet::channelSetsWithNumberOfChannels (channels))
        if (isLayoutSupported (set))
            return set;

    return AudioChannelSet::disabled();
}

AudioProcessor::BusesLayout AudioProcessor::Bus::getBusesLayoutForLayoutChangeOfBus (const AudioChannelSet& set) const
{
    auto layouts = owner.getBusesLayout();
    isLayoutSupported (set, &layouts);
    return layouts;
}

int AudioProcessor::Bus::getChannelIndexInProcessBlockBuffer (int channelIndex) const noexcept
{
    auto di = getDirectionAndIndex();
    return owner.getChannelIndexInProcessBlockBuffer (di.isInput, di.index, channelIndex);
}

void AudioProcessor::Bus::updateChannelCount() noexcept
{
    cachedChannelCount = layout.size();
}

//==============================================================================
void AudioProcessor::BusesProperties::addBus (bool isInput, const String& name,
                                              const AudioChannelSet& dfltLayout, bool isActivatedByDefault)
{
    jassert (! dfltLayout.isDisabled());

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
    auto retval = *this;
    retval.addBus (true, name, dfltLayout, isActivatedByDefault);
    return retval;
}

AudioProcessor::BusesProperties AudioProcessor::BusesProperties::withOutput (const String& name,
                                                                             const AudioChannelSet& dfltLayout,
                                                                             bool isActivatedByDefault) const
{
    auto retval = *this;
    retval.addBus (false, name, dfltLayout, isActivatedByDefault);
    return retval;
}

//==============================================================================
const char* AudioProcessor::getWrapperTypeDescription (AudioProcessor::WrapperType type) noexcept
{
    switch (type)
    {
        case AudioProcessor::wrapperType_Undefined:     return "Undefined";
        case AudioProcessor::wrapperType_VST:           return "VST";
        case AudioProcessor::wrapperType_VST3:          return "VST3";
        case AudioProcessor::wrapperType_AudioUnit:     return "AU";
        case AudioProcessor::wrapperType_AudioUnitv3:   return "AUv3";
        case AudioProcessor::wrapperType_AAX:           return "AAX";
        case AudioProcessor::wrapperType_Standalone:    return "Standalone";
        case AudioProcessor::wrapperType_Unity:         return "Unity";
        case AudioProcessor::wrapperType_LV2:           return "LV2";
        default:                                        jassertfalse; return {};
    }
}

//==============================================================================
VST2ClientExtensions* AudioProcessor::getVST2ClientExtensions()
{
    if (auto* extensions = dynamic_cast<VST2ClientExtensions*> (this))
    {
        //  To silence this jassert there are two options:
        //
        //  1. - Override AudioProcessor::getVST2ClientExtensions() and
        //       return the "this" pointer.
        //
        //     - This option has the advantage of being quick and easy,
        //       and avoids the above dynamic_cast.
        //
        //  2. - Create a new object that inherits from VST2ClientExtensions.
        //
        //     - Port your existing functionality from the AudioProcessor
        //       to the new object.
        //
        //     - Return a pointer to the object in AudioProcessor::getVST2ClientExtensions().
        //
        //     - This option has the advantage of allowing you to break
        //       up your AudioProcessor into smaller composable objects.
        jassertfalse;
        return extensions;
    }

    return nullptr;
}

VST3ClientExtensions* AudioProcessor::getVST3ClientExtensions()
{
    if (auto* extensions = dynamic_cast<VST3ClientExtensions*> (this))
    {
        //  To silence this jassert there are two options:
        //
        //  1. - Override AudioProcessor::getVST3ClientExtensions() and
        //       return the "this" pointer.
        //
        //     - This option has the advantage of being quick and easy,
        //       and avoids the above dynamic_cast.
        //
        //  2. - Create a new object that inherits from VST3ClientExtensions.
        //
        //     - Port your existing functionality from the AudioProcessor
        //       to the new object.
        //
        //     - Return a pointer to the object in AudioProcessor::getVST3ClientExtensions().
        //
        //     - This option has the advantage of allowing you to break
        //       up your AudioProcessor into smaller composable objects.
        jassertfalse;
        return extensions;
    }

    return nullptr;
}

//==============================================================================
JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

String AudioProcessor::getParameterName (int index, int maximumStringLength)
{
    if (auto* p = getParameters()[index])
        return p->getName (maximumStringLength);

    return isPositiveAndBelow (index, getNumParameters()) ? getParameterName (index).substring (0, maximumStringLength)
                                                          : String();
}

const String AudioProcessor::getParameterText (int index)
{
   #if JUCE_DEBUG
    // if you hit this, then you're probably using the old parameter control methods,
    // but have forgotten to implement either of the getParameterText() methods.
    jassert (! textRecursionCheck);
    ScopedValueSetter<bool> sv (textRecursionCheck, true, false);
   #endif

    return isPositiveAndBelow (index, getNumParameters()) ? getParameterText (index, 1024)
                                                          : String();
}

String AudioProcessor::getParameterText (int index, int maximumStringLength)
{
    if (auto* p = getParameters()[index])
        return p->getText (p->getValue(), maximumStringLength);

    return isPositiveAndBelow (index, getNumParameters()) ? getParameterText (index).substring (0, maximumStringLength)
                                                          : String();
}

int AudioProcessor::getNumParameters()
{
    return getParameters().size();
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
    if (auto* p = getParameters()[index])
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
    if (auto* p = dynamic_cast<HostedAudioProcessorParameter*> (getParameters()[index]))
        return p->getParameterID();

    return String (index);
}

int AudioProcessor::getParameterNumSteps (int index)
{
    if (auto* p = getParameters()[index])
        return p->getNumSteps();

    return getDefaultNumParameterSteps();
}

bool AudioProcessor::isParameterDiscrete (int index) const
{
    if (auto* p = getParameters()[index])
        return p->isDiscrete();

    return false;
}

String AudioProcessor::getParameterLabel (int index) const
{
    if (auto* p = getParameters()[index])
        return p->getLabel();

    return {};
}

bool AudioProcessor::isParameterAutomatable (int index) const
{
    if (auto* p = getParameters()[index])
        return p->isAutomatable();

    return true;
}

bool AudioProcessor::isParameterOrientationInverted (int index) const
{
    if (auto* p = getParameters()[index])
        return p->isOrientationInverted();

    return false;
}

bool AudioProcessor::isMetaParameter (int index) const
{
    if (auto* p = getParameters()[index])
        return p->isMetaParameter();

    return false;
}

AudioProcessorParameter::Category AudioProcessor::getParameterCategory (int index) const
{
    if (auto* p = getParameters()[index])
        return p->getCategory();

    return AudioProcessorParameter::genericParameter;
}

AudioProcessorParameter* AudioProcessor::getParamChecked (int index) const
{
    auto p = getParameters()[index];

    // If you hit this, then you're either trying to access parameters that are out-of-range,
    // or you're not using addParameter and the managed parameter list, but have failed
    // to override some essential virtual methods and implement them appropriately.
    jassert (p != nullptr);
    return p;
}

bool AudioProcessor::canAddBus ([[maybe_unused]] bool isInput) const                     { return false; }
bool AudioProcessor::canRemoveBus ([[maybe_unused]] bool isInput) const                  { return false; }

//==============================================================================
void AudioProcessor::ParameterChangeForwarder::parameterValueChanged (int index, float value)
{
    if (owner == nullptr)
        return;

    for (int i = owner->listeners.size(); --i >= 0;)
        if (auto* l = owner->listeners[i])
            l->audioProcessorParameterChanged (owner, index, value);
}

void AudioProcessor::ParameterChangeForwarder::parameterGestureChanged (int index, bool begin)
{
    if (owner == nullptr)
        return;

    const auto callback = begin
                        ? &AudioProcessorListener::audioProcessorParameterChangeGestureBegin
                        : &AudioProcessorListener::audioProcessorParameterChangeGestureEnd;

    for (int i = owner->listeners.size(); --i >= 0;)
        if (auto* l = owner->listeners[i])
            (l->*callback) (owner, index);
}

JUCE_END_IGNORE_DEPRECATION_WARNINGS

} // namespace juce
