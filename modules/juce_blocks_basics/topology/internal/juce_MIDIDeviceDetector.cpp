/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

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

struct MIDIDeviceDetector  : public PhysicalTopologySource::DeviceDetector
{
    MIDIDeviceDetector() {}

    juce::StringArray scanForDevices() override
    {
        juce::StringArray result;

        for (auto& pair : findDevices())
            result.add (pair.inputName + " & " + pair.outputName);

        return result;
    }

    PhysicalTopologySource::DeviceConnection* openDevice (int index) override
    {
        auto pair = findDevices()[index];

        if (pair.inputIndex >= 0 && pair.outputIndex >= 0)
        {
            std::unique_ptr<MIDIDeviceConnection> dev (new MIDIDeviceConnection());

            if (dev->lockAgainstOtherProcesses (pair.inputName, pair.outputName))
            {
                lockedFromOutside = false;

                dev->midiInput.reset (juce::MidiInput::openDevice (pair.inputIndex, dev.get()));
                dev->midiOutput.reset (juce::MidiOutput::openDevice (pair.outputIndex));

                if (dev->midiInput != nullptr)
                {
                    dev->midiInput->start();
                    return dev.release();
                }
            }
            else
            {
                lockedFromOutside = true;
            }
        }

        return nullptr;
    }

    bool isLockedFromOutside() const override
    {
        return lockedFromOutside && ! findDevices().isEmpty();
    }

    static bool isBlocksMidiDeviceName (const juce::String& name)
    {
        return name.indexOf (" BLOCK") > 0 || name.indexOf (" Block") > 0;
    }

    static String cleanBlocksDeviceName (juce::String name)
    {
        name = name.trim();

        if (name.endsWith (" IN)"))
            return name.dropLastCharacters (4);

        if (name.endsWith (" OUT)"))
            return name.dropLastCharacters (5);

        const int openBracketPosition = name.lastIndexOfChar ('[');
        if (openBracketPosition != -1 && name.endsWith ("]"))
            return name.dropLastCharacters (name.length() - openBracketPosition);

        return name;
    }

    struct MidiInputOutputPair
    {
        juce::String outputName, inputName;
        int outputIndex = -1, inputIndex = -1;
    };

    static juce::Array<MidiInputOutputPair> findDevices()
    {
        juce::Array<MidiInputOutputPair> result;

        auto midiInputs  = juce::MidiInput::getDevices();
        auto midiOutputs = juce::MidiOutput::getDevices();

        for (int j = 0; j < midiInputs.size(); ++j)
        {
            if (isBlocksMidiDeviceName (midiInputs[j]))
            {
                MidiInputOutputPair pair;
                pair.inputName = midiInputs[j];
                pair.inputIndex = j;

                String cleanedInputName = cleanBlocksDeviceName (pair.inputName);
                for (int i = 0; i < midiOutputs.size(); ++i)
                {
                    if (cleanBlocksDeviceName (midiOutputs[i]) == cleanedInputName)
                    {
                        pair.outputName = midiOutputs[i];
                        pair.outputIndex = i;
                        break;
                    }
                }

                result.add (pair);
            }
        }

        return result;
    }

private:
    bool lockedFromOutside = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIDeviceDetector)
};

} // namespace juce
