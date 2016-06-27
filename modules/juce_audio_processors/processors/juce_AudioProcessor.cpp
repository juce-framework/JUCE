/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

static ThreadLocalValue<AudioProcessor::WrapperType> wrapperTypeBeingCreated;

void JUCE_CALLTYPE AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::WrapperType type)
{
    wrapperTypeBeingCreated = type;
}

AudioProcessor::AudioProcessor()
    : wrapperType (wrapperTypeBeingCreated.get()),
      playHead (nullptr),
      currentSampleRate (0),
      blockSize (0),
      latencySamples (0),
     #if JUCE_DEBUG
      textRecursionCheck (false),
     #endif
      suspended (false),
      nonRealtime (false),
      processingPrecision (singlePrecision)
{
   #ifdef JucePlugin_PreferredChannelConfigurations
    const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
   #else
    const short channelConfigs[][2] = { {2, 2} };
   #endif

   #ifdef JucePlugin_MaxNumInputChannels
    const int maxInChannels = JucePlugin_MaxNumInputChannels;
   #else
    const int maxInChannels = std::numeric_limits<int>::max();
   #endif
    ignoreUnused (maxInChannels);

   #ifdef JucePlugin_MaxNumOutputChannels
    const int maxOutChannels = JucePlugin_MaxNumOutputChannels;
   #else
    const int maxOutChannels = std::numeric_limits<int>::max();
   #endif
    ignoreUnused (maxOutChannels);

 #if ! JucePlugin_IsMidiEffect
   #if ! JucePlugin_IsSynth
    const int numInChannels = jmin (maxInChannels, (int) channelConfigs[0][0]);

    if (numInChannels > 0)
        busArrangement.inputBuses.add  (AudioProcessorBus ("Input",  AudioChannelSet::canonicalChannelSet (numInChannels)));
   #endif

    const int numOutChannels = jmin (maxOutChannels, (int) channelConfigs[0][1]);
    if (numOutChannels > 0)
        busArrangement.outputBuses.add (AudioProcessorBus ("Output", AudioChannelSet::canonicalChannelSet (numOutChannels)));

  #ifdef JucePlugin_PreferredChannelConfigurations
   #if ! JucePlugin_IsSynth
    AudioProcessor::setPreferredBusArrangement (true,  0, AudioChannelSet::stereo());
   #endif
    AudioProcessor::setPreferredBusArrangement (false, 0, AudioChannelSet::stereo());
  #endif
 #endif
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
    const int oldNumInputs  = getTotalNumInputChannels();
    const int oldNumOutputs = getTotalNumOutputChannels();

    // if the user is using this method then they do not want any side-buses or aux outputs
    disableNonMainBuses (true);
    disableNonMainBuses (false);

    if (getTotalNumInputChannels()  != newNumIns)  setPreferredBusArrangement (true,  0, AudioChannelSet::canonicalChannelSet (newNumIns));
    if (getTotalNumOutputChannels() != newNumOuts) setPreferredBusArrangement (false, 0, AudioChannelSet::canonicalChannelSet (newNumOuts));

    // the processor may not support this arrangement at all
    jassert (newNumIns == getTotalNumInputChannels() && newNumOuts == getTotalNumOutputChannels());

    setRateAndBufferSizeDetails (newSampleRate, newBlockSize);

    if (oldNumInputs != newNumIns || oldNumOutputs != newNumOuts)
    {
        updateSpeakerFormatStrings();
        numChannelsChanged();
    }
}

void AudioProcessor::setRateAndBufferSizeDetails (double newSampleRate, int newBlockSize) noexcept
{
    currentSampleRate = newSampleRate;
    blockSize = newBlockSize;
}

int AudioProcessor::getMainBusNumInputChannels()  const noexcept
{
    const Array<AudioProcessorBus>& buses = busArrangement.inputBuses;
    return buses.size() > 0 ? buses.getReference (0).channels.size() : 0;
}

int AudioProcessor::getMainBusNumOutputChannels() const noexcept
{
    const Array<AudioProcessorBus>& buses = busArrangement.outputBuses;
    return buses.size() > 0 ? buses.getReference (0).channels.size() : 0;
}

void AudioProcessor::numChannelsChanged() {}

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
            if (AudioProcessorListener* l = getListenerLocked (i))
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
            if (AudioProcessorListener* l = getListenerLocked (i))
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
    if (AudioProcessorParameter* p = getParamChecked (index))
        return p->getValue();

    return 0;
}

void AudioProcessor::setParameter (int index, float newValue)
{
    if (AudioProcessorParameter* p = getParamChecked (index))
        p->setValue (newValue);
}

float AudioProcessor::getParameterDefaultValue (int index)
{
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->getDefaultValue();

    return 0;
}

const String AudioProcessor::getParameterName (int index)
{
    if (AudioProcessorParameter* p = getParamChecked (index))
        return p->getName (512);

    return String();
}

String AudioProcessor::getParameterID (int index)
{
    // Don't use getParamChecked here, as this must also work for legacy plug-ins
    if (AudioProcessorParameterWithID* p = dynamic_cast<AudioProcessorParameterWithID*> (managedParameters[index]))
        return p->paramID;

    return String (index);
}

String AudioProcessor::getParameterName (int index, int maximumStringLength)
{
    if (AudioProcessorParameter* p = managedParameters[index])
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
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->getText (p->getValue(), maximumStringLength);

    return getParameterText (index).substring (0, maximumStringLength);
}

int AudioProcessor::getParameterNumSteps (int index)
{
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->getNumSteps();

    return AudioProcessor::getDefaultNumParameterSteps();
}

int AudioProcessor::getDefaultNumParameterSteps() noexcept
{
    return 0x7fffffff;
}

String AudioProcessor::getParameterLabel (int index) const
{
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->getLabel();

    return String();
}

bool AudioProcessor::isParameterAutomatable (int index) const
{
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->isAutomatable();

    return true;
}

bool AudioProcessor::isParameterOrientationInverted (int index) const
{
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->isOrientationInverted();

    return false;
}

bool AudioProcessor::isMetaParameter (int index) const
{
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->isMetaParameter();

    return false;
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

void AudioProcessor::setProcessingPrecision (ProcessingPrecision precision) noexcept
{
    // If you hit this assertion then you're trying to use double precision
    // processing on a processor which does not support it!
    jassert (precision != doublePrecision || supportsDoublePrecisionProcessing());

    processingPrecision = precision;
}

bool AudioProcessor::supportsDoublePrecisionProcessing() const
{
    return false;
}

//==============================================================================
static String getChannelName (const Array<AudioProcessor::AudioProcessorBus>& buses, int index)
{
    return buses.size() > 0 ? AudioChannelSet::getChannelTypeName (buses.getReference(0).channels.getTypeOfChannel (index))
                            : String();
}

const String AudioProcessor::getInputChannelName (int index) const   { return getChannelName (busArrangement.inputBuses, index); }
const String AudioProcessor::getOutputChannelName (int index) const  { return getChannelName (busArrangement.outputBuses, index); }

static bool isStereoPair (const Array<AudioProcessor::AudioProcessorBus>& buses, int index)
{
    return index < 2
            && buses.size() > 0
            && buses.getReference(0).channels == AudioChannelSet::stereo();
}

bool AudioProcessor::isInputChannelStereoPair  (int index) const    { return isStereoPair (busArrangement.inputBuses, index); }
bool AudioProcessor::isOutputChannelStereoPair (int index) const    { return isStereoPair (busArrangement.outputBuses, index); }

//==============================================================================
bool AudioProcessor::setPreferredBusArrangement (bool isInput, int busIndex, const AudioChannelSet& preferredSet)
{
    const int oldNumInputs  = getTotalNumInputChannels();
    const int oldNumOutputs = getTotalNumOutputChannels();

    Array<AudioProcessorBus>& buses = isInput ? busArrangement.inputBuses  : busArrangement.outputBuses;

    const int numBuses  = buses.size();

    if (! isPositiveAndBelow (busIndex, numBuses))
        return false;

   #ifdef JucePlugin_MaxNumInputChannels
    if (isInput && preferredSet.size() > JucePlugin_MaxNumInputChannels)
        return false;
   #endif

   #ifdef JucePlugin_MaxNumOutputChannels
    if (! isInput && preferredSet.size() > JucePlugin_MaxNumOutputChannels)
        return false;
   #endif

    AudioProcessorBus& bus = buses.getReference (busIndex);

   #ifdef JucePlugin_PreferredChannelConfigurations
    // the user is using the deprecated way to specify channel configurations
    if (numBuses > 0 && busIndex == 0)
    {
        const short channelConfigs[][2] = { JucePlugin_PreferredChannelConfigurations };
        const int numChannelConfigs = sizeof (channelConfigs) / sizeof (*channelConfigs);

        // we need the main bus in the opposite direction
        Array<AudioProcessorBus>& oppositeBuses = isInput ? busArrangement.outputBuses : busArrangement.inputBuses;
        AudioProcessorBus* oppositeBus = (busIndex < oppositeBuses.size()) ? &oppositeBuses.getReference (0) : nullptr;

        // get the target number of channels
        const int mainBusNumChannels  = preferredSet.size();
        const int mainBusOppositeChannels = (oppositeBus != nullptr) ? oppositeBus->channels.size() : 0;
        const int dir = isInput ? 0 : 1;

        // find a compatible channel configuration on the opposite bus which is the closest match
        // to the current number of channels on that bus
        int distance = std::numeric_limits<int>::max();
        int bestConfiguration = -1;

        for (int i = 0; i < numChannelConfigs; ++i)
        {
            // is the configuration compatible with the preferred set
            if (channelConfigs[i][dir] == mainBusNumChannels)
            {
                const int configChannels = channelConfigs[i][dir^1];
                const int channelDifference = std::abs (configChannels - mainBusOppositeChannels);

                if (channelDifference < distance)
                {
                    distance = channelDifference;
                    bestConfiguration = configChannels;

                    // we can exit if we found a perfect match
                    if (distance == 0)
                        break;
                }
            }
        }

        // unable to find a good configuration
        if (bestConfiguration == -1)
            return false;

        // did the number of channels change on the opposite bus?
        if (mainBusOppositeChannels != bestConfiguration && oppositeBus != nullptr)
        {
            // if the channels on the opposite bus are the same as the preferred set
            // then also copy over the layout information. If not, then assume
            // a cononical channel layout
            if (bestConfiguration == mainBusNumChannels)
                oppositeBus->channels = preferredSet;
            else
                oppositeBus->channels = AudioChannelSet::canonicalChannelSet (bestConfiguration);
        }
    }
   #endif

    bus.channels = preferredSet;

    if (oldNumInputs != getTotalNumInputChannels() || oldNumOutputs != getTotalNumOutputChannels())
    {
        updateSpeakerFormatStrings();
        numChannelsChanged();
    }

    return true;
}

void AudioProcessor::disableNonMainBuses (bool isInput)
{
    const Array<AudioProcessorBus>& buses = (isInput ? busArrangement.inputBuses : busArrangement.outputBuses);

    for (int busIdx = 1; busIdx < buses.size(); ++busIdx)
    {
        if (buses.getReference (busIdx).channels != AudioChannelSet::disabled())
        {
            bool success = setPreferredBusArrangement (isInput, busIdx, AudioChannelSet::disabled());

            ignoreUnused (success);
            // You are using the setPlayConfigDetails method which should only be used on processors
            // with no aux outputs and sidechains. Please use setRateAndBufferSizeDetails and
            // setPreferredBusArrangement instead.
            jassert (success);
        }
    }
}

// Unfortunately the deprecated getInputSpeakerArrangement/getOutputSpeakerArrangement return
// references to strings. Therefore we need to keep a copy. Once getInputSpeakerArrangement is
// removed, we can also remove this function
void AudioProcessor::updateSpeakerFormatStrings()
{
    cachedInputSpeakerArrString.clear();
    cachedOutputSpeakerArrString.clear();

    if (busArrangement.inputBuses.size() > 0)
        cachedInputSpeakerArrString  = busArrangement.inputBuses. getReference (0).channels.getSpeakerArrangementAsString();

    if (busArrangement.outputBuses.size() > 0)
        cachedOutputSpeakerArrString = busArrangement.outputBuses.getReference (0).channels.getSpeakerArrangementAsString();
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

//==============================================================================
int AudioProcessor::AudioBusArrangement::getChannelIndexInProcessBlockBuffer (bool isInput, int busIndex, int channelIndex) const noexcept
{
    const Array<AudioProcessorBus>& ioBus = isInput ? inputBuses : outputBuses;
    jassert (busIndex < ioBus.size());

    for (int i = 0; i < ioBus.size() && i < busIndex; ++i)
        channelIndex += ioBus.getReference(i).channels.size();

    return channelIndex;
}

static int countTotalChannels (const Array<AudioProcessor::AudioProcessorBus>& buses) noexcept
{
    int n = 0;

    for (int i = 0; i < buses.size(); ++i)
        n += buses.getReference(i).channels.size();

    return n;
}

int AudioProcessor::AudioBusArrangement::getTotalNumInputChannels() const noexcept   { return countTotalChannels (inputBuses); }
int AudioProcessor::AudioBusArrangement::getTotalNumOutputChannels() const noexcept  { return countTotalChannels (outputBuses); }

AudioProcessor::AudioProcessorBus::AudioProcessorBus (const String& nm, const AudioChannelSet& chans)
   : name (nm), channels (chans)
{
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

bool AudioProcessorParameter::isOrientationInverted() const { return false; }
bool AudioProcessorParameter::isAutomatable() const         { return true; }
bool AudioProcessorParameter::isMetaParameter() const       { return false; }
int AudioProcessorParameter::getNumSteps() const            { return AudioProcessor::getDefaultNumParameterSteps(); }

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
