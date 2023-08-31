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

//TODO:wasm_support
namespace juce
{
    struct JuceRtMidiContext {
        MidiInput* midiIn;
        MidiInputCallback* callback{nullptr};
    };

    class MidiInput::Pimpl  {

    };

    class MidiOutput::Pimpl  {

    };

     Array<MidiDeviceInfo> MidiInput::getAvailableDevices() {
        return Array<MidiDeviceInfo>();
    }

    void MidiInput::start() {

     }

    void MidiInput::stop() {}

    MidiInput::~MidiInput() {}

    std::unique_ptr<MidiInput>
    MidiInput::openDevice(const juce::String &deviceIdentifier, juce::MidiInputCallback *callback) {
         return nullptr;
     }

    MidiDeviceInfo MidiInput::getDefaultDevice() {
        return MidiDeviceInfo{};
     }

     Array<MidiDeviceInfo> MidiOutput::getAvailableDevices() {
        return Array<MidiDeviceInfo>();
     }

    MidiDeviceInfo MidiOutput::getDefaultDevice() {
        return MidiDeviceInfo{};
     }

    std::unique_ptr<MidiOutput> MidiOutput::openDevice(const juce::String &deviceIdentifier) {
         return nullptr;
     }
    MidiDeviceListConnection MidiDeviceListConnection::make(std::function<void()> cb) {
        auto& broadcaster = MidiDeviceListConnectionBroadcaster::get();
        return { &broadcaster, broadcaster.add (std::move (cb)) };
     }
    bool MessageManager::postMessageToSystemQueue(juce::MessageManager::MessageBase *) { return true;}

    void MessageManager::doPlatformSpecificInitialisation() {}

    void Thread::killThread() {}

    MidiOutput::~MidiOutput() {}

} // namespace juce