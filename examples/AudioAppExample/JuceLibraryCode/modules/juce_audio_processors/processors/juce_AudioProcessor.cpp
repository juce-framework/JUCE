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

static ThreadLocalValue<AudioProcessor::WrapperType> wrapperTypeBeingCreated;

void JUCE_CALLTYPE AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::WrapperType type)
{
    wrapperTypeBeingCreated = type;
}

AudioProcessor::AudioProcessor()
    : wrapperType (wrapperTypeBeingCreated.get()),
      playHead (nullptr),
      sampleRate (0),
      blockSize (0),
      numInputChannels (0),
      numOutputChannels (0),
      latencySamples (0),
      suspended (false),
      nonRealtime (false)
{
}

AudioProcessor::~AudioProcessor()
{
    // ooh, nasty - the editor should have been deleted before the filter
    // that it refers to is deleted..
    jassert (activeEditor == nullptr);

   #if JUCE_DEBUG
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
                                           const int newBlockSize) noexcept
{
    sampleRate = newSampleRate;
    blockSize  = newBlockSize;

    if (numInputChannels != newNumIns || numOutputChannels != newNumOuts)
    {
        numInputChannels  = newNumIns;
        numOutputChannels = newNumOuts;

        numChannelsChanged();
    }
}

void AudioProcessor::numChannelsChanged() {}

void AudioProcessor::setSpeakerArrangement (const String& inputs, const String& outputs)
{
    inputSpeakerArrangement  = inputs;
    outputSpeakerArrangement = outputs;
}

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
       #if JUCE_DEBUG
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
       #if JUCE_DEBUG
        // This means you've called endParameterChangeGesture without having previously called
        // endParameterChangeGesture. That might be fine in most hosts, but better to keep the
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

String AudioProcessor::getParameterName (int index, int maximumStringLength)
{
    if (AudioProcessorParameter* p = managedParameters[index])
        return p->getName (maximumStringLength);

    return getParameterName (index).substring (0, maximumStringLength);
}

const String AudioProcessor::getParameterText (int index)
{
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
}

void AudioProcessor::suspendProcessing (const bool shouldBeSuspended)
{
    const ScopedLock sl (callbackLock);
    suspended = shouldBeSuspended;
}

void AudioProcessor::reset() {}
void AudioProcessor::processBlockBypassed (AudioSampleBuffer&, MidiBuffer&) {}

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
