/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2019 - ROLI Ltd.

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
    class BlockSerialReader
        : private MIDIDeviceConnection::Listener,
          private Timer
    {
    public:
        //==============================================================================
        BlockSerialReader (MIDIDeviceConnection& deviceConnectionToUse) : deviceConnection (deviceConnectionToUse)
        {
            deviceConnection.addListener (this);
            startTimer (10);
        }

        ~BlockSerialReader() override
        {
            deviceConnection.removeListener (this);
        }

        bool hasSerial() const { return serial.isNotEmpty(); }

        String getSerial() const { return serial; }

    private:
        MIDIDeviceConnection& deviceConnection;
        String serial;

        bool shouldStop() { return hasSerial(); }

        //==============================================================================
        void timerCallback() override
        {
            if (shouldStop())
            {
                stopTimer();
                return;
            }

            sendRequest();
            startTimer (300);
        }

        void sendRequest()
        {
            const uint8 dumpRequest[] = { 0xf0, 0x00, 0x21, 0x10, 0x78, 0x3f, 0xf7 };
            deviceConnection.sendMessageToDevice (dumpRequest, sizeof (dumpRequest));
        }

        void handleIncomingMidiMessage (const MidiMessage& message) override
        {
            if (hasSerial())
                return;

            if (isResponse (message))
                parseResponse (message);
        }

        void connectionBeingDeleted (const MIDIDeviceConnection&) override
        {
            stopTimer();
        }

        bool isResponse (const MidiMessage message)
        {
            const uint8 roliDumpHeader[] = { 0xf0, 0x00, 0x21, 0x10, 0x78};
            return memcmp (message.getRawData(), roliDumpHeader, sizeof (roliDumpHeader)) == 0;
        }

        void parseResponse (const MidiMessage& message)
        {
            int index = findMacAddressStart (message);

            if (index >= 0)
            {
                const int macSize = 17;
                const int offset = index + macSize;
                const int serialSize = 16;

                if (message.getRawDataSize() - offset < serialSize)
                {
                    jassertfalse;
                    return;
                }

                serial = String ((const char*)message.getRawData() + offset, serialSize);
            }
        }

        int findMacAddressStart (const MidiMessage& message)
        {
            const uint8 macStart[] = { '4', '8', ':', 'B', '6', ':', '2', '0', ':' };
            return findSequence (macStart, sizeof (macStart), message);
        }

        int findSequence (const uint8* sequence, int sequenceSize, const MidiMessage& message)
        {
            for (int i = 0; i < message.getRawDataSize() - sequenceSize; i++)
            {
                if (memcmp (message.getRawData() + i, sequence, size_t (sequenceSize)) == 0)
                    return i;
            }

            return -1;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlockSerialReader)
    };
}
