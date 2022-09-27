/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

template <typename Setup>
static auto getSetupInfo (Setup& s, bool isInput)
{
    struct SetupInfo
    {
        // double brackets so that we get the expression type, i.e. a (possibly const) reference
        decltype ((s.inputDeviceName)) name;
        decltype ((s.inputChannels)) channels;
        decltype ((s.useDefaultInputChannels)) useDefault;
    };

    return isInput ? SetupInfo { s.inputDeviceName,  s.inputChannels,  s.useDefaultInputChannels }
                   : SetupInfo { s.outputDeviceName, s.outputChannels, s.useDefaultOutputChannels };
}

static auto tie (const AudioDeviceManager::AudioDeviceSetup& s)
{
    return std::tie (s.outputDeviceName,
                     s.inputDeviceName,
                     s.sampleRate,
                     s.bufferSize,
                     s.inputChannels,
                     s.useDefaultInputChannels,
                     s.outputChannels,
                     s.useDefaultOutputChannels);
}

bool AudioDeviceManager::AudioDeviceSetup::operator== (const AudioDeviceManager::AudioDeviceSetup& other) const
{
    return tie (*this) == tie (other);
}

bool AudioDeviceManager::AudioDeviceSetup::operator!= (const AudioDeviceManager::AudioDeviceSetup& other) const
{
    return tie (*this) != tie (other);
}

//==============================================================================
class AudioDeviceManager::CallbackHandler  : public AudioIODeviceCallback,
                                             public MidiInputCallback,
                                             public AudioIODeviceType::Listener
{
public:
    CallbackHandler (AudioDeviceManager& adm) noexcept  : owner (adm) {}

private:
    void audioDeviceIOCallbackWithContext (const float* const* ins,
                                           int numIns,
                                           float* const* outs,
                                           int numOuts,
                                           int numSamples,
                                           const AudioIODeviceCallbackContext& context) override
    {
        owner.audioDeviceIOCallbackInt (ins, numIns, outs, numOuts, numSamples, context);
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
{
    callbackHandler.reset (new CallbackHandler (*this));
}

AudioDeviceManager::~AudioDeviceManager()
{
    currentAudioDevice.reset();
    defaultMidiOutput.reset();
}

//==============================================================================
void AudioDeviceManager::createDeviceTypesIfNeeded()
{
    if (availableDeviceTypes.size() == 0)
    {
        OwnedArray<AudioIODeviceType> types;
        createAudioDeviceTypes (types);

        for (auto* t : types)
            addAudioDeviceType (std::unique_ptr<AudioIODeviceType> (t));

        types.clear (false);

        for (auto* type : availableDeviceTypes)
            type->scanForDevices();

        pickCurrentDeviceTypeWithDevices();
    }
}

void AudioDeviceManager::pickCurrentDeviceTypeWithDevices()
{
    const auto deviceTypeHasDevices = [] (const AudioIODeviceType* ptr)
    {
        return ! ptr->getDeviceNames (true) .isEmpty()
            || ! ptr->getDeviceNames (false).isEmpty();
    };

    if (auto* type = findType (currentDeviceType))
        if (deviceTypeHasDevices (type))
            return;

    const auto iter = std::find_if (availableDeviceTypes.begin(),
                                    availableDeviceTypes.end(),
                                    deviceTypeHasDevices);

    if (iter != availableDeviceTypes.end())
        currentDeviceType = (*iter)->getTypeName();
}

const OwnedArray<AudioIODeviceType>& AudioDeviceManager::getAvailableDeviceTypes()
{
    scanDevicesIfNeeded();
    return availableDeviceTypes;
}

void AudioDeviceManager::updateCurrentSetup()
{
    if (currentAudioDevice != nullptr)
    {
        currentSetup.sampleRate     = currentAudioDevice->getCurrentSampleRate();
        currentSetup.bufferSize     = currentAudioDevice->getCurrentBufferSizeSamples();
        currentSetup.inputChannels  = currentAudioDevice->getActiveInputChannels();
        currentSetup.outputChannels = currentAudioDevice->getActiveOutputChannels();
    }
}

void AudioDeviceManager::audioDeviceListChanged()
{
    if (currentAudioDevice != nullptr)
    {
        auto currentDeviceStillAvailable = [&]
        {
            auto currentTypeName = currentAudioDevice->getTypeName();
            auto currentDeviceName = currentAudioDevice->getName();

            for (auto* deviceType : availableDeviceTypes)
            {
                if (currentTypeName == deviceType->getTypeName())
                {
                    for (auto& deviceName : deviceType->getDeviceNames (true))
                        if (currentDeviceName == deviceName)
                            return true;

                    for (auto& deviceName : deviceType->getDeviceNames (false))
                        if (currentDeviceName == deviceName)
                            return true;
                }
            }

            return false;
        }();

        if (! currentDeviceStillAvailable)
        {
            closeAudioDevice();

            if (auto e = createStateXml())
                initialiseFromXML (*e, true, preferredDeviceName, &currentSetup);
            else
                initialiseDefault (preferredDeviceName, &currentSetup);
        }

        updateCurrentSetup();
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
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode::shared));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode::exclusive));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode::sharedLowLatency));
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_DirectSound());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_ASIO());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_CoreAudio());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_iOSAudio());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_Bela());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_ALSA());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_JACK());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_Oboe());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_OpenSLES());
    addIfNotNull (list, AudioIODeviceType::createAudioIODeviceType_Android());
}

void AudioDeviceManager::addAudioDeviceType (std::unique_ptr<AudioIODeviceType> newDeviceType)
{
    if (newDeviceType != nullptr)
    {
        jassert (lastDeviceTypeConfigs.size() == availableDeviceTypes.size());

        availableDeviceTypes.add (newDeviceType.release());
        lastDeviceTypeConfigs.add (new AudioDeviceSetup());

        availableDeviceTypes.getLast()->addListener (callbackHandler.get());
    }
}

void AudioDeviceManager::removeAudioDeviceType (AudioIODeviceType* deviceTypeToRemove)
{
    if (deviceTypeToRemove != nullptr)
    {
        jassert (lastDeviceTypeConfigs.size() == availableDeviceTypes.size());

        auto index = availableDeviceTypes.indexOf (deviceTypeToRemove);

        if (auto removed = std::unique_ptr<AudioIODeviceType> (availableDeviceTypes.removeAndReturn (index)))
        {
            removed->removeListener (callbackHandler.get());
            lastDeviceTypeConfigs.remove (index, true);
        }
    }
}

static bool deviceListContains (AudioIODeviceType* type, bool isInput, const String& name)
{
    for (auto& deviceName : type->getDeviceNames (isInput))
        if (deviceName.trim().equalsIgnoreCase (name.trim()))
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
    pickCurrentDeviceTypeWithDevices();

    numInputChansNeeded = numInputChannelsNeeded;
    numOutputChansNeeded = numOutputChannelsNeeded;
    preferredDeviceName = preferredDefaultDeviceName;

    if (xml != nullptr && xml->hasTagName ("DEVICESETUP"))
        return initialiseFromXML (*xml, selectDefaultDeviceOnFailure,
                                  preferredDeviceName, preferredSetupOptions);

    return initialiseDefault (preferredDeviceName, preferredSetupOptions);
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
        const auto nameMatches = [&preferredDefaultDeviceName] (const String& name)
        {
            return name.matchesWildcard (preferredDefaultDeviceName, true);
        };

        struct WildcardMatch
        {
            String value;
            bool successful;
        };

        const auto getWildcardMatch = [&nameMatches] (const StringArray& names)
        {
            const auto iter = std::find_if (names.begin(), names.end(), nameMatches);
            return WildcardMatch { iter != names.end() ? *iter : String(), iter != names.end() };
        };

        struct WildcardMatches
        {
            WildcardMatch input, output;
        };

        const auto getMatchesForType = [&getWildcardMatch] (const AudioIODeviceType* type)
        {
            return WildcardMatches { getWildcardMatch (type->getDeviceNames (true)),
                                     getWildcardMatch (type->getDeviceNames (false)) };
        };

        struct SearchResult
        {
            String type, input, output;
        };

        const auto result = [&]
        {
            // First, look for a device type with an input and output which matches the preferred name
            for (auto* type : availableDeviceTypes)
            {
                const auto matches = getMatchesForType (type);

                if (matches.input.successful && matches.output.successful)
                    return SearchResult { type->getTypeName(), matches.input.value, matches.output.value };
            }

            // No device type has matching ins and outs, so fall back to a device where either the
            // input or output match
            for (auto* type : availableDeviceTypes)
            {
                const auto matches = getMatchesForType (type);

                if (matches.input.successful || matches.output.successful)
                    return SearchResult { type->getTypeName(), matches.input.value, matches.output.value };
            }

            // No devices match the query, so just use the default devices from the current type
            return SearchResult { currentDeviceType, {}, {} };
        }();

        currentDeviceType = result.type;
        setup.inputDeviceName = result.input;
        setup.outputDeviceName = result.output;
    }

    insertDefaultDeviceNames (setup);
    return setAudioDeviceSetup (setup, false);
}

String AudioDeviceManager::initialiseFromXML (const XmlElement& xml,
                                              bool selectDefaultDeviceOnFailure,
                                              const String& preferredDefaultDeviceName,
                                              const AudioDeviceSetup* preferredSetupOptions)
{
    lastExplicitSettings.reset (new XmlElement (xml));

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
        if (auto* type = findType (setup.inputDeviceName, setup.outputDeviceName))
            currentDeviceType = type->getTypeName();
        else if (auto* firstType = availableDeviceTypes.getFirst())
            currentDeviceType = firstType->getTypeName();
    }

    setup.bufferSize = xml.getIntAttribute ("audioDeviceBufferSize", setup.bufferSize);
    setup.sampleRate = xml.getDoubleAttribute ("audioDeviceRate", setup.sampleRate);

    setup.inputChannels .parseString (xml.getStringAttribute ("audioDeviceInChans",  "11"), 2);
    setup.outputChannels.parseString (xml.getStringAttribute ("audioDeviceOutChans", "11"), 2);

    setup.useDefaultInputChannels  = ! xml.hasAttribute ("audioDeviceInChans");
    setup.useDefaultOutputChannels = ! xml.hasAttribute ("audioDeviceOutChans");

    error = setAudioDeviceSetup (setup, true);

    if (error.isNotEmpty() && selectDefaultDeviceOnFailure)
        error = initialise (numInputChansNeeded, numOutputChansNeeded, nullptr, false, preferredDefaultDeviceName);

    midiDeviceInfosFromXml.clear();
    enabledMidiInputs.clear();

    for (auto* c : xml.getChildWithTagNameIterator ("MIDIINPUT"))
        midiDeviceInfosFromXml.add ({ c->getStringAttribute ("name"), c->getStringAttribute ("identifier") });

    auto isIdentifierAvailable = [] (const Array<MidiDeviceInfo>& available, const String& identifier)
    {
        for (auto& device : available)
            if (device.identifier == identifier)
                return true;

        return false;
    };

    auto getUpdatedIdentifierForName = [&] (const Array<MidiDeviceInfo>& available, const String& name) -> String
    {
        for (auto& device : available)
            if (device.name == name)
                return device.identifier;

        return {};
    };

    auto inputs = MidiInput::getAvailableDevices();

    for (auto& info : midiDeviceInfosFromXml)
    {
        if (isIdentifierAvailable (inputs, info.identifier))
        {
            setMidiInputDeviceEnabled (info.identifier, true);
        }
        else
        {
            auto identifier = getUpdatedIdentifierForName (inputs, info.name);

            if (identifier.isNotEmpty())
                setMidiInputDeviceEnabled (identifier, true);
        }
    }

    MidiDeviceInfo defaultOutputDeviceInfo (xml.getStringAttribute ("defaultMidiOutput"),
                                            xml.getStringAttribute ("defaultMidiOutputDevice"));

    auto outputs = MidiOutput::getAvailableDevices();

    if (isIdentifierAvailable (outputs, defaultOutputDeviceInfo.identifier))
    {
        setDefaultMidiOutputDevice (defaultOutputDeviceInfo.identifier);
    }
    else
    {
        auto identifier = getUpdatedIdentifierForName (outputs, defaultOutputDeviceInfo.name);

        if (identifier.isNotEmpty())
            setDefaultMidiOutputDevice (identifier);
    }

    return error;
}

String AudioDeviceManager::initialiseWithDefaultDevices (int numInputChannelsNeeded,
                                                         int numOutputChannelsNeeded)
{
    lastExplicitSettings.reset();

    return initialise (numInputChannelsNeeded, numOutputChannelsNeeded,
                       nullptr, false, {}, nullptr);
}

void AudioDeviceManager::insertDefaultDeviceNames (AudioDeviceSetup& setup) const
{
    enum class Direction { out, in };

    if (auto* type = getCurrentDeviceTypeObject())
    {
        // We avoid selecting a device pair that doesn't share a matching sample rate, if possible.
        // If not, other parts of the AudioDeviceManager and AudioIODevice classes should generate
        // an appropriate error message when opening or starting these devices.
        const auto getDevicesToTestForMatchingSampleRate = [&setup, type, this] (Direction dir)
        {
            const auto isInput = dir == Direction::in;
            const auto info = getSetupInfo (setup, isInput);

            if (! info.name.isEmpty())
                return StringArray { info.name };

            const auto numChannelsNeeded = isInput ? numInputChansNeeded : numOutputChansNeeded;
            auto deviceNames = numChannelsNeeded > 0 ? type->getDeviceNames (isInput) : StringArray {};
            deviceNames.move (type->getDefaultDeviceIndex (isInput), 0);

            return deviceNames;
        };

        std::map<std::pair<Direction, String>, Array<double>> sampleRatesCache;

        const auto getSupportedSampleRates = [&sampleRatesCache, type] (Direction dir, const String& deviceName)
        {
            const auto key = std::make_pair (dir, deviceName);

            auto& entry = [&]() -> auto&
            {
                auto it = sampleRatesCache.find (key);

                if (it != sampleRatesCache.end())
                    return it->second;

                auto& elem = sampleRatesCache[key];
                auto tempDevice = rawToUniquePtr (type->createDevice ((dir == Direction::in) ? "" : deviceName,
                                                                      (dir == Direction::in) ? deviceName : ""));
                if (tempDevice != nullptr)
                    elem = tempDevice->getAvailableSampleRates();

                return elem;
            }();

            return entry;
        };

        const auto validate = [&getSupportedSampleRates] (const String& outputDeviceName, const String& inputDeviceName)
        {
            jassert (! outputDeviceName.isEmpty() && ! inputDeviceName.isEmpty());

            const auto outputSampleRates = getSupportedSampleRates (Direction::out, outputDeviceName);
            const auto inputSampleRates  = getSupportedSampleRates (Direction::in,  inputDeviceName);

            return std::any_of (inputSampleRates.begin(),
                                inputSampleRates.end(),
                                [&] (auto inputSampleRate) { return outputSampleRates.contains (inputSampleRate); });
        };

        auto outputsToTest = getDevicesToTestForMatchingSampleRate (Direction::out);
        auto inputsToTest  = getDevicesToTestForMatchingSampleRate (Direction::in);

        // We set default device names, so in case no in-out pair passes the validation, we still
        // produce the same result as before
        if (setup.outputDeviceName.isEmpty() && ! outputsToTest.isEmpty())
            setup.outputDeviceName = outputsToTest[0];

        if (setup.inputDeviceName.isEmpty() && ! inputsToTest.isEmpty())
            setup.inputDeviceName = inputsToTest[0];

        // We check all possible in-out pairs until the first validation pass. If no pair passes we
        // leave the setup unchanged.
        for (const auto& out : outputsToTest)
        {
            for (const auto& in : inputsToTest)
            {
                if (validate (out, in))
                {
                    setup.outputDeviceName = out;
                    setup.inputDeviceName  = in;

                    return;
                }
            }
        }
    }
}

std::unique_ptr<XmlElement> AudioDeviceManager::createStateXml() const
{
    if (lastExplicitSettings != nullptr)
        return std::make_unique<XmlElement> (*lastExplicitSettings);

    return {};
}

//==============================================================================
void AudioDeviceManager::scanDevicesIfNeeded()
{
    if (listNeedsScanning)
    {
        listNeedsScanning = false;

        createDeviceTypesIfNeeded();

        for (auto* type : availableDeviceTypes)
            type->scanForDevices();
    }
}

AudioIODeviceType* AudioDeviceManager::findType (const String& typeName)
{
    scanDevicesIfNeeded();

    for (auto* type : availableDeviceTypes)
        if (type->getTypeName() == typeName)
            return type;

    return {};
}

AudioIODeviceType* AudioDeviceManager::findType (const String& inputName, const String& outputName)
{
    scanDevicesIfNeeded();

    for (auto* type : availableDeviceTypes)
        if ((inputName.isNotEmpty() && deviceListContains (type, true, inputName))
             || (outputName.isNotEmpty() && deviceListContains (type, false, outputName)))
            return type;

    return {};
}

AudioDeviceManager::AudioDeviceSetup AudioDeviceManager::getAudioDeviceSetup() const
{
    return currentSetup;
}

void AudioDeviceManager::getAudioDeviceSetup (AudioDeviceSetup& setup) const
{
    setup = currentSetup;
}

void AudioDeviceManager::deleteCurrentDevice()
{
    currentAudioDevice.reset();
    currentSetup.inputDeviceName.clear();
    currentSetup.outputDeviceName.clear();
}

void AudioDeviceManager::setCurrentAudioDeviceType (const String& type, bool treatAsChosenDevice)
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
    for (auto* type : availableDeviceTypes)
        if (type->getTypeName() == currentDeviceType)
            return type;

    return availableDeviceTypes.getFirst();
}

static void updateSetupChannels (AudioDeviceManager::AudioDeviceSetup& setup, int defaultNumIns, int defaultNumOuts)
{
    auto updateChannels = [] (const String& deviceName, BigInteger& channels, int defaultNumChannels)
    {
        if (deviceName.isEmpty())
        {
            channels.clear();
        }
        else if (defaultNumChannels != -1)
        {
            channels.clear();
            channels.setRange (0, defaultNumChannels, true);
        }
    };

    updateChannels (setup.inputDeviceName,  setup.inputChannels,  setup.useDefaultInputChannels  ? defaultNumIns  : -1);
    updateChannels (setup.outputDeviceName, setup.outputChannels, setup.useDefaultOutputChannels ? defaultNumOuts : -1);
}

String AudioDeviceManager::setAudioDeviceSetup (const AudioDeviceSetup& newSetup,
                                                bool treatAsChosenDevice)
{
    jassert (&newSetup != &currentSetup);    // this will have no effect

    if (newSetup != currentSetup)
        sendChangeMessage();
    else if (currentAudioDevice != nullptr)
        return {};

    stopDevice();

    if (getCurrentDeviceTypeObject() == nullptr
        || (newSetup.inputDeviceName.isEmpty() && newSetup.outputDeviceName.isEmpty()))
    {
        deleteCurrentDevice();

        if (treatAsChosenDevice)
            updateXml();

        return {};
    }

    String error;

    const auto needsNewDevice = currentSetup.inputDeviceName  != newSetup.inputDeviceName
                             || currentSetup.outputDeviceName != newSetup.outputDeviceName
                             || currentAudioDevice == nullptr;

    if (needsNewDevice)
    {
        deleteCurrentDevice();
        scanDevicesIfNeeded();

        auto* type = getCurrentDeviceTypeObject();

        for (const auto isInput : { false, true })
        {
            const auto name = getSetupInfo (newSetup, isInput).name;

            if (name.isNotEmpty() && ! deviceListContains (type, isInput, name))
                return "No such device: " + name;
        }

        currentAudioDevice.reset (type->createDevice (newSetup.outputDeviceName, newSetup.inputDeviceName));

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
    }

    currentSetup = newSetup;

    if (! currentSetup.useDefaultInputChannels)    numInputChansNeeded  = currentSetup.inputChannels.countNumberOfSetBits();
    if (! currentSetup.useDefaultOutputChannels)   numOutputChansNeeded = currentSetup.outputChannels.countNumberOfSetBits();

    updateSetupChannels (currentSetup, numInputChansNeeded, numOutputChansNeeded);

    if (currentSetup.inputChannels.isZero() && currentSetup.outputChannels.isZero())
    {
        if (treatAsChosenDevice)
            updateXml();

        return {};
    }

    currentSetup.sampleRate = chooseBestSampleRate (currentSetup.sampleRate);
    currentSetup.bufferSize = chooseBestBufferSize (currentSetup.bufferSize);

    error = currentAudioDevice->open (currentSetup.inputChannels,
                                      currentSetup.outputChannels,
                                      currentSetup.sampleRate,
                                      currentSetup.bufferSize);

    if (error.isEmpty())
    {
        currentDeviceType = currentAudioDevice->getTypeName();

        currentAudioDevice->start (callbackHandler.get());

        error = currentAudioDevice->getLastError();
    }

    if (error.isEmpty())
    {
        updateCurrentSetup();

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

    auto rates = currentAudioDevice->getAvailableSampleRates();

    if (rate > 0 && rates.contains (rate))
        return rate;

    rate = currentAudioDevice->getCurrentSampleRate();

    if (rate > 0 && rates.contains (rate))
        return rate;

    double lowestAbove44 = 0.0;

    for (int i = rates.size(); --i >= 0;)
    {
        auto sr = rates[i];

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

    testSound.reset();
}

void AudioDeviceManager::closeAudioDevice()
{
    stopDevice();
    currentAudioDevice.reset();
    loadMeasurer.reset();
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
            // one first, with setAudioDeviceSetup().
            jassertfalse;
            return;
        }

        AudioDeviceSetup s (currentSetup);
        setAudioDeviceSetup (s, false);
    }
}

void AudioDeviceManager::updateXml()
{
    lastExplicitSettings.reset (new XmlElement ("DEVICESETUP"));

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

    for (auto& input : enabledMidiInputs)
    {
        auto* child = lastExplicitSettings->createNewChildElement ("MIDIINPUT");

        child->setAttribute ("name",       input->getName());
        child->setAttribute ("identifier", input->getIdentifier());
    }

    if (midiDeviceInfosFromXml.size() > 0)
    {
        // Add any midi devices that have been enabled before, but which aren't currently
        // open because the device has been disconnected.
        auto availableMidiDevices = MidiInput::getAvailableDevices();

        for (auto& d : midiDeviceInfosFromXml)
        {
            if (! availableMidiDevices.contains (d))
            {
                auto* child = lastExplicitSettings->createNewChildElement ("MIDIINPUT");

                child->setAttribute ("name",       d.name);
                child->setAttribute ("identifier", d.identifier);
            }
        }
    }

    if (defaultMidiOutputDeviceInfo != MidiDeviceInfo())
    {
        lastExplicitSettings->setAttribute ("defaultMidiOutput",       defaultMidiOutputDeviceInfo.name);
        lastExplicitSettings->setAttribute ("defaultMidiOutputDevice", defaultMidiOutputDeviceInfo.identifier);
    }
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
        newCallback->audioDeviceAboutToStart (currentAudioDevice.get());

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

void AudioDeviceManager::audioDeviceIOCallbackInt (const float* const* inputChannelData,
                                                   int numInputChannels,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const AudioIODeviceCallbackContext& context)
{
    const ScopedLock sl (audioCallbackLock);

    inputLevelGetter->updateLevel (inputChannelData, numInputChannels, numSamples);

    if (callbacks.size() > 0)
    {
        AudioProcessLoadMeasurer::ScopedTimer timer (loadMeasurer, numSamples);

        tempBuffer.setSize (jmax (1, numOutputChannels), jmax (1, numSamples), false, false, true);

        callbacks.getUnchecked(0)->audioDeviceIOCallbackWithContext (inputChannelData,
                                                                     numInputChannels,
                                                                     outputChannelData,
                                                                     numOutputChannels,
                                                                     numSamples,
                                                                     context);

        auto* const* tempChans = tempBuffer.getArrayOfWritePointers();

        for (int i = callbacks.size(); --i > 0;)
        {
            callbacks.getUnchecked(i)->audioDeviceIOCallbackWithContext (inputChannelData,
                                                                         numInputChannels,
                                                                         tempChans,
                                                                         numOutputChannels,
                                                                         numSamples,
                                                                         context);

            for (int chan = 0; chan < numOutputChannels; ++chan)
            {
                if (auto* src = tempChans [chan])
                    if (auto* dst = outputChannelData [chan])
                        for (int j = 0; j < numSamples; ++j)
                            dst[j] += src[j];
            }
        }
    }
    else
    {
        for (int i = 0; i < numOutputChannels; ++i)
            zeromem (outputChannelData[i], (size_t) numSamples * sizeof (float));
    }

    if (testSound != nullptr)
    {
        auto numSamps = jmin (numSamples, testSound->getNumSamples() - testSoundPosition);
        auto* src = testSound->getReadPointer (0, testSoundPosition);

        for (int i = 0; i < numOutputChannels; ++i)
            if (auto* dst = outputChannelData [i])
                for (int j = 0; j < numSamps; ++j)
                    dst[j] += src[j];

        testSoundPosition += numSamps;

        if (testSoundPosition >= testSound->getNumSamples())
            testSound.reset();
    }

    outputLevelGetter->updateLevel (outputChannelData, numOutputChannels, numSamples);
}

void AudioDeviceManager::audioDeviceAboutToStartInt (AudioIODevice* const device)
{
    loadMeasurer.reset (device->getCurrentSampleRate(),
                        device->getCurrentBufferSizeSamples());

    updateCurrentSetup();

    {
        const ScopedLock sl (audioCallbackLock);

        for (int i = callbacks.size(); --i >= 0;)
            callbacks.getUnchecked(i)->audioDeviceAboutToStart (device);
    }

    sendChangeMessage();
}

void AudioDeviceManager::audioDeviceStoppedInt()
{
    sendChangeMessage();

    const ScopedLock sl (audioCallbackLock);

    loadMeasurer.reset();

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
    return loadMeasurer.getLoadAsProportion();
}

//==============================================================================
void AudioDeviceManager::setMidiInputDeviceEnabled (const String& identifier, bool enabled)
{
    if (enabled != isMidiInputDeviceEnabled (identifier))
    {
        if (enabled)
        {
            if (auto midiIn = MidiInput::openDevice (identifier, callbackHandler.get()))
            {
                enabledMidiInputs.push_back (std::move (midiIn));
                enabledMidiInputs.back()->start();
            }
        }
        else
        {
            auto removePredicate = [identifier] (const std::unique_ptr<MidiInput>& in) { return in->getIdentifier() == identifier; };
            enabledMidiInputs.erase (std::remove_if (std::begin (enabledMidiInputs), std::end (enabledMidiInputs), removePredicate),
                                     std::end (enabledMidiInputs));
        }

        updateXml();
        sendChangeMessage();
    }
}

bool AudioDeviceManager::isMidiInputDeviceEnabled (const String& identifier) const
{
    for (auto& mi : enabledMidiInputs)
        if (mi->getIdentifier() == identifier)
            return true;

    return false;
}

void AudioDeviceManager::addMidiInputDeviceCallback (const String& identifier, MidiInputCallback* callbackToAdd)
{
    removeMidiInputDeviceCallback (identifier, callbackToAdd);

    if (identifier.isEmpty() || isMidiInputDeviceEnabled (identifier))
    {
        const ScopedLock sl (midiCallbackLock);
        midiCallbacks.add ({ identifier, callbackToAdd });
    }
}

void AudioDeviceManager::removeMidiInputDeviceCallback (const String& identifier, MidiInputCallback* callbackToRemove)
{
    for (int i = midiCallbacks.size(); --i >= 0;)
    {
        auto& mc = midiCallbacks.getReference (i);

        if (mc.callback == callbackToRemove && mc.deviceIdentifier == identifier)
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

        for (auto& mc : midiCallbacks)
            if (mc.deviceIdentifier.isEmpty() || mc.deviceIdentifier == source->getIdentifier())
                mc.callback->handleIncomingMidiMessage (source, message);
    }
}

//==============================================================================
void AudioDeviceManager::setDefaultMidiOutputDevice (const String& identifier)
{
    if (defaultMidiOutputDeviceInfo.identifier != identifier)
    {
        std::unique_ptr<MidiOutput> oldMidiPort;
        Array<AudioIODeviceCallback*> oldCallbacks;

        {
            const ScopedLock sl (audioCallbackLock);
            oldCallbacks.swapWith (callbacks);
        }

        if (currentAudioDevice != nullptr)
            for (int i = oldCallbacks.size(); --i >= 0;)
                oldCallbacks.getUnchecked (i)->audioDeviceStopped();

        std::swap (oldMidiPort, defaultMidiOutput);

        if (identifier.isNotEmpty())
            defaultMidiOutput = MidiOutput::openDevice (identifier);

        if (defaultMidiOutput != nullptr)
            defaultMidiOutputDeviceInfo = defaultMidiOutput->getDeviceInfo();
        else
            defaultMidiOutputDeviceInfo = {};

        if (currentAudioDevice != nullptr)
            for (auto* c : oldCallbacks)
                c->audioDeviceAboutToStart (currentAudioDevice.get());

        {
            const ScopedLock sl (audioCallbackLock);
            oldCallbacks.swapWith (callbacks);
        }

        updateXml();
        sendSynchronousChangeMessage();
    }
}

//==============================================================================
AudioDeviceManager::LevelMeter::LevelMeter() noexcept : level() {}

void AudioDeviceManager::LevelMeter::updateLevel (const float* const* channelData, int numChannels, int numSamples) noexcept
{
    if (getReferenceCount() <= 1)
        return;

    auto localLevel = level.get();

    if (numChannels > 0)
    {
        for (int j = 0; j < numSamples; ++j)
        {
            float s = 0;

            for (int i = 0; i < numChannels; ++i)
                s += std::abs (channelData[i][j]);

            s /= (float) numChannels;

            const float decayFactor = 0.99992f;

            if (s > localLevel)
                localLevel = s;
            else if (localLevel > 0.001f)
                localLevel *= decayFactor;
            else
                localLevel = 0;
        }
    }
    else
    {
        localLevel = 0;
    }

    level = localLevel;
}

double AudioDeviceManager::LevelMeter::getCurrentLevel() const noexcept
{
    jassert (getReferenceCount() > 1);
    return level.get();
}

void AudioDeviceManager::playTestSound()
{
    { // cunningly nested to swap, unlock and delete in that order.
        std::unique_ptr<AudioBuffer<float>> oldSound;

        {
            const ScopedLock sl (audioCallbackLock);
            std::swap (oldSound, testSound);
        }
    }

    testSoundPosition = 0;

    if (currentAudioDevice != nullptr)
    {
        auto sampleRate = currentAudioDevice->getCurrentSampleRate();
        auto soundLength = (int) sampleRate;

        double frequency = 440.0;
        float amplitude = 0.5f;

        auto phasePerSample = MathConstants<double>::twoPi / (sampleRate / frequency);

        std::unique_ptr<AudioBuffer<float>> newSound (new AudioBuffer<float> (1, soundLength));

        for (int i = 0; i < soundLength; ++i)
            newSound->setSample (0, i, amplitude * (float) std::sin (i * phasePerSample));

        newSound->applyGainRamp (0, 0, soundLength / 10, 0.0f, 1.0f);
        newSound->applyGainRamp (0, soundLength - soundLength / 4, soundLength / 4, 1.0f, 0.0f);

        {
            const ScopedLock sl (audioCallbackLock);
            std::swap (testSound, newSound);
        }
    }
}

int AudioDeviceManager::getXRunCount() const noexcept
{
    auto deviceXRuns = (currentAudioDevice != nullptr ? currentAudioDevice->getXRunCount() : -1);
    return jmax (0, deviceXRuns) + loadMeasurer.getXRunCount();
}

//==============================================================================
// Deprecated
void AudioDeviceManager::setMidiInputEnabled (const String& name, const bool enabled)
{
    for (auto& device : MidiInput::getAvailableDevices())
    {
        if (device.name == name)
        {
            setMidiInputDeviceEnabled (device.identifier, enabled);
            return;
        }
    }
}

bool AudioDeviceManager::isMidiInputEnabled (const String& name) const
{
    for (auto& device : MidiInput::getAvailableDevices())
        if (device.name == name)
            return isMidiInputDeviceEnabled (device.identifier);

    return false;
}

void AudioDeviceManager::addMidiInputCallback (const String& name, MidiInputCallback* callbackToAdd)
{
    if (name.isEmpty())
    {
        addMidiInputDeviceCallback ({}, callbackToAdd);
    }
    else
    {
        for (auto& device : MidiInput::getAvailableDevices())
        {
            if (device.name == name)
            {
                addMidiInputDeviceCallback (device.identifier, callbackToAdd);
                return;
            }
        }
    }
}

void AudioDeviceManager::removeMidiInputCallback (const String& name, MidiInputCallback* callbackToRemove)
{
    if (name.isEmpty())
    {
        removeMidiInputDeviceCallback ({}, callbackToRemove);
    }
    else
    {
        for (auto& device : MidiInput::getAvailableDevices())
        {
            if (device.name == name)
            {
                removeMidiInputDeviceCallback (device.identifier, callbackToRemove);
                return;
            }
        }
    }
}

void AudioDeviceManager::setDefaultMidiOutput (const String& name)
{
    for (auto& device : MidiOutput::getAvailableDevices())
    {
        if (device.name == name)
        {
            setDefaultMidiOutputDevice (device.identifier);
            return;
        }
    }
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class AudioDeviceManagerTests : public UnitTest
{
public:
    AudioDeviceManagerTests() : UnitTest ("AudioDeviceManager", UnitTestCategories::audio) {}

    void runTest() override
    {
        beginTest ("When the AudioDeviceSetup has non-empty device names, initialise uses the requested devices");
        {
            AudioDeviceManager manager;
            initialiseManager (manager);

            expectEquals (manager.getAvailableDeviceTypes().size(), 2);

            AudioDeviceManager::AudioDeviceSetup setup;
            setup.outputDeviceName = "z";
            setup.inputDeviceName = "c";

            expect (manager.initialise (2, 2, nullptr, true, String{}, &setup).isEmpty());

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);
            expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        }

        beginTest ("When the AudioDeviceSetup has empty device names, initialise picks suitable default devices");
        {
            AudioDeviceManager manager;
            initialiseManager (manager);

            AudioDeviceManager::AudioDeviceSetup setup;

            expect (manager.initialise (2, 2, nullptr, true, String{}, &setup).isEmpty());

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, String ("x"));
            expectEquals (newSetup.inputDeviceName, String ("a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        }

        beginTest ("When the preferred device name matches an input and an output on the same type, that type is used");
        {
            AudioDeviceManager manager;
            initialiseManagerWithDifferentDeviceNames (manager);

            expect (manager.initialise (2, 2, nullptr, true, "bar *").isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), String ("bar"));

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, String ("bar out a"));
            expectEquals (newSetup.inputDeviceName, String ("bar in a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);

            expect (manager.getCurrentAudioDevice() != nullptr);
        }

        beginTest ("When the preferred device name matches either an input and an output, but not both, that type is used");
        {
            AudioDeviceManager manager;
            initialiseManagerWithDifferentDeviceNames (manager);

            expect (manager.initialise (2, 2, nullptr, true, "bar out b").isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), String ("bar"));

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, String ("bar out b"));
            expectEquals (newSetup.inputDeviceName, String ("bar in a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);

            expect (manager.getCurrentAudioDevice() != nullptr);
        }

        beginTest ("When the preferred device name does not match any inputs or outputs, defaults are used");
        {
            AudioDeviceManager manager;
            initialiseManagerWithDifferentDeviceNames (manager);

            expect (manager.initialise (2, 2, nullptr, true, "unmatchable").isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), String ("foo"));

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, String ("foo out a"));
            expectEquals (newSetup.inputDeviceName, String ("foo in a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);

            expect (manager.getCurrentAudioDevice() != nullptr);
        }

        beginTest ("When first device type has no devices, a device type with devices is used instead");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType (manager);

            AudioDeviceManager::AudioDeviceSetup setup;

            expect (manager.initialise (2, 2, nullptr, true, {}, &setup).isEmpty());

            const auto& newSetup = manager.getAudioDeviceSetup();

            expectEquals (newSetup.outputDeviceName, String ("x"));
            expectEquals (newSetup.inputDeviceName, String ("a"));

            expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);
            expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        }

        beginTest ("If a device type has been explicitly set to a type with devices, "
                   "initialisation should respect this choice");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType (manager);
            manager.setCurrentAudioDeviceType (mockBName, true);

            AudioDeviceManager::AudioDeviceSetup setup;
            expect (manager.initialise (2, 2, nullptr, true, {}, &setup).isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), mockBName);
        }

        beginTest ("If a device type has been explicitly set to a type without devices, "
                   "initialisation should pick a type with devices instead");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType (manager);
            manager.setCurrentAudioDeviceType (emptyName, true);

            AudioDeviceManager::AudioDeviceSetup setup;
            expect (manager.initialise (2, 2, nullptr, true, {}, &setup).isEmpty());

            expectEquals (manager.getCurrentAudioDeviceType(), mockAName);
        }

        beginTest ("Carry out a long sequence of configuration changes");
        {
            AudioDeviceManager manager;
            initialiseManagerWithEmptyDeviceType    (manager);
            initialiseWithDefaultDevices            (manager);
            disableInputChannelsButLeaveDeviceOpen  (manager);
            selectANewInputDevice                   (manager);
            disableInputDevice                      (manager);
            reenableInputDeviceWithNoChannels       (manager);
            enableInputChannels                     (manager);
            disableInputChannelsButLeaveDeviceOpen  (manager);
            switchDeviceType                        (manager);
            enableInputChannels                     (manager);
            closeDeviceByRequestingEmptyNames       (manager);
        }

        beginTest ("AudioDeviceManager updates its current settings before notifying callbacks when device restarts itself");
        {
            AudioDeviceManager manager;
            auto deviceType = std::make_unique<MockDeviceType> ("foo",
                                                                StringArray { "foo in a", "foo in b" },
                                                                StringArray { "foo out a", "foo out b" });
            auto* ptr = deviceType.get();
            manager.addAudioDeviceType (std::move (deviceType));

            AudioDeviceManager::AudioDeviceSetup setup;
            setup.sampleRate = 48000.0;
            setup.bufferSize = 256;
            setup.inputDeviceName = "foo in a";
            setup.outputDeviceName = "foo out a";
            setup.useDefaultInputChannels = true;
            setup.useDefaultOutputChannels = true;
            manager.setAudioDeviceSetup (setup, true);

            const auto currentSetup = manager.getAudioDeviceSetup();
            expectEquals (currentSetup.sampleRate, setup.sampleRate);
            expectEquals (currentSetup.bufferSize, setup.bufferSize);

            MockCallback callback;
            manager.addAudioCallback (&callback);

            constexpr auto newSr = 10000.0;
            constexpr auto newBs = 1024;
            auto numCalls = 0;

            // Compilers disagree about whether newSr and newBs need to be captured
            callback.aboutToStart = [&]
            {
                ++numCalls;
                const auto current = manager.getAudioDeviceSetup();
                expectEquals (current.sampleRate, newSr);
                expectEquals (current.bufferSize, newBs);
            };

            ptr->restartDevices (newSr, newBs);
            expectEquals (numCalls, 1);
        }
    }

private:
    void initialiseWithDefaultDevices (AudioDeviceManager& manager)
    {
        manager.initialiseWithDefaultDevices (2, 2);
        const auto& setup = manager.getAudioDeviceSetup();

        expectEquals (setup.inputChannels.countNumberOfSetBits(), 2);
        expectEquals (setup.outputChannels.countNumberOfSetBits(), 2);

        expect (setup.useDefaultInputChannels);
        expect (setup.useDefaultOutputChannels);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    void disableInputChannelsButLeaveDeviceOpen (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputChannels.clear();
        setup.useDefaultInputChannels = false;

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    void selectANewInputDevice (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "b";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    void disableInputDevice (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    void reenableInputDeviceWithNoChannels (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "a";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    void enableInputChannels (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = manager.getCurrentDeviceTypeObject()->getDeviceNames (true)[0];
        setup.inputChannels = 3;
        setup.useDefaultInputChannels = false;

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (! newSetup.useDefaultInputChannels);
        expect (newSetup.useDefaultOutputChannels);

        expectEquals (newSetup.inputDeviceName, setup.inputDeviceName);
        expectEquals (newSetup.outputDeviceName, setup.outputDeviceName);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    void switchDeviceType (AudioDeviceManager& manager)
    {
        const auto oldSetup = manager.getAudioDeviceSetup();

        expectEquals (manager.getCurrentAudioDeviceType(), String (mockAName));

        manager.setCurrentAudioDeviceType (mockBName, true);

        expectEquals (manager.getCurrentAudioDeviceType(), String (mockBName));

        const auto newSetup = manager.getAudioDeviceSetup();

        expect (newSetup.outputDeviceName.isNotEmpty());
        // We had no channels enabled, which means we don't need to open a new input device
        expect (newSetup.inputDeviceName.isEmpty());

        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 0);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (manager.getCurrentAudioDevice() != nullptr);
    }

    void closeDeviceByRequestingEmptyNames (AudioDeviceManager& manager)
    {
        auto setup = manager.getAudioDeviceSetup();
        setup.inputDeviceName = "";
        setup.outputDeviceName = "";

        expect (manager.setAudioDeviceSetup (setup, true).isEmpty());

        const auto newSetup = manager.getAudioDeviceSetup();
        expectEquals (newSetup.inputChannels.countNumberOfSetBits(), 2);
        expectEquals (newSetup.outputChannels.countNumberOfSetBits(), 2);

        expect (newSetup.inputDeviceName.isEmpty());
        expect (newSetup.outputDeviceName.isEmpty());

        expect (manager.getCurrentAudioDevice() == nullptr);
    }

    const String mockAName = "mockA";
    const String mockBName = "mockB";
    const String emptyName = "empty";

    struct Restartable
    {
        virtual ~Restartable() = default;
        virtual void restart (double newSr, int newBs) = 0;
    };

    class MockDevice : public AudioIODevice,
                       private Restartable
    {
    public:
        MockDevice (ListenerList<Restartable>& l, String typeNameIn, String outNameIn, String inNameIn)
            : AudioIODevice ("mock", typeNameIn), listeners (l), outName (outNameIn), inName (inNameIn)
        {
            listeners.add (this);
        }

        ~MockDevice() override
        {
            listeners.remove (this);
        }

        StringArray getOutputChannelNames() override { return { "o1", "o2", "o3" }; }
        StringArray getInputChannelNames()  override { return { "i1", "i2", "i3" }; }

        Array<double> getAvailableSampleRates() override { return { 44100.0, 48000.0 }; }
        Array<int> getAvailableBufferSizes() override { return { 128, 256 }; }
        int getDefaultBufferSize() override { return 128; }

        String open (const BigInteger& inputs, const BigInteger& outputs, double sr, int bs) override
        {
            inChannels = inputs;
            outChannels = outputs;
            sampleRate = sr;
            blockSize = bs;
            on = true;
            return {};
        }

        void close() override { on = false; }
        bool isOpen() override { return on; }

        void start (AudioIODeviceCallback* c) override
        {
            callback = c;
            callback->audioDeviceAboutToStart (this);
            playing = true;
        }

        void stop() override
        {
            playing = false;
            callback->audioDeviceStopped();
        }

        bool isPlaying() override { return playing; }

        String getLastError() override { return {}; }
        int getCurrentBufferSizeSamples() override { return blockSize; }
        double getCurrentSampleRate() override { return sampleRate; }
        int getCurrentBitDepth() override { return 16; }

        BigInteger getActiveOutputChannels() const override { return outChannels; }
        BigInteger getActiveInputChannels()  const override { return inChannels; }

        int getOutputLatencyInSamples() override { return 0; }
        int getInputLatencyInSamples() override { return 0; }

    private:
        void restart (double newSr, int newBs) override
        {
            stop();
            close();
            open (inChannels, outChannels, newSr, newBs);
            start (callback);
        }

        ListenerList<Restartable>& listeners;
        AudioIODeviceCallback* callback = nullptr;
        String outName, inName;
        BigInteger outChannels, inChannels;
        double sampleRate = 0.0;
        int blockSize = 0;
        bool on = false, playing = false;
    };

    class MockDeviceType : public AudioIODeviceType
    {
    public:
        explicit MockDeviceType (String kind)
            : MockDeviceType (std::move (kind), { "a", "b", "c" }, { "x", "y", "z" }) {}

        MockDeviceType (String kind, StringArray inputNames, StringArray outputNames)
            : AudioIODeviceType (std::move (kind)),
              inNames (std::move (inputNames)),
              outNames (std::move (outputNames)) {}

        ~MockDeviceType() override
        {
            // A Device outlived its DeviceType!
            jassert (listeners.isEmpty());
        }

        void scanForDevices() override {}

        StringArray getDeviceNames (bool isInput) const override
        {
            return getNames (isInput);
        }

        int getDefaultDeviceIndex (bool) const override { return 0; }

        int getIndexOfDevice (AudioIODevice* device, bool isInput) const override
        {
            return getNames (isInput).indexOf (device->getName());
        }

        bool hasSeparateInputsAndOutputs() const override { return true; }

        AudioIODevice* createDevice (const String& outputName, const String& inputName) override
        {
            if (inNames.contains (inputName) || outNames.contains (outputName))
                return new MockDevice (listeners, getTypeName(), outputName, inputName);

            return nullptr;
        }

        // Call this to emulate the device restarting itself with new settings.
        // This might happen e.g. when a user changes the ASIO settings.
        void restartDevices (double newSr, int newBs)
        {
            listeners.call ([&] (auto& l) { return l.restart (newSr, newBs); });
        }

    private:
        const StringArray& getNames (bool isInput) const { return isInput ? inNames : outNames; }

        const StringArray inNames, outNames;
        ListenerList<Restartable> listeners;
    };

    class MockCallback : public AudioIODeviceCallback
    {
    public:
        std::function<void()> callback;
        std::function<void()> aboutToStart;
        std::function<void()> stopped;
        std::function<void()> error;

        void audioDeviceIOCallbackWithContext (const float* const*,
                                               int,
                                               float* const*,
                                               int,
                                               int,
                                               const AudioIODeviceCallbackContext&) override
        {
            NullCheckedInvocation::invoke (callback);
        }

        void audioDeviceAboutToStart (AudioIODevice*) override { NullCheckedInvocation::invoke (aboutToStart); }
        void audioDeviceStopped()                     override { NullCheckedInvocation::invoke (stopped); }
        void audioDeviceError (const String&)         override { NullCheckedInvocation::invoke (error); }
    };

    void initialiseManager (AudioDeviceManager& manager)
    {
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> (mockAName));
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> (mockBName));
    }

    void initialiseManagerWithEmptyDeviceType (AudioDeviceManager& manager)
    {
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> (emptyName, StringArray{}, StringArray{}));
        initialiseManager (manager);
    }

    void initialiseManagerWithDifferentDeviceNames (AudioDeviceManager& manager)
    {
        manager.addAudioDeviceType (std::make_unique<MockDeviceType> ("foo",
                                                                      StringArray { "foo in a", "foo in b" },
                                                                      StringArray { "foo out a", "foo out b" }));

        manager.addAudioDeviceType (std::make_unique<MockDeviceType> ("bar",
                                                                      StringArray { "bar in a", "bar in b" },
                                                                      StringArray { "bar out a", "bar out b" }));
    }
};

static AudioDeviceManagerTests audioDeviceManagerTests;

#endif

} // namespace juce
