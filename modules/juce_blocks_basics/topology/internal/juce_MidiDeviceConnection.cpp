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

struct MIDIDeviceConnection  : public PhysicalTopologySource::DeviceConnection,
                               public MidiInputCallback
{
    MIDIDeviceConnection() {}

    ~MIDIDeviceConnection() override
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        listeners.call ([this] (Listener& l) { l.connectionBeingDeleted (*this); });

        if (midiInput != nullptr)
            midiInput->stop();
    }

    void setLockAgainstOtherProcesses (std::shared_ptr<InterProcessLock> newLock)
    {
        midiPortLock = newLock;
    }

    struct Listener
    {
        virtual ~Listener() {}

        virtual void handleIncomingMidiMessage (const MidiMessage& message) = 0;
        virtual void connectionBeingDeleted (const MIDIDeviceConnection&) = 0;
    };

    void addListener (Listener* l)
    {
        ScopedLock scopedLock (criticalSecton);
        listeners.add (l);
    }

    void removeListener (Listener* l)
    {
        ScopedLock scopedLock (criticalSecton);
        listeners.remove (l);
    }

    bool sendMessageToDevice (const void* data, size_t dataSize) override
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED // This method must only be called from the message thread!

        jassert (dataSize > sizeof (BlocksProtocol::roliSysexHeader) + 1);
        jassert (memcmp (data, BlocksProtocol::roliSysexHeader, sizeof (BlocksProtocol::roliSysexHeader) - 1) == 0);
        jassert (static_cast<const uint8*> (data)[dataSize - 1] == 0xf7);

        if (midiOutput != nullptr)
        {
            midiOutput->sendMessageNow (MidiMessage (data, (int) dataSize));
            return true;
        }

        return false;
    }

    void handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override
    {
        ScopedTryLock lock (criticalSecton);

        if (lock.isLocked())
        {
            const auto data = message.getRawData();
            const int dataSize = message.getRawDataSize();
            const int bodySize = dataSize - (int) (sizeof (BlocksProtocol::roliSysexHeader) + 1);

            if (bodySize > 0 && memcmp (data, BlocksProtocol::roliSysexHeader, sizeof (BlocksProtocol::roliSysexHeader)) == 0)
                if (handleMessageFromDevice != nullptr)
                    handleMessageFromDevice (data + sizeof (BlocksProtocol::roliSysexHeader), (size_t) bodySize);

            listeners.call ([&] (Listener& l) { l.handleIncomingMidiMessage (message); });
        }
    }

    std::unique_ptr<MidiInput> midiInput;
    std::unique_ptr<MidiOutput> midiOutput;

    CriticalSection criticalSecton;

private:
    ListenerList<Listener> listeners;
    std::shared_ptr<InterProcessLock> midiPortLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIDeviceConnection)
};

} // namespace juce
