/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../../../Assets/DemoUtilities.h"
#include "JUCEDemos.h"

//==============================================================================
std::vector<JUCEDemos::DemoCategory>& JUCEDemos::getCategories()
{
    static std::vector<DemoCategory> categories;
    return categories;
}

JUCEDemos::DemoCategory& JUCEDemos::getCategory (const String& name)
{
    auto& categories = getCategories();

    for (auto& c : categories)
        if (c.name == name)
            return c;

    std::vector<FileAndCallback> fc;
    categories.push_back ({ name, fc });

    return categories.back();
}

void JUCEDemos::registerDemo (std::function<Component*()> constructorCallback, const String& filePath, const String& category, bool isHeavyweight)
{
   #if JUCE_MAC
    auto f = File::getSpecialLocation (File::currentExecutableFile)
                  .getParentDirectory().getParentDirectory().getChildFile ("Resources");
   #else
    auto f = findExamplesDirectoryFromExecutable (File::getSpecialLocation (File::currentApplicationFile));
   #endif

    #if ! (JUCE_ANDROID || JUCE_IOS)
    if (f == File())
    {
        jassertfalse;
        return;
    }
    #endif

    getCategory (category).demos.push_back ({ f.getChildFile (filePath), constructorCallback, isHeavyweight });
}

File JUCEDemos::findExamplesDirectoryFromExecutable (File exec)
{
    int numTries = 15;
    auto exampleDir = exec.getParentDirectory().getChildFile ("examples");

    if (exampleDir.exists())
        return exampleDir;

    while (exec.getFileName() != "examples" && numTries-- > 0)
        exec = exec.getParentDirectory();
    if (exec.getFileName() == "examples")
        return exec;
    return {};
}

//==============================================================================
static String getCurrentDefaultAudioDeviceName (AudioDeviceManager& deviceManager, bool isInput)
{
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    jassert (deviceType != nullptr);

    if (deviceType != nullptr)
    {
        auto deviceNames = deviceType->getDeviceNames();
        return deviceNames [deviceType->getDefaultDeviceIndex (isInput)];
    }

    return {};
}

// (returns a shared AudioDeviceManager object that all the demos can use)
AudioDeviceManager& getSharedAudioDeviceManager (int numInputChannels, int numOutputChannels)
{
    if (sharedAudioDeviceManager == nullptr)
        sharedAudioDeviceManager.reset (new AudioDeviceManager());

    auto* currentDevice = sharedAudioDeviceManager->getCurrentAudioDevice();

    if (numInputChannels < 0)
        numInputChannels = (currentDevice != nullptr ? currentDevice->getActiveInputChannels().countNumberOfSetBits() : 1);

    if (numOutputChannels < 0)
        numOutputChannels = (currentDevice != nullptr ? currentDevice->getActiveOutputChannels().countNumberOfSetBits() : 2);

    if (numInputChannels > 0 && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [numInputChannels, numOutputChannels] (bool granted)
                                     {
                                         if (granted)
                                             getSharedAudioDeviceManager (numInputChannels, numOutputChannels);
                                     });

        numInputChannels = 0;
    }

    if (sharedAudioDeviceManager->getCurrentAudioDevice() != nullptr)
    {
        auto setup = sharedAudioDeviceManager->getAudioDeviceSetup();

        auto numInputs  = jmax (numInputChannels,  setup.inputChannels.countNumberOfSetBits());
        auto numOutputs = jmax (numOutputChannels, setup.outputChannels.countNumberOfSetBits());

        auto oldInputs  = setup.inputChannels.countNumberOfSetBits();
        auto oldOutputs = setup.outputChannels.countNumberOfSetBits();

        if (oldInputs != numInputs || oldOutputs != numOutputs)
        {
            if (oldInputs == 0 && oldOutputs == 0)
            {
                sharedAudioDeviceManager->initialise (numInputChannels, numOutputChannels, nullptr, true, {}, nullptr);
            }
            else
            {
                setup.useDefaultInputChannels = setup.useDefaultOutputChannels = false;

                setup.inputChannels.clear();
                setup.outputChannels.clear();

                setup.inputChannels.setRange (0, numInputs, true);
                setup.outputChannels.setRange (0, numOutputs, true);

                if (oldInputs == 0 && numInputs > 0 && setup.inputDeviceName.isEmpty())
                    setup.inputDeviceName = getCurrentDefaultAudioDeviceName (*sharedAudioDeviceManager, true);

                if (oldOutputs == 0 && numOutputs > 0 && setup.outputDeviceName.isEmpty())
                    setup.outputDeviceName = getCurrentDefaultAudioDeviceName (*sharedAudioDeviceManager, false);

                sharedAudioDeviceManager->setAudioDeviceSetup (setup, false);
            }
        }
    }
    else
    {
        sharedAudioDeviceManager->initialise (numInputChannels, numOutputChannels, nullptr, true, {}, nullptr);
    }

    return *sharedAudioDeviceManager;
}

//==============================================================================
void registerAllDemos() noexcept
{
    registerDemos_One();
    registerDemos_Two();
}
