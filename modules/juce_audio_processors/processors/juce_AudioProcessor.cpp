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

String AudioProcessor::getParameterName (int parameterIndex, int maximumStringLength)
{
    return getParameterName (parameterIndex).substring (0, maximumStringLength);
}

String AudioProcessor::getParameterText (int parameterIndex, int maximumStringLength)
{
    return getParameterText (parameterIndex).substring (0, maximumStringLength);
}

int AudioProcessor::getParameterNumSteps (int /*parameterIndex*/)        { return 0x7fffffff; }
float AudioProcessor::getParameterDefaultValue (int /*parameterIndex*/)  { return 0.0f; }

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

String AudioProcessor::getParameterLabel (int) const            { return String(); }
bool AudioProcessor::isParameterOrientationInverted (int) const { return false; }
bool AudioProcessor::isParameterAutomatable (int) const         { return true; }
bool AudioProcessor::isMetaParameter (int) const                { return false; }

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

    // You must make your hasEditor() method return a consistent result!
    jassert (hasEditor() == (ed != nullptr));

    if (ed != nullptr)
    {
        // you must give your editor comp a size before returning it..
        jassert (ed->getWidth() > 0 && ed->getHeight() > 0);

        const ScopedLock sl (callbackLock);
        activeEditor = ed;
    }

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
