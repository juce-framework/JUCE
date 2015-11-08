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

AudioDeviceManager::AudioDeviceSetup::AudioDeviceSetup()
    : sampleRate (0),
      bufferSize (0),
      useDefaultInputChannels (true),
      useDefaultOutputChannels (true)
{
}

bool AudioDeviceManager::AudioDeviceSetup::operator== (const AudioDeviceManager::AudioDeviceSetup& other) const
{
    return outputDeviceName == other.outputDeviceName
            && inputDeviceName == other.inputDeviceName
            && sampleRate == other.sampleRate
            && bufferSize == other.bufferSize
            && inputChannels == other.inputChannels
            && useDefaultInputChannels == other.useDefaultInputChannels
            && outputChannels == other.outputChannels
            && useDefaultOutputChannels == other.useDefaultOutputChannels;
}

//==============================================================================
class AudioDeviceManager::CallbackHandler  : public AudioIODeviceCallback,
                                             public MidiInputCallback,
                                             public AudioIODeviceType::Listener
{
public:
    CallbackHandler (AudioDeviceManager& adm) noexcept  : owner (adm) {}

private:
    void audioDeviceIOCallback (const float** ins, int numIns, float** outs, int numOuts, int numSamples) override
    {
        owner.audioDeviceIOCallbackInt (ins, numIns, outs, numOuts, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        owner.audioDeviceAboutToStartInt (device);
    }

    void audioDeviceStopped() override
    {
        owner.audioDeviceStoppedInt();
    }

    void audioDeviceError (const String& message) override
    {
        owner.audioDeviceErrorInt (message);
    }

    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) override
    {
        owner.handleIncomingMidiMessageInt (source, message);
    }

    void audioDeviceListChanged() override
    {
        owner.audioDeviceListChanged();
    }

    AudioDeviceManager& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CallbackHandler)
};


//==============================================================================
AudioDeviceManager::AudioDeviceManager()
    : numInputChansNeeded (0),
      numOutputChansNeeded (2),
      listNeedsScanning (true),
      inputLevel (0),
      cpuUsageMs (0),
      timeToCpuScale (0)
{
    callbackHandler = new CallbackHandler (*this);
}

AudioDeviceManager::~AudioDeviceManager()
{
    currentAudioDevice = nullptr;
    defaultMidiOutput = nullptr;
}


//==============================================================================
void AudioDeviceManager::createDeviceTypesIfNeeded()
{
    if (availableDeviceTypes.size() == 0)
    {
        OwnedArray<AudioIODeviceType> types;
        createAudioDeviceTypes (types);

        for (int i = 0; i < types.size(); ++i)
            addAudioDeviceType (types.getUnchecked(i));

        types.clear (false);

        if (AudioIODeviceType* first = availableDeviceTypes.getFirst())
            currentDeviceType = first->getTypeName();
    }
}

const OwnedArray<AudioIODeviceType>& AudioDeviceManager::getAvailableDeviceTypes()
{
    scanDevicesIfNeeded();
    return availableDeviceTypes;
}

void AudioDeviceManager::audioDeviceListChanged()
{
    if (currentAudioDevice != nullptr)
    {
        currentSetup.sampleRate     = currentAudioDevice->getCurrentSampleRate();
        currentSetup.bufferSize     = currentAudioDevice->getCurrentBufferSizeSamples();
        currentSetup.inputChannels  = currentAudioDevice->getActiveInputChannels();
        currentSetup.outputChannels = currentAudioDevice->getActiveOutputChannels();
    }

    sendChangeMessage();
}

//==============================================================================
static void addIfNotNull (OwnedArray<AudioIODeviceType>& list, AudioIODeviceType* const device)
{
    if (device != nullptr)
        list.add (device);
}

void AudioDeviceManager::createAudioDeviceTypes (OwnedArray<AudioIODeviceType>& list)
{
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (false));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (true));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_DirectSound());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_ASIO());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_CoreAudio());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_iOSAudio());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_ALSA());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_JACK());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_OpenSLES());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_Android());
}

void AudioDeviceManager::addAudioDeviceType (AudioIODeviceType* newDeviceType)
{
    if (newDeviceType != nullptr)
    {
        jassert (lastDeviceTypeConfigs.size() == availableDeviceTypes.size());
        availableDeviceTypes.add (newDeviceType);
        lastDeviceTypeConfigs.add (new AudioDeviceSetup());

        newDeviceType->addListener (callbackHandler);
    }
}

static bool deviceListContains (AudioIODeviceType* type, bool isInput, const String& name)
{
    StringArray devices (type->getDeviceNames (isInput));

    for (int i = devices.size(); --i >= 0;)
        if (devices[i].trim().equalsIgnoreCase (name.trim()))
            return true;

    return false;
}

//==============================================================================
String AudioDeviceManager::initialise (const int numInputChannelsNeeded,
                                       const int numOutputChannelsNeeded,
                                       const XmlElement* const xml,
                                       const bool selectDefaultDeviceOnFailure,
                                       const String& preferredDefaultDeviceName,
                                       const AudioDeviceSetup* preferredSetupOptions)
{
    scanDevicesIfNeeded();

    numInputChansNeeded = numInputChannelsNeeded;
    numOutputChansNeeded = numOutputChannelsNeeded;

    if (xml != nullptr && xml->hasTagName ("DEVICESETUP"))
        return initialiseFromXML (*xml, selectDefaultDeviceOnFailure,
                                  preferredDefaultDeviceName, preferredSetupOptions);

    return initialiseDefault (preferredDefaultDeviceName, preferredSetupOptions);
}

String AudioDeviceManager::initialiseDefault (const String& preferredDefaultDeviceName,
                                              const AudioDeviceSetup* preferredSetupOptions)
{
    AudioDeviceSetup setup;

    if (preferredSetupOptions != nullptr)
    {
        setup = *preferredSetupOptions;
    }
    else if (preferredDefaultDeviceName.isNotEmpty())
    {
        for (int j = availableDeviceTypes.size(); --j >= 0;)
        {
            AudioIODeviceType* const type = availableDeviceTypes.getUnchecked(j);

            const StringArray outs (type->getDeviceNames (false));

            for (int i = 0; i < outs.size(); ++i)
            {
                if (outs[i].matchesWildcard (preferredDefaultDeviceName, true))
                {
                    setup.outputDeviceName = outs[i];
                    break;
                }
            }

            const StringArray ins (type->getDeviceNames (true));

            for (int i = 0; i < ins.size(); ++i)
            {
                if (ins[i].matchesWildcard (preferredDefaultDeviceName, true))
                {
                    setup.inputDeviceName = ins[i];
                    break;
                }
            }
        }
    }

    insertDefaultDeviceNames (setup);
    return setAudioDeviceSetup (setup, false);
}

String AudioDeviceManager::initialiseFromXML (const XmlElement& xml,
                                              const bool selectDefaultDeviceOnFailure,
                                              const String& preferredDefaultDeviceName,
                                              const AudioDeviceSetup* preferredSetupOptions)
{
    lastExplicitSettings = new XmlElement (xml);

    String error;
    AudioDeviceSetup setup;

    if (preferredSetupOptions != nullptr)
        setup = *preferredSetupOptions;

    if (xml.getStringAttribute ("audioDeviceName").isNotEmpty())
    {
        setup.inputDeviceName = setup.outputDeviceName
            = xml.getStringAttribute ("audioDeviceName");
    }
    else
    {
        setup.inputDeviceName  = xml.getStringAttribute ("audioInputDeviceName");
        setup.outputDeviceName = xml.getStringAttribute ("audioOutputDeviceName");
    }

    currentDeviceType = xml.getStringAttribute ("deviceType");

    if (findType (currentDeviceType) == nullptr)
    {
        if (AudioIODeviceType* const type = findType (setup.inputDeviceName, setup.outputDeviceName))
            currentDeviceType = type->getTypeName();
        else if (availableDeviceTypes.size() > 0)
            currentDeviceType = availableDeviceTypes.getUnchecked(0)->getTypeName();
    }

    setup.bufferSize = xml.getIntAttribute ("audioDeviceBufferSize");
    setup.sampleRate = xml.getDoubleAttribute ("audioDeviceRate");

    setup.inputChannels .parseString (xml.getStringAttribute ("audioDeviceInChans",  "11"), 2);
    setup.outputChannels.parseString (xml.getStringAttribute ("audioDeviceOutChans", "11"), 2);

    setup.useDefaultInputChannels  = ! xml.hasAttribute ("audioDeviceInChans");
    setup.useDefaultOutputChannels = ! xml.hasAttribute ("audioDeviceOutChans");

    error = setAudioDeviceSetup (setup, true);

    midiInsFromXml.clear();

    forEachXmlChildElementWithTagName (xml, c, "MIDIINPUT")
        midiInsFromXml.add (c->getStringAttribute ("name"));

    const StringArray allMidiIns (MidiInput::getDevices());

    for (int i = allMidiIns.size(); --i >= 0;)
        setMidiInputEnabled (allMidiIns[i], midiInsFromXml.contains (allMidiIns[i]));

    if (error.isNotEmpty() && selectDefaultDeviceOnFailure)
        error = initialise (numInputChansNeeded, numOutputChansNeeded,
                            nullptr, false, preferredDefaultDeviceName);

    setDefaultMidiOutput (xml.getStringAttribute ("defaultMidiOutput"));

    return error;
}

String AudioDeviceManager::initialiseWithDefaultDevices (int numInputChannelsNeeded,
                                                         int numOutputChannelsNeeded)
{
    lastExplicitSettings = nullptr;

    return initialise (numInputChannelsNeeded, numOutputChannelsNeeded,
                       nullptr, false, String(), nullptr);
}

void AudioDeviceManager::insertDefaultDeviceNames (AudioDeviceSetup& setup) const
{
    if (AudioIODeviceType* type = getCurrentDeviceTypeObject())
    {
        if (setup.outputDeviceName.isEmpty())
            setup.outputDeviceName = type->getDeviceNames (false) [type->getDefaultDeviceIndex (false)];

        if (setup.inputDeviceName.isEmpty())
            setup.inputDeviceName = type->getDeviceNames (true) [type->getDefaultDeviceIndex (true)];
    }
}

XmlElement* AudioDeviceManager::createStateXml() const
{
    return lastExplicitSettings.createCopy();
}

//==============================================================================
void AudioDeviceManager::scanDevicesIfNeeded()
{
    if (listNeedsScanning)
    {
        listNeedsScanning = false;

        createDeviceTypesIfNeeded();

        for (int i = availableDeviceTypes.size(); --i >= 0;)
            availableDeviceTypes.getUnchecked(i)->scanForDevices();
    }
}

AudioIODeviceType* AudioDeviceManager::findType (const String& typeName)
{
    scanDevicesIfNeeded();

    for (int i = availableDeviceTypes.size(); --i >= 0;)
        if (availableDeviceTypes.getUnchecked(i)->getTypeName() == typeName)
            return availableDeviceTypes.getUnchecked(i);

    return nullptr;
}

AudioIODeviceType* AudioDeviceManager::findType (const String& inputName, const String& outputName)
{
    scanDevicesIfNeeded();

    for (int i = availableDeviceTypes.size(); --i >= 0;)
    {
        AudioIODeviceType* const type = availableDeviceTypes.getUnchecked(i);

        if ((inputName.isNotEmpty() && deviceListContains (type, true, inputName))
             || (outputName.isNotEmpty() && deviceListContains (type, false, outputName)))
        {
            return type;
        }
    }

    return nullptr;
}

void AudioDeviceManager::getAudioDeviceSetup (AudioDeviceSetup& setup)
{
    setup = currentSetup;
}

void AudioDeviceManager::deleteCurrentDevice()
{
    currentAudioDevice = nullptr;
    currentSetup.inputDeviceName.clear();
    currentSetup.outputDeviceName.clear();
}

void AudioDeviceManager::setCurrentAudioDeviceType (const String& type,
                                                    const bool treatAsChosenDevice)
{
    for (int i = 0; i < availableDeviceTypes.size(); ++i)
    {
        if (availableDeviceTypes.getUnchecked(i)->getTypeName() == type
             && currentDeviceType != type)
        {
            if (currentAudioDevice != nullptr)
            {
                closeAudioDevice();
                Thread::sleep (1500); // allow a moment for OS devices to sort themselves out, to help
                                      // avoid things like DirectSound/ASIO clashes
            }

            currentDeviceType = type;

            AudioDeviceSetup s (*lastDeviceTypeConfigs.getUnchecked(i));
            insertDefaultDeviceNames (s);

            setAudioDeviceSetup (s, treatAsChosenDevice);

            sendChangeMessage();
            break;
        }
    }
}

AudioIODeviceType* AudioDeviceManager::getCurrentDeviceTypeObject() const
{
    for (int i = 0; i < availableDeviceTypes.size(); ++i)
        if (availableDeviceTypes.getUnchecked(i)->getTypeName() == currentDeviceType)
            return availableDeviceTypes.getUnchecked(i);

    return availableDeviceTypes[0];
}

String AudioDeviceManager::setAudioDeviceSetup (const AudioDeviceSetup& newSetup,
                                                const bool treatAsChosenDevice)
{
    jassert (&newSetup != &currentSetup);    // this will have no effect

    if (newSetup == currentSetup && currentAudioDevice != nullptr)
        return String();

    if (! (newSetup == currentSetup))
        sendChangeMessage();

    stopDevice();

    const String newInputDeviceName  (numInputChansNeeded  == 0 ? String() : newSetup.inputDeviceName);
    const String newOutputDeviceName (numOutputChansNeeded == 0 ? String() : newSetup.outputDeviceName);

    String error;
    AudioIODeviceType* type = getCurrentDeviceTypeObject();

    if (type == nullptr || (newInputDeviceName.isEmpty() && newOutputDeviceName.isEmpty()))
    {
        deleteCurrentDevice();

        if (treatAsChosenDevice)
            updateXml();

        return String();
    }

    if (currentSetup.inputDeviceName != newInputDeviceName
         || currentSetup.outputDeviceName != newOutputDeviceName
         || currentAudioDevice == nullptr)
    {
        deleteCurrentDevice();
        scanDevicesIfNeeded();

        if (newOutputDeviceName.isNotEmpty() && ! deviceListContains (type, false, newOutputDeviceName))
            return "No such device: " + newOutputDeviceName;

        if (newInputDeviceName.isNotEmpty() && ! deviceListContains (type, true, newInputDeviceName))
            return "No such device: " + newInputDeviceName;

        currentAudioDevice = type->createDevice (newOutputDeviceName, newInputDeviceName);

        if (currentAudioDevice == nullptr)
            error = "Can't open the audio device!\n\n"
                    "This may be because another application is currently using the same device - "
                    "if so, you should close any other applications and try again!";
        else
            error = currentAudioDevice->getLastError();

        if (error.isNotEmpty())
        {
            deleteCurrentDevice();
            return error;
        }

        if (newSetup.useDefaultInputChannels)
        {
            inputChannels.clear();
            inputChannels.setRange (0, numInputChansNeeded, true);
        }

        if (newSetup.useDefaultOutputChannels)
        {
            outputChannels.clear();
            outputChannels.setRange (0, numOutputChansNeeded, true);
        }

        if (newInputDeviceName.isEmpty())  inputChannels.clear();
        if (newOutputDeviceName.isEmpty()) outputChannels.clear();
    }

    if (! newSetup.useDefaultInputChannels)    inputChannels  = newSetup.inputChannels;
    if (! newSetup.useDefaultOutputChannels)   outputChannels = newSetup.outputChannels;

    currentSetup = newSetup;

    currentSetup.sampleRate = chooseBestSampleRate (newSetup.sampleRate);
    currentSetup.bufferSize = chooseBestBufferSize (newSetup.bufferSize);

    error = currentAudioDevice->open (inputChannels,
                                      outputChannels,
                                      currentSetup.sampleRate,
                                      currentSetup.bufferSize);

    if (error.isEmpty())
    {
        currentDeviceType = currentAudioDevice->getTypeName();

        currentAudioDevice->start (callbackHandler);

        currentSetup.sampleRate     = currentAudioDevice->getCurrentSampleRate();
        currentSetup.bufferSize     = currentAudioDevice->getCurrentBufferSizeSamples();
        currentSetup.inputChannels  = currentAudioDevice->getActiveInputChannels();
        currentSetup.outputChannels = currentAudioDevice->getActiveOutputChannels();

        for (int i = 0; i < availableDeviceTypes.size(); ++i)
            if (availableDeviceTypes.getUnchecked (i)->getTypeName() == currentDeviceType)
                *(lastDeviceTypeConfigs.getUnchecked (i)) = currentSetup;

        if (treatAsChosenDevice)
            updateXml();
    }
    else
    {
        deleteCurrentDevice();
    }

    return error;
}

double AudioDeviceManager::chooseBestSampleRate (double rate) const
{
    jassert (currentAudioDevice != nullptr);

    const Array<double> rates (currentAudioDevice->getAvailableSampleRates());

    if (rate > 0 && rates.contains (rate))
        return rate;

    rate = currentAudioDevice->getCurrentSampleRate();

    if (rate > 0 && rates.contains (rate))
        return rate;

    double lowestAbove44 = 0.0;

    for (int i = rates.size(); --i >= 0;)
    {
        const double sr = rates[i];

        if (sr >= 44100.0 && (lowestAbove44 < 1.0 || sr < lowestAbove44))
            lowestAbove44 = sr;
    }

    if (lowestAbove44 > 0.0)
        return lowestAbove44;

    return rates[0];
}

int AudioDeviceManager::chooseBestBufferSize (int bufferSize) const
{
    jassert (currentAudioDevice != nullptr);

    if (bufferSize > 0 && currentAudioDevice->getAvailableBufferSizes().contains (bufferSize))
        return bufferSize;

    return currentAudioDevice->getDefaultBufferSize();
}

void AudioDeviceManager::stopDevice()
{
    if (currentAudioDevice != nullptr)
        currentAudioDevice->stop();
}

void AudioDeviceManager::closeAudioDevice()
{
    stopDevice();
    currentAudioDevice = nullptr;
}

void AudioDeviceManager::restartLastAudioDevice()
{
    if (currentAudioDevice == nullptr)
    {
        if (currentSetup.inputDeviceName.isEmpty()
              && currentSetup.outputDeviceName.isEmpty())
        {
            // This method will only reload the last device that was running
            // before closeAudioDevice() was called - you need to actually open
            // one first, with setAudioDevice().
            jassertfalse;
            return;
        }

        AudioDeviceSetup s (currentSetup);
        setAudioDeviceSetup (s, false);
    }
}

void AudioDeviceManager::updateXml()
{
    lastExplicitSettings = new XmlElement ("DEVICESETUP");

    lastExplicitSettings->setAttribute ("deviceType", currentDeviceType);
    lastExplicitSettings->setAttribute ("audioOutputDeviceName", currentSetup.outputDeviceName);
    lastExplicitSettings->setAttribute ("audioInputDeviceName", currentSetup.inputDeviceName);

    if (currentAudioDevice != nullptr)
    {
        lastExplicitSettings->setAttribute ("audioDeviceRate", currentAudioDevice->getCurrentSampleRate());

        if (currentAudioDevice->getDefaultBufferSize() != currentAudioDevice->getCurrentBufferSizeSamples())
            lastExplicitSettings->setAttribute ("audioDeviceBufferSize", currentAudioDevice->getCurrentBufferSizeSamples());

        if (! currentSetup.useDefaultInputChannels)
            lastExplicitSettings->setAttribute ("audioDeviceInChans", currentSetup.inputChannels.toString (2));

        if (! currentSetup.useDefaultOutputChannels)
            lastExplicitSettings->setAttribute ("audioDeviceOutChans", currentSetup.outputChannels.toString (2));
    }

    for (int i = 0; i < enabledMidiInputs.size(); ++i)
        lastExplicitSettings->createNewChildElement ("MIDIINPUT")
                            ->setAttribute ("name", enabledMidiInputs[i]->getName());

    if (midiInsFromXml.size() > 0)
    {
        // Add any midi devices that have been enabled before, but which aren't currently
        // open because the device has been disconnected.
        const StringArray availableMidiDevices (MidiInput::getDevices());

        for (int i = 0; i < midiInsFromXml.size(); ++i)
            if (! availableMidiDevices.contains (midiInsFromXml[i], true))
                lastExplicitSettings->createNewChildElement ("MIDIINPUT")
                                    ->setAttribute ("name", midiInsFromXml[i]);
    }

    if (defaultMidiOutputName.isNotEmpty())
        lastExplicitSettings->setAttribute ("defaultMidiOutput", defaultMidiOutputName);
}

//==============================================================================
void AudioDeviceManager::addAudioCallback (AudioIODeviceCallback* newCallback)
{
    {
        const ScopedLock sl (audioCallbackLock);
        if (callbacks.contains (newCallback))
            return;
    }

    if (currentAudioDevice != nullptr && newCallback != nullptr)
        newCallback->audioDeviceAboutToStart (currentAudioDevice);

    const ScopedLock sl (audioCallbackLock);
    callbacks.add (newCallback);
}

void AudioDeviceManager::removeAudioCallback (AudioIODeviceCallback* callbackToRemove)
{
    if (callbackToRemove != nullptr)
    {
        bool needsDeinitialising = currentAudioDevice != nullptr;

        {
            const ScopedLock sl (audioCallbackLock);

            needsDeinitialising = needsDeinitialising && callbacks.contains (callbackToRemove);
            callbacks.removeFirstMatchingValue (callbackToRemove);
        }

        if (needsDeinitialising)
            callbackToRemove->audioDeviceStopped();
    }
}

void AudioDeviceManager::audioDeviceIOCallbackInt (const float** inputChannelData,
                                                   int numInputChannels,
                                                   float** outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples)
{
    const ScopedLock sl (audioCallbackLock);

    if (inputLevelMeasurementEnabledCount.get() > 0 && numInputChannels > 0)
    {
        for (int j = 0; j < numSamples; ++j)
        {
            float s = 0;

            for (int i = 0; i < numInputChannels; ++i)
                s += std::abs (inputChannelData[i][j]);

            s /= numInputChannels;

            const double decayFactor = 0.99992;

            if (s > inputLevel)
                inputLevel = s;
            else if (inputLevel > 0.001f)
                inputLevel *= decayFactor;
            else
                inputLevel = 0;
        }
    }
    else
    {
        inputLevel = 0;
    }

    if (callbacks.size() > 0)
    {
        const double callbackStartTime = Time::getMillisecondCounterHiRes();

        tempBuffer.setSize (jmax (1, numOutputChannels), jmax (1, numSamples), false, false, true);

        callbacks.getUnchecked(0)->audioDeviceIOCallback (inputChannelData, numInputChannels,
                                                          outputChannelData, numOutputChannels, numSamples);

        float** const tempChans = tempBuffer.getArrayOfWritePointers();

        for (int i = callbacks.size(); --i > 0;)
        {
            callbacks.getUnchecked(i)->audioDeviceIOCallback (inputChannelData, numInputChannels,
                                                              tempChans, numOutputChannels, numSamples);

            for (int chan = 0; chan < numOutputChannels; ++chan)
            {
                if (const float* const src = tempChans [chan])
                    if (float* const dst = outputChannelData [chan])
                        for (int j = 0; j < numSamples; ++j)
                            dst[j] += src[j];
            }
        }

        const double msTaken = Time::getMillisecondCounterHiRes() - callbackStartTime;
        const double filterAmount = 0.2;
        cpuUsageMs += filterAmount * (msTaken - cpuUsageMs);
    }
    else
    {
        for (int i = 0; i < numOutputChannels; ++i)
            zeromem (outputChannelData[i], sizeof (float) * (size_t) numSamples);
    }
}

void AudioDeviceManager::audioDeviceAboutToStartInt (AudioIODevice* const device)
{
    cpuUsageMs = 0;

    const double sampleRate = device->getCurrentSampleRate();
    const int blockSize = device->getCurrentBufferSizeSamples();

    if (sampleRate > 0.0 && blockSize > 0)
    {
        const double msPerBlock = 1000.0 * blockSize / sampleRate;
        timeToCpuScale = (msPerBlock > 0.0) ? (1.0 / msPerBlock) : 0.0;
    }

    {
        const ScopedLock sl (audioCallbackLock);
        for (int i = callbacks.size(); --i >= 0;)
            callbacks.getUnchecked(i)->audioDeviceAboutToStart (device);
    }

    sendChangeMessage();
}

void AudioDeviceManager::audioDeviceStoppedInt()
{
    cpuUsageMs = 0;
    timeToCpuScale = 0;
    sendChangeMessage();

    const ScopedLock sl (audioCallbackLock);
    for (int i = callbacks.size(); --i >= 0;)
        callbacks.getUnchecked(i)->audioDeviceStopped();
}

void AudioDeviceManager::audioDeviceErrorInt (const String& message)
{
    const ScopedLock sl (audioCallbackLock);
    for (int i = callbacks.size(); --i >= 0;)
        callbacks.getUnchecked(i)->audioDeviceError (message);
}

double AudioDeviceManager::getCpuUsage() const
{
    return jlimit (0.0, 1.0, timeToCpuScale * cpuUsageMs);
}

//==============================================================================
void AudioDeviceManager::setMidiInputEnabled (const String& name, const bool enabled)
{
    if (enabled != isMidiInputEnabled (name))
    {
        if (enabled)
        {
            const int index = MidiInput::getDevices().indexOf (name);

            if (index >= 0)
            {
                if (MidiInput* const midiIn = MidiInput::openDevice (index, callbackHandler))
                {
                    enabledMidiInputs.add (midiIn);
                    midiIn->start();
                }
            }
        }
        else
        {
            for (int i = enabledMidiInputs.size(); --i >= 0;)
                if (enabledMidiInputs[i]->getName() == name)
                    enabledMidiInputs.remove (i);
        }

        updateXml();
        sendChangeMessage();
    }
}

bool AudioDeviceManager::isMidiInputEnabled (const String& name) const
{
    for (int i = enabledMidiInputs.size(); --i >= 0;)
        if (enabledMidiInputs[i]->getName() == name)
            return true;

    return false;
}

void AudioDeviceManager::addMidiInputCallback (const String& name, MidiInputCallback* callbackToAdd)
{
    removeMidiInputCallback (name, callbackToAdd);

    if (name.isEmpty() || isMidiInputEnabled (name))
    {
        const ScopedLock sl (midiCallbackLock);

        MidiCallbackInfo mc;
        mc.deviceName = name;
        mc.callback = callbackToAdd;
        midiCallbacks.add (mc);
    }
}

void AudioDeviceManager::removeMidiInputCallback (const String& name, MidiInputCallback* callbackToRemove)
{
    for (int i = midiCallbacks.size(); --i >= 0;)
    {
        const MidiCallbackInfo& mc = midiCallbacks.getReference(i);

        if (mc.callback == callbackToRemove && mc.deviceName == name)
        {
            const ScopedLock sl (midiCallbackLock);
            midiCallbacks.remove (i);
        }
    }
}

void AudioDeviceManager::handleIncomingMidiMessageInt (MidiInput* source, const MidiMessage& message)
{
    if (! message.isActiveSense())
    {
        const ScopedLock sl (midiCallbackLock);

        for (int i = 0; i < midiCallbacks.size(); ++i)
        {
            const MidiCallbackInfo& mc = midiCallbacks.getReference(i);

            if (mc.deviceName.isEmpty() || mc.deviceName == source->getName())
                mc.callback->handleIncomingMidiMessage (source, message);
        }
    }
}

//==============================================================================
void AudioDeviceManager::setDefaultMidiOutput (const String& deviceName)
{
    if (defaultMidiOutputName != deviceName)
    {
        Array<AudioIODeviceCallback*> oldCallbacks;

        {
            const ScopedLock sl (audioCallbackLock);
            oldCallbacks.swapWith (callbacks);
        }

        if (currentAudioDevice != nullptr)
            for (int i = oldCallbacks.size(); --i >= 0;)
                oldCallbacks.getUnchecked(i)->audioDeviceStopped();

        defaultMidiOutput = nullptr;
        defaultMidiOutputName = deviceName;

        if (deviceName.isNotEmpty())
            defaultMidiOutput = MidiOutput::openDevice (MidiOutput::getDevices().indexOf (deviceName));

        if (currentAudioDevice != nullptr)
            for (int i = oldCallbacks.size(); --i >= 0;)
                oldCallbacks.getUnchecked(i)->audioDeviceAboutToStart (currentAudioDevice);

        {
            const ScopedLock sl (audioCallbackLock);
            oldCallbacks.swapWith (callbacks);
        }

        updateXml();
        sendChangeMessage();
    }
}

//==============================================================================
// This is an AudioTransportSource which will own it's assigned source
class AudioSourceOwningTransportSource  : public AudioTransportSource
{
public:
    AudioSourceOwningTransportSource() {}
    ~AudioSourceOwningTransportSource()  { setSource (nullptr); }

    void setSource (PositionableAudioSource* newSource)
    {
        if (src != newSource)
        {
            ScopedPointer<PositionableAudioSource> oldSourceDeleter (src);
            src = newSource;

            // tell the base class about the new source before deleting the old one
            AudioTransportSource::setSource (newSource);
        }
    }

private:
    ScopedPointer<PositionableAudioSource> src;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourceOwningTransportSource)
};

//==============================================================================
// An Audio player which will remove itself from the AudioDeviceManager's
// callback list once it finishes playing its source
class AutoRemovingSourcePlayer  : public AudioSourcePlayer,
                                  private ChangeListener
{
public:
    struct DeleteOnMessageThread  : public CallbackMessage
    {
        DeleteOnMessageThread (AutoRemovingSourcePlayer* p)  : parent (p) {}
        void messageCallback() override     { delete parent; }

        AutoRemovingSourcePlayer* parent;
    };

    //==============================================================================
    AutoRemovingSourcePlayer (AudioDeviceManager& deviceManager, bool ownSource)
        : manager (deviceManager),
          deleteWhenDone (ownSource),
          hasAddedCallback (false),
          recursiveEntry (false)
    {
    }

    void changeListenerCallback (ChangeBroadcaster* newSource) override
    {
        if (AudioTransportSource* currentTransport
               = dynamic_cast<AudioTransportSource*> (getCurrentSource()))
        {
            ignoreUnused (newSource);
            jassert (newSource == currentTransport);

            if (! currentTransport->isPlaying())
            {
                // this will call audioDeviceStopped!
                manager.removeAudioCallback (this);
            }
            else if (! hasAddedCallback)
            {
                hasAddedCallback = true;
                manager.addAudioCallback (this);
            }
        }
    }

    void audioDeviceStopped() override
    {
        if (! recursiveEntry)
        {
            ScopedValueSetter<bool> s (recursiveEntry, true, false);

            manager.removeAudioCallback (this);
            AudioSourcePlayer::audioDeviceStopped();

            if (MessageManager* mm = MessageManager::getInstanceWithoutCreating())
            {
                if (mm->isThisTheMessageThread())
                    delete this;
                else
                    (new DeleteOnMessageThread (this))->post();
            }
        }
    }

    void setSource (AudioTransportSource* newSource)
    {
        AudioSource* oldSource = getCurrentSource();

        if (AudioTransportSource* oldTransport = dynamic_cast<AudioTransportSource*> (oldSource))
            oldTransport->removeChangeListener (this);

        if (newSource != nullptr)
            newSource->addChangeListener (this);

        AudioSourcePlayer::setSource (newSource);

        if (deleteWhenDone)
            delete oldSource;
    }

private:
    // only allow myself to be deleted when my audio callback has been removed
    ~AutoRemovingSourcePlayer()
    {
        setSource (nullptr);
    }

    AudioDeviceManager& manager;
    bool deleteWhenDone, hasAddedCallback, recursiveEntry;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoRemovingSourcePlayer)
};

//==============================================================================
// An AudioSource which simply outputs a buffer
class AudioSampleBufferSource : public PositionableAudioSource
{
public:
    AudioSampleBufferSource (AudioSampleBuffer* audioBuffer, bool shouldLoop, bool ownBuffer)
        : position (0),
          buffer (audioBuffer),
          looping (shouldLoop),
          deleteWhenDone (ownBuffer)
    {}

    ~AudioSampleBufferSource()
    {
        if (deleteWhenDone)
            delete buffer;
    }

    //==============================================================================
    void setNextReadPosition (int64 newPosition) override
    {
        jassert (newPosition >= 0);

        if (looping)
            newPosition = newPosition % static_cast<int64> (buffer->getNumSamples());

        position = jmin (buffer->getNumSamples(), static_cast<int> (newPosition));
    }

    int64 getNextReadPosition() const override
    {
        return static_cast<int64> (position);
    }

    int64 getTotalLength() const override
    {
        return static_cast<int64> (buffer->getNumSamples());
    }

    bool isLooping() const override
    {
        return looping;
    }

    void setLooping (bool shouldLoop) override
    {
        looping = shouldLoop;
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        ignoreUnused (samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() override
    {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        int max = jmin (buffer->getNumSamples() - position, bufferToFill.numSamples);

        jassert (max >= 0);
        {
            int ch;
            int maxInChannels = buffer->getNumChannels();
            int maxOutChannels = jmin (bufferToFill.buffer->getNumChannels(),
                                       jmax (maxInChannels, 2));

            for (ch = 0; ch < maxOutChannels; ch++)
            {
                int inChannel = ch % maxInChannels;

                if (max > 0)
                    bufferToFill.buffer->copyFrom (ch, bufferToFill.startSample, *buffer, inChannel, position, max);
            }

            for (; ch < bufferToFill.buffer->getNumChannels(); ++ch)
                bufferToFill.buffer->clear (ch, bufferToFill.startSample, bufferToFill.numSamples);
        }

        position += max;

        if (looping)
            position = position % buffer->getNumSamples();
    }

private:
    //==============================================================================
    int position;
    AudioSampleBuffer* buffer;
    bool looping, deleteWhenDone;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSampleBufferSource)
};

void AudioDeviceManager::playSound (const File& file)
{
    if (file.existsAsFile())
    {
        AudioFormatManager formatManager;

        formatManager.registerBasicFormats();
        playSound (formatManager.createReaderFor (file), true);
    }
}

void AudioDeviceManager::playSound (const void* resourceData, size_t resourceSize)
{
    if (resourceData != nullptr && resourceSize > 0)
    {
        AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        MemoryInputStream* mem = new MemoryInputStream (resourceData, resourceSize, false);
        playSound (formatManager.createReaderFor (mem), true);
    }
}

void AudioDeviceManager::playSound (AudioFormatReader* reader, bool deleteWhenFinished)
{
    playSound (new AudioFormatReaderSource (reader, deleteWhenFinished), true);
}

void AudioDeviceManager::playSound (PositionableAudioSource* audioSource, bool deleteWhenFinished)
{
    if (audioSource != nullptr && currentAudioDevice != nullptr)
    {
        if (AudioTransportSource* transport = dynamic_cast<AudioTransportSource*> (audioSource))
        {
            AutoRemovingSourcePlayer* player = new AutoRemovingSourcePlayer (*this, deleteWhenFinished);
            player->setSource (transport);
        }
        else
        {
            AudioTransportSource* transportSource;

            if (deleteWhenFinished)
            {
                AudioSourceOwningTransportSource* owningTransportSource = new AudioSourceOwningTransportSource();
                owningTransportSource->setSource (audioSource);
                transportSource = owningTransportSource;
            }
            else
            {
                transportSource = new AudioTransportSource;
                transportSource->setSource (audioSource);
            }

            // recursively call myself
            playSound (transportSource, true);
            transportSource->start();
        }
    }
    else
    {
        if (deleteWhenFinished)
            delete audioSource;
    }
}

void AudioDeviceManager::playSound (AudioSampleBuffer* buffer, bool deleteWhenFinished)
{
    playSound (new AudioSampleBufferSource (buffer, false, deleteWhenFinished), true);
}

void AudioDeviceManager::playTestSound()
{
    const double sampleRate = currentAudioDevice->getCurrentSampleRate();
    const int soundLength = (int) sampleRate;

    const double frequency = 440.0;
    const float amplitude = 0.5f;

    const double phasePerSample = double_Pi * 2.0 / (sampleRate / frequency);

    AudioSampleBuffer* newSound = new AudioSampleBuffer (1, soundLength);

    for (int i = 0; i < soundLength; ++i)
        newSound->setSample (0, i, amplitude * (float) std::sin (i * phasePerSample));

    newSound->applyGainRamp (0, 0, soundLength / 10, 0.0f, 1.0f);
    newSound->applyGainRamp (0, soundLength - soundLength / 4, soundLength / 4, 1.0f, 0.0f);

    playSound (newSound, true);
}

//==============================================================================
void AudioDeviceManager::enableInputLevelMeasurement (const bool enableMeasurement)
{
    if (enableMeasurement)
        ++inputLevelMeasurementEnabledCount;
    else
        --inputLevelMeasurementEnabledCount;

    inputLevel = 0;
}

double AudioDeviceManager::getCurrentInputLevel() const
{
    jassert (inputLevelMeasurementEnabledCount.get() > 0); // you need to call enableInputLevelMeasurement() before using this!
    return inputLevel;
}
