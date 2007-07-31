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

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioDeviceManager.h"
#include "../../gui/components/juce_Desktop.h"


//==============================================================================
AudioDeviceManager::AudioDeviceManager()
    : currentAudioDevice (0),
      currentCallback (0),
      numInputChansNeeded (0),
      numOutputChansNeeded (2),
      lastExplicitSettings (0),
      listNeedsScanning (true),
      useInputNames (false),
      enabledMidiInputs (4),
      midiCallbacks (4),
      midiCallbackDevices (4),
      cpuUsageMs (0),
      timeToCpuScale (0)
{
    callbackHandler.owner = this;

    AudioIODeviceType::createDeviceTypes (availableDeviceTypes);
}

AudioDeviceManager::~AudioDeviceManager()
{
    stopDevice();
    deleteAndZero (currentAudioDevice);
    delete lastExplicitSettings;
}


//==============================================================================
const String AudioDeviceManager::initialise (const int numInputChannelsNeeded,
                                             const int numOutputChannelsNeeded,
                                             const XmlElement* const e,
                                             const bool selectDefaultDeviceOnFailure)
{
    if (listNeedsScanning)
        refreshDeviceList();

    numInputChansNeeded = numInputChannelsNeeded;
    numOutputChansNeeded = numOutputChannelsNeeded;

    if (e != 0 && e->hasTagName (T("DEVICESETUP")))
    {
        lastExplicitSettings = new XmlElement (*e);

        BitArray ins, outs;
        ins.parseString (e->getStringAttribute (T("audioDeviceInChans"), T("11")), 2);
        outs.parseString (e->getStringAttribute (T("audioDeviceOutChans"), T("11")), 2);

        String error (setAudioDevice (e->getStringAttribute (T("audioDeviceName")),
                                      e->getIntAttribute (T("audioDeviceBufferSize")),
                                      e->getDoubleAttribute (T("audioDeviceRate")),
                                      e->hasAttribute (T("audioDeviceInChans")) ? &ins : 0,
                                      e->hasAttribute (T("audioDeviceOutChans")) ? &outs : 0,
                                      true));

        forEachXmlChildElementWithTagName (*e, c, T("MIDIINPUT"))
        {
            setMidiInputEnabled (c->getStringAttribute (T("name")), true);
        }

        if (error.isNotEmpty() && selectDefaultDeviceOnFailure)
            initialise (numInputChannelsNeeded, numOutputChannelsNeeded, 0, false);

        return error;
    }
    else
    {
        setInputDeviceNamesUsed (numOutputChannelsNeeded == 0);

        String defaultDevice;

        if (availableDeviceTypes [0] != 0)
            defaultDevice = availableDeviceTypes[0]->getDefaultDeviceName (numOutputChannelsNeeded == 0);

        return setAudioDevice (defaultDevice, 0, 0, 0, 0, false);
    }
}

XmlElement* AudioDeviceManager::createStateXml() const
{
    return lastExplicitSettings != 0 ? new XmlElement (*lastExplicitSettings) : 0;
}

//==============================================================================
const StringArray AudioDeviceManager::getAvailableAudioDeviceNames() const
{
    if (listNeedsScanning)
        refreshDeviceList();

    StringArray names;

    for (int i = 0; i < availableDeviceTypes.size(); ++i)
        names.addArray (availableDeviceTypes[i]->getDeviceNames (useInputNames));

    return names;
}

void AudioDeviceManager::refreshDeviceList() const
{
    listNeedsScanning = false;

    for (int i = 0; i < availableDeviceTypes.size(); ++i)
        availableDeviceTypes[i]->scanForDevices();
}

void AudioDeviceManager::setInputDeviceNamesUsed (const bool useInputNames_)
{
    useInputNames = useInputNames_;
    sendChangeMessage (this);
}

void AudioDeviceManager::addDeviceNamesToComboBox (ComboBox& combo) const
{
    int n = 0;

    for (int i = 0; i < availableDeviceTypes.size(); ++i)
    {
        AudioIODeviceType* const type = availableDeviceTypes[i];

        if (availableDeviceTypes.size() > 1)
            combo.addSectionHeading (type->getTypeName() + T(" devices:"));

        const StringArray names (type->getDeviceNames (useInputNames));

        for (int j = 0; j < names.size(); ++j)
            combo.addItem (names[j], ++n);

        combo.addSeparator();
    }

    combo.addItem ("<< no audio device >>", -1);
}

const String AudioDeviceManager::getCurrentAudioDeviceName() const
{
    if (currentAudioDevice != 0)
        return currentAudioDevice->getName();

    return String::empty;
}

const String AudioDeviceManager::setAudioDevice (const String& deviceNameToUse,
                                                 int blockSizeToUse,
                                                 double sampleRateToUse,
                                                 const BitArray* inChans,
                                                 const BitArray* outChans,
                                                 const bool treatAsChosenDevice)
{
    stopDevice();

    String error;

    if (deviceNameToUse.isNotEmpty())
    {
        const StringArray devNames (getAvailableAudioDeviceNames());

        int index = devNames.indexOf (deviceNameToUse, true);

        if (index >= 0)
        {
            if (currentAudioDevice == 0
                 || currentAudioDevice->getLastError().isNotEmpty()
                 || ! deviceNameToUse.equalsIgnoreCase (currentAudioDevice->getName()))
            {
                // change of device..
                deleteAndZero (currentAudioDevice);

                int n = 0;

                for (int i = 0; i < availableDeviceTypes.size(); ++i)
                {
                    AudioIODeviceType* const type = availableDeviceTypes[i];
                    const StringArray names (type->getDeviceNames (useInputNames));

                    if (index >= n && index < n + names.size())
                    {
                        currentAudioDevice = type->createDevice (deviceNameToUse);
                        break;
                    }

                    n += names.size();
                }

                error = currentAudioDevice->getLastError();

                if (error.isNotEmpty())
                {
                    deleteAndZero (currentAudioDevice);
                    return error;
                }

                inputChannels.clear();
                inputChannels.setRange (0, numInputChansNeeded, true);
                outputChannels.clear();
                outputChannels.setRange (0, numOutputChansNeeded, true);
            }

            if (inChans != 0)
                inputChannels = *inChans;

            if (outChans != 0)
                outputChannels = *outChans;

            error = restartDevice (blockSizeToUse,
                                   sampleRateToUse,
                                   inputChannels,
                                   outputChannels);

            if (error.isNotEmpty())
            {
                deleteAndZero (currentAudioDevice);
            }
        }
        else
        {
            deleteAndZero (currentAudioDevice);
            error << "No such device: " << deviceNameToUse;
        }
    }
    else
    {
        deleteAndZero (currentAudioDevice);
    }

    if (treatAsChosenDevice && error.isEmpty())
    {
        delete lastExplicitSettings;

        lastExplicitSettings = new XmlElement (T("DEVICESETUP"));

        lastExplicitSettings->setAttribute (T("audioDeviceName"), getCurrentAudioDeviceName());

        if (currentAudioDevice != 0)
        {
            lastExplicitSettings->setAttribute (T("audioDeviceRate"), currentAudioDevice->getCurrentSampleRate());

            if (currentAudioDevice->getDefaultBufferSize() != currentAudioDevice->getCurrentBufferSizeSamples())
                lastExplicitSettings->setAttribute (T("audioDeviceBufferSize"), currentAudioDevice->getCurrentBufferSizeSamples());

            lastExplicitSettings->setAttribute (T("audioDeviceInChans"), inputChannels.toString (2));
            lastExplicitSettings->setAttribute (T("audioDeviceOutChans"), outputChannels.toString (2));
        }

        for (int i = 0; i < enabledMidiInputs.size(); ++i)
        {
            XmlElement* m = new XmlElement (T("MIDIINPUT"));
            m->setAttribute (T("name"), enabledMidiInputs[i]->getName());

            lastExplicitSettings->addChildElement (m);
        }
    }

    return error;
}

const String AudioDeviceManager::restartDevice (int blockSizeToUse,
                                                double sampleRateToUse,
                                                const BitArray& inChans,
                                                const BitArray& outChans)
{
    stopDevice();

    inputChannels = inChans;
    outputChannels = outChans;

    if (sampleRateToUse > 0)
    {
        bool ok = false;

        for (int i = currentAudioDevice->getNumSampleRates(); --i >= 0;)
        {
            const double sr = currentAudioDevice->getSampleRate (i);

            if (sr == sampleRateToUse)
                ok = true;
        }

        if (! ok)
            sampleRateToUse = 0;
    }

    if (sampleRateToUse == 0)
    {
        double lowestAbove44 = 0.0;

        for (int i = currentAudioDevice->getNumSampleRates(); --i >= 0;)
        {
            const double sr = currentAudioDevice->getSampleRate (i);

            if (sr >= 44100.0 && (lowestAbove44 == 0 || sr < lowestAbove44))
                lowestAbove44 = sr;
        }

        if (lowestAbove44 == 0.0)
            sampleRateToUse = currentAudioDevice->getSampleRate (0);
        else
            sampleRateToUse = lowestAbove44;
    }

    const String error (currentAudioDevice->open (inChans, outChans,
                                                  sampleRateToUse, blockSizeToUse));

    if (error.isEmpty())
        currentAudioDevice->start (&callbackHandler);

    sendChangeMessage (this);
    return error;
}

void AudioDeviceManager::stopDevice()
{
    if (currentAudioDevice != 0)
        currentAudioDevice->stop();
}

void AudioDeviceManager::setInputChannels (const BitArray& newEnabledChannels,
                                           const bool treatAsChosenDevice)
{
    if (currentAudioDevice != 0
         && newEnabledChannels != inputChannels)
    {
        setAudioDevice (currentAudioDevice->getName(),
                        currentAudioDevice->getCurrentBufferSizeSamples(),
                        currentAudioDevice->getCurrentSampleRate(),
                        &newEnabledChannels, 0,
                        treatAsChosenDevice);
    }
}

void AudioDeviceManager::setOutputChannels (const BitArray& newEnabledChannels,
                                            const bool treatAsChosenDevice)
{
    if (currentAudioDevice != 0
         && newEnabledChannels != inputChannels)
    {
        setAudioDevice (currentAudioDevice->getName(),
                        currentAudioDevice->getCurrentBufferSizeSamples(),
                        currentAudioDevice->getCurrentSampleRate(),
                        0, &newEnabledChannels,
                        treatAsChosenDevice);
    }
}

//==============================================================================
void AudioDeviceManager::setAudioCallback (AudioIODeviceCallback* newCallback)
{
    if (newCallback != currentCallback)
    {
        AudioIODeviceCallback* lastCallback = currentCallback;

        audioCallbackLock.enter();
        currentCallback = 0;
        audioCallbackLock.exit();

        if (currentAudioDevice != 0)
        {
            if (lastCallback != 0)
                lastCallback->audioDeviceStopped();

            if (newCallback != 0)
                newCallback->audioDeviceAboutToStart (currentAudioDevice->getCurrentSampleRate(),
                                                      currentAudioDevice->getCurrentBufferSizeSamples());
        }

        currentCallback = newCallback;
    }
}

void AudioDeviceManager::audioDeviceIOCallbackInt (const float** inputChannelData,
                                                   int totalNumInputChannels,
                                                   float** outputChannelData,
                                                   int totalNumOutputChannels,
                                                   int numSamples)
{
    const ScopedLock sl (audioCallbackLock);

    if (currentCallback != 0)
    {
        const double callbackStartTime = Time::getMillisecondCounterHiRes();

        currentCallback->audioDeviceIOCallback (inputChannelData,
                                                totalNumInputChannels,
                                                outputChannelData,
                                                totalNumOutputChannels,
                                                numSamples);

        const double msTaken = Time::getMillisecondCounterHiRes() - callbackStartTime;
        const double filterAmount = 0.2;
        cpuUsageMs += filterAmount * (msTaken - cpuUsageMs);
    }
}

void AudioDeviceManager::audioDeviceAboutToStartInt (double sampleRate, int blockSize)
{
    cpuUsageMs = 0;

    if (sampleRate > 0.0 && blockSize > 0)
    {
        const double msPerBlock = 1000.0 * blockSize / sampleRate;
        timeToCpuScale = (msPerBlock > 0.0) ? (1.0 / msPerBlock) : 0.0;
    }

    if (currentCallback != 0)
        currentCallback->audioDeviceAboutToStart (sampleRate, blockSize);

    sendChangeMessage (this);
}

void AudioDeviceManager::audioDeviceStoppedInt()
{
    cpuUsageMs = 0;
    timeToCpuScale = 0;
    sendChangeMessage (this);

    if (currentCallback != 0)
        currentCallback->audioDeviceStopped();
}

double AudioDeviceManager::getCpuUsage() const
{
    return jlimit (0.0, 1.0, timeToCpuScale * cpuUsageMs);
}

//==============================================================================
void AudioDeviceManager::setMidiInputEnabled (const String& name,
                                              const bool enabled)
{
    if (enabled != isMidiInputEnabled (name))
    {
        if (enabled)
        {
            const int index = MidiInput::getDevices().indexOf (name);

            if (index >= 0)
            {
                MidiInput* const min = MidiInput::openDevice (index, &callbackHandler);

                if (min != 0)
                {
                    enabledMidiInputs.add (min);
                    min->start();
                }
            }
        }
        else
        {
            for (int i = enabledMidiInputs.size(); --i >= 0;)
                if (enabledMidiInputs[i]->getName() == name)
                    enabledMidiInputs.remove (i);
        }

        sendChangeMessage (this);
    }
}

bool AudioDeviceManager::isMidiInputEnabled (const String& name) const
{
    for (int i = enabledMidiInputs.size(); --i >= 0;)
        if (enabledMidiInputs[i]->getName() == name)
            return true;

    return false;
}

void AudioDeviceManager::addMidiInputCallback (const String& name,
                                               MidiInputCallback* callback)
{
    removeMidiInputCallback (callback);

    if (name.isEmpty())
    {
        midiCallbacks.add (callback);
        midiCallbackDevices.add (0);
    }
    else
    {
        for (int i = enabledMidiInputs.size(); --i >= 0;)
        {
            if (enabledMidiInputs[i]->getName() == name)
            {
                const ScopedLock sl (midiCallbackLock);

                if (! midiCallbacks.contains (callback))
                {
                    midiCallbacks.add (callback);
                    midiCallbackDevices.add (enabledMidiInputs[i]);
                }

                break;
            }
        }
    }
}

void AudioDeviceManager::removeMidiInputCallback (MidiInputCallback* callback)
{
    const ScopedLock sl (midiCallbackLock);

    const int index = midiCallbacks.indexOf (callback);
    midiCallbacks.remove (index);
    midiCallbackDevices.remove (index);
}

void AudioDeviceManager::handleIncomingMidiMessageInt (MidiInput* source,
                                                       const MidiMessage& message)
{
    if (! message.isActiveSense())
    {
        const bool isDefaultSource = (source == 0 || source == enabledMidiInputs.getFirst());

        const ScopedLock sl (midiCallbackLock);

        for (int i = midiCallbackDevices.size(); --i >= 0;)
        {
            MidiInput* const md = midiCallbackDevices.getUnchecked(i);

            if (md == source || (md == 0 && isDefaultSource))
                midiCallbacks.getUnchecked(i)->handleIncomingMidiMessage (source, message);
        }
    }
}

//==============================================================================
void AudioDeviceManager::CallbackHandler::audioDeviceIOCallback (const float** inputChannelData,
                                                                 int totalNumInputChannels,
                                                                 float** outputChannelData,
                                                                 int totalNumOutputChannels,
                                                                 int numSamples)
{
    owner->audioDeviceIOCallbackInt (inputChannelData, totalNumInputChannels, outputChannelData, totalNumOutputChannels, numSamples);
}

void AudioDeviceManager::CallbackHandler::audioDeviceAboutToStart (double sampleRate, int blockSize)
{
    owner->audioDeviceAboutToStartInt (sampleRate, blockSize);
}

void AudioDeviceManager::CallbackHandler::audioDeviceStopped()
{
    owner->audioDeviceStoppedInt();
}

void AudioDeviceManager::CallbackHandler::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message)
{
    owner->handleIncomingMidiMessageInt (source, message);
}


END_JUCE_NAMESPACE
