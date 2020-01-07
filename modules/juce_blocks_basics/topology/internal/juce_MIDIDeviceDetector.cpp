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

    StringArray scanForDevices() override
    {
        StringArray result;

        for (auto& pair : findDevices())
            result.add (pair.input.identifier + " & " + pair.output.identifier);

        return result;
    }

    PhysicalTopologySource::DeviceConnection* openDevice (int index) override
    {
        const auto allDevices = findDevices();

        if (allDevices.size() > index)
        {
            const auto pair = allDevices[index];
            auto dev = std::make_unique<MIDIDeviceConnection>();

            if (auto lock = createMidiPortLock (pair.input.name, pair.output.name))
            {
                lockedFromOutside = false;

                dev->setLockAgainstOtherProcesses (lock);
                dev->midiInput  = MidiInput::openDevice  (pair.input.identifier, dev.get());
                dev->midiOutput = MidiOutput::openDevice (pair.output.identifier);

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

    static bool isBlocksMidiDeviceName (const String& name)
    {
        return name.indexOf (" BLOCK") > 0 || name.indexOf (" Block") > 0;
    }

    static String cleanBlocksDeviceName (String name)
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
        MidiDeviceInfo input, output;
    };

    static Array<MidiInputOutputPair> findDevices()
    {
        Array<MidiInputOutputPair> result;

        auto midiInputs  = MidiInput::getAvailableDevices();
        auto midiOutputs = MidiOutput::getAvailableDevices();

        for (const auto& input : midiInputs)
        {
            if (isBlocksMidiDeviceName (input.name))
            {
                MidiInputOutputPair pair;
                pair.input = input;

                String cleanedInputName = cleanBlocksDeviceName (input.name);

                int inputOccurences = 0;
                int outputOccurences = 0;

                for (const auto& p : result)
                    if (cleanBlocksDeviceName (p.input.name) == cleanedInputName)
                        ++inputOccurences;

                for (const auto& output : midiOutputs)
                {
                    if (cleanBlocksDeviceName (output.name) == cleanedInputName)
                    {
                        if (outputOccurences == inputOccurences)
                        {
                            pair.output = output;
                            break;
                        }

                        ++outputOccurences;
                    }
                }

                result.add (pair);
            }
        }

        return result;
    }

private:
    bool lockedFromOutside = true;

    /** For backwards compatibility, the block interprocess lock has to use the midi input name.
        The below is necessary because blocks of the same type might duplicate a port name, so
        must share an interprocess lock.
     */
    std::shared_ptr<InterProcessLock> createMidiPortLock (const String& midiInName, const String& midiOutName)
    {
        const juce::String lockIdentifier = "blocks_sdk_"
                                            + File::createLegalFileName (midiInName)
                                            + "_" + File::createLegalFileName (midiOutName);

        const auto existingLock = midiPortLocks.find (lockIdentifier);

        if (existingLock != midiPortLocks.end())
            if (existingLock->second.use_count() > 0)
                return existingLock->second.lock();

        auto interprocessLock = std::make_shared<InterProcessLock> (lockIdentifier);

        if (interprocessLock->enter (500))
        {
            midiPortLocks[lockIdentifier] = interprocessLock;
            return interprocessLock;
        }

        return nullptr;
    }

    std::map<juce::String, std::weak_ptr<InterProcessLock>> midiPortLocks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIDeviceDetector)
};

} // namespace juce
